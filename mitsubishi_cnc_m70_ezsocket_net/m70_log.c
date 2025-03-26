#include "m70_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#endif

// Global configuration for log module
static struct {
    bool initialized;                // Whether initialized
    m70_log_level_e level;           // Log level
    m70_log_target_e target;         // Log output target
    char log_file_path[256];         // Log file path
    FILE* log_file;                  // Log file handle
    bool include_timestamp;          // Whether to include timestamp
    bool include_level;              // Whether to include log level
    bool include_file_line;          // Whether to include file and line number
    unsigned int max_file_size;      // Maximum log file size (KB)
    unsigned int max_file_count;     // Maximum number of log files
    unsigned int current_file_size;  // Current log file size (KB)
} g_log_config = {
    false,                  // Not initialized
    M70_LOG_LEVEL_INFO,    // Default INFO level
    M70_LOG_TARGET_CONSOLE,// Default console output
    "",                    // Default log file path is empty
    NULL,                  // Log file handle is NULL
    true,                  // Default include timestamp
    true,                  // Default include log level
    false,                 // Default don't include file and line number
    10240,                 // Default maximum file size 10MB
    5,                     // Default maximum file count 5
    0                      // Current file size 0KB
};

// Log level strings
static const char* g_log_level_str[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "FATAL"
};

// Get current time string
static void get_current_time(char* buffer, size_t buffer_size) {
    time_t now;
    struct tm timeinfo;
    
    time(&now);
    
#ifdef _WIN32
    localtime_s(&timeinfo, &now);
#else
    localtime_r(&now, &timeinfo);
#endif
    
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", &timeinfo);
}

// Check and create log directory
static bool ensure_log_directory(const char* file_path) {
    char dir_path[256] = {0};
    char* last_slash = NULL;
    
    strncpy(dir_path, file_path, sizeof(dir_path) - 1);
    
    // Find the last path separator
#ifdef _WIN32
    last_slash = strrchr(dir_path, '\\');
    if(last_slash == NULL)
    {
        last_slash = strrchr(dir_path, '/');
    }
#else
    last_slash = strrchr(dir_path, '/');
#endif
    
    if (last_slash) {
        *last_slash = '\0';  // Truncate string, keep only the directory part
        
        // Create directory
#ifdef _WIN32
        return (_mkdir(dir_path) == 0 || GetLastError() == ERROR_ALREADY_EXISTS);
#else
        return (mkdir(dir_path, 0755) == 0 || errno == EEXIST);
#endif
    }
    
    return true;  // No directory part, return success directly
}

// Log file rotation
static void rotate_log_file_if_needed() {
    if (!g_log_config.log_file || g_log_config.current_file_size < g_log_config.max_file_size) {
        return;
    }
    
    // Close current log file
    fclose(g_log_config.log_file);
    g_log_config.log_file = NULL;
    g_log_config.current_file_size = 0;
    
    // Perform log file rotation
    char base_path[256] = {0};
    char old_path[256] = {0};
    char new_path[256] = {0};
    
    strncpy(base_path, g_log_config.log_file_path, sizeof(base_path) - 1);
    
    // Delete the oldest log file
    snprintf(old_path, sizeof(old_path), "%s.%d", base_path, g_log_config.max_file_count);
    remove(old_path);
    
    // Rename existing log files
    for (int i = g_log_config.max_file_count - 1; i >= 1; i--) {
        snprintf(old_path, sizeof(old_path), "%s.%d", base_path, i);
        snprintf(new_path, sizeof(new_path), "%s.%d", base_path, i + 1);
        rename(old_path, new_path);
    }
    
    // Rename current log file
    snprintf(new_path, sizeof(new_path), "%s.1", base_path);
    rename(base_path, new_path);
    
    // Reopen log file
    g_log_config.log_file = fopen(g_log_config.log_file_path, "a");
}

