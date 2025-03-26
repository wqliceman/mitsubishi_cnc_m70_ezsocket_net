/**
 * 日志和错误处理系统使用示例
 * 本文件展示了如何在项目中正确使用日志和错误处理系统
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "m70_log.h"
#include "m70_error.h"
#include "m70_ezsocket.h"

/**
 * 初始化日志系统示例
 */
void log_system_init_example(void)
{
    // 创建日志配置
    m70_log_config_t log_config;
    memset(&log_config, 0, sizeof(m70_log_config_t));
    
    // 设置日志级别 - 开发时可使用DEBUG级别，生产环境可使用INFO或更高级别
    log_config.level = M70_LOG_LEVEL_DEBUG;
    
    // 设置日志输出目标 - 可以是控制台、文件或两者都有
    log_config.target = M70_LOG_TARGET_ALL;
    
    // 设置日志文件路径
    strcpy(log_config.log_file_path, "./logs/mitsubishi_cnc.log");
    
    // Set log format options
    log_config.include_timestamp = true;   // Include timestamp
    log_config.include_level = true;       // Include log level
    log_config.include_file_line = true;   // Include filename and line number
    
    // Set log file rotation options
    log_config.max_file_size = 10240;      // Maximum file size 10MB
    log_config.max_file_count = 5;         // Keep 5 historical log files
    
    // Initialize the log system
    if (!m70_log_init(&log_config)) {
        fprintf(stderr, "日志系统初始化失败\n");
        exit(1);
    }
    
    // Output initialization success log
    M70_LOG_INFO("Log system initialized successfully");
}

/**
 * 错误处理示例函数
 */
m70_error_code_e error_handling_example(void)
{
    // Simulate an error situation
    M70_LOG_DEBUG("Starting to execute an operation that might fail");
    
    // Use M70_ERROR_SET to set error information
    M70_ERROR_SET(M70_ERROR_CODE_EX_FILE_NOT_FOUND, "Cannot find configuration file: %s", "config.ini");
    
    // 获取最后一次错误信息
    const m70_error_info_t* error_info = m70_error_get_last();
    
    // Format error information and output to log
    char error_message[512] = {0};
    m70_error_format(error_message, sizeof(error_message), error_info);
    M70_LOG_ERROR("Error occurred: %s", error_message);
    
    // Use M70_ERROR_RETURN macro to return error code
    M70_ERROR_RETURN(M70_ERROR_CODE_EX_FILE_NOT_FOUND, "Cannot find configuration file: %s", "config.ini");
    
    // 此处代码不会执行
    return M70_ERROR_CODE_OK;
}

/**
 * 网络操作错误处理示例
 */
bool network_error_handling_example(void)
{
    m70_conn_t conn;
    
    // Try to connect to a non-existent address, which will trigger an error
    M70_LOG_INFO("Attempting to connect to a non-existent address");
    if (!m70_cnc_connect("192.168.1.250", 8000, EZNC_SYS_MELDAS700M, &conn)) {
        // 获取并记录错误信息
        const m70_error_info_t* error_info = m70_error_get_last();
        char error_message[512] = {0};
        m70_error_format(error_message, sizeof(error_message), error_info);
        
        M70_LOG_ERROR("连接失败: %s", error_message);
        return false;
    }
    
    // 如果连接成功，断开连接
    m70_cnc_disconnect(&conn);
    return true;
}

/**
 * 不同日志级别使用示例
 */
void log_levels_example(void)
{
    // Debug information - Detailed program running information, usually only enabled in development environment
    M70_LOG_DEBUG("This is a debug message, containing detailed program running data: value=%d", 42);
    
    // General information - Normal program running information
    M70_LOG_INFO("This is a general message, indicating the program is running normally");
    
    // Warning information - Possible issues, but does not affect program continuation
    M70_LOG_WARNING("This is a warning message, indicating there might be a problem");
    
    // Error information - An error occurred, but the program can continue running
    M70_LOG_ERROR("This is an error message, indicating an error has occurred");
    
    // Fatal error - Serious error, program cannot continue running
    M70_LOG_FATAL("This is a fatal error message, the program will terminate");
}

/**
 * 主函数 - 演示日志和错误处理系统的使用
 */
int main(void)
{
    // Initialize the log system
    log_system_init_example();
    
    // 演示不同日志级别的使用
    log_levels_example();
    
    // 演示错误处理
    m70_error_code_e error_code = error_handling_example();
    if (error_code != M70_ERROR_CODE_OK) {
        M70_LOG_WARNING("错误处理示例返回错误码: %d", error_code);
    }
    
    // 演示网络错误处理
    if (!network_error_handling_example()) {
        M70_LOG_WARNING("网络错误处理示例失败");
    }
    
    // Close log system
    M70_LOG_INFO("Example program ended, closing log system");
    m70_log_shutdown();
    
    return 0;
}