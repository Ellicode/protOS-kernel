#ifndef PROTO_WM_H
#define PROTO_WM_H

#include <proto/graphics.h>

/*****************************************************************************
 * Window manager IPC protocol
 *****************************************************************************/

// Client -> WM: dispatch("wm.window.create", &opts, sizeof(opts))
// WM -> Client: "wm.window.created" with win_response_t payload
// Client -> WM: send(wm_pid, "wm.window.refresh", &id, sizeof(id))

typedef struct win_options_t {
    int x;  int y;
    int w;  int h;
    char    name[255];
    int     frameless;
} win_options_t;

typedef struct win_response_t {
    int id;
    fb_info_t *fb_addr;
} win_response_t;

#endif
