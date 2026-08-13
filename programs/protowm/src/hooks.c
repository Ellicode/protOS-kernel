#include "hooks.h"
#include "proto/graphics.h"
#include "window.h"
#include <stdint.h>
#include <proto/core.h>

void handle_create_window(win_options_t *config, ev_meta_t *meta) {
    if (config == NULL || meta == NULL) { return; }
    
    window_t *win = create_window(
        config->x, config->y,
        config->w, config->h,
        config->name,
        config->frameless
    );
    if (win == NULL) { return; }

    win->pid = meta->sender;
    
    // Share the fb_info_t struct AND the pixel buffer it points to,
    // otherwise the client pagefaults when touching fb->address.
    share(meta->sender, win->fb, sizeof(fb_info_t));
    share(meta->sender, (void *)win->fb->address, win->fb->pitch * win->fb->height);

    win_response_t res = (win_response_t) {
        .id         = win->id,
        .fb_addr    = win->fb
    };
    
    send(meta->sender, "wm.window.created", &res, sizeof(res));
}
