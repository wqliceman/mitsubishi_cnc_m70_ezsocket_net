#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "m70_giop.h"
#include "socket.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib") /* Linking with winsock library */
#pragma warning(disable : 4996)
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#define HtoNs(isLittleEndian, A) ((isLittleEndian != is_little_endian()) ? htons(A) : A)
#define HtoNl(isLittleEndian, A) ((isLittleEndian != is_little_endian()) ? htonl(A) : A)
#define SET_AXIS_NO(axis_no) (axis_no >= 1 ? (1 << (axis_no - 1)) : 0)
#define SET_DWORD_TO_CHARS(A, chrs)           \
	{                                         \
		chrs[0] = (((uint32)A) >> 24) & 0xff; \
		chrs[1] = (((uint32)A) >> 16) & 0xff; \
		chrs[2] = (((uint32)A) >> 8) & 0xff;  \
		chrs[3] = ((uint32)A) & 0xff;         \
	}

#define T_CHAR_SIZE (1)
#define T_SHORT_SIZE (2)
#define T_LONG_SIZE (4)
#define T_DLONG_SIZE (8)
#define T_DOUBLE_SIZE (8)
#define T_FLOATBIN_SIZE (16)
#define T_CLCTDATA_SIZE (36)

#define T_CHAR 0x1
#define T_SHORT 0x2
#define T_LONG 0x3
#define T_DLONG 0x4
#define T_DOUBLE 0x5
#define T_FLOATBIN 0x6
#define T_CLCTDATA 0x100

// open mode
#define M_FSOPEN_RDONLY 0x0000
#define M_FSOPEN_WRONLY 0x0001
#define M_FSOPEN_RDWR 0x0002

const char op_command_get_data[] = "mochaGetData";
const char op_command_set_data[] = "mochaSetData";
const char op_command_get_alarm_msg[] = "mochaGetCurrentAlarmMsgFirst";
const char op_command_get_prog_block[] = "mochaGetCurrentPrgBlockFirst";
const char op_command_fs_open_file[] = "mochaFSOpenFile";
const char op_command_fs_read_file[] = "mochaFSReadFile";
const char op_command_fs_close_file[] = "mochaFSCloseFile";
const char op_command_fs_create_file[] = "mochaFSCreateFile";
const char op_command_fs_remove_file[] = "mochaFSRemoveFile";
const char op_command_fs_write_file[] = "mochaFSWriteFile";
const char op_command_fs_stat_file[] = "mochaFSStatFile";
const char op_command_fs_open_dir[] = "mochaFSOpenDirectory";
const char op_command_fs_close_dir[] = "mochaFSCloseDirectory";
const char op_command_fs_read_dir[] = "mochaFSReadDirectory";
const char op_command_cancel_modal2[] = "mochaCancelModal2";

int get_data_type_length(int datatype)
{
	int len = T_CHAR_SIZE;
	if (T_SHORT == datatype)
		len = T_SHORT_SIZE;
	else if (T_LONG == datatype)
		len = T_LONG_SIZE;
	else if (T_DLONG == datatype)
		len = T_DLONG_SIZE;
	else if (T_DOUBLE == datatype)
		len = T_DOUBLE_SIZE;
	else if (T_FLOATBIN == datatype)
		len = T_FLOATBIN_SIZE;
	else if (T_CLCTDATA == datatype)
		len = T_CLCTDATA_SIZE;
	return len;
}

long receive_error_data_response(m70_conn_t* conn, int* remain_length)
{
	if (!check_conn_is_valid(conn))
		return -1;

	int exceptionLen = 0;
	int code = 0;
	*remain_length -= socket_recv_data_one_loop(conn->socket, &exceptionLen, 4);
	*remain_length -= receive_remain_info_response(conn, &exceptionLen);
	mel_error_code errorPack;
	*remain_length -= socket_recv_data_one_loop(conn->socket, &errorPack, sizeof(errorPack));
	code = errorPack.error_code;
	return code;
}

