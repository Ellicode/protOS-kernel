/********************************************************************************
 * @file        main.c
 * @brief       Main entry point of the ProtOS kernel.
 * 
 * @author      Elliot Laborieux
 * @copyright   Copyright (c) 2026 Ellicode
 ********************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "debug/errors.h"
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

// MARK: Limine requests

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

// MARK: Methods

void k_main() {
    // Clear the buffer and homes the cursor to the origin [0, 0]
    term_clear_buffer();
    fill_screen(PROTO_BG);
    set_cursor(0, 0);

    // TODO: Add a proper init system instead of loading the shell executable
    create_process("/System/Programs/corgi", 1, NULL, NULL, 0);

    // Enable interrupts
    enable_interrupts();

    // Halt until the dawn of time... :3
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

void k_early_main() {
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == 0) {
        hcf();
    }
    
    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    struct limine_memmap_response *memmap = memmap_request.response;
    struct limine_hhdm_response *hhdm = hhdm_request.response;
    struct limine_executable_address_response *kaddr = address_request.response;
    struct limine_module_response *modules = module_request.response;

    // Run init script
    if (k_init(framebuffer, memmap, hhdm, kaddr, modules) == PROTO_OK) {
        k_main();
    } else {
        panic("Init script returned non-zero status code.");
    }
}