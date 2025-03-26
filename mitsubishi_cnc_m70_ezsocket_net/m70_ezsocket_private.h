#ifndef __H_M70_EZSOCKET_PRIVATE_H__
#define __H_M70_EZSOCKET_PRIVATE_H__
#include "typedef.h"

uint32 get_axis_real_no(uint32 axis_index);

typedef enum tag_giop_msg_types
{
	MSG_TYPES_Request = 0,
	MSG_TYPES_Reply = 1,
	MSG_TYPES_CancelRequest = 2,
	MSG_TYPES_LocateRequest = 3,
	MSG_TYPES_LocateReply = 4,
	MSG_TYPES_CloseConnection = 5,
	MSG_TYPES_MessageError = 6
} giop_msg_types;

#pragma pack(push)
#pragma pack(1)

typedef struct
{
	short int_data_nos;
	short dec_data_nos;
	int32 option;
	float fData;
	int32 data;
} double_data;

typedef struct
{
	short int_data_nos;
	short dec_data_nos;
	int32 option;
	double data;
} float_bin_data;

typedef struct
{
	int msg_length;
	char text[512];
} T_string;

typedef struct
{
	float f;  // Floating point number
	uint32 d; // Integer
} T_set_float;

typedef union
{
	T_set_float u;
	uint32 u2;
} T_data_value;

typedef struct
{
	uint32 mode;
	uint32 file_size;
	uint32 year;  // Base 1950+year = current year
	uint32 month; //
	uint32 day;
	uint32 hour;
	uint32 minute;
	uint32 second;
} FileStat;

typedef struct
{
	char magic_number[4]; // Header flag 'GIOP'
	ushort version;		  // Protocol version (1.0) 0x0100
	byte byte_order;	  // 01 little-endian
	byte msg_type;		  // Message type (00)Request Message type
	uint32 data_length;	  // Data area length
} giop_header;

typedef struct
{
	uint32 sc_list;			  // ServiceContextList is 0x00000000
	uint32 request_id;		  // Session ID Request id random number
	byte expected;			  // 0x01  byte   Response expected
	char reserved[3];		  // 000000    3 bytes
	uint32 object_key_length; // 0x00000004
	uint32 object_key;		  // 0x00000001
	uint32 operation_length;  // Operation Length
} request_pack_header;

typedef struct
{
	uint32 sc_list;	   // 0x00000000
	uint32 request_id; // Session ID
	uint32 is_error;   // Error status: 0 success; 1 user error; 2 system error
} response_pack_header;

typedef struct
{
	uint32 u1; //
	uint32 data_type;
	uint32 data_length;
} get_data_response_header;

typedef struct
{
	uint32 u1; //
	uint32 u2; //
	uint32 data_length;
	uint32 data_type;
	uint32 data_count;
} get_data_float_bin_response_header;

typedef struct
{
	uint32 u1;			// 4 bytes 0
	uint32 data_length; // 4 bytes
	byte reserved[8];
} FS_stat_file_response_header;

typedef struct
{
	uint32 mode;
	byte reserved1[8];
	uint32 file_size;
	byte reserved2[24];
	uint32 year; // 基数1950+year = 当前年份
	uint32 month;
	uint32 day;
	uint32 hour;
	uint32 minute;
	uint32 second;
} file_FS_stat;

typedef struct
{
	char u[3];
	uint32 error_code;
	uint32 u1;
} mel_error_code;

typedef struct
{
	giop_header giop;
	request_pack_header request;
	char op[32];	  // mochaGetCurrentPrgBlockFirst\0 --29
	uint32 principal; // 0x00000000
	uint32 system_no;
	uint32 row_count;
} prog_block_pack;

typedef struct
{
	giop_header giop;
	request_pack_header request;
	char op[32];	  // mochaGetCurrentAlarmMsgFirst\0 --29
	uint32 principal; // 0x00000000
	uint32 system_no; // System number
	uint32 msg_count; // Number of returned messages
	uint32 msg_type;  // Message type 0=all messages
} alarm_info_pack;
typedef struct
{
	giop_header giop;
	request_pack_header request;
	char op[16];		// mochaGetData\0 --13
	uint32 principal;	// 0x00000000
	uint32 section;		// 36000000 uint32 53    Main section
	uint32 sub_section; // 10290000 uint32 10512 Sub-section
	uint32 system_no;	// 01000000 uint32 System number
	uint32 axis_no;		// 02000000 uint32 2 Axis number (when 4 axes, the 4th bit is 1, 0x80...)
	uint32 u2;			// 00000000 uint32
	uint32 data_type;	// 03000000 uint32
} get_data_pack;

typedef struct
{
	uint32 f; // Floating point number
	uint32 d; // Integer
} set_float;

