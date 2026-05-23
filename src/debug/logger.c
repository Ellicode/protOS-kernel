#include "graphics/tty.h"

void k_error(const char *message, const char *scope) {
    print_f("[");
    set_color(PROTO_RED);
    print_f("ERROR");
    set_color(PROTO_WHITE);
    print_f("] (%s) %s", scope, message);
}

void k_info(const char *message, const char *scope) {
    print_f("[");
    set_color(PROTO_BLUE);
    print_f("INFO ");
    set_color(PROTO_WHITE);
    print_f("] (%s) %s", scope, message);
}

void k_success(const char *message, const char *scope) {
    print_f("[");
    set_color(PROTO_GREEN);
    print_f("OK   ");
    set_color(PROTO_WHITE);
    print_f("] (%s) %s", scope, message);
}

void k_warning(const char *message, const char *scope) {
    print_f("[");
    set_color(PROTO_YELLOW);
    print_f("WARN ");
    set_color(PROTO_WHITE);
    print_f("] (%s) %s", scope, message);
}

void k_debug(const char *message, const char *scope) {
    #if (PROTO_DEBUG == 1)
        print_f("[");
        set_color(PROTO_MAGENTA);
        print_f("DEBUG");
        set_color(PROTO_WHITE);
        print_f("] (%s) %s", scope, message);
    #endif
}