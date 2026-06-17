#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "debug/logger.h"
#include "limine/limine.h"
#include "utils/utils.h"
#include "graphics/console.h"
#include "filesystems/vfs.h"
#include "globals.h"
#include "memory/heap.h"
#include "boot.h"
#include "userspace/scheduler.h"
#include "userspace/userspace.h"
#include "userspace/process.h"

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

__attribute__((used, section(".limine_requests"))) static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
    .revision = 0};

__attribute__((used, section(".limine_requests_start"))) static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;


// =============================================================================================

void k_main() {
    fill_screen(PROTO_BG);
    cursor_set(0, 0);

    create_process("./system/programs/executable.elf");

    enable_interrupts();

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
    struct limine_module_response *modules = module_request.response;

    if (k_init(framebuffer, memmap, hhdm, kaddr, modules) == 0) {
        k_main();
    } else {
        k_error("Init script returned non-zero status code. The system will reboot now...", "proto.kernel.k_early_main");
    }
}