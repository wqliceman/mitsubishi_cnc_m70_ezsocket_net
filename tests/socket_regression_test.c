/*
 * Minimal regression tests for transport-layer framing fixes.
 *
 * Covers:
 * 1. socket_recv_data_one_loop reads fragmented payloads to completion.
 * 2. mel_receive_response disconnects and reports an error on incomplete headers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "m70_error.h"
#include "m70_giop.h"
#include "socket.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#define TEST_INVALID_SOCKET (-1)

typedef struct delayed_send_ctx
{
	int fd;
	const char* data;
	size_t data_len;
	unsigned int delay_ms;
	int send_result;
} delayed_send_ctx_t;

#ifdef _WIN32
typedef HANDLE test_thread_t;

static void sleep_ms(unsigned int delay_ms)
{
	Sleep(delay_ms);
}

static DWORD WINAPI delayed_send_thread(LPVOID param)
#else
typedef pthread_t test_thread_t;

static void sleep_ms(unsigned int delay_ms)
{
	struct timespec req;
	req.tv_sec = (time_t)(delay_ms / 1000);
	req.tv_nsec = (long)((delay_ms % 1000) * 1000000L);
	nanosleep(&req, NULL);
}

static void* delayed_send_thread(void* param)
#endif
{
	delayed_send_ctx_t* ctx = (delayed_send_ctx_t*)param;
	size_t sent_total = 0;

	sleep_ms(ctx->delay_ms);
	while (sent_total < ctx->data_len)
	{
		int sent_now = (int)send(ctx->fd, ctx->data + sent_total, (int)(ctx->data_len - sent_total), 0);
		if (sent_now <= 0)
		{
			ctx->send_result = -1;
			goto done;
		}

		sent_total += (size_t)sent_now;
	}

	ctx->send_result = 0;

done:
#ifdef _WIN32
	return 0;
#else
	return NULL;
#endif
}

static int start_test_thread(test_thread_t* thread, delayed_send_ctx_t* ctx)
{
#ifdef _WIN32
	*thread = CreateThread(NULL, 0, delayed_send_thread, ctx, 0, NULL);
	return *thread == NULL ? -1 : 0;
#else
	return pthread_create(thread, NULL, delayed_send_thread, ctx);
#endif
}

static int join_test_thread(test_thread_t thread)
{
#ifdef _WIN32
	DWORD wait_result = WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);
	return wait_result == WAIT_OBJECT_0 ? 0 : -1;
#else
	return pthread_join(thread, NULL);
#endif
}

static int close_test_socket(int fd)
{
	if (fd < 0)
		return 0;

#ifdef _WIN32
	return closesocket((SOCKET)fd);
#else
	return close(fd);
#endif
}

static int create_connected_pair(int fds[2])
{
	int listen_fd = TEST_INVALID_SOCKET;
	int client_fd = TEST_INVALID_SOCKET;
	int server_fd = TEST_INVALID_SOCKET;
	struct sockaddr_in addr;

#ifdef _WIN32
	int addr_len = (int)sizeof(addr);
#else
	socklen_t addr_len = (socklen_t)sizeof(addr);
#endif

	memset(&addr, 0, sizeof(addr));

	listen_fd = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_fd < 0)
		goto fail;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = 0;

	if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0)
		goto fail;

	if (getsockname(listen_fd, (struct sockaddr*)&addr, &addr_len) != 0)
		goto fail;

	if (listen(listen_fd, 1) != 0)
		goto fail;

	client_fd = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client_fd < 0)
		goto fail;

	if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0)
		goto fail;

	server_fd = (int)accept(listen_fd, NULL, NULL);
	if (server_fd < 0)
		goto fail;

	close_test_socket(listen_fd);
	fds[0] = server_fd;
	fds[1] = client_fd;
	return 0;

fail:
	close_test_socket(server_fd);
	close_test_socket(client_fd);
	close_test_socket(listen_fd);
	return -1;
}

static int send_all(int fd, const void* data, size_t data_len)
{
	const char* buffer = (const char*)data;
	size_t sent_total = 0;

	while (sent_total < data_len)
	{
		int sent_now = (int)send(fd, buffer + sent_total, (int)(data_len - sent_total), 0);
		if (sent_now <= 0)
			return -1;

		sent_total += (size_t)sent_now;
	}

	return 0;
}

static int test_socket_recv_reads_fragmented_payload(void)
{
	const char expected[] = "GIOP-fragmented-frame";
	char actual[sizeof(expected)] = {0};
	const size_t split_at = 5;
	int sockets[2] = { TEST_INVALID_SOCKET, TEST_INVALID_SOCKET };
	delayed_send_ctx_t ctx;
	test_thread_t thread;
	int read_count = 0;

	if (create_connected_pair(sockets) != 0)
	{
		fprintf(stderr, "Failed to create connected socket pair\n");
		return 1;
	}

	if (send_all(sockets[1], expected, split_at) != 0)
	{
		fprintf(stderr, "Failed to send first payload fragment\n");
		close_test_socket(sockets[0]);
		close_test_socket(sockets[1]);
		return 1;
	}

	memset(&ctx, 0, sizeof(ctx));
	ctx.fd = sockets[1];
	ctx.data = expected + split_at;
	ctx.data_len = sizeof(expected) - split_at;
	ctx.delay_ms = 50;
	ctx.send_result = -1;

	if (start_test_thread(&thread, &ctx) != 0)
	{
		fprintf(stderr, "Failed to start delayed send thread\n");
		close_test_socket(sockets[0]);
		close_test_socket(sockets[1]);
		return 1;
	}

	read_count = socket_recv_data_one_loop(sockets[0], actual, (int)sizeof(expected));
	if (join_test_thread(thread) != 0 || ctx.send_result != 0)
	{
		fprintf(stderr, "Delayed send thread failed\n");
		close_test_socket(sockets[0]);
		close_test_socket(sockets[1]);
		return 1;
	}

	close_test_socket(sockets[0]);
	close_test_socket(sockets[1]);

	if (read_count != (int)sizeof(expected))
	{
		fprintf(stderr, "Expected %zu bytes, got %d\n", sizeof(expected), read_count);
		return 1;
	}

	if (memcmp(actual, expected, sizeof(expected)) != 0)
	{
		fprintf(stderr, "Fragmented payload read mismatch\n");
		return 1;
	}

	return 0;
}

static int test_mel_receive_response_closes_incomplete_header(void)
{
	int sockets[2] = { TEST_INVALID_SOCKET, TEST_INVALID_SOCKET };
	giop_header header;
	giop_header received;
	m70_conn_t conn;
	int remain_length = 0;
	int ret = 0;
	const m70_error_info_t* last_error = NULL;

	if (create_connected_pair(sockets) != 0)
	{
		fprintf(stderr, "Failed to create connected socket pair\n");
		return 1;
	}

	memset(&header, 0, sizeof(header));
	memcpy(header.magic_number, "GIOP", 4);
	header.version = 1;
	header.byte_order = 1;
	header.msg_type = (byte)MSG_TYPES_Reply;
	header.data_length = (uint32)sizeof(response_pack_header);

	if (send_all(sockets[1], &header, sizeof(header) - 2) != 0)
	{
		fprintf(stderr, "Failed to send incomplete header\n");
		close_test_socket(sockets[0]);
		close_test_socket(sockets[1]);
		return 1;
	}

	close_test_socket(sockets[1]);
	sockets[1] = TEST_INVALID_SOCKET;

	memset(&received, 0, sizeof(received));
	memset(&conn, 0, sizeof(conn));
	conn.socket = sockets[0];
	conn.connected = true;
	conn.little_endian = true;

	ret = mel_receive_response(&conn, &received, &remain_length);
	last_error = m70_error_get_last();

	if (ret != -1)
	{
		fprintf(stderr, "Expected mel_receive_response to fail on incomplete header, got %d\n", ret);
		return 1;
	}

	if (conn.connected || conn.socket >= 0)
	{
		fprintf(stderr, "Expected connection cleanup after incomplete header\n");
		return 1;
	}

	if (last_error == NULL || last_error->error_code != M70_ERROR_CODE_EX_TRANS_INCOMPLETE)
	{
		fprintf(stderr, "Expected TRANS_INCOMPLETE error code after incomplete header\n");
		return 1;
	}

	return 0;
}

int main(void)
{
	int failed = 0;

#ifdef _WIN32
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
	{
		fprintf(stderr, "WSAStartup failed\n");
		return 1;
	}
#endif

	printf("[TEST] fragmented read drains full payload\n");
	failed += test_socket_recv_reads_fragmented_payload();
	printf("[TEST] incomplete header disconnects connection\n");
	failed += test_mel_receive_response_closes_incomplete_header();

#ifdef _WIN32
	WSACleanup();
#endif

	if (failed != 0)
	{
		fprintf(stderr, "Socket regression tests failed: %d\n", failed);
		return 1;
	}

	printf("Socket regression tests passed\n");
	return 0;
}