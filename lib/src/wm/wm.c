#include <proto/events.h>
#include <wm/core.h>

int create_window(win_options_t config) {
    return dispatch(CREATE_WINDOW_MSG, &config, sizeof(win_options_t));
}

int refresh_window(int id) {
    return dispatch(REFRESH_WINDOW_MSG, &id, sizeof(id));
}