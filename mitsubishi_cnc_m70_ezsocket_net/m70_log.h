#ifndef __H_M70_LOG_H__
#define __H_M70_LOG_H__

#include <stdbool.h>

// Log level definition
typedef enum _tag_m70_log_level
{
    M70_LOG_LEVEL_DEBUG = 0,    // Debug information
    M70_LOG_LEVEL_INFO = 1,     // General information
    M70_LOG_LEVEL_WARNING = 2,  // Warning information
    M70_LOG_LEVEL_ERROR = 3,    // Error information
    M70_LOG_LEVEL_FATAL = 4,    // Fatal error
    M70_LOG_LEVEL_OFF = 5       // Turn off logging
} m70_log_level_e;

// Log output target
typedef enum _tag_m70_log_target
{
    M70_LOG_TARGET_CONSOLE = 0x01,  // Console output
    M70_LOG_TARGET_FILE = 0x02,      // File output
    M70_LOG_TARGET_ALL = 0x03        // All output targets
} m70_log_target_e;

// Log configuration structure
typedef struct _tag_m70_log_config
{
    m70_log_level_e level;           // Log level
    m70_log_target_e target;          // Log output target
    char log_file_path[256];          // Log file path
    bool include_timestamp;           // Whether to include timestamp
    bool include_level;               // Whether to include log level
    bool include_file_line;           // Whether to include file and line number
    unsigned int max_file_size;       // Maximum log file size (KB)
    unsigned int max_file_count;      // Maximum number of log files
} m70_log_config_t;

// Log initialization and configuration functions
bool m70_log_init(const m70_log_config_t* config);
void m70_log_shutdown(void);
void m70_log_set_level(m70_log_level_e level);
void m70_log_set_target(m70_log_target_e target);
bool m70_log_set_file(const char* file_path);

// Log output functions
void m70_log_debug(const char* format, ...);
void m70_log_info(const char* format, ...);
void m70_log_warning(const char* format, ...);
void m70_log_error(const char* format, ...);
void m70_log_fatal(const char* format, ...);

// Log output macros with file and line number
#define M70_LOG_DEBUG(format, ...) m70_log_debug_ex(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define M70_LOG_INFO(format, ...) m70_log_info_ex(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define M70_LOG_WARNING(format, ...) m70_log_warning_ex(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define M70_LOG_ERROR(format, ...) m70_log_error_ex(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define M70_LOG_FATAL(format, ...) m70_log_fatal_ex(__FILE__, __LINE__, format, ##__VA_ARGS__)

// Log output functions with file and line number
void m70_log_debug_ex(const char* file, int line, const char* format, ...);
void m70_log_info_ex(const char* file, int line, const char* format, ...);
void m70_log_warning_ex(const char* file, int line, const char* format, ...);
void m70_log_error_ex(const char* file, int line, const char* format, ...);
void m70_log_fatal_ex(const char* file, int line, const char* format, ...);

#endif // __H_M70_LOG_H__