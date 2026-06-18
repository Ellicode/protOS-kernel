# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 23)

add_compile_definitions(PROTO_DEBUG=1)

project(kernel
        VERSION 0.1.0
        DESCRIPTION "A kernel for protOS"
        LANGUAGES C ASM_NASM
)

set(KERNEL_SOURCES
        src/main.c
        include/main.h

        src/string.c
        include/string.h

        src/io.c
        include/io.h

        src/globals.c
        include/globals.h

        src/boot.c
        include/boot.h

        src/gdt.c
        include/gdt.h

        src/pit.c
        include/pit.h

        include/utils/utils.h
        include/utils/linked_lists.h
        include/utils/ticket_lock.h

        # DEBUG ================================================================

        src/debug/logger.c
        include/debug/logger.h

        src/debug/serial.c
        include/debug/serial.h

        # GRAPHICS =============================================================

        src/graphics/console.c
        include/graphics/console.h

        src/graphics/vga.c
        include/graphics/vga.h

        # INTERRUPTS ===========================================================

        src/interrupts/idt.c
        include/interrupts/idt.h

        src/interrupts/interrupt-stub.asm

        src/interrupts/pic.c
        include/interrupts/pic.h

        src/interrupts/interrupts.c
        include/interrupts/interrupts.h

        # DRIVERS ===============================================================

        src/drivers/ps2.c
        include/drivers/ps2.h

        # MEMORY ================================================================

        src/memory/pmm.c
        include/memory/pmm.h

        src/memory/vmm.c
        include/memory/vmm.h

        src/memory/memory.c
        include/memory/memory.h

        src/memory/freelist_pmm.c
        include/memory/freelist_pmm.h

        src/memory/heap.c
        include/memory/heap.h

        # USERSPACE =============================================================

        src/userspace/scheduler.c
        include/userspace/scheduler.h

        src/userspace/elf.c
        include/userspace/elf.h

        src/userspace/syscalls.c
        include/userspace/syscalls.h 

        src/userspace/process.c
        include/userspace/process.h 

        src/userspace/user_fb.c
        include/userspace/user_fb.h

        src/userspace/enter-userspace.asm
        include/userspace/userspace.h 

        # FILESYSTEMS ===========================================================

        src/filesystems/vfs.c
        include/filesystems/vfs.h

        src/filesystems/tarfs.c
        include/filesystems/tarfs.h

        src/filesystems/devfs.c
        include/filesystems/devfs.h
)

add_executable(kernel ${KERNEL_SOURCES})
set_target_properties(kernel PROPERTIES
    OUTPUT_NAME "kernel.elf"
    SUFFIX ""
)

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

target_compile_options(kernel PRIVATE
    $<$<COMPILE_LANGUAGE:C>:-m64>
    $<$<COMPILE_LANGUAGE:C>:-ffreestanding>
    $<$<COMPILE_LANGUAGE:C>:-fno-stack-protector>
    $<$<COMPILE_LANGUAGE:C>:-mno-red-zone>
    $<$<COMPILE_LANGUAGE:C>:-Wall>
    $<$<COMPILE_LANGUAGE:C>:-Wextra>
    $<$<COMPILE_LANGUAGE:C>:-g>
)

target_include_directories(kernel PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_options(kernel PRIVATE
        -m64
        -nostdlib
        -nostartfiles
        -no-pie
        -nodefaultlibs
        "-Wl,-T,${CMAKE_SOURCE_DIR}/linker.ld"
        -Wl,--build-id=none
        -Wl,-z,max-page-size=0x1000
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)