long receive_remain_info_response(m70_conn_t* conn, int* remain_length)
{
	if (!check_conn_is_valid(conn))
		return -1;

	long sum = 0;
	long rl = 0;
	while (*remain_length > 0)
	{
		char ch = 0;
		rl = socket_recv_data_one_loop(conn->socket, &ch, 1);
		sum += rl;
		*remain_length -= rl;
	}
	return sum;
}
bool is_mutiple_axis(int axis_flag)
{
	bool isMutiple = false;
	bool bitOn = false;
	int i = 0;
	for (i = 0; i < 32; i++)
	{
		if (((axis_flag >> i) & 0x01) != 0)
		{
			if (bitOn)
			{
				isMutiple = true;
				break;
			}
			else
				bitOn = true;
		}
	}
	return isMutiple;
}

int receive_get_data_response(m70_conn_t* conn, int len, m70_data_type_e* data_type, int axis_flag, void* data)
{
	if (!check_conn_is_valid(conn))
		return 0;

	int readLen = 0;
	int headLen = 0;
	int dataLen = 0;

	if (*data_type == T_FLOATBIN && is_mutiple_axis(axis_flag))
	{
		get_data_float_bin_response_header rsp;
		headLen = sizeof(rsp);
		readLen = socket_recv_data_one_loop(conn->socket, &rsp, headLen);
		dataLen = rsp.data_length - 8;
		*data_type = rsp.data_type;
	}
	else
	{
		get_data_response_header rsp;
		headLen = sizeof(rsp);
		readLen = socket_recv_data_one_loop(conn->socket, &rsp, headLen);
		dataLen = rsp.data_length;
		*data_type = rsp.data_type;
	}

	if (len > headLen)
	{
		readLen += socket_recv_data_one_loop(conn->socket, data, dataLen);
	}
	return readLen;
}

int receive_data_response(m70_conn_t* conn, int len, int data_type, void* data)
{
	if (!check_conn_is_valid(conn))
		return 0;

	// Bit,Word,DWord,String
	return socket_recv_data_one_loop(conn->socket, data, len);
}

bool giop_connect(const char* ip, int type, int port, m70_conn_t* conn)
{
	if (ip == NULL || strlen(ip) <= 0 || port <= 0 || conn == NULL)
		return false;

	conn->nc_type = (m70_nc_type_e)type;
	conn->little_endian = true;
	srand((uint32)time(NULL));
	conn->request_id = rand() % 0xFFFF;

	if (conn->socket > 0)
		giop_disconnect(conn);

	conn->socket = socket_open_tcp_client_socket((char*)ip, port);
	if (conn->socket > 0)
	{
		conn->connected = true;
		return true;
	}

	return false;
}
void giop_disconnect(m70_conn_t* conn)
{
	if (conn == NULL)
		return;

	socket_close_tcp_socket(conn->socket);
	conn->socket = -1;
	conn->connected = false;
}

