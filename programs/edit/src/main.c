#include <proto/core.h>
#include <proto/events.h>

int pmain(char argv[16][64], int argc) {
    ev_meta_t *ev_meta = malloc(sizeof(ev_meta_t));
    keyboard_event_t *ev_data = malloc(sizeof(keyboard_event_t));

    subscribe("proto.keyboard.keydown");

    while (1)
    {
        int res = receive(ev_meta, ev_data);
        if (ev_data != NULL && res == PROTO_OK) {
            if (ev_data->keycode == KBD_Q) { break; }
            if (ev_data->character > 0) {
                printf("%c", ev_data->character);
            }
        }
        consume(ev_meta);
    }

    unsubscribe("proto.keyboard.keydown");

    free(ev_meta);
    free(ev_data);

    return 0;
}