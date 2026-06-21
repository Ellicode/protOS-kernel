#ifndef LOGGER_H
#define LOGGER_H

#define k_error(msg) do {                   \
    print_f("[");                           \
    set_color(PROTO_RED, PROTO_BG);         \
    print_f("ERROR");                       \
    set_color(PROTO_WHITE, PROTO_BG);       \
    print_f("] (%s:%d) %s",                 \
            __FILE_NAME__,                  \
            __LINE__,                       \
            msg);                           \
} while (0)

#define k_info(msg) do {                    \
    print_f("[");                           \
    set_color(PROTO_BLUE, PROTO_BG);        \
    print_f("INFO ");                       \
    set_color(PROTO_WHITE, PROTO_BG);       \
    print_f("] (%s:%d) %s",                 \
            __FILE_NAME__,                  \
            __LINE__,                       \
            msg);                           \
} while (0)

#define k_success(msg) do {                 \
    print_f("[");                           \
    set_color(PROTO_GREEN, PROTO_BG);       \
    print_f("OK   ");                       \
    set_color(PROTO_WHITE, PROTO_BG);       \
    print_f("] (%s:%d) %s",                 \
            __FILE_NAME__,                  \
            __LINE__,                       \
            msg);                           \
} while (0)

#define k_warning(msg) do {                 \
    print_f("[");                           \
    set_color(PROTO_YELLOW, PROTO_BG);      \
    print_f("WARN ");                       \
    set_color(PROTO_WHITE, PROTO_BG);       \
    print_f("] (%s:%d) %s",                 \
            __FILE_NAME__,                  \
            __LINE__,                       \
            msg);                           \
} while (0)

#if (PROTO_DEBUG == 1)
#define k_debug(msg) do {                   \
    print_f("[");                           \
    set_color(PROTO_MAGENTA, PROTO_BG);     \
    print_f("DEBUG");                       \
    set_color(PROTO_WHITE, PROTO_BG);       \
    print_f("] (%s:%d) %s",                 \
            __FILE_NAME__,                  \
            __LINE__,                       \
            msg);                           \
} while (0)
#endif

#endif // LOGGER_H