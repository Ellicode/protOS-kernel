#include "string.h"
#include "graphics/console.h"
#include "memory/vmm.h"
#include "userspace/scheduler.h"
#include "globals.h"
#include "debug/symbols.h"

struct stack_frame {
    struct stack_frame * next_frame;
    void * rip;
};

#define MAX_DEPTH 20
void unwind_stack(void *rbp) {
    uint64_t cr3;

    if (g_current_thread == NULL || g_current_thread->process == nullptr) {
        cr3 = (uint64_t)kernel_pml4 - g_lim_hhdm->offset;
    } else {
        cr3 = g_current_thread->process->cr3;
    }

    size_t depth = 0;
    struct stack_frame * frame = rbp;
    for (;
        vmm_virt_to_phys(cr3, (uintptr_t)frame) != 0 &&
        vmm_virt_to_phys(cr3, (uintptr_t)frame + sizeof(void*)) != 0 &&
        depth < MAX_DEPTH;
        depth++)
    {
        if (frame->rip == NULL) break;
        print_f("    %d - %x <%s>\n", depth, frame->rip, get_symbol((uint64_t)frame->rip));
        frame = frame->next_frame;
    }
    if (vmm_virt_to_phys(cr3, (uintptr_t)frame) == 0 ||
        vmm_virt_to_phys(cr3, (uintptr_t)frame + sizeof(void*)) == 0 ||
        frame->rip != NULL)
    {
        if (depth == MAX_DEPTH)
            print_f("     ...\n");
        else
            print_f("    %d - 0x????????????????\n", depth);
    }
}
