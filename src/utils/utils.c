/********************************************************************************
 * @file        utils.c
 * @brief       Miscellaneous I/O and other assembly utilities
 * 
 * @author      Elliot Laborieux
 * @copyright   Copyright (c) 2026 Ellicode
 ********************************************************************************/

#include <stdint.h>

#include "utils/utils.h"

/**
 * Send a single byte of data to a specific hardware I/O port
 * 
 * @param port  The port to output data to
 * @param value The value to output
 */
void outb(port_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

/**
 * Reads a single byte from a specific hardware I/O port
 * 
 * @param port  The port to read data from
 * 
 * @return      8 bit value from the port
 */
uint8_t inb(port_t port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port));
    return ret;
}

/**
 * Small delay to wait for X operation to complete
 */
void io_wait() {
    outb(0x80, 0);
}

/**
 * Halt and catch fire (endless loop)
 */
void hcf() {
    for (;;) {
        __asm__ ("hlt");
    }
}

/**
 * Read from a MSR (Model-Specific Register)
 * 
 * @param id    Identifier of the MSR
 * 
 * @return      Contents of the MSR
 */
uint64_t rdmsr(msr_id_t id) {
    uint32_t low;
    uint32_t high;
    __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(id));
    
    return ((uint64_t)high << 32) | low;
}

/**
 * Write to a MSR
 * 
 * @param id    Identifier of the MSR
 * @param value Value to write to the MSR
 */
void wrmsr(msr_id_t id, uint64_t value) {
    uint32_t low = (uint32_t)(value & 0xFFFFFFFF);
    uint32_t high = (uint32_t)(value >> 32);
    __asm__ volatile ("wrmsr" :: "c"(id), "a"(low), "d"(high));
}