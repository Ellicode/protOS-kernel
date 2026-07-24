#include <proto/core.h>
#include <proto/graphics.h>
#include <proto/events.h>

#include "globals.h"
#include "init.h"
#include "cursor.h"

char *subs[] = {
    "proto.keyboard.keydown",
    "proto.mouse.move",
    "proto.mouse.down"
};

int wm_init() {
    g_fb = malloc(sizeof(fb_info_t));
    if (g_fb == NULL) {
        fprintf(STDERR, "[ERROR] Cannot allocate framebuffer\n");
        return 1;
    }

    int fb_res = fetch_framebuffer(g_fb);
    if (fb_res != PROTO_OK) {
        fprintf(STDERR, "[ERROR] Cannot request framebuffer\n");
        free(g_fb);
        return 1;
    }

    g_cursor_bmp = bmp_load(CURSOR_PATH);
    if (g_cursor_bmp == NULL) {
        printf("[WARN] Failed to load cursor bitmap\n");
    }    

    for (int i = 0; i < (sizeof(subs) / sizeof(subs[0])); i++) {
        if (subscribe(subs[i]) != PROTO_OK) {
            fprintf(STDERR, "[ERROR] Subscribe failed -> \"%s\"", subs[i]);
            free(g_fb);
            return 1;
        }   
    }

    g_small_font = font_load("/System/Assets/Fonts/npsmall8.fmp");

    draw_rect(g_fb, 0, 0, g_fb->width, g_fb->height, BG_COLOR);
    
    init_cursor();
    
    return 0;
}