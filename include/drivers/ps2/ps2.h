#ifndef PS2_H
#define PS2_H

#define PS2_COMMAND_PORT    0x64
#define PS2_STATUS_PORT     0x64
#define PS2_DATA_PORT       0x60

#define ACK                 0xFA

void ps2_wait_write();
void ps2_wait_read();
void ps2_write_cmd(unsigned char cmd);
void ps2_write_data(unsigned char data);

unsigned char ps2_read_data();

void ps2_init();

#endif // PS2_H