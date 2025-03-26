#include "m70_giop.h"
#include "m70_ezsocket.h"
#include "m70_ezsocket_private.h"
#include "m70_error.h"
#include "m70_log.h"

#include "socket.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
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

bool m70_cnc_connect(const char* ip_addr, int port, m70_nc_type_e type, m70_conn_t* conn)
{
	M70_LOG_INFO("Attempting to connect to CNC device: %s:%d, Type: %d", ip_addr, port, type);
	
	if (ip_addr == NULL || port <= 0 || conn == NULL) {
		M70_ERROR_SET(M70_ERROR_CODE_EX_CNC_INVALID_PARAM, "Invalid connection parameters: IP=%s, Port=%d", 
				  ip_addr ? ip_addr : "NULL", port);
		M70_LOG_ERROR("Invalid connection parameters: IP=%s, Port=%d", ip_addr ? ip_addr : "NULL", port);
		return false;
	}

	memset((void*)conn, 0, sizeof(m70_conn_t));
	bool result = giop_connect(ip_addr, type, port, conn);
	
	if (result) {
		M70_LOG_INFO("Successfully connected to CNC device: %s:%d, Socket=%d", ip_addr, port, conn->socket);
	} else {
		M70_LOG_ERROR("Failed to connect to CNC device: %s:%d", ip_addr, port);
		M70_ERROR_SET(M70_ERROR_CODE_EX_CONN_FAILED, "Failed to connect to CNC device: %s:%d", ip_addr, port);
	}
	
	return result;
}

void m70_cnc_disconnect(m70_conn_t* conn)
{
	if (!check_conn_is_valid(conn)) {
		M70_LOG_WARNING("Attempting to disconnect an invalid connection");
		M70_ERROR_SET(M70_ERROR_CODE_EX_CONN_INVALID, "Attempting to disconnect an invalid connection");
		return;
	}

	M70_LOG_INFO("Disconnecting from CNC device, Socket=%d", conn->socket);
	giop_disconnect(conn);
	M70_LOG_DEBUG("CNC device connection has been disconnected");
}

m70_error_code_e m70_cnc_read_status(m70_conn_t* conn, short system_no, m70_device_status_e* status, m70_run_mode_e* mode, m70_run_status_e* run_status)
{
	M70_LOG_DEBUG("Reading CNC status, System No: %d", system_no);
	
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn)) {
		M70_ERROR_SET(M70_ERROR_CODE_EX_CONN_INVALID, "Invalid connection");
		M70_LOG_ERROR("Failed to read CNC status: Invalid connection");
		return ret;
	}

	*mode = MEM;
	*run_status = RST;
	*status = OFFLINE;

	m70_data_type_e data_type = T_SHORT;
	short temp_mode = 0;
	if (0 == melGetData(conn, 35, 11, system_no, 0, &data_type, &temp_mode))
	{
		*mode = (m70_run_mode_e)temp_mode;
		*status = IDLE;
		M70_LOG_DEBUG("CNC running mode: %d", temp_mode);
		
		if (temp_mode == MEM || temp_mode == DNC)
		{
			long auto_status = 0;
			data_type = T_DLONG;
			if (0 == melGetData(conn, 35, 20, system_no, 0, &data_type, &auto_status))
			{
				if (auto_status == 1) {
					*status = RUN;
					M70_LOG_DEBUG("CNC device status: Running");
				}
			}
			else {
				M70_LOG_WARNING("Failed to get automatic status");
			}
		}
		else if (temp_mode >= LNK && temp_mode <= LIN)
		{
			*status = DEBUG;
			M70_LOG_DEBUG("CNC device status: Debugging");
		}

		short temp_status = 0;
		data_type = T_SHORT;
		if (0 == melGetData(conn, 35, 10, system_no, 0, &data_type, &temp_status))
		{
			*run_status = (m70_run_status_e)temp_status;
			M70_LOG_DEBUG("CNC running status: %d", temp_status);
			
			if (temp_status == EMG) {
				*status = STOP;
				M70_LOG_WARNING("CNC device is in emergency stop state");
			}
		}
		else {
			M70_LOG_WARNING("Failed to get running status");
		}

		ret = M70_ERROR_CODE_OK;
		M70_LOG_INFO("Successfully read CNC status: system_no=%d, status=%d, mode=%d, run_status=%d", 
				 system_no, *status, *mode, *run_status);
	}
	else {
		M70_ERROR_SET(M70_ERROR_CODE_EX_CNC_NOT_READY, "Failed to read CNC mode");
		M70_LOG_ERROR("Failed to read CNC status: Unable to get running mode");
	}

	return ret;
}

