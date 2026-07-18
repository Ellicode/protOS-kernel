#include "debug/errors.h"

int status_types[] = {
    [PROTO_OK]                          = PROTO_STATUS_SUCCESS,
    [PROTO_ERR_UNKNOWN]                 = PROTO_STATUS_ERROR,

    [PROTO_ERR_FILE_NOT_FOUND]          = PROTO_STATUS_ERROR,
    [PROTO_ERR_NOT_A_DIRECTORY]         = PROTO_STATUS_ERROR,
    [PROTO_ERR_IS_A_DIRECTORY]          = PROTO_STATUS_ERROR,
    [PROTO_ERR_ALREADY_EXISTS]          = PROTO_STATUS_ERROR,
    [PROTO_ERR_FILE_UNAUTHORIZED_OP]    = PROTO_STATUS_ERROR,
    [PROTO_ERR_MAX_FD_REACHED]          = PROTO_STATUS_ERROR,
    [PROTO_ERR_INVALID_FD]              = PROTO_STATUS_ERROR,
    
    [PROTO_ERR_INIT_FAILED]             = PROTO_STATUS_CRITICAL,
    [PROTO_ERR_OUT_OF_MEMORY]           = PROTO_STATUS_ERROR,

    [PROTO_ERR_INVALID_ARGUMENT]        = PROTO_STATUS_ERROR,
    [PROTO_ERR_SYSCALL_OUT_OF_BOUNDS]   = PROTO_STATUS_ERROR,
    
    [PROTO_ERR_ELF_INVALID_HDR]         = PROTO_STATUS_ERROR,
    [PROTO_ERR_ELF_UNSUPPORTED]         = PROTO_STATUS_ERROR,
    [PROTO_ERR_ELF_CORRUPTED]           = PROTO_STATUS_ERROR,
    [PROTO_ERR_ELF_CANNOT_LOAD]         = PROTO_STATUS_ERROR,
};

char *status_messages[] = {
    [PROTO_OK]                          = "Success",
    [PROTO_ERR_UNKNOWN]                 = "Unknown error",

    [PROTO_ERR_FILE_NOT_FOUND]          = "File cannot be found",
    [PROTO_ERR_NOT_A_DIRECTORY]         = "File is not a directory",
    [PROTO_ERR_IS_A_DIRECTORY]          = "File is a directory",
    [PROTO_ERR_ALREADY_EXISTS]          = "File already exists",
    [PROTO_ERR_FILE_UNAUTHORIZED_OP]    = "Unauthorized operation",
    [PROTO_ERR_MAX_FD_REACHED]          = "File descriptor limit reached",
    [PROTO_ERR_INVALID_FD]              = "Invalid file descriptor index",
    
    [PROTO_ERR_INIT_FAILED]             = "Initialization failed",
    [PROTO_ERR_OUT_OF_MEMORY]           = "Out of memory",
    [PROTO_ERR_INVALID_ARGUMENT]        = "Invalid argument",
    [PROTO_ERR_SYSCALL_OUT_OF_BOUNDS]   = "Syscall ID out of bounds",
    [PROTO_ERR_INVALID_CONTEXT]         = "Invalid context",

    [PROTO_ERR_ELF_INVALID_HDR]         = "Invalid ELF Header",
    [PROTO_ERR_ELF_UNSUPPORTED]         = "Unsupported platform",
    [PROTO_ERR_ELF_CORRUPTED]           = "Corrupted ELF executable",
    [PROTO_ERR_ELF_CANNOT_LOAD]         = "Cannot load ELF executable",

    [PROTO_ERR_PROCESS_NOT_FOUND]       = "Process not found",
};

char *status_hints[] = {
    [PROTO_OK]                          = "You did it!",
    [PROTO_ERR_UNKNOWN]                 = "Check the exact code for more details.",

    [PROTO_ERR_FILE_NOT_FOUND]          = NULL,
    [PROTO_ERR_NOT_A_DIRECTORY]         = NULL,
    [PROTO_ERR_IS_A_DIRECTORY]          = NULL,
    [PROTO_ERR_ALREADY_EXISTS]          = NULL,
    [PROTO_ERR_FILE_UNAUTHORIZED_OP]    = "Did you set the right flags when opening the file?",
    [PROTO_ERR_MAX_FD_REACHED]          = "Try closing some unused file descriptors!",
    [PROTO_ERR_INVALID_FD]              = "Make sure this file descriptor is opened properly!",
    
    [PROTO_ERR_INIT_FAILED]             = NULL,
    [PROTO_ERR_OUT_OF_MEMORY]           = "RAM?!!!! IN THIS ECONOMY?!!!!",
    [PROTO_ERR_INVALID_ARGUMENT]        = "Verify for the correct type and null-check your argument first!",
    [PROTO_ERR_SYSCALL_OUT_OF_BOUNDS]   = "So you're not using my libc, huh?",
    [PROTO_ERR_INVALID_CONTEXT]         = NULL,

    [PROTO_ERR_ELF_INVALID_HDR]         = "Try rebuilding your executable again!",
    [PROTO_ERR_ELF_UNSUPPORTED]         = "Did you build it for x86_64?",
    [PROTO_ERR_ELF_CORRUPTED]           = "Try rebuilding your executable again!",
    [PROTO_ERR_ELF_CANNOT_LOAD]         = NULL,

    [PROTO_ERR_PROCESS_NOT_FOUND]       = NULL,
};