// Log initialization
bool m70_log_init(const m70_log_config_t* config) {
    if (g_log_config.initialized) {
        m70_log_shutdown();  // If already initialized, shut down first
    }
    
    if (!config) {
        // Use default configuration
        g_log_config.initialized = true;
        return true;
    }
    
    // Apply configuration
    g_log_config.level = config->level;
    g_log_config.target = config->target;
    g_log_config.include_timestamp = config->include_timestamp;
    g_log_config.include_level = config->include_level;
    g_log_config.include_file_line = config->include_file_line;
    g_log_config.max_file_size = config->max_file_size;
    g_log_config.max_file_count = config->max_file_count;
    
    // If file output is needed, open log file
    if (g_log_config.target & M70_LOG_TARGET_FILE) {
        if (config->log_file_path[0] != '\0') {
            strncpy(g_log_config.log_file_path, config->log_file_path, sizeof(g_log_config.log_file_path) - 1);
            
            // Ensure log directory exists
            if (!ensure_log_directory(g_log_config.log_file_path)) {
                fprintf(stderr, "Failed to create log directory for %s\n", g_log_config.log_file_path);
                return false;
            }
            
            // Open log file
            g_log_config.log_file = fopen(g_log_config.log_file_path, "a");
            if (!g_log_config.log_file) {
                fprintf(stderr, "Failed to open log file: %s\n", g_log_config.log_file_path);
                return false;
            }
            
            // Get current file size
            fseek(g_log_config.log_file, 0, SEEK_END);
            g_log_config.current_file_size = ftell(g_log_config.log_file) / 1024;
        } else {
            fprintf(stderr, "Log file path is empty\n");
            return false;
        }
    }
    
    g_log_config.initialized = true;
    
    // Output initialization log
    m70_log_info("Log system initialization completed, level: %s, target: %d", 
                g_log_level_str[g_log_config.level], 
                g_log_config.target);
    
    return true;
}

// Log shutdown
void m70_log_shutdown(void) {
    if (!g_log_config.initialized) {
        return;
    }
    
    if (g_log_config.log_file) {
        m70_log_info("Log system shutdown");
        fclose(g_log_config.log_file);
        g_log_config.log_file = NULL;
    }
    
    g_log_config.initialized = false;
}

// Set log level
void m70_log_set_level(m70_log_level_e level) {
    if (level >= M70_LOG_LEVEL_DEBUG && level <= M70_LOG_LEVEL_OFF) {
        g_log_config.level = level;
        if (g_log_config.initialized) {
            m70_log_info("Log level has been set to: %s", level < M70_LOG_LEVEL_OFF ? g_log_level_str[level] : "OFF");
        }
    }
}

// Set log output target
void m70_log_set_target(m70_log_target_e target) {
    g_log_config.target = target;
    
    // If file output is needed but file is not open, try to open it
    if ((target & M70_LOG_TARGET_FILE) && !g_log_config.log_file && g_log_config.log_file_path[0] != '\0') {
        g_log_config.log_file = fopen(g_log_config.log_file_path, "a");
        if (g_log_config.log_file) {
            // Get current file size
            fseek(g_log_config.log_file, 0, SEEK_END);
            g_log_config.current_file_size = ftell(g_log_config.log_file) / 1024;
        }
    }
    
    // If file output is not needed but file is open, close the file
    if (!(target & M70_LOG_TARGET_FILE) && g_log_config.log_file) {
        fclose(g_log_config.log_file);
        g_log_config.log_file = NULL;
    }
    
    if (g_log_config.initialized) {
        m70_log_info("Log output target has been set to: %d", target);
    }
}

// Set log file path
bool m70_log_set_file(const char* file_path) {
    if (!file_path || file_path[0] == '\0') {
        return false;
    }
    
    // Close current log file
    if (g_log_config.log_file) {
        fclose(g_log_config.log_file);
        g_log_config.log_file = NULL;
    }
    
    // Set new log file path
    strncpy(g_log_config.log_file_path, file_path, sizeof(g_log_config.log_file_path) - 1);
    
    // Ensure log directory exists
    if (!ensure_log_directory(g_log_config.log_file_path)) {
        fprintf(stderr, "Failed to create log directory for %s\n", g_log_config.log_file_path);
        return false;
    }
    
    // If file output is needed, open new log file
    if (g_log_config.target & M70_LOG_TARGET_FILE) {
        g_log_config.log_file = fopen(g_log_config.log_file_path, "a");
        if (!g_log_config.log_file) {
            fprintf(stderr, "Failed to open log file: %s\n", g_log_config.log_file_path);
            return false;
        }
        
        // Get current file size
        fseek(g_log_config.log_file, 0, SEEK_END);
        g_log_config.current_file_size = ftell(g_log_config.log_file) / 1024;
    }
    
    if (g_log_config.initialized) {
        m70_log_info("Log file has been set to: %s", g_log_config.log_file_path);
    }
    
    return true;
}