m70_error_code_e m70_cnc_read_counter(m70_conn_t* conn, short system_no, uint32* counter)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	*counter = 0;
	m70_data_type_e data_type = T_LONG;
	if (0 == melGetData(conn, 126, 8002, system_no, 0, &data_type, counter))
		ret = M70_ERROR_CODE_OK;

	return ret;
}

static m70_error_code_e read_data_count(m70_conn_t* conn, uint32* count, int param)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	byte axis_count = 0;
	m70_data_type_e data_type = T_CHAR;
	if (0 == melGetData(conn, 2, param, 0, 0, &data_type, &axis_count))
	{
		*count = (short)axis_count;
		ret = M70_ERROR_CODE_OK;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_system_count(m70_conn_t* conn, uint32* count)
{
	return read_data_count(conn, count, 1);
}

m70_error_code_e m70_cnc_read_nc_axis_count(m70_conn_t* conn, uint32* count)
{
	return read_data_count(conn, count, 2);
}

m70_error_code_e m70_cnc_read_all_axis_count(m70_conn_t* conn, uint32* count)
{
	return read_data_count(conn, count, 3);
}

m70_error_code_e m70_cnc_read_spindle_axis_count(m70_conn_t* conn, uint32* count)
{
	return read_data_count(conn, count, 4);
}

m70_error_code_e m70_cnc_read_plc_axis_count(m70_conn_t* conn, uint32* count)
{
	return read_data_count(conn, count, 5);
}

m70_error_code_e m70_cnc_read_nc_type(m70_conn_t* conn, m70_nc_machine_type_e* type)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	*type = MACHINE_TYPE_MC;
	byte temp = 0;
	m70_data_type_e data_type = T_CHAR;
	if (0 == melGetData(conn, 2, 100, 0, 0, &data_type, &temp))
	{
		if (1 == (short)temp)
			*type = MACHINE_TYPE_Lathe;

		ret = M70_ERROR_CODE_OK;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_nc_version(m70_conn_t* conn, char* version)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn) || version == NULL)
		return ret;

	T_string data = { 0 };
	m70_data_type_e data_type = T_STR;
	if (0 == melGetData(conn, 67, 1, 0, 0, &data_type, &data))
	{
		strncpy(version, data.text, data.msg_length);
		ret = M70_ERROR_CODE_OK;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_nc_name_version(m70_conn_t* conn, char* version)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn) || version == NULL)
		return ret;

	T_string data = { 0 };
	m70_data_type_e data_type = T_STR;
	if (0 == melGetData(conn, 68, 1, 0, 0, &data_type, &data))
	{
		strncpy(version, data.text, data.msg_length);
		ret = M70_ERROR_CODE_OK;
	}
	return ret;
}