long melGetData(m70_conn_t* conn, int section, int sub_section, int system_no, int axis_flag, m70_data_type_e* int_out_data_type, void* out_data_value)
{
	long code = -1;
	if (!check_conn_is_valid(conn))
		return code;

	if (conn->connected)
	{
		giop_header giop;
		build_giop_header(conn, &giop);

		request_pack_header request;
		build_request_pack_header(conn, &request, 0x0D);

		get_data_pack pack;
		memset((void*)&pack, 0, sizeof(pack));
		giop.data_length = sizeof(pack) - sizeof(giop);

		pack.giop = giop;
		pack.request = request;
		strncpy(pack.op, op_command_get_data, sizeof(pack.op));
		pack.op[sizeof(pack.op) - 1] = '\0';

		pack.principal = HtoNl(conn->little_endian, 0x00);

		pack.section = section;
		pack.sub_section = sub_section;
		pack.system_no = system_no;
		pack.axis_no = axis_flag; // SET_AXIS_NO(axis_no));
		pack.u2 = 0x00;
		pack.data_type = *int_out_data_type;

		socket_send_data(conn->socket, &pack, sizeof(pack));
		memset((void*)&giop, 0, sizeof(giop));
		int msg_length = 0;
		code = mel_receive_response(conn, &giop, &msg_length);
		if (code == 0)
		{
			msg_length -= receive_get_data_response(conn, msg_length, int_out_data_type, axis_flag, out_data_value);
		}
		receive_remain_info_response(conn, &msg_length);
	}
	return code;
}
long melSetData(m70_conn_t* conn, int section, int sub_section, int system_no, int axis_flag, m70_data_type_e data_type, void* data)
{
	long code = 1;
	if (!check_conn_is_valid(conn))
		return code;

	if (conn->connected)
	{
		giop_header giop;
		build_giop_header(conn, &giop);

		request_pack_header request;
		build_request_pack_header(conn, &request, 0x0D);

		set_data_pack pack;
		memset((void*)&pack, 0, sizeof(pack));

		pack.giop = giop;
		pack.request = request;
		strncpy(pack.op, op_command_set_data, sizeof(pack.op));
		pack.op[sizeof(pack.op) - 1] = '\0';
		pack.principal = HtoNl(conn->little_endian, 0x00000000);

		pack.section = section;
		pack.sub_section = sub_section;
		pack.system_no = system_no;
		pack.axis_no = axis_flag; // SET_AXIS_NO(axis_no));
		pack.u2 = 0x00000000;
		pack.data_type = data_type;
		pack.byte_numbers = get_data_type_length(data_type);

		if (T_CHAR == data_type)
		{
			pack.data.u_byte = *(byte*)data;
		}
		if (T_SHORT == data_type)
		{
			pack.data.u_word = *(ushort*)data;
		}
		else if ((T_LONG == data_type) || (T_DLONG == data_type))
		{
			pack.data.u_dword = *(uint32*)data;
		}
		else if (T_DOUBLE == data_type)
		{
			pack.data.u.f = *(uint32*)data;
			pack.data.u.d = *((uint32*)data + 1);
		}
		else if (T_FLOATBIN == data_type)
		{
			pack.data.u3.int_data_nos = *(ushort*)data;
			pack.data.u3.dec_data_nos = *((ushort*)data + 1);
			pack.data.u3.option = *((uint32*)data + 1);
			pack.data.u3.data = *((double*)data + 1);
		}
		else if (T_STR == data_type)
		{
			T_string* temp = (T_string*)data;
			if (temp->msg_length >= 512)
				temp->msg_length = 512;

			pack.data.u_str.msg_size = temp->msg_length;
			strncpy(pack.data.u_str.text, temp->text, temp->msg_length);
			pack.byte_numbers = 4 + temp->msg_length; // Length + content
		}
		pack.giop.data_length = 48 + sizeof(request) + pack.byte_numbers;

		int send_length = pack.giop.data_length + sizeof(giop);
		socket_send_data(conn->socket, &pack, send_length);
		memset((void*)&giop, 0, sizeof(giop));
		int msg_length = 0;
		code = mel_receive_response(conn, &giop, &msg_length);
		receive_remain_info_response(conn, &msg_length);
	}
	return code;
}
long melGetCurrentAlarmMsg(m70_conn_t* conn, int system_no, int msg_count, int msg_type, void* msg)
{
	long code = 1;
	if (!check_conn_is_valid(conn))
		return code;

	if (conn->connected)
	{
		giop_header giop;
		build_giop_header(conn, &giop);

		request_pack_header request;
		build_request_pack_header(conn, &request, 0x1D);

		alarm_info_pack pack;
		memset((void*)&pack, 0, sizeof(pack));
		giop.data_length = sizeof(pack) - sizeof(giop);

		pack.giop = giop;
		pack.request = request;
		strncpy(pack.op, op_command_get_alarm_msg, sizeof(pack.op));
		pack.op[sizeof(pack.op) - 1] = '\0';
		pack.principal = HtoNl(conn->little_endian, 0x00);
		pack.system_no = system_no;
		pack.msg_count = msg_count;
		pack.msg_type = msg_type;

		socket_send_data(conn->socket, &pack, sizeof(pack));
		memset((void*)&giop, 0, sizeof(giop));
		int msg_length = 0;
		code = mel_receive_response(conn, &giop, &msg_length);
		if (code == 0)
		{
			msg_length -= receive_data_response(conn, msg_length, (int)T_STR, msg);
		}
		receive_remain_info_response(conn, &msg_length);
	}
	return code;
}
long melGetCurrentPrgBlock(m70_conn_t* conn, int system_no, int row_count, void* msg)
{
	long code = 1;
	if (!check_conn_is_valid(conn))
		return code;

	if (conn->connected)
	{
		giop_header giop;
		build_giop_header(conn, &giop);

		request_pack_header request;
		build_request_pack_header(conn, &request, 0x1D);

		prog_block_pack pack;
		memset((void*)&pack, 0, sizeof(pack));
		giop.data_length = sizeof(pack) - sizeof(giop);

		pack.giop = giop;
		pack.request = request;
		strncpy(pack.op, op_command_get_prog_block, sizeof(pack.op));
		pack.op[sizeof(pack.op) - 1] = '\0';
		pack.principal = HtoNl(conn->little_endian, 0x00000000);
		pack.system_no = system_no;
		pack.row_count = row_count;

		socket_send_data(conn->socket, &pack, sizeof(pack));
		memset((void*)&giop, 0, sizeof(giop));
		int msg_length = 0;
		code = mel_receive_response(conn, &giop, &msg_length);
		if (code == 0)
		{
			msg_length -= receive_data_response(conn, msg_length, (int)T_STR, msg);
		}
		receive_remain_info_response(conn, &msg_length);
	}
	return code;
}

