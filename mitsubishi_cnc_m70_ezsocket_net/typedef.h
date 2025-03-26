#ifndef __H_TYPEDEF_H__
#define __H_TYPEDEF_H__

#include <stdint.h>
#include <stdbool.h>

typedef unsigned char byte;
typedef unsigned short ushort;
typedef signed int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;

#define BUFFER_SIZE 512

typedef enum _tag_m70_error_code
{
	M70_ERROR_CODE_OK = 0,			  // Success
	M70_ERROR_CODE_FAILED = 1,		  // Error
	M70_ERROR_CODE_SOCKET_FAILED = 2, // Network exception
	M70_ERROR_CODE_UNKOWN = 99,		  // Unknown error
} m70_error_code_e;

typedef enum _tag_m70_NCType
{
	EZNC_SYS_MAGICCARD64 = 0,  // MELDASMAGIC Card64
	EZNC_SYS_MAGICBOARD64 = 1, // MELDASMAGIC64
	EZNC_SYS_MELDAS6X5L = 2,   // MELDAS600L(M6x5L)
	EZNC_SYS_MELDAS6X5M = 3,   // MELDAS600M(M6x5M)
	EZNC_SYS_MELDASC6C64 = 4,  // MELDASC6C64
	EZNC_SYS_MELDAS700L = 5,   // MELDAS700L
	EZNC_SYS_MELDAS700M = 6,   // MELDAS700M
	EZNC_SYS_MELDASC70 = 7,	   // MELDASC70
	EZNC_SYS_MELDAS800M = 8,
	EZNC_SYS_MELDAS800L = 9
} m70_nc_type_e;

typedef enum _tag_data_type
{
	T_CHAR = 0x1,	  // 1
	T_SHORT = 0x2,	  // 2
	T_LONG = 0x3,	  // 4
	T_DLONG = 0x4,	  // 8
	T_DOUBLE = 0x5,	  // 8
	T_FLOATBIN = 0x6, // 16

	T_STR = 0x10,
	T_DecStr = 0x11,
	T_HexStr = 0x12,
	T_BinStr = 0x13,
	T_FloatStr = 0x14,
	T_WStr = 0x15,
	T_DecWStr = 0x16,
	T_HexWStr = 0x17,
	T_BinWStr = 0x18,
	T_FloatWStr = 0x19,
	T_CharBuff = 0x1a,

	T_UCHAR = 0X21,
	T_USHORT = 0X22,
	T_UINT32 = 0X23,

	T_CLCTDATA = 0x100, // 36 byte
	T_BUFF = 0x103		// 0x103 T_BUF
} m70_data_type_e;

typedef enum _tag_device_status
{
	Unkown = 0,
	STOP = 1,	 // Stop alarm
	RUN = 2,	 // Running
	IDLE = 3,	 // Idle
	OFFLINE = 4, // Offline
	DEBUG = 5,	 // Debugging
} m70_device_status_e;

// 0: MEM,1: RS232(DNC),2: LNK,3: MDI,4: PC,5: MNL,6: JOG,7: J+H,8: R+H,
// 9: HDL,10: STP,11: STP,12: ZRN,13: DRT,14: INI,15: NON,16: LIN
typedef enum _tag_run_mode
{
	MEM = 0,
	DNC = 1,
	LNK = 2,
	MDI = 3,
	PC = 4,
	MNL = 5,
	JOG = 6,
	J_H = 7,
	R_H = 8,
	HDL = 9,
	STP = 10,
	STP1 = 11,
	ZRN = 12,
	DRT = 13,
	INI = 14,
	NON = 15,
	LIN = 16
} m70_run_mode_e;

typedef enum _tag_run_status
{
	RST = 0,
	EMG = 1,
	RDY = 2,
	AUT = 3,
	SYN = 4,
	CRS = 5,
	BST = 6,
	HLD = 7
} m70_run_status_e;

typedef enum _tag_nc_machine_type
{
	MACHINE_TYPE_MC = 0,
	MACHINE_TYPE_Lathe = 1
} m70_nc_machine_type_e;

