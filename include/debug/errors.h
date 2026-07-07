#ifndef ERRORS_H
#define ERRORS_H

#include <stddef.h>

#include "debug/logger.h"
#include "graphics/console.h"

enum status_code_t {
    PROTO_OK                            = 0,
    PROTO_ERR_UNKNOWN                   = 1,

    PROTO_ERR_FILE_NOT_FOUND            = 11,
    PROTO_ERR_NOT_A_DIRECTORY           = 12,
    PROTO_ERR_IS_A_DIRECTORY            = 13,
    PROTO_ERR_ALREADY_EXISTS            = 14,
    PROTO_ERR_FILE_UNSUPPORTED_OP       = 15,
    PROTO_ERR_FILE_UNAUTHORIZED_OP      = 16,
    PROTO_ERR_MAX_FD_REACHED            = 17,
    PROTO_ERR_INVALID_FD                = 18,

    PROTO_ERR_INIT_FAILED               = 21,
    PROTO_ERR_OUT_OF_MEMORY             = 22,

    PROTO_ERR_INVALID_ARGUMENT          = 31,
    PROTO_ERR_SYSCALL_OUT_OF_BOUNDS     = 32,

    PROTO_ERR_ELF_INVALID_HDR           = 41,
    PROTO_ERR_ELF_UNSUPPORTED           = 42,
    PROTO_ERR_ELF_CORRUPTED             = 43,
    PROTO_ERR_ELF_CANNOT_LOAD           = 44,

    PROTO_ERR_IPC_QUEUE_FULL            = 51,
    PROTO_ERR_IPC_QUEUE_EMPTY           = 52
};

enum status_type_t {
    PROTO_STATUS_SUCCESS                = 0,
    PROTO_STATUS_ERROR                  = 1,
    PROTO_STATUS_CRITICAL               = 2,
};

extern int status_types[];
extern char* status_messages[];
extern char* status_hints[];

#define k_assert(e) do {                            \
    if (status_types[e] == PROTO_STATUS_ERROR || status_types[e] == PROTO_STATUS_CRITICAL) {  \
        k_error(status_messages[e]);                \
        print_f(".\n");                             \
        if (status_hints[e] != NULL) {              \
            print_f("(tip!) %s.\n");                \
        }                                           \
    }                                               \
} while (0)

#endif // ERRORS_H