#include "io.h"
#include "globals.h"
#include "interrupts/pic.h"

#include "pit.h"

void pit_init(uint64_t frequency) {
    g_pit_frequency = frequency;
    unsigned int divisor = PIT_FREQUENCY / frequency;

    outb(PIT_CONTROL_PORT, 0x36 | 0x02);
    io_wait();
    outb(PIT_CHANNEL0_PORT, divisor & 0xFF);
    io_wait();
    outb(PIT_CHANNEL0_PORT, divisor >> 8);
    io_wait();
}