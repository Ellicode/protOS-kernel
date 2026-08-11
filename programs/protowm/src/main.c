#include <proto/core.h>
#include <proto/events.h>
#include <proto/graphics.h>

#include "cursor.h"
#include "globals.h"
#include "init.h"
#include "window.h"
#include "hooks.h"
#include "utils.h"

window_t *hovering      = NULL;
window_t *dragging_win  = NULL;
int is_mouse_down       = 0;
int last_mouse_x        = 0;
int last_mouse_y        = 0;
int running             = 1;

void redraw_rect(int x, int y, int w, int h) {
    window_t *win = g_window_stack;
    while (win != NULL) {
        if (win == dragging_win) {
            win = win->next;
            continue;
        }
        if (collide_rect(x, y, w, h, win->x, win->y, win->owidth, win->oheight)) {
            // printf("redrawing window \"%s\"\n", win->name);
            draw_window_clipped(win, x, y, w, h);
        }
        win = win->next;
    }
}

void check_cursor_collision(int x, int y) {
    window_t *win = g_window_stack;
    while (win != NULL) {
        if (win->frameless == 0 && collide_point(x, y, win->x, win->y, win->width, TITLEBAR_HEIGHT)) {
            hovering = win;
            return;
        }
        win = win->next;
    }
    hovering = NULL;
}

void handle_event(ev_meta_t *meta, void *data) {
    if (meta == NULL) return;

    int res = receive(meta, data);
    if (res != PROTO_OK) {
        fprintf(STDERR, "[ERROR] Receive failed with code %d", res);
        return;
    }

    if (strcmp(meta->name, "proto.mouse.move") == 0) {
        mouse_move_packet_t *pkt = (mouse_move_packet_t *)data;
        if (is_mouse_down && dragging_win) {
            invert_rect_o(g_fb, dragging_win->x, dragging_win->y, dragging_win->owidth, dragging_win->oheight);
            dragging_win->x += pkt->vel_x;
            dragging_win->y += pkt->vel_y;
            if (dragging_win->x < 0) dragging_win->x = 0;
            if (dragging_win->y < 0) dragging_win->y = 0;
            if (dragging_win->x + dragging_win->owidth > g_fb->width)
                dragging_win->x = (int)g_fb->width - dragging_win->owidth;
            if (dragging_win->y + dragging_win->oheight > g_fb->height)
                dragging_win->y = (int)g_fb->height - dragging_win->oheight;
            last_mouse_x = pkt->x;
            last_mouse_y = pkt->y;
            draw_cursor(pkt->x, pkt->y);
            invert_rect_o(g_fb, dragging_win->x, dragging_win->y, dragging_win->owidth, dragging_win->oheight);
        } else {
            draw_cursor(pkt->x, pkt->y);
        }
        check_cursor_collision(pkt->x, pkt->y);
    } else if (strcmp(meta->name, "proto.mouse.down") == 0) {
        is_mouse_down = 1;
        dragging_win = hovering;
        if (dragging_win != NULL) {
            redraw_rect(dragging_win->x, dragging_win->y, dragging_win->owidth, dragging_win->oheight);
            invert_rect_o(g_fb, dragging_win->x, dragging_win->y, dragging_win->owidth, dragging_win->oheight);
        }
    } else if (strcmp(meta->name, "proto.mouse.up") == 0) {
        if (dragging_win) {
            invert_rect_o(g_fb, dragging_win->x, dragging_win->y, dragging_win->owidth, dragging_win->oheight);
            draw_window(dragging_win);
            LL_UNLINK(dragging_win, g_window_stack);
            LL_APPEND(dragging_win, g_window_stack);
            last_mouse_x = 0;
            last_mouse_y = 0;
        }
        is_mouse_down = 0;
        dragging_win = NULL;
    } else if (strcmp(meta->name, "proto.keyboard.keydown") == 0) {
        char *c = data;
        if (*c == 'w') {
            window_t *test = create_window(200, 200, 100, 100, "test", 0);
            draw_window(test);
        } else if (*c == 'q') {
            running = 0;
        } else if (*c == 't') {
            create_nonblocking_process("System/Programs/testapp", NULL, 0);
        }
    } else if (strcmp(meta->name, "wm.window.create") == 0) {
        handle_create_window((win_options_t *)data, meta);
    } else if (strcmp(meta->name, "wm.window.refresh") == 0) {
        window_t *win = get_win_from_id(*(int *)data);
        if (win != NULL) {
            refresh_window(win);
        }
    } 

    consume(meta);
}

int pmain(char argv[16][64], int argc) {
    wm_init();

    ev_meta_t *meta = malloc(sizeof(ev_meta_t));
    char *data = malloc(512);

    window_t *root = create_window(0, 0, g_fb->width, g_fb->height, "root", 1);
    draw_rect(root->fb, 0, 0, root->fb->width, root->fb->height, BG_COLOR);
    //bmp_draw(root->fb, g_wallpaper, 0, 0);
    refresh_window(root);

    while (running) {
        handle_event(meta, data);
    }

    free(meta);
    free(g_fb);
    bmp_free(g_cursor_bmp);
    free(g_small_font);

    return 0;
}