long melFsOpenFile(m70_conn_t* conn, const char* filename, long mode, long* fd)
{
	long code = 1;
	if (!check_conn_is_valid(conn))
		return code;

	if (!filename || strlen(filename) <= 0)
		return code;

	if (conn->connected)
	{
		giop_header giop;
		build_giop_header(conn, &giop);

		request_pack_header request;
		build_request_pack_header(conn, &request, 0x10);

		FS_open_file_pack pack;
		memset((void*)&pack, 0, sizeof(pack));

		strncpy(pack.op, op_command_fs_open_file, sizeof(pack.op));
		pack.op[sizeof(pack.op) - 1] = '\0';
		pack.principal = HtoNl(conn->little_endian, 0x00000000);
		pack.mode = HtoNl(conn->little_endian, 0x00000000);
		pack.flag = mode;
		size_t fsize = strlen(filename);
		pack.file_name_size = fsize;
		memset(pack.file_name, 0, 256);
		memcpy((void*)pack.file_name, filename, fsize);

		giop.data_length = sizeof(pack) - sizeof(giop) - (sizeof(pack.file_name) - fsize);
		pack.giop = giop;
		pack.request = request;

		socket_send_data(conn->socket, &pack, sizeof(pack));
		memset((void*)&giop, 0, sizeof(giop));
		int msg_length = 0;
		code = mel_receive_response(conn, &giop, &msg_length);
		if (code == 0)
		{
			long ret;
			msg_length -= socket_recv_data_one_loop(conn->socket, &ret, sizeof(ret));
			msg_length -= socket_recv_data_one_loop(conn->socket, fd, sizeof(uint32));
		}
		receive_remain_info_response(conn, &msg_length);
	}
	return code;
}

long melFsReadFile(m70_conn_t* conn, long fd, void* file_data, long* read_size, long need_read_size)
{
	long code = 1;
	if (!check_conn_is_valid(conn))
		return code;

	if (file_data == NULL || read_size == NULL || need_read_size == 0)
		return code;

	if (conn->connected)
	{
		giop_header giop;
		build_giop_header(conn, &giop);

		request_pack_header request;
		build_request_pack_header(conn, &request, 0x10);

		FS_read_file_pack pack;
		memset((void*)&pack, 0, sizeof(pack));
		strncpy(pack.op, op_command_fs_read_file, sizeof(pack.op));
		pack.op[sizeof(pack.op) - 1] = '\0';
		pack.principal = HtoNl(conn->little_endian, 0x00000000);
		pack.file_handle = fd;
		pack.file_size = need_read_size;
		giop.data_length = sizeof(pack) - sizeof(giop);
		pack.giop = giop;
		pack.request = request;

		socket_send_data(conn->socket, &pack, sizeof(pack));
		memset((void*)&giop, 0, sizeof(giop));
		int msg_length = 0;
		code = mel_receive_response(conn, &giop, &msg_length);
		if (code == 0)
		{
			long ret = 0;
			msg_length -= socket_recv_data_one_loop(conn->socket, &ret, sizeof(ret));
			msg_length -= socket_recv_data_one_loop(conn->socket, read_size, sizeof(long));
			if (*read_size)
			{
				msg_length -= receive_data_response(conn, *read_size, 0, file_data);
			}
		}
		receive_remain_info_response(conn, &msg_length);
	}
	return code;
}

