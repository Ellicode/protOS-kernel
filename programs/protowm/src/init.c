#include <proto/core.h>
#include <proto/graphics.h>
#include <proto/events.h>

#include "globals.h"
#include "init.h"
#include "cursor.h"

char *subs[] = {
    "proto.keyboard.keydown",
    "proto.mouse.move",
    "proto.mouse.down",
    "proto.mouse.up",
    "wm.window.create",
    "wm.window.refresh"
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

    //g_wallpaper = bmp_load("/System/Assets/Images/xp.bmp");
    //if (g_wallpaper == NULL) {
    //    printf("[WARN] Failed to load wallpaper\n");
    //}

    for (int i = 0; i < (sizeof(subs) / sizeof(subs[0])); i++) {
        if (subscribe(subs[i]) != PROTO_OK) {
            fprintf(STDERR, "[ERROR] Subscribe failed -> \"%s\"", subs[i]);
            free(g_fb);
            return 1;
        }   
    }

    g_small_font = font_load("/System/Assets/Fonts/ter-u12n.fmp");

    init_cursor();
    
    return 0;
}
