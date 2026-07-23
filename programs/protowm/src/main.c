#include <proto/core.h>
#include <proto/graphics.h>
#include <proto/events.h>

#include "init.h"
#include "cursor.h"
#include "globals.h"

void handle_event(ev_meta_t *meta, void *data) {
    if (strcmp(meta->name, "proto.mouse.move") == 0) {
        mouse_move_packet_t pkt;
        memcpy(&pkt, data, sizeof(pkt));

        draw_cursor(pkt.x, pkt.y);
    } else if (strcmp(meta->name, "proto.keyboard.keydown") == 0) {
        char *c = (char *)data;
        if (*c == 'q') {
            exit();
        }
        printf("%c", *c);
    }
}

int pmain(char argv[16][64], int argc) {
    wm_init();
    
    ev_meta_t *meta = malloc(sizeof(ev_meta_t));
    char *data = malloc(256);

    // draw_rect(g_fb, 100, 10, 100, 20, 0x8aadf4);
    // draw_rect(g_fb, 100, 30, 100, 20, 0xf5bde6);
    // draw_rect(g_fb, 100, 50, 100, 20, 0xFFFFFF);
    // draw_rect(g_fb, 100, 70, 100, 20, 0xf5bde6);
    // draw_rect(g_fb, 100, 90, 100, 20, 0x8aadf4);

    while (1) {
        memset(data, 0, 256);
        int res = recieve(meta, data);
        if (res != PROTO_OK) {
            fprintf(STDERR, "[ERROR] Recieve failed with code %d", res);
            break;
        }
        handle_event(meta, data);
        consume(meta);
    }

    free(meta);
    free(g_fb);
    bmp_free(g_cursor_bmp);

    return 0;
}