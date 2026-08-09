#ifndef SERIAL_H
#define SERIAL_H

#define PORT_COM1   0x3f8          // COM1

int serial_init();
void serial_write(char c);
char serial_read();

#endif // SERIAL_H
