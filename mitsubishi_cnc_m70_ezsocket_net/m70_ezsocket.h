#ifndef __H_M70_EZSOCKET_H__
#define __H_M70_EZSOCKET_H__

#include "typedef.h"

bool m70_cnc_connect(const char* ip_addr, int port, m70_nc_type_e type, m70_conn_t* conn);
void m70_cnc_disconnect(m70_conn_t* conn);

// read
m70_error_code_e m70_cnc_read_status(m70_conn_t* conn, short system_no, m70_device_status_e* status, m70_run_mode_e* mode, m70_run_status_e* run_status);
m70_error_code_e m70_cnc_read_counter(m70_conn_t* conn, short system_no, uint32* status);
m70_error_code_e m70_cnc_read_system_count(m70_conn_t* conn, uint32* count);
m70_error_code_e m70_cnc_read_nc_axis_count(m70_conn_t* conn, uint32* count);
m70_error_code_e m70_cnc_read_all_axis_count(m70_conn_t* conn, uint32* count);
m70_error_code_e m70_cnc_read_spindle_axis_count(m70_conn_t* conn, uint32* count);
m70_error_code_e m70_cnc_read_plc_axis_count(m70_conn_t* conn, uint32* count);
m70_error_code_e m70_cnc_read_nc_type(m70_conn_t* conn, m70_nc_machine_type_e* type);
m70_error_code_e m70_cnc_read_nc_version(m70_conn_t* conn, char* version);
m70_error_code_e m70_cnc_read_nc_name_version(m70_conn_t* conn, char* version);
m70_error_code_e m70_cnc_read_plc_version(m70_conn_t* conn, char* version);
m70_error_code_e m70_cnc_read_main_program_name(m70_conn_t* conn, short system_no, program_name_type_e type, char* prog);
m70_error_code_e m70_cnc_read_sub_program_name(m70_conn_t* conn, short system_no, program_name_type_e type, char* prog);
m70_error_code_e m70_cnc_read_program_file_info(m70_conn_t* conn, short system_no, m70_file_info_type_e type, int* numbers);
m70_error_code_e m70_cnc_read_program_block(m70_conn_t* conn, short system_no, int row_count, prog_block* block);
m70_error_code_e m70_cnc_read_alarm(m70_conn_t* conn, short system_no, int msg_count, alarm_message_type_e type, alarm_string* alarms);
m70_error_code_e m70_cnc_read_is_alarm(m70_conn_t* conn, short system_no, bool* alarm);
m70_error_code_e m70_cnc_read_current_tool_no(m70_conn_t* conn, short system_no, uint32* tool_no);
m70_error_code_e m70_cnc_read_svo_load(m70_conn_t* conn, short system_no, short* svo_load, uint32 axis_index, bool is_abs);
m70_error_code_e m70_cnc_read_axis_position(m70_conn_t* conn, short system_no, double* pos, uint32 axis_index, position_type_e type);
m70_error_code_e m70_cnc_read_all_axis_position(m70_conn_t* conn, short system_no, double* pos, int* pos_count, position_type_e type);
m70_error_code_e m70_cnc_read_axis_name(m70_conn_t* conn, short system_no, char* names, int* axis_count);
m70_error_code_e m70_cnc_read_spindle_speed(m70_conn_t* conn, short system_no, uint32* speed, uint32 axis_index);
m70_error_code_e m70_cnc_read_spindle_override(m70_conn_t* conn, short system_no, short* speed);
m70_error_code_e m70_cnc_read_spindle_load(m70_conn_t* conn, short system_no, int32* load, uint32 axis_index, bool is_abs);
m70_error_code_e m70_cnc_read_feed_speed(m70_conn_t* conn, short system_no, double* speed, feed_speed_type_e type);
m70_error_code_e m70_cnc_read_feed_override(m70_conn_t* conn, short system_no, short* free_override);
m70_error_code_e m70_cnc_read_power_on_time(m70_conn_t* conn, uint32* time);
m70_error_code_e m70_cnc_read_auto_operation_time(m70_conn_t* conn, uint32* time);
m70_error_code_e m70_cnc_read_auto_startup_time(m70_conn_t* conn, uint32* time);
m70_error_code_e m70_cnc_read_cycle_time(m70_conn_t* conn, uint32* time);
m70_error_code_e m70_cnc_read_external_accumulative_time(m70_conn_t* conn, uint32* time1, uint32* time2);
m70_error_code_e m70_cnc_read_cutting_time(m70_conn_t* conn, uint32* time);
m70_error_code_e m70_cnc_read_system_datetime(m70_conn_t* conn, uint32* date, uint32* time);

#endif // __H_M70_EZSOCKET_H__