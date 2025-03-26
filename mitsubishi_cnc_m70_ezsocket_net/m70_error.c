#include "m70_error.h"
#include "m70_log.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <errno.h>
#endif

// Last error information
static m70_error_info_t g_last_error = {0};

// Error code description table
static const struct {
    m70_error_code_ex_e code;
    const char* description;
} g_error_descriptions[] = {
    // Basic error codes
    {M70_ERROR_CODE_EX_OK, "Operation successful"},
    {M70_ERROR_CODE_EX_FAILED, "Operation failed"},
    {M70_ERROR_CODE_EX_SOCKET_FAILED, "Network exception"},
    {M70_ERROR_CODE_EX_UNKNOWN, "Unknown error"},
    
    // Connection related error codes
    {M70_ERROR_CODE_EX_CONN_TIMEOUT, "Connection timeout"},
    {M70_ERROR_CODE_EX_CONN_REFUSED, "Connection refused"},
    {M70_ERROR_CODE_EX_CONN_CLOSED, "Connection closed"},
    {M70_ERROR_CODE_EX_CONN_INVALID, "Invalid connection"},
    {M70_ERROR_CODE_EX_CONN_HOST_NOT_FOUND, "Host not found"},
    
    // Data transmission related error codes
    {M70_ERROR_CODE_EX_TRANS_TIMEOUT, "Data transmission timeout"},
    {M70_ERROR_CODE_EX_TRANS_INCOMPLETE, "Incomplete data transmission"},
    {M70_ERROR_CODE_EX_TRANS_INVALID_DATA, "Invalid data"},
    {M70_ERROR_CODE_EX_TRANS_BUFFER_OVERFLOW, "Buffer overflow"},
    
    // Protocol related error codes
    {M70_ERROR_CODE_EX_PROTO_INVALID_FORMAT, "Invalid protocol format"},
    {M70_ERROR_CODE_EX_PROTO_VERSION_MISMATCH, "Protocol version mismatch"},
    {M70_ERROR_CODE_EX_PROTO_INVALID_COMMAND, "Invalid command"},
    {M70_ERROR_CODE_EX_PROTO_INVALID_RESPONSE, "Invalid response"},
    
    // CNC device related error codes
    {M70_ERROR_CODE_EX_CNC_NOT_READY, "CNC device not ready"},
    {M70_ERROR_CODE_EX_CNC_BUSY, "CNC device busy"},
    {M70_ERROR_CODE_EX_CNC_ALARM, "CNC device alarm"},
    {M70_ERROR_CODE_EX_CNC_INVALID_PARAM, "Invalid parameter"},
    {M70_ERROR_CODE_EX_CNC_INVALID_STATE, "Invalid state"},
    {M70_ERROR_CODE_EX_CNC_ACCESS_DENIED, "Access denied"},
    
    // File operation related error codes
    {M70_ERROR_CODE_EX_FILE_NOT_FOUND, "File not found"},
    {M70_ERROR_CODE_EX_FILE_ACCESS_DENIED, "File access denied"},
    {M70_ERROR_CODE_EX_FILE_INVALID_FORMAT, "Invalid file format"},
    {M70_ERROR_CODE_EX_FILE_IO_ERROR, "File IO error"},
    
    // System related error codes
    {M70_ERROR_CODE_EX_SYS_OUT_OF_MEMORY, "Out of memory"},
    {M70_ERROR_CODE_EX_SYS_RESOURCE_LIMIT, "Resource limit"},
    {M70_ERROR_CODE_EX_SYS_INTERNAL_ERROR, "Internal error"}
};

// Get system error code
static int get_system_error_code(void) {
#ifdef _WIN32
    return (int)GetLastError();
#else
    return errno;
#endif
}

// Get last error information
const m70_error_info_t* m70_error_get_last(void) {
    return &g_last_error;
}

