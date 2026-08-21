#ifndef PROTO_WM_H
#define PROTO_WM_H

#include <proto/graphics.h>

/*****************************************************************************
 * Window manager IPC protocol
 *****************************************************************************/

#define CREATE_WINDOW_MSG   "wm.window.create"
#define REFRESH_WINDOW_MSG  "wm.window.refresh"

typedef struct win_options_t {
    int x;  int y;
    int w;  int h;
    char    name[255];
    int     frameless;
} __attribute__((packed)) win_options_t;

typedef struct win_response_t {
    int id;
    fb_info_t *fb_addr;
} __attribute__((packed)) win_response_t;

int create_window(win_options_t config);
int refresh_window(int id);

#endif