m70_error_code_e m70_cnc_read_plc_version(m70_conn_t* conn, char* version)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn) || version == NULL)
		return ret;

	T_string data = { 0 };
	m70_data_type_e data_type = T_STR;
	if (0 == melGetData(conn, 67, 2, 0, 0, &data_type, &data))
	{
		strncpy(version, data.text, data.msg_length);
		ret = M70_ERROR_CODE_OK;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_main_program_name(m70_conn_t* conn, short system_no, program_name_type_e type, char* prog)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	bool is_ok = false;
	T_string strData = { 0 };
	long data = 0;
	m70_data_type_e data_type = T_STR;
	switch (type)
	{
	case PRG_TYPE_ProgramNo:
		is_ok = 0 == melGetData(conn, 45, 101, system_no, 0, &data_type, &strData);
		break;

	case PRG_TYPE_SequenceNumber:
		data_type = T_DLONG;
		is_ok = 0 == melGetData(conn, 45, 102, system_no, 0, &data_type, &data);
		break;

	case PRG_TYPE_BlockNumber:
		data_type = T_DLONG;
		is_ok = 0 == melGetData(conn, 45, 103, system_no, 0, &data_type, &data);
		break;

	case PRG_TYPE_ProgramPath:
		is_ok = 0 == melGetData(conn, 45, 100, system_no, 0, &data_type, &strData);
		break;
	}

	if (is_ok)
	{
		if (type == PRG_TYPE_ProgramNo || type == PRG_TYPE_ProgramPath)
			strcpy(prog, strData.text);
		else
		{
			char strTemp[32] = { 0 };
			sprintf(strTemp, "%ld", data);
			strcpy(prog, strTemp);
		}
		ret = M70_ERROR_CODE_OK;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_sub_program_name(m70_conn_t* conn, short system_no, program_name_type_e type, char* prog)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	bool is_ok = false;
	T_string strData = { 0 };
	long data = 0;
	m70_data_type_e data_type = T_DLONG;
	switch (type)
	{
	case PRG_TYPE_ProgramNo:
		data_type = T_STR;
		is_ok = 0 == melGetData(conn, 45, 201, system_no, 0, &data_type, &strData);
		break;

	case PRG_TYPE_SequenceNumber:
		is_ok = 0 == melGetData(conn, 45, 202, system_no, 0, &data_type, &data);
		break;

	case PRG_TYPE_BlockNumber:
		is_ok = 0 == melGetData(conn, 45, 203, system_no, 0, &data_type, &data);
		break;
	}

	if (is_ok)
	{
		if (type == PRG_TYPE_ProgramNo || type == PRG_TYPE_ProgramPath)
			strcpy(prog, strData.text);
		else
		{
			char strTemp[32] = { 0 };
			sprintf(strTemp, "%ld", data);
			strcpy(prog, strTemp);
		}
		ret = M70_ERROR_CODE_OK;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_program_file_info(m70_conn_t* conn, short system_no, m70_file_info_type_e type, int* numbers)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	bool is_ok = false;

	int section = 25;
	int sub_section = 1;

	switch (type)
	{
	case REG_PROG_NOS:
		sub_section = 1;
		break;

	case USED_PROG_NOS:
		sub_section = 2;
		break;

	case CAPA_CHAR_NOS:
		sub_section = 3;
		break;

	case FREE_CHAR_NOS:
		sub_section = 4;
		break;

	case TRANS_SIZE:
		sub_section = 10;
		break;
	}

	long data = 0;
	m70_data_type_e data_type = T_DLONG;
	is_ok = 0 == melGetData(conn, section, sub_section, system_no, 0, &data_type, &data);
	if (is_ok)
	{
		ret = M70_ERROR_CODE_OK;
		*numbers = data;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_program_block(m70_conn_t* conn, short system_no, int row_count, prog_block* block)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn) || block == NULL)
		return ret;

	if (0 == melGetCurrentPrgBlock(conn, system_no, row_count, block))
		ret = M70_ERROR_CODE_OK;
	return ret;
}

m70_error_code_e m70_cnc_read_alarm(m70_conn_t* conn, short system_no, int msg_count, alarm_message_type_e type, alarm_string* alarms)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn) || alarms == NULL)
		return ret;

	if (0 == melGetCurrentAlarmMsg(conn, system_no, msg_count, type, alarms))
		ret = M70_ERROR_CODE_OK;

	return ret;
}

m70_error_code_e m70_cnc_read_is_alarm(m70_conn_t* conn, short system_no, bool* alarm)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	*alarm = false;
	alarm_string alarm_info;
	bool isOk = 0 == melGetCurrentAlarmMsg(conn, system_no, 1, M_ALM_ALL_ALARM, &alarm_info);
	if (isOk && alarm_info.alarm_length > 0)
		*alarm = true;

	return ret;
}