long melFsCloseFile(m70_conn_t* conn, long fd)
{
	long code = 1;
	if (!check_conn_is_valid(conn))
		return code;

	if (conn->connected)
	{
		giop_header giop;
		build_giop_header(conn, &giop);

		request_pack_header request;
		build_request_pack_header(conn, &request, 0x11);

		FS_close_file_pack pack;
		memset((void*)&pack, 0, sizeof(pack));
		strncpy(pack.op, op_command_fs_close_file, sizeof(pack.op));
		pack.op[sizeof(pack.op) - 1] = '\0';

		pack.giop = giop;
		pack.request = request;
		memcpy(pack.reserved, "\x0\x0\x0", sizeof(pack.reserved));
		pack.principal = 0x00000000;
		pack.file_handle = fd;

		giop.data_length = sizeof(pack) - sizeof(giop);
		pack.giop = giop;
		pack.request = request;
		giop.data_length = sizeof(pack) - sizeof(giop);

		pack.giop = giop;
		pack.request = request;

		socket_send_data(conn->socket, &pack, sizeof(pack));
		memset((void*)&giop, 0, sizeof(giop));
		int msg_length = 0;
		code = mel_receive_response(conn, &giop, &msg_length);
		receive_remain_info_response(conn, &msg_length);
	}
	return code;
}

long melFsCreateFile(m70_conn_t* conn, const char* filename, long mode, long* fd)
{
	long code = 1;
	if (!check_conn_is_valid(conn))
		return code;

	if (!filename || strlen(filename) <= 0)
		return code;

	if (conn->connected)
	{
		giop_header giop;
		build_giop_header(conn, &giop);

		request_pack_header request;
		build_request_pack_header(conn, &request, 0x12);

		FS_create_file_pack pack;
		memset((void*)&pack, 0, sizeof(pack));
		strncpy(pack.op, op_command_fs_create_file, sizeof(pack.op));
		pack.op[sizeof(pack.op) - 1] = '\0';

		pack.giop = giop;
		pack.request = request;
		memcpy(pack.reserved, "\x00\x00", sizeof(pack.reserved));
		pack.principal = 0x00000000;
		size_t fsize = strlen(filename);

		pack.mode = mode;
		pack.file_name_size = fsize;
		memcpy(pack.file_name, filename, fsize);

		giop.data_length = sizeof(pack) - sizeof(giop) - (sizeof(pack.file_name) - fsize);

		pack.giop = giop;
		pack.request = request;

		socket_send_data(conn->socket, &pack, sizeof(pack));
		memset((void*)&giop, 0, sizeof(giop));
		int msg_length = 0;
		code = mel_receive_response(conn, &giop, &msg_length);
		if (code == 0)
		{
			long ret = 0;
			msg_length -= socket_recv_data_one_loop(conn->socket, &ret, sizeof(uint32));
			msg_length -= socket_recv_data_one_loop(conn->socket, fd, sizeof(uint32));
		}
		receive_remain_info_response(conn, &msg_length);
	}
	return code;
}

