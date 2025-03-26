#include "socket.h"
#include <stdio.h>
#include <string.h>
#include "m70_log.h"
#include "m70_error.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib") /* Linking with winsock library */
#else
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

int socket_send_data(int fd, void* buf, int nbytes)
{
	int nleft, nwritten;
	char* ptr = (char*)buf;

	if (fd < 0) {
		M70_LOG_ERROR("Send data failed: Invalid socket descriptor %d", fd);
		return -1;
	}

	nleft = nbytes;
	while (nleft > 0)
	{
		nwritten = send(fd, ptr, nleft, 0);
		if (nwritten <= 0)
		{
			if (errno == EINTR) {
				M70_LOG_DEBUG("Send data interrupted, continuing to try");
				continue;
			} else {
				M70_LOG_ERROR("Send data failed: %s (errno: %d)", strerror(errno), errno);
				return -1;
			}
		}
		else
		{
			nleft -= nwritten;
			ptr += nwritten;
			M70_LOG_DEBUG("Sent %d bytes of data, %d bytes remaining", nwritten, nleft);
		}
	}

	M70_LOG_DEBUG("Successfully sent all %d bytes of data", nbytes);
	return (nbytes - nleft);
}

int socket_recv_data(int fd, void* buf, int nbytes)
{
	int nleft, nread;
	char* ptr = (char*)buf;

	if (fd < 0) {
		M70_LOG_ERROR("Receive data failed: Invalid socket descriptor %d", fd);
		return -1;
	}

	nleft = nbytes;
	while (nleft > 0)
	{
		nread = recv(fd, ptr, nleft, 0);
		if (nread == 0)
		{
			M70_LOG_WARNING("Connection closed, received EOF");
			break;
		}
		else if (nread < 0)
		{
			if (errno == EINTR) {
				M70_LOG_DEBUG("Receive data interrupted, continuing to try");
				continue;
			} else {
				M70_LOG_ERROR("Receive data failed: %s (errno: %d)", strerror(errno), errno);
				return -1;
			}
		}
		else
		{
			nleft -= nread;
			ptr += nread;
			M70_LOG_DEBUG("Received %d bytes of data, %d bytes remaining", nread, nleft);
		}
	}

	M70_LOG_DEBUG("Successfully received %d bytes of data", nbytes - nleft);
	return (nbytes - nleft);
}

int socket_recv_data_one_loop(int fd, void* buf, int nbytes)
{
	int nleft, nread;
	char* ptr = (char*)buf;

	if (fd < 0) {
		M70_LOG_ERROR("Single receive data failed: Invalid socket descriptor %d", fd);
		return -1;
	}

	nleft = nbytes;
	while (nleft > 0)
	{
		nread = recv(fd, ptr, nleft, 0);
		if (nread == 0)
		{
			M70_LOG_WARNING("Connection closed, received EOF");
			break;
		}
		else if (nread < 0)
		{
			if (errno == EINTR) {
				M70_LOG_DEBUG("Single receive data interrupted, continuing to try");
				continue;
			} else {
				M70_LOG_ERROR("Single receive data failed: %s (errno: %d)", strerror(errno), errno);
				return -1;
			}
		}
		else
		{
			nleft -= nread;
			ptr += nread;
			M70_LOG_DEBUG("Single receive received %d bytes of data", nread);

			// Currently only receive once
			break;
		}
	}

	M70_LOG_DEBUG("Single receive completed, received a total of %d bytes of data", nbytes - nleft);
	return (nbytes - nleft);
}

int socket_open_tcp_client_socket(char* dest_ip, short dest_port)
{
	int sockFd = 0;
	struct sockaddr_in server_addr;
	int ret;

	M70_LOG_INFO("Attempting to create TCP client socket connection to %s:%d", dest_ip, dest_port);
	sockFd = (int)socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sockFd < 0)
	{
		M70_LOG_ERROR("Failed to create socket: %s (errno: %d)", strerror(errno), errno);
		return -1;
#pragma warning(disable : 4996)
	}

	memset((char*)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(dest_ip);
	server_addr.sin_port = (uint16_t)htons((uint16_t)dest_port);

	M70_LOG_DEBUG("Attempting to connect to server %s:%d", dest_ip, dest_port);
	ret = connect(sockFd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (ret != 0)
	{ 
		M70_LOG_ERROR("Failed to connect to %s:%d: %s (errno: %d)", dest_ip, dest_port, strerror(errno), errno);
		M70_ERROR_SET(M70_ERROR_CODE_EX_CONN_REFUSED, "Failed to connect to %s:%d: %s", dest_ip, dest_port, strerror(errno));
		socket_close_tcp_socket(sockFd);
		sockFd = -1;
	}

#ifdef _WIN32
	int timeout = 5000; // 5s
	ret = setsockopt(sockFd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
	ret = setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
#else
	struct timeval timeout = { 5, 0 }; // 3s
	ret = setsockopt(sockFd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
	ret = setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
#endif

	return sockFd;
}

void socket_close_tcp_socket(int sockFd)
{
	if (sockFd > 0)
	{
		M70_LOG_DEBUG("Closing socket connection: %d", sockFd);
#ifdef _WIN32
		closesocket(sockFd);
#else
		close(sockFd);
#endif
	}
	else
	{
		M70_LOG_WARNING("Attempting to close invalid socket: %d", sockFd);
	}
}

void tinet_ntoa(char* ipstr, unsigned int ip)
{
	sprintf(ipstr, "%d.%d.%d.%d", ip & 0xFF, (ip >> 8) & 0xFF, (ip >> 16) & 0xFF, ip >> 24);
}