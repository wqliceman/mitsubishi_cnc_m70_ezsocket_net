#ifndef __H_M70_ERROR_H__
#define __H_M70_ERROR_H__

#include "typedef.h"
#include <stddef.h>

// Extended error code definitions
typedef enum _tag_m70_error_code_ex
{
    // Basic error codes
    M70_ERROR_CODE_EX_OK = 0,                // Operation successful
    M70_ERROR_CODE_EX_FAILED = 1,            // Operation failed
    M70_ERROR_CODE_EX_SOCKET_FAILED = 2,     // Network exception
    M70_ERROR_CODE_EX_UNKNOWN = 99,          // Unknown error
    
    // Connection related error codes (100-199)
    M70_ERROR_CODE_EX_CONN_TIMEOUT = 100,     // Connection timeout
    M70_ERROR_CODE_EX_CONN_REFUSED = 101,     // Connection refused
    M70_ERROR_CODE_EX_CONN_CLOSED = 102,      // Connection closed
    M70_ERROR_CODE_EX_CONN_INVALID = 103,     // Invalid connection
    M70_ERROR_CODE_EX_CONN_HOST_NOT_FOUND = 104, // Host not found
    M70_ERROR_CODE_EX_CONN_FAILED = 105,     // Connection failed 
    
    // Data transmission related error codes (200-299)
    M70_ERROR_CODE_EX_TRANS_TIMEOUT = 200,    // Data transmission timeout
    M70_ERROR_CODE_EX_TRANS_INCOMPLETE = 201, // Incomplete data transmission
    M70_ERROR_CODE_EX_TRANS_INVALID_DATA = 202, // Invalid data
    M70_ERROR_CODE_EX_TRANS_BUFFER_OVERFLOW = 203, // Buffer overflow
    
    // Protocol related error codes (300-399)
    M70_ERROR_CODE_EX_PROTO_INVALID_FORMAT = 300, // Invalid protocol format
    M70_ERROR_CODE_EX_PROTO_VERSION_MISMATCH = 301, // Protocol version mismatch
    M70_ERROR_CODE_EX_PROTO_INVALID_COMMAND = 302, // Invalid command
    M70_ERROR_CODE_EX_PROTO_INVALID_RESPONSE = 303, // Invalid response
    
    // CNC device related error codes (400-499)
    M70_ERROR_CODE_EX_CNC_NOT_READY = 400,   // CNC device not ready
    M70_ERROR_CODE_EX_CNC_BUSY = 401,        // CNC device busy
    M70_ERROR_CODE_EX_CNC_ALARM = 402,       // CNC device alarm
    M70_ERROR_CODE_EX_CNC_INVALID_PARAM = 403, // Invalid parameter
    M70_ERROR_CODE_EX_CNC_INVALID_STATE = 404, // Invalid state
    M70_ERROR_CODE_EX_CNC_ACCESS_DENIED = 405, // Access denied
    
    // File operation related error codes (500-599)
    M70_ERROR_CODE_EX_FILE_NOT_FOUND = 500,   // File not found
    M70_ERROR_CODE_EX_FILE_ACCESS_DENIED = 501, // File access denied
    M70_ERROR_CODE_EX_FILE_INVALID_FORMAT = 502, // Invalid file format
    M70_ERROR_CODE_EX_FILE_IO_ERROR = 503,   // File IO error
    
    // System related error codes (900-999)
    M70_ERROR_CODE_EX_SYS_OUT_OF_MEMORY = 900, // Out of memory
    M70_ERROR_CODE_EX_SYS_RESOURCE_LIMIT = 901, // Resource limit
    M70_ERROR_CODE_EX_SYS_INTERNAL_ERROR = 999  // Internal error
} m70_error_code_ex_e;

// Error information structure
typedef struct _tag_m70_error_info
{
    m70_error_code_ex_e error_code;     // Error code
    int system_error_code;              // System error code
    char message[256];                  // Error message
    char file[128];                     // File where error occurred
    int line;                           // Line number where error occurred
    char function[64];                  // Function where error occurred
} m70_error_info_t;

// Error handling macro definitions
#define M70_ERROR_SET(code, ...) m70_error_set(code, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define M70_ERROR_RETURN(code, ...) do { M70_ERROR_SET(code, __VA_ARGS__); return m70_error_ex_to_code(code); } while(0)
#define M70_ERROR_RETURN_VALUE(code, retval, ...) do { M70_ERROR_SET(code, __VA_ARGS__); return retval; } while(0)

// Error handling functions
void m70_error_set(m70_error_code_ex_e code, const char* file, int line, const char* func, const char* message, ...);
const m70_error_info_t* m70_error_get_last(void);
const char* m70_error_get_description(m70_error_code_ex_e code);
void m70_error_format(char* buffer, size_t buffer_size, const m70_error_info_t* error_info);

// Error code conversion functions
m70_error_code_ex_e m70_error_code_to_ex(m70_error_code_e code);
m70_error_code_e m70_error_ex_to_code(m70_error_code_ex_e ex_code);

#endif // __H_M70_ERROR_H__