#include <stdint.h>

#ifndef PIT_H
#define PIT_H

#define PIT_FREQUENCY 1193182
#define PIT_CONTROL_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40

void pit_init(uint64_t frequency);

#endif // PIT_H