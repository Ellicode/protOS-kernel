#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_PORT 0x3f8          // COM1

int serial_init();
void serial_write(char c);

#endif // SERIAL_H