// Internal log write function
static void log_write(m70_log_level_e level, const char* file, int line, const char* format, va_list args) {
    if (!g_log_config.initialized || level < g_log_config.level || level >= M70_LOG_LEVEL_OFF) {
        return;
    }
    
    char time_str[32] = {0};
    char message[1024] = {0};
    char full_message[1280] = {0};
    
    // Format log message
    vsnprintf(message, sizeof(message) - 1, format, args);
    
    // Get current time
    if (g_log_config.include_timestamp) {
        get_current_time(time_str, sizeof(time_str));
    }
    
    // Build complete log message
    if (g_log_config.include_timestamp && g_log_config.include_level && g_log_config.include_file_line && file) {
        snprintf(full_message, sizeof(full_message), "[%s] [%s] [%s:%d] %s\n", 
                time_str, g_log_level_str[level], file, line, message);
    } else if (g_log_config.include_timestamp && g_log_config.include_level) {
        snprintf(full_message, sizeof(full_message), "[%s] [%s] %s\n", 
                time_str, g_log_level_str[level], message);
    } else if (g_log_config.include_timestamp) {
        snprintf(full_message, sizeof(full_message), "[%s] %s\n", time_str, message);
    } else if (g_log_config.include_level) {
        snprintf(full_message, sizeof(full_message), "[%s] %s\n", g_log_level_str[level], message);
    } else {
        snprintf(full_message, sizeof(full_message), "%s\n", message);
    }
    
    // Output to console
    if (g_log_config.target & M70_LOG_TARGET_CONSOLE) {
#ifdef _WIN32
        HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO console_info;
        WORD original_attributes = 0;
        
        // Save original console attributes
        GetConsoleScreenBufferInfo(console_handle, &console_info);
        original_attributes = console_info.wAttributes;
        
        // Set color according to log level
        switch (level) {
            case M70_LOG_LEVEL_DEBUG:
                SetConsoleTextAttribute(console_handle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Bright white
                break;
            case M70_LOG_LEVEL_INFO:
                SetConsoleTextAttribute(console_handle, FOREGROUND_INTENSITY | FOREGROUND_GREEN); // Bright green
                break;
            case M70_LOG_LEVEL_WARNING:
                SetConsoleTextAttribute(console_handle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN); // Bright yellow
                break;
            case M70_LOG_LEVEL_ERROR:
                SetConsoleTextAttribute(console_handle, FOREGROUND_INTENSITY | FOREGROUND_RED); // Bright red
                break;
            case M70_LOG_LEVEL_FATAL:
                SetConsoleTextAttribute(console_handle, BACKGROUND_RED | FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // White text on red background
                break;
            default:
                break;
        }
        
        // Output log
        printf("%s", full_message);
        
        // Restore original console attributes
        SetConsoleTextAttribute(console_handle, original_attributes);
#else
        // Set color according to log level
        switch (level) {
            case M70_LOG_LEVEL_DEBUG:
                printf("\033[37m%s\033[0m", full_message); // White
                break;
            case M70_LOG_LEVEL_INFO:
                printf("\033[32m%s\033[0m", full_message); // Green
                break;
            case M70_LOG_LEVEL_WARNING:
                printf("\033[33m%s\033[0m", full_message); // Yellow
                break;
            case M70_LOG_LEVEL_ERROR:
                printf("\033[31m%s\033[0m", full_message); // Red
                break;
            case M70_LOG_LEVEL_FATAL:
                printf("\033[41;37m%s\033[0m", full_message); // White text on red background
                break;
            default:
                printf("%s", full_message);
                break;
        }
#endif
    }
    
    // Output to file
    if ((g_log_config.target & M70_LOG_TARGET_FILE) && g_log_config.log_file) {
        // Check if log file rotation is needed
        rotate_log_file_if_needed();
        
        // Write log
        if (g_log_config.log_file) {
            size_t written = fprintf(g_log_config.log_file, "%s", full_message);
            fflush(g_log_config.log_file);
            
            // Update current file size
            g_log_config.current_file_size += written / 1024;
        }
    }
}

// Log output function implementation
void m70_log_debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_write(M70_LOG_LEVEL_DEBUG, NULL, 0, format, args);
    va_end(args);
}

void m70_log_info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_write(M70_LOG_LEVEL_INFO, NULL, 0, format, args);
    va_end(args);
}

void m70_log_warning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_write(M70_LOG_LEVEL_WARNING, NULL, 0, format, args);
    va_end(args);
}

void m70_log_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_write(M70_LOG_LEVEL_ERROR, NULL, 0, format, args);
    va_end(args);
}

void m70_log_fatal(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_write(M70_LOG_LEVEL_FATAL, NULL, 0, format, args);
    va_end(args);
}

// Implementation of log output function with file and line number
void m70_log_debug_ex(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_write(M70_LOG_LEVEL_DEBUG, file, line, format, args);
    va_end(args);
}

void m70_log_info_ex(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_write(M70_LOG_LEVEL_INFO, file, line, format, args);
    va_end(args);
}

void m70_log_warning_ex(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_write(M70_LOG_LEVEL_WARNING, file, line, format, args);
    va_end(args);
}

void m70_log_error_ex(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_write(M70_LOG_LEVEL_ERROR, file, line, format, args);
    va_end(args);
}

void m70_log_fatal_ex(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_write(M70_LOG_LEVEL_FATAL, file, line, format, args);
    va_end(args);
}