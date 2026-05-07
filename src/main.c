#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "limine/limine.h"
#include "graphics/tty.h"
#include "boot.h"

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

__attribute__((used, section(".limine_requests_start"))) static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

// =============================================================================================

static void hcf(void) {
    for (;;) {
        __asm__ ("hlt");
    }
}

void k_main() {
    print("\nDovahkiin, dragonborn by ok zin los sworn to dein evil Mahfaeraak ahst vaal!\nand fin Norok fodro rout when nust hear zinddo zaan, dragonborn, fah hin kogaan Mu draal!\n");
    print("\n:3\n");

    hcf();
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

    k_init(framebuffer);

    k_main();
}