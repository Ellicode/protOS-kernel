#ifndef PS2_H
#define PS2_H

#define PS2_COMMAND_PORT 0x64
#define PS2_STATUS_PORT  0x64
#define PS2_DATA_PORT    0x60

void ps2_init();

#endif // PS2_H