typedef struct
{
	short int_data_nos;
	short dec_data_nos;
	uint32 option;
	double data;
} set_float_bin;

typedef struct
{
	int32 msg_size;
	char text[512];
} set_string;

typedef union
{
	set_float_bin u3;
	set_float u;
	set_string u_str;
	uint32 u_dword;
	ushort u_word;
	byte u_byte;
} data_value;

typedef struct
{
	giop_header giop;
	request_pack_header request;
	char op[16];		 // mochaGetData\0 --13
	uint32 principal;	 // 0x00000000
	uint32 section;		 // 36000000 uint32 53    主分区
	uint32 sub_section;	 // 10290000 uint32 10512 子分区
	uint32 system_no;	 // 01000000 uint32 系统号
	uint32 axis_no;		 // 02000000 uint32 2 轴号（4轴时，第4位为1，0x80...）
	uint32 u2;			 // 00000000 uint32
	uint32 data_type;	 // 03000000 uint32
	uint32 byte_numbers; // 04000000 byte numbers
	data_value data;
} set_data_pack;

typedef struct
{
	giop_header giop;
	request_pack_header request;
	char op[20];	  // mochaCancelModal2\0 --18
	uint32 principal; // 0x00000000
	uint32 u1;		  // 系统号
	uint32 u2;		  // 返回的信息数量
	uint32 u3;		  // 信息类别0=所有消息
} cancel_modal2_pack;

typedef struct
{
	giop_header giop;
	request_pack_header request;
	char op[16];		   // mochaFSOpenFile\0 --16
	uint32 principal;	   // 0x00000000
	uint32 flag;		   // 访问方式
	uint32 mode;		   // Fixed to 0 (Not used)
	uint32 file_name_size; // 路径长度
	byte file_name[256];   // 路径信息
} FS_open_file_pack;

typedef struct
{
	giop_header giop;
	request_pack_header request;
	char op[16];		// mochaFSReadFile\0 --16
	uint32 principal;	// 0x00000000
	uint32 file_handle; // 文件句柄
	uint32 file_size;	// 文件长度
} FS_read_file_pack;

typedef struct
{
	giop_header giop;
	request_pack_header request;
	char op[0x12]; // mochaFSCreateFile\0 --
	char reserved[2];
	uint32 principal;	   // 0x00000000
	uint32 mode;		   // 文件访问方式
	uint32 file_name_size; // 文件名称长度
	char file_name[256];   // 文件名长度
} FS_create_file_pack;

typedef struct
{
	giop_header giop;
	request_pack_header request;
	char op[0x11]; // mochaFSCloseFile\0 --
	char reserved[3];
	uint32 principal;	// 0x00000000
	uint32 file_handle; // 文件句柄
} FS_close_file_pack;

typedef struct
{
	giop_header giop;
	request_pack_header request;
	char op[20];		 // mochaFSWriteFile\x0\x0\x0\x0 --20
	uint32 principal;	 // 0x00000000
	uint32 file_handle;	 // 文件句柄
	uint32 file_size;	 // 文件内容长度
	byte file_data[512]; // 文件内容
} FS_write_file_pack;

typedef struct
{
	// packLen+ opLen + principalLen(4)+subdataLen(4+parmLen)
	giop_header giop;
	request_pack_header request;
	char op[18]; // mochaFSRemoveFile\x0\x0\x0 --18
	char reserved[2];
	uint32 principal; // 0x00000000
	uint32 fileLen;	  // 0x13000000
	char file_name[256];
} FS_remove_pack;

typedef struct
{
	giop_header giop;
	request_pack_header request;
	char op[24];		   // mochaFSOpenDirectory\x0\x0\x0\x0 --24
	uint32 principal;	   // 0x00000000
	uint32 path_name_size; // 目录路径长度
	byte path_name[256];   // 目录路径
} FS_open_directory_pack;

typedef struct
{
	giop_header giop;
	request_pack_header request;
	char op[24];		// mochaFSReadDirectory\x0\x0\x0\x0 --24
	uint32 principal;	// 0x00000000
	uint32 file_handle; // 文件句柄
} FS_read_directory_pack;

typedef struct
{
	giop_header giop;
	request_pack_header request;
	char op[24];		// mochaFSCloseDirectory\x0\x0\x0 --24
	uint32 principal;	// 0x00000000
	uint32 file_handle; // 文件句柄
} FS_close_directory_pack;

typedef struct
{
	giop_header giop;
	request_pack_header request;
	char op[16];		   // mochaFSStatFile\x0 --16
	uint32 principal;	   // 0x00000000
	uint32 file_name_size; // 文件路径长度
	byte file_name[256];   // 文件路径
} FS_stat_file_pack;

#pragma pack(pop)

#endif //__H_M70_EZSOCKET_PRIVATE_H__