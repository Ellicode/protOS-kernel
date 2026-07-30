#ifndef PIT_H
#define PIT_H

#include <stdint.h>

#define PIT_FREQUENCY 1193182
#define PIT_CONTROL_PORT 0x43
#define PIT_CHANNEL_0_PORT 0x40

extern int g_pit_frequency;
extern int g_pit_ticks;

void pit_init(int frequency);

#endif // PIT_H