#include <proto/core.h>
#include <proto/events.h>

int pmain(char argv[16][64], int argc) {
    ev_meta_t *ev_meta = malloc(sizeof(ev_meta_t));
    char *ev_data = malloc(1);

    subscribe("proto.keyboard.keydown");

    while (1)
    {
        int res = recieve(ev_meta, ev_data);
        if (ev_data != NULL && res == PROTO_OK) {
            char c = *ev_data;
            if (c == 'q') { break; }
            printf("%c", *ev_data);
        }
        consume(ev_meta);
    }

    unsubscribe("proto.keyboard.keydown");

    free(ev_meta);
    free(ev_data);

    return 0;
}