// Set error information
void m70_error_set(m70_error_code_ex_e code, const char* file, int line, const char* func, const char* message, ...) {
    // Clear previous error information
    memset(&g_last_error, 0, sizeof(m70_error_info_t));
    
    // Set error code
    g_last_error.error_code = code;
    
    // Set system error code
    g_last_error.system_error_code = get_system_error_code();
    
    // Set file name, line number and function name
    if (file) {
        strncpy(g_last_error.file, file, sizeof(g_last_error.file) - 1);
    }
    g_last_error.line = line;
    if (func) {
        strncpy(g_last_error.function, func, sizeof(g_last_error.function) - 1);
    }
    
    // Format error message
    if (message) {
        va_list args;
        va_start(args, message);
        vsnprintf(g_last_error.message, sizeof(g_last_error.message) - 1, message, args);
        va_end(args);
    }
    
    // Log error message
    m70_log_error("[ERROR] %s (Code: %d, SysCode: %d) at %s:%d in %s()", 
                 g_last_error.message, 
                 g_last_error.error_code, 
                 g_last_error.system_error_code,
                 g_last_error.file,
                 g_last_error.line,
                 g_last_error.function);
}

// Get description information corresponding to error code
const char* m70_error_get_description(m70_error_code_ex_e code) {
    for (size_t i = 0; i < sizeof(g_error_descriptions) / sizeof(g_error_descriptions[0]); i++) {
        if (g_error_descriptions[i].code == code) {
            return g_error_descriptions[i].description;
        }
    }
    return "Undefined error";
}

// Convert basic error code to extended error code
m70_error_code_ex_e m70_error_code_to_ex(m70_error_code_e code) {
    switch (code) {
        case M70_ERROR_CODE_OK:
            return M70_ERROR_CODE_EX_OK;
        case M70_ERROR_CODE_FAILED:
            return M70_ERROR_CODE_EX_FAILED;
        case M70_ERROR_CODE_SOCKET_FAILED:
            return M70_ERROR_CODE_EX_SOCKET_FAILED;
        case M70_ERROR_CODE_UNKOWN:
        default:
            return M70_ERROR_CODE_EX_UNKNOWN;
    }
}

// Convert extended error code to basic error code
m70_error_code_e m70_error_ex_to_code(m70_error_code_ex_e ex_code) {
    if (ex_code == M70_ERROR_CODE_EX_OK) {
        return M70_ERROR_CODE_OK;
    } else if (ex_code == M70_ERROR_CODE_EX_SOCKET_FAILED ||
               ex_code == M70_ERROR_CODE_EX_CONN_TIMEOUT ||
               ex_code == M70_ERROR_CODE_EX_CONN_REFUSED ||
               ex_code == M70_ERROR_CODE_EX_CONN_CLOSED ||
               ex_code == M70_ERROR_CODE_EX_CONN_INVALID ||
               ex_code == M70_ERROR_CODE_EX_CONN_HOST_NOT_FOUND ||
               ex_code == M70_ERROR_CODE_EX_TRANS_TIMEOUT ||
               ex_code == M70_ERROR_CODE_EX_TRANS_INCOMPLETE ||
               ex_code == M70_ERROR_CODE_EX_TRANS_INVALID_DATA ||
               ex_code == M70_ERROR_CODE_EX_TRANS_BUFFER_OVERFLOW) {
        return M70_ERROR_CODE_SOCKET_FAILED;
    } else if (ex_code == M70_ERROR_CODE_EX_UNKNOWN) {
        return M70_ERROR_CODE_UNKOWN;
    } else {
        return M70_ERROR_CODE_FAILED;
    }
}

// Format error information to string
void m70_error_format(char* buffer, size_t buffer_size, const m70_error_info_t* error_info) {
    if (!buffer || buffer_size == 0 || !error_info) {
        return;
    }
    
    const char* description = m70_error_get_description(error_info->error_code);
    
    snprintf(buffer, buffer_size,
             "Error: %s (Code: %d, System Error: %d)\n"
             "Message: %s\n"
             "Location: %s:%d in %s()",
             description,
             error_info->error_code,
             error_info->system_error_code,
             error_info->message,
             error_info->file,
             error_info->line,
             error_info->function);
}