#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "debug/logger.h"
#include "limine/limine.h"
#include "graphics/tty.h"
#include "globals.h"
#include "boot.h"

#include "threads/scheduler.h"

// LIMINE REQUESTS ==============================================================================

__attribute__((used, section(".limine_requests"))) static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(5);
__attribute__((used, section(".limine_requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_executable_address_request address_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
    .revision = 0};


__attribute__((used, section(".limine_requests_start"))) static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

// =============================================================================================

static void hcf(void) {
    for (;;) {
        __asm__ ("hlt");
    }
}

static void panic(void) {
    __asm__ volatile ("ud2"); // Trigger invalid opcode error
}

// =============================================================================================

void thread_1() {
    while (1) {
        set_color(PROTO_BLUE);
        print("1");
        set_color(PROTO_WHITE);
    }
}
void thread_2() {
    while (1) {
        set_color(PROTO_RED);
        print("2");
        set_color(PROTO_WHITE);
    }
}

void k_main() {
    thread_t *t1 = create_kernel_thread(thread_1);
    thread_t *t2 = create_kernel_thread(thread_2);

    __asm__ volatile("sti");  // enable interrupts only after threads are ready
    
    // panic();
    for (;;) {
        __asm__ ("hlt");
    }
}

void k_early_main() {
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }
    
    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    struct limine_memmap_response *memmap = memmap_request.response;
    struct limine_hhdm_response *hhdm = hhdm_request.response;
    struct limine_executable_address_response *kaddr = address_request.response;

    if (k_init(framebuffer, memmap, hhdm, kaddr) == 0) {
        k_main();
    } else {
        k_error("k_init returned non-zero status code", "proto.kernel.k_early_main");
    }
}