#ifndef __H_M70_GIOP_H__
#define __H_M70_GIOP_H__

#include "utill.h"
#include "typedef.h"
#include "m70_ezsocket_private.h"

// Connection management
bool giop_connect(const char* ip, int type, int port, m70_conn_t* conn);
void giop_disconnect(m70_conn_t* conn);
bool check_conn_is_valid(m70_conn_t* conn);

// Data operations
long melGetData(m70_conn_t* conn, int section, int sub_section, int system_no, int axis_no, m70_data_type_e* in_out_data_type, void* out_data_value);
long melSetData(m70_conn_t* conn, int section, int sub_section, int system_no, int axis_no, m70_data_type_e data_type, void* in_data_value);

// Alarm and program block
long melGetCurrentAlarmMsg(m70_conn_t* conn, int system_no, int msg_count, int msg_type, void* msg);
long melGetCurrentPrgBlock(m70_conn_t* conn, int system_no, int row_count, void* msg);

// File operations
long melRemoveFile(m70_conn_t* conn, const char* filename);
long melFsCreateFile(m70_conn_t* conn, const char* filename, long mode, long* fd);
long melFsCloseFile(m70_conn_t* conn, long fd);
long melFSStatFile(m70_conn_t* conn, const char* filename, file_FS_stat* stat);
long melFsOpenFile(m70_conn_t* conn, const char* filename, long mode, long* fd);
long melFsReadFile(m70_conn_t* conn, long fd, void* file_data, long* read_size, long need_read_size);
long melFsWriteFile(m70_conn_t* conn, long fd, void* file_data, long write_size, long* real_write_size);

// Directory operations
long melFsOpenDirectory(m70_conn_t* conn, const char* filepath, long* fd);
long melFsCloseDirectory(m70_conn_t* conn, long fd);
long melFsReadDirectory(m70_conn_t* conn, long fd, char* directory_list);

// Miscellaneous
long CancelModal2(m70_conn_t* conn);
long receive_remain_info_response(m70_conn_t* conn, int* len);

// Internal utilities
int mel_receive_response(m70_conn_t* conn, giop_header* giop, int* remain_length);
void build_giop_header(m70_conn_t* conn, giop_header* giop);
void build_request_pack_header(m70_conn_t* conn, request_pack_header* request, int op_name_length);

#endif // __H_M70_GIOP_H__