long melRemoveFile(m70_conn_t* conn, const char* file_name)
{
	long code = 1;
	if (!check_conn_is_valid(conn))
		return code;

	if (conn->connected)
	{
		giop_header giop;
		build_giop_header(conn, &giop);

		request_pack_header request;
		build_request_pack_header(conn, &request, 0x12);

		FS_remove_pack pack;
		memset((void*)&pack, 0, sizeof(pack));
		strncpy(pack.op, op_command_fs_remove_file, sizeof(pack.op));
		pack.op[sizeof(pack.op) - 1] = '\0';

		size_t fileLen = strlen(file_name);
		memcpy(pack.reserved, "\x0\x0", 2);
		pack.principal = 0x00000000;
		pack.fileLen = fileLen;

		memcpy(pack.file_name, file_name, fileLen);
		giop.data_length = sizeof(pack) - sizeof(giop) - (sizeof(pack.file_name) - fileLen);
		pack.giop = giop;
		pack.request = request;

		socket_send_data(conn->socket, &pack, sizeof(pack));
		memset((void*)&giop, 0, sizeof(giop));
		int msg_length = 0;
		code = mel_receive_response(conn, &giop, &msg_length);
		receive_remain_info_response(conn, &msg_length);
	}
	return code;
}

long melFsWriteFile(m70_conn_t* conn, long fd, void* file_data, long write_size, long* real_write_size)
{
	long code = 1;
	if (!check_conn_is_valid(conn))
		return code;

	if (file_data == NULL || write_size <= 0)
		return code;
	if (conn->connected)
	{
		giop_header giop;
		build_giop_header(conn, &giop);

		request_pack_header request;
		build_request_pack_header(conn, &request, 0x11);

		FS_write_file_pack pack;
		memset((void*)&pack, 0, sizeof(pack));
		strncpy(pack.op, op_command_fs_write_file, sizeof(pack.op));
		pack.op[sizeof(pack.op) - 1] = '\0';

		pack.principal = HtoNl(conn->little_endian, 0x00000000);
		pack.file_handle = fd;
		pack.file_size = write_size;
		memcpy((void*)pack.file_data, file_data, write_size);

		giop.data_length = sizeof(pack) - sizeof(giop) - (sizeof(pack.file_data) - write_size);
		pack.giop = giop;
		pack.request = request;
		socket_send_data(conn->socket, &pack, giop.data_length + sizeof(giop));

		int msg_length = 0;
		memset(&giop, 0, sizeof(giop));
		code = mel_receive_response(conn, &giop, &msg_length);
		if (code == 0)
		{
			long ret = 0;
			msg_length -= socket_recv_data_one_loop(conn->socket, &ret, sizeof(ret));
			msg_length -= socket_recv_data_one_loop(conn->socket, real_write_size, sizeof(real_write_size));
		}
		receive_remain_info_response(conn, &msg_length);
	}
	return code;
}

int ReceiveFsStatData(m70_conn_t* conn, int len, file_FS_stat* stat)
{
	long code = 1;
	if (!check_conn_is_valid(conn))
		return code;

	int readLen = 0;
	int headLen = 0;
	int dataLen = 0;

	FS_stat_file_response_header rsp;
	headLen = sizeof(rsp);
	readLen = socket_recv_data_one_loop(conn->socket, &rsp, headLen);
	dataLen = rsp.data_length;

	if (len > headLen)
	{
		readLen += socket_recv_data_one_loop(conn->socket, (void*)stat, dataLen);
	}
	return readLen;
}
long melFSStatFile(m70_conn_t* conn, const char* filename, file_FS_stat* stat)
{
	long code = 1;
	if (!check_conn_is_valid(conn))
		return code;

	if (filename == NULL || strlen(filename) <= 0)
		return code;
	if (conn->connected)
	{
		giop_header giop;
		build_giop_header(conn, &giop);

		request_pack_header request;
		build_request_pack_header(conn, &request, 0x10);

		FS_stat_file_pack pack;
		memset((void*)&pack, 0, sizeof(pack));
		strncpy(pack.op, op_command_fs_stat_file, sizeof(pack.op));
		pack.op[sizeof(pack.op) - 1] = '\0';

		size_t fileLen = strlen(filename);
		pack.principal = 0x00000000;
		pack.file_name_size = fileLen;
		memcpy(pack.file_name, filename, fileLen);

		giop.data_length = sizeof(pack) - sizeof(giop) - (sizeof(pack.file_name) - fileLen);
		pack.giop = giop;
		pack.request = request;

		socket_send_data(conn->socket, &pack, sizeof(pack));
		memset((void*)&giop, 0, sizeof(giop));
		int msg_length = 0;
		code = mel_receive_response(conn, &giop, &msg_length);
		if (code == 0)
		{
			msg_length -= ReceiveFsStatData(conn, msg_length, stat);
		}
		receive_remain_info_response(conn, &msg_length);
	}
	return code;
}

