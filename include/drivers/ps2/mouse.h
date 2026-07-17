#ifndef PS2_MOUSE_H
#define PS2_MOUSE_H

typedef struct mouse_move_packet_t {
    int x;
    int y;

    int vel_x;
    int vel_y;
} mouse_move_packet_t;


void ps2mouse_init();
void ps2mouse_read();

#endif // PS2_MOUSE_H