typedef enum _tag_common_variable_type
{
	COMM_VAR_Type_100 = 0,
	COMM_VAR_Type_500
} m70_common_variable_type_e;

typedef enum _tag_program_name_type
{
	PRG_TYPE_ProgramNo = 0,
	PRG_TYPE_SequenceNumber,
	PRG_TYPE_BlockNumber,
	PRG_TYPE_ProgramPath,
} program_name_type_e;

typedef enum _tag_file_info_type
{
	REG_PROG_NOS = 0, //  Number of registered machining programs
	USED_PROG_NOS,	  //  Remaining machining programs
	CAPA_CHAR_NOS,	  //  Machining program character capacity
	FREE_CHAR_NOS,	  //  Remaining characters in machining program
	TRANS_SIZE		  //  Transfer data size for melCopyFile
} m70_file_info_type_e;

typedef enum _tag_position_type
{
	POS_WRK = 0, // Workpiece coordinate position counter
	POS_MCH,	 // Machine position counter
	POS_CURRENT, // Current position counter
	POS_RELATV,	 // Relative position counter
	POS_PROGRAM, // Program position counter
	DISTANCE	 // Remaining command
} position_type_e;

typedef enum _tag_feed_speed_type
{
	FA = 0, // F command feed rate (FA)
	FM,		// Manual effective feed rate (FM)
	FS,		// Synchronous feed rate (FS)
	FC,		// Automatic effective feed rate (Fc)
	FE		// Screw feed (FE)
} feed_speed_type_e;

typedef enum _tag_plc_data_type
{
	PLC_Bit = 0,
	PLC_Char = 1,
	PLC_Short = 2,
	PLC_Int32 = 3,
	PLC_DLong = 4,
	PLC_Double = 5
} plc_data_type_e;

typedef enum _tag_alarm_message_type
{
	M_ALM_ALL_ALARM = 0x000,
	M_ALM_NC_ALARM = 0x100,
	M_ALM_STOP_CODE = 0x200,
	M_ALM_PLC_ALARM = 0x300,
	M_ALM_OPE_MSG = 0x400,
	M_ALM_ALL_NON_STOPCD = 0x1000,
	M_ALM_NC_SYSTEM = 0x101,
	M_ALM_NC_SERVO = 0x102,
	M_ALM_NC_MCP = 0x103,
	M_ALM_NC_BASICPLC = 0x104,
	M_ALM_NC_USERPLC = 0x105,
	M_ALM_NC_PROGRAM = 0x106,
	M_ALM_NC_SERVO_WARNING = 0x107,
	M_ALM_NC_MCP_WARNING = 0x108,
	M_ALM_NC_SYSTEM_WARNING = 0x109,
	M_ALM_NC_OPERATION = 0x10A,
	M_ALM_OPE_ALARM = 0x10B
} alarm_message_type_e;

typedef struct m70_conn
{
	int32 socket;
	bool connected;
	m70_nc_type_e nc_type;
	uint32 request_id;
	bool little_endian;
} m70_conn_t;

#pragma pack(push)
#pragma pack(1)

typedef struct
{
	int32 alarm_no;		// Alarm number
	int32 alarm_length; // Alarm length
	byte text[256];
} alarm_string;

typedef struct
{
	int32 current_block; // Current block number
	int32 current_row;	 // Current row number
	int32 u1;			 // Unknown
	int32 block_length;	 // Character length
	byte text[512];
} prog_block;

typedef struct
{
	uint32 f; // Float number
	uint32 d; // Integer
} get_float;

typedef struct
{
	short int_data_nos;
	short dec_data_nos;
	uint32 option;
	double data;
} get_float_bin;

typedef struct
{
	int32 msg_size;
	char text[512];
} get_string;

typedef union
{
	get_float_bin u_float_bin;
	get_float u_float;
	get_string u_str;
	double u_double;
	uint32 u_dword;
	ushort u_word;
	byte u_byte;
} get_data_value;

#pragma pack(pop)

#endif // !__H_TYPEDEF_H__