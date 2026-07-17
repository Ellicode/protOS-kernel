#include <proto.h>

#include "graphics.h"

#define BG_COLOR 0x008888

int prev_x = 0;
int prev_y = 0;

int pmain(char argv[16][64], int argc) {
    fb_info_t *front = malloc(sizeof(fb_info_t));
    fetch_framebuffer(front);

    if (front == NULL) {
        fprintf(STDERR, "Requested framebuffer is null\n");
        return 1;
    }

    bmp_t *cursor = load_bitmap("/System/Assets/Cursor.bmp");
    if (cursor == NULL) {
        fprintf(STDERR, "failed to load cursor bmp\n");
    }

    // size_t fb_size = (front->width * front->height);
    // void* back_data = malloc(fb_size);
    // fb_info_t *back = malloc(sizeof(fb_info_t));
    // back->address = (uint64_t)back_data;
    // back->bpp       = front->bpp;
    // back->width     = front->width;
    // back->height    = front->height;
    // back->pitch     = front->pitch;


    draw_rect(front, 0, 0, front->width, front->height, BG_COLOR);

    // // swap buffers
    // memcpy((void *)front->address, (void *)back->address, fb_size);

    printf("resolution=%dx%d; bpp=%d\n", front->width, front->height, front->bpp);
    printf("pid=%d\n", getpid());
    
    ipc_meta_t *meta = malloc(sizeof(ipc_meta_t));
    char *data = malloc(256);

    subscribe("proto.keyboard.keydown");
    subscribe("proto.mouse.move");

    while (1) {
        memset(data, 0, 256);
        int res = recieve(meta, data);
        
        if (res != PROTO_OK) {
            fprintf(STDERR, "recieve failed with code %d", res);
            consume(meta);
            break;
        }

        if (strcmp(meta->name, "proto.mouse.move") == 0) {
            mouse_move_packet_t pkt;
            memcpy(&pkt, data, sizeof(pkt));

            draw_rect(front, prev_x, prev_y, cursor->width, cursor->height, BG_COLOR);
            draw_bitmap(front, cursor, pkt.x, pkt.y);

            prev_x = pkt.x;
            prev_y = pkt.y;
        } else if (strcmp(meta->name, "proto.keyboard.keydown") == 0) {
            char c = *data;
            printf("%c", c);
        } else {
            printf("unknown event \"%s\"\n", meta->name);
        }

        consume(meta);
    }

    // free(back_data);
    // free(back);
    free(meta);
    free(front);
    free_bitmap(cursor);

    return 0;
}