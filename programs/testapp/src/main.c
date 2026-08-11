#include <proto/graphics.h>
#include <proto/core.h>
#include <proto/events.h>
#include <proto/wm.h>
#include <stdint.h>

int running = 1;

int pmain(char argv[16][64], int argc) {
    ev_meta_t *meta = malloc(sizeof(ev_meta_t));
    char *data = malloc(256);

    win_options_t config = (win_options_t) {
        .x = 100,
        .y = 100,
        .w = 500,
        .h = 300,
        .name = "/System/Programs/testapp"
    };
    dispatch("wm.window.create", &config, sizeof(win_options_t));
    font_t *fnt = font_load("System/Assets/Fonts/ter-u12n.fmp");
    while (running) {
        int res = receive(meta, data);
        if (res != PROTO_OK) {
            fprintf(STDERR, "[ERROR] Receive failed with code %d", res);
            return 1;
        }

        if (strcmp(meta->name, "wm.window.created") == 0) {
            win_response_t *win_res = (win_response_t *)data;
            draw_rect(win_res->fb_addr, 0, 0, win_res->fb_addr->width, win_res->fb_addr->height, 0xCCCCCC);
            draw_text(win_res->fb_addr, fnt, "Hello from child process!", 0, 0, 0x000000);
            dispatch("wm.window.refresh", &win_res->id, sizeof(win_res->id));
        }

        consume(meta);
    }

    return 0;   
}
