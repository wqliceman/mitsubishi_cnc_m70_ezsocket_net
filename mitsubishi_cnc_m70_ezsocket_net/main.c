#ifdef _WIN32
#include <WinSock2.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "m70_log.h"
#include "m70_error.h"
#pragma warning(disable : 4996)

#define GET_RESULT(ret)     \
    {                       \
        if (ret != 0)       \
            failed_count++; \
    }

// 1. Define constant configurations
#define DEFAULT_IP "192.168.123.130"
#define DEFAULT_PORT 683
#define TEST_COUNT 5000
#define TEST_SLEEP_TIME 2
#define MAX_VERSION_LEN 128
#define MAX_PROG_NAME_LEN 64

#include "m70_ezsocket.h"

void log_system_init_example(void);


// 2. Extract function for reading basic information
static m70_error_code_e read_basic_info(m70_conn_t *conn)
{
    m70_error_code_e ret;
    int failed_count = 0;

    // Read CNC version information
    char nc_version[MAX_VERSION_LEN] = {0};
    ret = m70_cnc_read_nc_version(conn, nc_version);
    GET_RESULT(ret);
    printf("cnc version: %s\n", nc_version);

    char nc_name_version[MAX_VERSION_LEN] = {0};
    ret = m70_cnc_read_nc_name_version(conn, nc_name_version);
    GET_RESULT(ret);
    printf("cnc name version: %s\n", nc_name_version);

    char nc_plc_version[MAX_VERSION_LEN] = {0};
    ret = m70_cnc_read_plc_version(conn, nc_plc_version);
    GET_RESULT(ret);
    printf("cnc plc version: %s\n", nc_plc_version);

    // Read machine type and system information
    m70_nc_machine_type_e type = MACHINE_TYPE_MC;
    ret = m70_cnc_read_nc_type(conn, &type);
    GET_RESULT(ret);
    printf("nc type: %d\n", type);

    uint32 sys_count = 0;
    ret = m70_cnc_read_system_count(conn, &sys_count);
    GET_RESULT(ret);
    printf("system count : %d\n", sys_count);

    // Read axis count information
    uint32 nc_axis_count = 0;
    ret = m70_cnc_read_nc_axis_count(conn, &nc_axis_count);
    GET_RESULT(ret);
    printf("nc axis count : %d\n", nc_axis_count);

    uint32 sp_axis_count = 0;
    ret = m70_cnc_read_spindle_axis_count(conn, &sp_axis_count);
    GET_RESULT(ret);
    printf("nc spindle axis count : %d\n", sp_axis_count);

    uint32 all_axis_count = 0;
    ret = m70_cnc_read_all_axis_count(conn, &all_axis_count);
    GET_RESULT(ret);
    printf("nc all axis count : %d\n", all_axis_count);

    // Read running status
    m70_device_status_e status;
    m70_run_mode_e mode;
    m70_run_status_e run_status;
    ret = m70_cnc_read_status(conn, 1, &status, &mode, &run_status);
    GET_RESULT(ret);
    printf("nc status: %d, mode: %d, run status: %d\n", status, mode, run_status);

    // Read program information
    char main_prog[MAX_PROG_NAME_LEN] = {0};
    ret = m70_cnc_read_main_program_name(conn, 1, PRG_TYPE_ProgramNo, main_prog);
    GET_RESULT(ret);
    printf("main prog: %s\n", main_prog);

    ret = m70_cnc_read_main_program_name(conn, 1, PRG_TYPE_SequenceNumber, main_prog);
    GET_RESULT(ret);
    printf("main prog seq no: %s\n", main_prog);

    char sub_prog[MAX_PROG_NAME_LEN] = {0};
    ret = m70_cnc_read_sub_program_name(conn, 1, PRG_TYPE_ProgramNo, sub_prog);
    GET_RESULT(ret);
    printf("sub prog: %s\n", sub_prog);

    prog_block block = {0};
    ret = m70_cnc_read_program_block(conn, 1, 10, &block);
    GET_RESULT(ret);
    printf("prog block: %d, %s\n", block.block_length, block.text);

    uint32 tool_no = 0;
    ret = m70_cnc_read_current_tool_no(conn, 1, &tool_no);
    GET_RESULT(ret);
    printf("tool no: %d\n", tool_no);

    uint32 counter = 0;
    ret = m70_cnc_read_counter(conn, 1, &counter);
    GET_RESULT(ret);
    printf("counter: %d\n", counter);

    alarm_string alarms = {0};
    ret = m70_cnc_read_alarm(conn, 1, 10, M_ALM_ALL_ALARM, &alarms);
    GET_RESULT(ret);
    printf("alarm: %d\n", alarms.alarm_length);

    short sp_override = 0;
    int32 sp_load = 0;
    uint32 sp_speed = 0;
    short feed_override = 0;
    double feed_speed = 0;
    ret = m70_cnc_read_spindle_override(conn, 1, &sp_override);
    GET_RESULT(ret);
    printf("spindle override: %d\n", sp_override);

    ret = m70_cnc_read_spindle_load(conn, 1, &sp_load, 1, false);
    GET_RESULT(ret);
    printf("spindle load: %d\n", sp_load);

    ret = m70_cnc_read_spindle_speed(conn, 1, &sp_speed, 1);
    GET_RESULT(ret);
    printf("spindle speed: %d\n", sp_speed);

    ret = m70_cnc_read_feed_override(conn, 1, &feed_override);
    GET_RESULT(ret);
    printf("feed override: %d\n", feed_override);

    ret = m70_cnc_read_feed_speed(conn, 1, &feed_speed, FC);
    GET_RESULT(ret);
    printf("feed speed: %lf\n", feed_speed);

    uint32 power_on_time = 0;
    uint32 auto_op_time = 0;
    uint32 auto_startup_time = 0;
    uint32 cycle_time = 0;
    uint32 cutting_time = 0;
    uint32 sys_date = 0;
    uint32 sys_time = 0;

    ret = m70_cnc_read_power_on_time(conn, &power_on_time);
    GET_RESULT(ret);
    printf("power on time: %d\n", power_on_time);

    ret = m70_cnc_read_auto_operation_time(conn, &auto_op_time);
    GET_RESULT(ret);
    printf("auto op time: %d\n", auto_op_time);

    ret = m70_cnc_read_auto_startup_time(conn, &auto_op_time);
    GET_RESULT(ret);
    printf("auto startup time: %d\n", auto_startup_time);

    ret = m70_cnc_read_cycle_time(conn, &cycle_time);
    GET_RESULT(ret);
    printf("cycle time: %d\n", cycle_time);

    ret = m70_cnc_read_cutting_time(conn, &cutting_time);
    GET_RESULT(ret);
    printf("cutting time: %d\n", cutting_time);

    ret = m70_cnc_read_system_datetime(conn, &sys_date, &sys_time);
    GET_RESULT(ret);
    printf("system date time: %d, %d\n", sys_date, sys_time);

    return ret;
}

