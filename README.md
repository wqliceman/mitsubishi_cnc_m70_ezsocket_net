# Mitsubishi CNC M70 Ethernet Communication Library [中文说明](./README_zh.md)

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

## Overview

This project provides a C library for communicating with Mitsubishi CNC M70 series machines over Ethernet using the EZSocket protocol. It enables developers to easily interact with CNC machines for data collection, monitoring, and control purposes.

## Features

- **Cross-platform**: Supports both Windows and Linux operating systems.
- **Protocol Implementation**: Implements the EZSocket protocol for seamless communication.
- **Data Collection**: Provides APIs to read various machine data such as status, axis positions, spindle speed, and more.
- **Extensible**: Easily extendable to support additional CNC models and data points.

## Getting Started

### Prerequisites

- **Ethernet Configuration**: Ensure the Ethernet module on the Mitsubishi CNC M70 machine is properly configured.
- **Development Environment**: A C compiler (e.g., GCC, MSVC) and necessary libraries for socket programming.

### Installation

1. Clone the repository:

```bash
git clone https://github.com/wqliceman/mitsubishi_cnc_m70_ezsocket_net.git
```
2. Compile the project:

```bash
cd mitsubishi_cnc_m70_ezsocket_net
make
```
3. Usage Example:
```bash
./mitsubishi_cnc_m70_ezsocket_net
```

# Program Overview

- Project Name: mitsubishi_cnc_m70_net
- Development Language: C
- Supported Operating Systems: Windows/Linux
- Test Device: Mitsubishi M70

Currently implemented functionality includes a communication class for Mitsubishi CNC M70, implemented using the (ezsocket) protocol. The Ethernet module on the machine side needs to be configured first.

## Header File

```c
#include "m70_ezsocket.h" //Protocol method interface
```

## Connection Instructions

### Connection Properties

- port: Port number,
- type: CNC model, such as CNC M700 series/CNC M700V series/CNC M70 series/CNC M70V series/CNC E70 series/CNC C70 series/CNC M800 series/CNC M80 series, etc.

### Supported Data Collection

Preliminary data collection can be seen in the API below. For more data, you need to query the relevant address partition table.

## Implementation Methods

### 1. Connect to PLC Device

```c
bool m70_cnc_connect(const char* ip_addr, int port, m70_nc_type_e type, m70_conn_t* conn);
void m70_cnc_disconnect(m70_conn_t* conn);
```

### 2. Data Reading

```c
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
```

#### 3. Data Writing

xxxx

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Usage Example

For a complete example, see the **main.c** file in the code. The main code and usage methods are provided below:

```c
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

        printf("----------------- test common variable --------------\n");
        double write_data = i;
        double read_data = 0;
        char write_comment[10] = {0};
        char read_comment[40] = {0};
        uint32 index = 500;
        ret = m70_cnc_write_common_variable(&conn, 1, index, write_data);
        GET_RESULT(ret);
        ret = m70_cnc_read_common_variable(&conn, 1, index, &read_data);
        GET_RESULT(ret);
        printf("test value common variable [%d], write:[%lf], read: [%lf]\n", index, write_data, read_data);

        sprintf(write_comment, "M%d", i);
        ret = m70_cnc_write_common_variable_comment(&conn, index, write_comment);
        GET_RESULT(ret);
        ret = m70_cnc_read_common_variable_comment(&conn, index, read_comment);
        printf("test comment common variable [%d], write:[%s], read: [%s]\n", index, write_comment, read_comment);

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