long melFsOpenDirectory(m70_conn_t* conn, const char* filepath, long* fd)
{
	long code = 1;
	if (!check_conn_is_valid(conn))
		return code;

	if (filepath == NULL || strlen(filepath) <= 0 || fd == NULL)
		return code;

	if (conn->connected)
	{
		giop_header giop;
		build_giop_header(conn, &giop);

		request_pack_header request;
		build_request_pack_header(conn, &request, 0x15);

		FS_open_directory_pack pack;
		memset((void*)&pack, 0, sizeof(pack));
		strncpy(pack.op, op_command_fs_open_dir, sizeof(pack.op));
		pack.op[sizeof(pack.op) - 1] = '\0';
		size_t fileLen = strlen(filepath);

		pack.principal = 0x00000000;
		pack.path_name_size = fileLen;
		memcpy(pack.path_name, filepath, fileLen);

		giop.data_length = sizeof(pack) - sizeof(giop) - (sizeof(pack.path_name) - fileLen);
		pack.giop = giop;
		pack.request = request;

		socket_send_data(conn->socket, &pack, sizeof(pack));
		memset((void*)&giop, 0, sizeof(giop));
		int msg_length = 0;
		code = mel_receive_response(conn, &giop, &msg_length);
		if (code == 0)
		{
			long ret = 0;
			msg_length -= socket_recv_data_one_loop(conn->socket, &ret, sizeof(uint32));
			msg_length -= socket_recv_data_one_loop(conn->socket, fd, sizeof(uint32));
		}
		receive_remain_info_response(conn, &msg_length);
	}
	return code;
}

long melFsCloseDirectory(m70_conn_t* conn, long fd)
{
	long code = 1;
	if (!check_conn_is_valid(conn))
		return code;

	if (conn->connected)
	{
		giop_header giop;
		build_giop_header(conn, &giop);

		request_pack_header request;
		build_request_pack_header(conn, &request, 0x16);

		FS_close_directory_pack pack;
		memset((void*)&pack, 0, sizeof(pack));
		strncpy(pack.op, op_command_fs_close_dir, sizeof(pack.op));
		pack.op[sizeof(pack.op) - 1] = '\0';
		pack.principal = HtoNl(conn->little_endian, 0x00000000);
		pack.file_handle = fd;
		giop.data_length = sizeof(pack) - sizeof(giop);
		pack.giop = giop;
		pack.request = request;

		socket_send_data(conn->socket, &pack, sizeof(pack));
		memset((void*)&giop, 0, sizeof(giop));
		int msg_length = 0;
		code = mel_receive_response(conn, &giop, &msg_length);
		receive_remain_info_response(conn, &msg_length);
	}
	return code;
}

long melFsReadDirectory(m70_conn_t* conn, long fd, char* dirname)
{
	long code = 1;
	if (!check_conn_is_valid(conn))
		return code;

	if (dirname == NULL)
		return code;

	if (conn->connected)
	{
		giop_header giop;
		build_giop_header(conn, &giop);

		request_pack_header request;
		build_request_pack_header(conn, &request, 0x15);

		FS_read_directory_pack pack;
		memset((void*)&pack, 0, sizeof(pack));
		strncpy(pack.op, op_command_fs_read_dir, sizeof(pack.op));
		pack.op[sizeof(pack.op) - 1] = '\0';

		pack.principal = HtoNl(conn->little_endian, 0x00000000);
		pack.file_handle = fd;
		giop.data_length = sizeof(pack) - sizeof(giop);
		pack.giop = giop;
		pack.request = request;

		socket_send_data(conn->socket, &pack, sizeof(pack));
		memset((void*)&giop, 0, sizeof(giop));
		int msg_length = 0;
		code = mel_receive_response(conn, &giop, &msg_length);
		if (code == 0)
		{
			long ret = 0;
			long datasize = 0;
			msg_length -= socket_recv_data_one_loop(conn->socket, &ret, sizeof(ret));
			msg_length -= socket_recv_data_one_loop(conn->socket, &datasize, sizeof(datasize));
			if (datasize)
			{
				long size = 0;
				msg_length -= socket_recv_data_one_loop(conn->socket, &ret, sizeof(ret));
				msg_length -= socket_recv_data_one_loop(conn->socket, &size, sizeof(size));
				if (size > 0)
				{
					msg_length -= receive_data_response(conn, size, 0, dirname);
					dirname[size - 1] = '\n';
				}
			}
		}
		receive_remain_info_response(conn, &msg_length);
	}
	return code;
}

