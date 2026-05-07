#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_PORT 0x3f8          // COM1

static inline void outb(unsigned short port, unsigned char value);
static inline unsigned char inb(unsigned short port);

int serial_init();
void serial_write(char c);

#endif // SERIAL_H