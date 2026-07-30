#include <proto/core.h>
#include <proto/graphics.h>
#include <proto/events.h>

#include "init.h"
#include "cursor.h"
#include "window.h"
#include "globals.h"

window_t *hovering = NULL;
window_t *dragging_win = NULL;
int is_mouse_down = 0;
int last_mouse_x = 0;
int last_mouse_y = 0;

static int collide_rect(int cx, int cy, int x, int y, int w, int h) {
    return (cx >= x && cx < x + w && cy >= y && cy < y + h);
}

void check_cursor_collision(int x, int y) {
    window_t *win = g_window_stack;
    while (win != NULL) {
        if (collide_rect(x, y, win->x, win->y, win->width, TITLEBAR_HEIGHT)) {
            hovering = win;
            return;
        }
        win = win->next;
    }
    hovering = NULL;
}

void handle_event(ev_meta_t *meta, void *data) {
    if (meta == NULL) return;

    memset(data, 0, 256);
    int res = receive(meta, data);
    if (res != PROTO_OK) {
        fprintf(STDERR, "[ERROR] Receive failed with code %d", res);
    }

    if (strcmp(meta->name, "proto.mouse.move") == 0) {
        mouse_move_packet_t pkt;
        memcpy(&pkt, data, sizeof(pkt));

        if (is_mouse_down && dragging_win) {
            invert_rect_o(g_fb, dragging_win->x, dragging_win->y, dragging_win->owidth, dragging_win->oheight);
            dragging_win->x += pkt.vel_x;
            dragging_win->y += pkt.vel_y;
            if (dragging_win->x < 0)
                dragging_win->x = 0;
            if (dragging_win->y < 0)
                dragging_win->y = 0;
            if (dragging_win->x + dragging_win->owidth > g_fb->width)
                dragging_win->x = (int)g_fb->width - dragging_win->owidth;
            if (dragging_win->y + dragging_win->oheight > g_fb->height)
                dragging_win->y = (int)g_fb->height - dragging_win->oheight;
            last_mouse_x = pkt.x;
            last_mouse_y = pkt.y;
            invert_rect_o(g_fb, dragging_win->x, dragging_win->y, dragging_win->owidth, dragging_win->oheight);
        } else {
            check_cursor_collision(pkt.x, pkt.y);
            draw_cursor(pkt.x, pkt.y);
        }

    } else if (strcmp(meta->name, "proto.mouse.down") == 0) {
        is_mouse_down = 1;
        dragging_win = hovering;
        if (dragging_win) {
            draw_img(g_fb, dragging_win->under, dragging_win->x, dragging_win->y, dragging_win->owidth, dragging_win->oheight);
            invert_rect_o(g_fb, dragging_win->x, dragging_win->y, dragging_win->owidth, dragging_win->oheight);
        }
    } else if (strcmp(meta->name, "proto.mouse.up") == 0) {
        if (dragging_win) {
            invert_rect_o(g_fb, dragging_win->x, dragging_win->y, dragging_win->owidth, dragging_win->oheight);
            // already done in draw window
            //capture_rect(g_fb, dragging_win->under, dragging_win->x, dragging_win->y, dragging_win->owidth, dragging_win->oheight);
            draw_window(dragging_win, 0);
            _draw_cursor_impl(last_mouse_x, last_mouse_y);
            last_mouse_x = 0;
            last_mouse_y = 0;
        }
        is_mouse_down = 0;
        dragging_win = NULL;
    } else if (strcmp(meta->name, "proto.keyboard.keydown") == 0) {
        //exit();
    }

    consume(meta);
}

int pmain(char argv[16][64], int argc) {
    wm_init();
    
    ev_meta_t *meta = malloc(sizeof(ev_meta_t));
    char *data = malloc(256);

    draw_rect(g_fb, 1000, 700, 100, 100, 0x0000FF);

    window_t *win = create_window(100, 100, 500, 350, "A simple window");
    draw_rect(win->fb, 0, 0, 50, 50, 0xFF0000);
    refresh_window(win);
    
    while (1) {
        handle_event(meta, data);
    }

    free(meta);
    free(g_fb);
    bmp_free(g_cursor_bmp);

    return 0;
}