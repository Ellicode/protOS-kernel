#include "string.h"
#include "graphics/console.h"
#include "memory/vmm.h"
#include "userspace/scheduler.h"

struct stack_frame {
    struct stack_frame * next_frame;
    void * rip;
};

#define MAX_DEPTH 20
void unwind_stack(void *rbp) {
    size_t depth = 0;
    struct stack_frame * frame = rbp;
    for (;
        vmm_virt_to_phys(g_current_thread->process->cr3, (uintptr_t)frame) != 0 &&
        vmm_virt_to_phys(g_current_thread->process->cr3, (uintptr_t)frame + sizeof(void*)) != 0 &&
        depth < MAX_DEPTH;
        depth++)
    {
        if (frame->rip == NULL) break;
        print_f("    %d - %x\n", depth, frame->rip);
        frame = frame->next_frame;
    }
    if (vmm_virt_to_phys(g_current_thread->process->cr3, (uintptr_t)frame) == 0 ||
        vmm_virt_to_phys(g_current_thread->process->cr3, (uintptr_t)frame + sizeof(void*)) == 0 ||
        frame->rip != NULL)
    {
        if (depth == MAX_DEPTH)
            print_f("     ...\n");
        else
            print_f("    %d - 0x????????????????\n", depth);
    }
}