// 3. Extract function for reading axis information
static m70_error_code_e read_axis_position_info(m70_conn_t *conn)
{
    m70_error_code_e ret;
    int failed_count = 0;

    char axis_names[10] = {0};
    int axis_count = 0;
    ret = m70_cnc_read_axis_name(conn, 1, axis_names, &axis_count);
    GET_RESULT(ret);

    if (ret == 0)
    {
        double positions[10] = {0};
        printf("axis:\t %s\n", axis_names);

        const int pos_types[] = {POS_PROGRAM, POS_RELATV, POS_WRK, POS_MCH, DISTANCE};
        const char *type_names[] = {"POS_PROGRAM", "POS_RELATV", "POS_WRK", "POS_MCH", "DISTANCE"};
        const int type_count = sizeof(pos_types) / sizeof(pos_types[0]);

        for (int i = 0; i < type_count; i++)
        {
            printf("%s:\t", type_names[i]);
            ret = m70_cnc_read_all_axis_position(conn, 1, positions, &axis_count, pos_types[i]);
            GET_RESULT(ret);

            for (int j = 0; j < axis_count; j++)
            {
                printf("%lf\t", positions[j]);
            }
            printf("\n");
        }
    }

    return ret;
}

// 4. Main function optimization
int main(int argc, char **argv)
{
// Initialize network
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        return -1;
    }
#endif

    log_system_init_example();

    // Get connection parameters
    const char *plc_ip = argc > 1 ? argv[1] : DEFAULT_IP;
    int plc_port = argc > 2 ? atoi(argv[2]) : DEFAULT_PORT;

    // Establish connection
    m70_conn_t conn = {0};
    if (!m70_cnc_connect(plc_ip, plc_port, EZNC_SYS_MELDAS700M, &conn) || conn.socket <= 0)
    {
        printf("Failed to connect to PLC\n");
        return -1;
    }

    // Execute test loop
    int failed_count = 0;
    for (int i = 0; i < TEST_COUNT; i++)
    {
        printf("===================== [%d] ===================\n", i);

        // Read basic information
        m70_error_code_e ret = read_basic_info(&conn);
        if (ret != M70_ERROR_CODE_OK)
            failed_count++;

        printf("------------------------ TEST AXIS ----------------------\n");
        // Read axis information
        ret = read_axis_position_info(&conn);
        if (ret != M70_ERROR_CODE_OK)
            failed_count++;

// Test delay
#ifdef _WIN32
        Sleep(TEST_SLEEP_TIME);
#else
        sleep(TEST_SLEEP_TIME);
#endif
    }

    printf("All Failed count: %d\n", failed_count);

    // Clean up resources
    m70_cnc_disconnect(&conn);

#ifdef _WIN32
    WSACleanup();
#endif

    // Close log system
    M70_LOG_INFO("Example program ended, closing log system");
    m70_log_shutdown();

    return 0;
}


/**
 * Log system initialization example
 */
void log_system_init_example(void)
{
    // Create log configuration
    m70_log_config_t log_config;
    memset(&log_config, 0, sizeof(m70_log_config_t));

    // Set log level - DEBUG level can be used during development, INFO or higher level can be used in production environment
    log_config.level = M70_LOG_LEVEL_WARNING;

    // Set log output target - can be console, file, or both
    log_config.target = M70_LOG_TARGET_FILE;

    // Set log file path
    strcpy(log_config.log_file_path, "./logs/mitsubishi_cnc.log");

    // Set log format options
    log_config.include_timestamp = true;   // Include timestamp
    log_config.include_level = true;       // Include log level
    log_config.include_file_line = true;   // Include filename and line number

    // Set log file rotation options
    log_config.max_file_size = 10240;      // Maximum file size 10MB
    log_config.max_file_count = 5;         // Keep 5 historical log files

    // Initialize log system
    if (!m70_log_init(&log_config)) {
        fprintf(stderr, "Log system initialization failed\n");
        exit(1);
    }

    // Output initialization success log
    M70_LOG_INFO("Log system initialized successfully");
}