long CancelModal2(m70_conn_t* conn)
{
	long code = 1;
	if (!check_conn_is_valid(conn))
		return code;

	if (conn->connected)
	{
		giop_header giop;
		build_giop_header(conn, &giop);

		request_pack_header request;
		build_request_pack_header(conn, &request, 0x12);

		cancel_modal2_pack pack;
		memset((void*)&pack, 0, sizeof(pack));
		strncpy(pack.op, op_command_cancel_modal2, sizeof(pack.op));
		pack.op[sizeof(pack.op) - 1] = '\0';
		giop.data_length = sizeof(pack) - sizeof(giop);

		pack.giop = giop;
		pack.request = request;
		pack.principal = HtoNl(conn->little_endian, 0x00000000);
		pack.u1 = 0xFFFFFFFF;
		pack.u2 = 0x000002AB;
		pack.u3 = 0x00000000;

		socket_send_data(conn->socket, &pack, sizeof(pack));
		memset((void*)&giop, 0, sizeof(giop));
		int msg_length = 0;
		code = mel_receive_response(conn, &giop, &msg_length);
		receive_remain_info_response(conn, &msg_length);
	}
	return code;
}

int mel_receive_response(m70_conn_t* conn, giop_header* giop, int* remain_length)
{
	int ret_code = 0;
	if (giop == NULL || conn == NULL)
		return -1;

	int recv_count = sizeof(giop_header);
	int count = socket_recv_data_one_loop(conn->socket, giop, recv_count);
	if (recv_count == count)
	{
		*remain_length = giop->data_length;
		if (*remain_length >= recv_count && giop->msg_type == (byte)MSG_TYPES_Reply)
		{
			response_pack_header rpp;
			memset((void*)&rpp, 0, sizeof(response_pack_header));
			*remain_length -= socket_recv_data_one_loop(conn->socket, &rpp, sizeof(rpp));
			ret_code = rpp.is_error;
			if (ret_code != 0)
				ret_code = receive_error_data_response(conn, remain_length);
			else
				return ret_code;
		}
		receive_remain_info_response(conn, remain_length);
	}

	// Return value of -1 indicates socket exception
	if (count == -1)
	{
		conn->connected = false;
		ret_code = -1;
	}
	return ret_code;
}

void build_giop_header(m70_conn_t* conn, giop_header* giop)
{
	if (giop == NULL)
		return;

	if (!check_conn_is_valid(conn))
		return;

	memcpy(giop->magic_number, "GIOP", 4);
	giop->version = 1;
	giop->byte_order = conn->little_endian ? 1 : 0;
	giop->msg_type = MSG_TYPES_Request;
}

void build_request_pack_header(m70_conn_t* conn, request_pack_header* request, int op_name_length)
{
	if (request == NULL)
		return;

	if (!check_conn_is_valid(conn))
		return;

	request->sc_list = HtoNl(conn->little_endian, 0x00);
	request->request_id = conn->request_id;
	request->expected = 0x01;
	memcpy(request->reserved, "\x0\x0\x0", 3);
	request->object_key_length = HtoNl(conn->little_endian, 0x04);
	request->object_key = HtoNl(conn->little_endian, 0x01);

	request->operation_length = HtoNl(conn->little_endian, op_name_length);
}

bool check_conn_is_valid(m70_conn_t* conn)
{
	return conn != NULL && conn->socket > 0 && conn->connected;
}