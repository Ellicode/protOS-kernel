/********************************************************************************
 * @file        pit.c
 * @brief       Basic Programmable Input Timer (PIT) utilities.
 * 
 * @author      Elliot Laborieux
 * @copyright   Copyright (c) 2026 Ellicode
 ********************************************************************************/

#include "utils/utils.h"
#include "globals.h"
#include "interrupts/pic.h"
#include "pit.h"

int g_pit_frequency = 0;
int g_pit_ticks = 0;

/**
 * Sends a byte of data to the PIT control port
 * 
 * @param value The data to send
 */
static inline void _pit_send_control(uint8_t value) {
    outb(PIT_CONTROL_PORT, value);
    io_wait();
}

/**
 * Sets the frequency of the channel 0 PIT
 * 
 * @param divisor The frequency divisor
 */
static inline void _pit_set_c0(uint64_t divisor) {
    outb(PIT_CHANNEL_0_PORT, divisor & 0xFF);
    io_wait();
    outb(PIT_CHANNEL_0_PORT, divisor >> 8);
    io_wait();
}


/**
 * Initializes Programmable Input Timer at a certain frequency, then unmasks its IRQ.
 * 
 * @param frequency Frequency of the timer in hertz
 */
void pit_init(int frequency) {
    g_pit_frequency = frequency;
    uint64_t divisor = PIT_FREQUENCY / frequency;

    _pit_send_control(0x36);
    _pit_set_c0(divisor);

    unmask_irq(0);
}