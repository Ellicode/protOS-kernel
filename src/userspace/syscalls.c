#include "graphics/console.h"

#include "userspace/syscalls.h"

void syscall_handler(idt_frame_t *frame) {
    print_f("syscall :3");
}