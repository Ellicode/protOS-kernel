#include <proto/graphics.h>
#include <proto/core.h>
#include <proto/events.h>
#include <wm/core.h>
#include <stdint.h>

#include "console.h"

int running = 1;
int cursor_x = 0;
int cursor_y = 0;
font_t *fnt;
int win_id = -1;
char buf[255];
char write_buf[255];

int create_pty() {
    int fd = open("/dev/ptymx", "rwa");
    if (fd < PROTO_OK) { return -PROTO_ERR_UNKNOWN; }
    char *pty_path = malloc(12);
    int res = read(fd, pty_path, 0);
    setiofd(pty_path);
    create_nonblocking_process("/system/bin/corgi", NULL, 0);
    return fd;
}

int pmain(char argv[16][64], int argc) {
    ev_meta_t *meta = malloc(sizeof(ev_meta_t));
    char *data = malloc(512);

    win_options_t config = (win_options_t) {
        .x = 100,
        .y = 100,
        .w = 500,
        .h = 300,
        .name = "protOS terminal"
    };

    create_window(config);
    fnt = font_load("/system/assets/fonts/ter-u12n.fmp");
    int fd = create_pty();

    while (running) {
        if (win_id != -1) {
            int read_res = PROTO_OK;
            while (read_res != -PROTO_ERR_WOULD_BLOCK) {
                read_res = read(fd, buf, 255);
                print(buf);
                memset(buf, 0, 255);
            }
            refresh_window(win_id);
        }
        
        int res = receive(meta, data);
        if (res != PROTO_OK) {
            fprintf(STDOUT, "[ERROR] Receive failed with code %d", res);
            return 1;
        }

        if (strcmp(meta->name, "wm.window.created") == 0) {
            win_response_t *win_res = (win_response_t *)data;
            win_id = win_res->id;
            terminal_init(win_res->fb_addr, fnt);
        } else if (strcmp(meta->name, "wm.keyboard.keydown") == 0) {
            keyboard_event_t *ev = (keyboard_event_t *)data;
            size_t len = strlen(write_buf);
            char c = ev->character;
            if (c == '\n' || c == 0xD) {
                write(fd, write_buf, len);
                print_char('\n');
                memset(write_buf, 0, sizeof(write_buf));
            } else if (c == '\b' || c == 0x7F) {
                if (len > 0) {
                    write_buf[len - 1] = '\0';
                    print_char('\b');
                }
            } else if (c >= 32 && c <= 126 && len < sizeof(write_buf)) {
                write_buf[len] = c;
                write_buf[len + 1] = '\0';
                print_char(c);
            }
            refresh_window(win_id);
        } else if (strcmp(meta->name, "proto.aio.readable") == 0) {
            int read_res = PROTO_OK;
            while (read_res != -PROTO_ERR_WOULD_BLOCK) {
                read_res = read(fd, buf, 255);
                print(buf);
                memset(buf, 0, 255);
            }
            refresh_window(win_id);
        }
        
        consume(meta);
    }

    return 0;   
}
