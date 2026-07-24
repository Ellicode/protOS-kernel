#include <proto/core.h>
#include <proto/graphics.h>
#include <proto/events.h>

#include "init.h"
#include "cursor.h"
#include "window.h"
#include "globals.h"

void handle_event(ev_meta_t *meta, void *data) {
    memset(data, 0, 256);
    int res = recieve(meta, data);
    if (res != PROTO_OK) {
        fprintf(STDERR, "[ERROR] Recieve failed with code %d", res);
    }

    if (strcmp(meta->name, "proto.mouse.move") == 0) {
        mouse_move_packet_t pkt;
        memcpy(&pkt, data, sizeof(pkt));

        draw_cursor(pkt.x, pkt.y);
    } else if (strcmp(meta->name, "proto.keyboard.keydown") == 0) {
        char *c = (char *)data;
        if (*c == 'q') {
            exit();
        }
    }

    consume(meta);
}

int pmain(char argv[16][64], int argc) {
    wm_init();
    
    ev_meta_t *meta = malloc(sizeof(ev_meta_t));
    char *data = malloc(256);

    window_t *win = create_window(100, 100, 500, 200, "My Window");
    draw_rect(win->fb, 0, 0, 50, 50, 0xFF0000);
    draw_window(win);
    
    while (1) {
        handle_event(meta, data);
    }

    free(meta);
    free(g_fb);
    bmp_free(g_cursor_bmp);

    return 0;
}