m70_error_code_e m70_cnc_read_current_tool_no(m70_conn_t* conn, short system_no, uint32* tool_no)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	// 目前发现三种：R10620、R536、大小分区
	// 主轴刀号
	*tool_no = 0;
	int section = 21;	// M_SEC_SP_WAIT
	int subSection = 1; // M_SSEC_SPWAIT_TOOL_NO(1)	 Spindle stand by (Tool No.) (x=0 to 4) , T_LONG
	m70_data_type_e type = T_LONG;
#if true
	// R536
	section = 55;
	subSection = 100536;
	type = T_SHORT;
#endif
	long data = 0;
	if (0 == melGetData(conn, section, subSection, system_no, 0, &type, &data))
	{
		*tool_no = data;
		ret = M70_ERROR_CODE_OK;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_svo_load(m70_conn_t* conn, short system_no, short* svo_load, uint32 axis_index, bool is_abs)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	uint32 axis_flag = 1;
	axis_flag = get_axis_real_no(axis_flag);
	short data;
	m70_data_type_e data_type = T_SHORT;
	if (0 == melGetData(conn, 59, 4, system_no, axis_flag, &data_type, &data))
	{
		*svo_load = is_abs ? abs(data) : data;
		ret = M70_ERROR_CODE_OK;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_axis_position(m70_conn_t* conn, short system_no, double* pos, uint32 axis_index, position_type_e type)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	*pos = 0.0;
	int pos_type = 2;
	switch (type)
	{
	case POS_WRK:
		pos_type = 1;
		break;

	case POS_RELATV:
		pos_type = 4;
		break;

	case POS_CURRENT:
		pos_type = 3;
		break;

	case DISTANCE:
		pos_type = 6;
		break;

	case POS_PROGRAM:
		pos_type = 5;
		break;

	case POS_MCH:
	default:
		pos_type = 2;
		break;
	}

	uint32 axis_flag = get_axis_real_no(axis_index);
	get_data_value data = { 0 };
	m70_data_type_e data_type = T_FLOATBIN;
	if (0 == melGetData(conn, 37, pos_type, system_no, axis_flag, &data_type, &data))
	{
		ret = M70_ERROR_CODE_OK;
		if (data_type == T_FLOATBIN)
			*pos = data.u_float_bin.data;
		else
			*pos = data.u_double;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_all_axis_position(m70_conn_t* conn, short system_no, double* pos, int* pos_count, position_type_e type)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	byte data = 0;
	m70_data_type_e data_type = T_CHAR;
	if (0 == melGetData(conn, 2, 2, 0, 0, &data_type, &data))
	{
		*pos_count = data;
		int num = data;
		for (int i = 1; i <= num; i++)
		{
			double temp = 0;
			m70_cnc_read_axis_position(conn, system_no, &temp, i, type);
			pos[i - 1] = temp;
		}
		ret = M70_ERROR_CODE_OK;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_axis_name(m70_conn_t* conn, short system_no, char* names, int* axis_count)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	byte data = 0;
	int i = 0;
	m70_data_type_e data_type = T_CHAR;
	if (0 == melGetData(conn, 2, 2, 0, 0, &data_type, &data))
	{
		*axis_count = data;
		size_t num = 0;
		for (i = 1; i <= data; i++)
		{
			T_string temp = { 0 };
			data_type = T_STR;
			melGetData(conn, 127, 1, system_no, get_axis_real_no(i), &data_type, &temp);
			strcpy(names + num, temp.text);
			num += strlen(temp.text);
			if (i < data)
			{
				strcat(names + num, ",");
				num++;
			}
		}
		ret = M70_ERROR_CODE_OK;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_spindle_speed(m70_conn_t* conn, short system_no, uint32* speed, uint32 axis_index)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	*speed = 0;
	axis_index = get_axis_real_no(axis_index);
	long data = 0;
	m70_data_type_e data_type = T_DLONG;
	if (0 == melGetData(conn, 34, 1, system_no, axis_index, &data_type, &data))
	{
		*speed = data;
		ret = M70_ERROR_CODE_OK;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_spindle_override(m70_conn_t* conn, short system_no, short* spindle_override)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	short temp = 0;
	// Y188F SPS1 Spindle override setting method selection First spindle {(4 axes) difference 96} Second axis Y18EF
	int subSection = 16287 + 96 * (system_no - 1);
	byte bType = 0;
	m70_data_type_e data_type = T_CHAR;
	if (0 == melGetData(conn, 53, subSection, 0, 0, &data_type, &bType)) // Y188F
	{
		if (bType == 0)
		{
			data_type = T_UCHAR;
			// Y1888 SP11 主轴倍率 代码1 第1主轴 {(6个主轴） 相差 96} 第二轴 Y18E8
			if (0 == melGetData(conn, 54, 16280 + 96 * (system_no - 1), 0, 0, &data_type, &bType)) // Y1888
			{
				switch (bType)
				{
				case 0x7:
					temp = 50;
					break;
				case 0x3:
					temp = 60;
					break;
				case 0x2:
					temp = 70;
					break;
				case 0x6:
					temp = 80;
					break;
				case 0x4:
					temp = 90;
					break;
				case 0x1:
					temp = 110;
					break;
				case 0x5:
					temp = 120;
					break;
				case 0x0:
				default:
					temp = 100;
					break;
				}
				ret = M70_ERROR_CODE_OK;
			}
		}
		else
		{ // R7008 S command override First spindle {(6 spindles) difference 50} Second axis R7058
			short ret = 0;
			data_type = T_SHORT;
			if (0 == melGetData(conn, 55, 107008 + 50 * (system_no - 1), 0, 0, &data_type, &ret)) // R7008
			{
				temp = ret;
				ret = M70_ERROR_CODE_OK;
			}
		}
	}

	*spindle_override = temp;

	return ret;
}

m70_error_code_e m70_cnc_read_spindle_load(m70_conn_t* conn, short system_no, int32* load, uint32 axis_index, bool is_abs)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	*load = 0;
	axis_index = get_axis_real_no(axis_index);
	long data = 0;
	m70_data_type_e data_type = T_DLONG;
	if (0 == melGetData(conn, 63, 4, system_no, axis_index, &data_type, &data))
	{
		*load = is_abs ? abs(data) : data;
		ret = M70_ERROR_CODE_OK;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_feed_speed(m70_conn_t* conn, short system_no, double* speed, feed_speed_type_e type)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	*speed = 0;

	int section = 42;
	int subSection = 1;
	switch (type)
	{
	case FA:
		section = 42;
		subSection = 1;
		break;

	case FM:
		section = 42;
		subSection = 2;
		break;

	case FS:
		section = 42;
		subSection = 3;
		break;

	case FE:
		section = 42;
		subSection = 4;
		break;

	default:
	case FC:
		section = 33;
		subSection = 1;
		break;
	}
	float_bin_data temp = { 0 };
	m70_data_type_e data_type = T_FLOATBIN;
	if (0 == melGetData(conn, section, subSection, system_no, 0, &data_type, &temp))
	{
		*speed = temp.data;
		ret = M70_ERROR_CODE_OK;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_feed_override(m70_conn_t* conn, short system_no, short* free_override)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	short temp_override = 0;
	// YC67 Cutting feed override value setting method First system {(4 systems) difference 320} Second axis YDA7
	byte bType = 0;
	m70_data_type_e data_type = T_CHAR;
	if (0 == melGetData(conn, 53, 13175 + 320 * (system_no - 1), 0, 0, &data_type, &bType)) // YC67
	{
		if (bType == 0)
		{
			data_type = T_CHAR;
			// YC60 Cutting feed override code 1 First system {(4 systems) difference 320} Second axis YDA0
			if (0 == melGetData(conn, 54, 13168 + 320 * (system_no - 1), 0, 0, &data_type, &bType)) // YC60
			{
				temp_override = (0x0F - (short)(bType & 0x0F)) * 10;
				ret = M70_ERROR_CODE_OK;
			}
		}
		else
		{ // R2500 第1切削进给倍率 第1系统 {(4个系统） 相差 200} 第二轴 R2700
			short ret = 0;
			data_type = T_SHORT;
			if (0 == melGetData(conn, 55, 102500 + 200 * (system_no - 1), 0, 0, &data_type, &ret)) // R2500
			{
				temp_override = ret;
				ret = M70_ERROR_CODE_OK;
			}
		}
	}

	*free_override = temp_override;
	return ret;
}

m70_error_code_e m70_cnc_read_power_on_time(m70_conn_t* conn, uint32* time)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	*time = 0;
	uint32 data = 0;
	m70_data_type_e data_type = T_UINT32;
	if (0 == melGetData(conn, 40, 1, 0, 0, &data_type, &data))
	{
		*time = data;
		ret = M70_ERROR_CODE_OK;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_auto_operation_time(m70_conn_t* conn, uint32* time)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	*time = 0;
	uint32 data = 0;
	m70_data_type_e data_type = T_UINT32;
	if (0 == melGetData(conn, 40, 2, 0, 0, &data_type, &data))
	{
		*time = data;
		ret = M70_ERROR_CODE_OK;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_auto_startup_time(m70_conn_t* conn, uint32* time)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	*time = 0;
	uint32 data = 0;
	m70_data_type_e data_type = T_UINT32;
	if (0 == melGetData(conn, 40, 3, 0, 0, &data_type, &data))
	{
		*time = data;
		ret = M70_ERROR_CODE_OK;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_cycle_time(m70_conn_t* conn, uint32* time)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	*time = 0;
	uint32 data = 0;
	m70_data_type_e data_type = T_UINT32;
	if (0 == melGetData(conn, 40, 8, 0, 0, &data_type, &data))
	{
		*time = data;
		ret = M70_ERROR_CODE_OK;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_external_accumulative_time(m70_conn_t* conn, uint32* time1, uint32* time2)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	*time1 = 0;
	*time2 = 0;
	uint32 tempTime1 = 0, tempTime2 = 0;
	m70_data_type_e data_type = T_UINT32;
	if (0 == melGetData(conn, 40, 4, 0, 0, &data_type, &tempTime1))
	{
		data_type = T_UINT32;
		if (0 == melGetData(conn, 40, 5, 0, 0, &data_type, &tempTime2))
		{
			*time1 = tempTime1;
			*time2 = tempTime2;
			ret = M70_ERROR_CODE_OK;
		}
	}

	return ret;
}

m70_error_code_e m70_cnc_read_cutting_time(m70_conn_t* conn, uint32* time)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	*time = 0;
	uint32 data = 0;
	m70_data_type_e data_type = T_UINT32;
	if (0 == melGetData(conn, 40, 100, 0, 0, &data_type, &data))
	{
		*time = data;
		ret = M70_ERROR_CODE_OK;
	}

	return ret;
}

m70_error_code_e m70_cnc_read_system_datetime(m70_conn_t* conn, uint32* date, uint32* time)
{
	m70_error_code_e ret = M70_ERROR_CODE_FAILED;
	if (!check_conn_is_valid(conn))
		return ret;

	*date = 0;
	*time = 0;
	uint32 tempTime1 = 0, tempTime2 = 0;
	m70_data_type_e data_type = T_UINT32;
	if (0 == melGetData(conn, 40, 6, 0, 0, &data_type, &tempTime1))
	{
		data_type = T_UINT32;
		if (0 == melGetData(conn, 40, 7, 0, 0, &data_type, &tempTime2))
		{
			*date = tempTime1;
			*time = tempTime2;
			ret = M70_ERROR_CODE_OK;
		}
	}

	return ret;
}


uint32 get_axis_real_no(uint32 axis_index)
{
	uint32 axis_flag = 1;
	switch (axis_index)
	{
	case 1:
		axis_flag = 0x01;
		break;
	case 2:
		axis_flag = 0x02;
		break;
	case 3:
		axis_flag = 0x04;
		break;
	case 4:
		axis_flag = 0x08;
		break;
	case 5:
		axis_flag = 0x10;
		break;
	case 6:
		axis_flag = 0x20;
		break;
	case 7:
		axis_flag = 0x40;
		break;
	case 8:
		axis_flag = 0x80;
		break;
	}

	return axis_flag;
}