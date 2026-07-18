#ifndef ERRORS_H
#define ERRORS_H

#include <stddef.h>

#include "debug/logger.h"
#include "graphics/console.h"
#include "interrupts/interrupts.h"

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
    PROTO_ERR_INVALID_ARGUMENT          = 23,
    PROTO_ERR_SYSCALL_OUT_OF_BOUNDS     = 24,
    PROTO_ERR_INVALID_CONTEXT           = 25,

    PROTO_ERR_ELF_INVALID_HDR           = 31,
    PROTO_ERR_ELF_UNSUPPORTED           = 32,
    PROTO_ERR_ELF_CORRUPTED             = 33,
    PROTO_ERR_ELF_CANNOT_LOAD           = 34,

    PROTO_ERR_PROCESS_NOT_FOUND         = 41,
};

enum status_type_t {
    PROTO_STATUS_SUCCESS                = 0,
    PROTO_STATUS_ERROR                  = 1,
    PROTO_STATUS_CRITICAL               = 2,
};

extern int status_types[];
extern char* status_messages[];
extern char* status_hints[];

#if (PROTO_DEBUG == 1)
#define k_assert(e) do {                                    \
    if (status_types[e] == PROTO_STATUS_ERROR) {            \
        k_error(status_messages[e]);                        \
        print_f(".\n");                                     \
        if (status_hints[e] != NULL) {                      \
            set_color(PROTO_GREY, PROTO_BG);                \
            print_f("   (tip!) %s.\n", status_hints[e]);    \
            set_color(PROTO_WHITE, PROTO_BG);               \
        }                                                   \
    } else if (status_types[e] == PROTO_STATUS_CRITICAL) {  \
        panic(status_messages[e]);                          \
    }                                                       \
} while (0)
#endif

#endif // ERRORS_H