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
#include "userspace/elf.h"

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

#define USER_STACK_BASE 0x7FFFF000ULL
#define USER_STACK_SIZE 0x100000ULL

void k_main() {
    file_descriptor_t *f = vfs_open(rootfs->root, "./system/programs/executable.elf", FD_READ);
    size_t size = f->inode->size;
    char *buffer = k_alloc(size);
    vfs_read(f, size, buffer);
    vfs_close(f);

    void *entry = elf_load(buffer, size);
    static uint8_t fault_stack[4096];
    tss.rsp0 = (uint64_t)(fault_stack + sizeof(fault_stack));

    vmm_map_range(USER_STACK_BASE, USER_STACK_SIZE, F_PRESENT | F_USER | F_WRITE);
    enter_userspace((uint64_t)entry, USER_STACK_BASE + USER_STACK_SIZE);

    k_info("Welcome to ", "proto.kernel.k_init");
    set_color(PROTO_CYAN);
    print_f("ProtOS");
    set_color(PROTO_WHITE);
    print_f("! System will halt...");

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