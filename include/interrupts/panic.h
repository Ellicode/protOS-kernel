#ifndef PANIC_H
#define PANIC_H

#include "interrupts/interrupts.h"

void _panic_stub(char *ename, int is_frame, idt_frame_t *frame);
void panic(char *err);

#endif // PANIC_H