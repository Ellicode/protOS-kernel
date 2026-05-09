# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 17)

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

        src/debug/logger.c
        include/debug/logger.h

        src/debug/serial.c
        include/debug/serial.h

        src/graphics/tty.c
        include/graphics/tty.h

        src/graphics/vga.c
        include/graphics/vga.h

        src/globals.c
        include/globals.h

        src/boot.c
        include/boot.h

        src/gdt.c
        include/gdt.h

        src/interrupts/idt.c
        include/interrupts/idt.h

        src/interrupts/interrupt-stub.asm

        src/interrupts/pic.c
        include/interrupts/pic.h

        src/interrupts/handlers.c
        include/interrupts/handlers.h

        src/pit.c
        include/pit.h

        src/memory.c
        include/memory.h

        src/drivers/ps2.c
        include/drivers/ps2.h
)

add_executable(kernel ${KERNEL_SOURCES})

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

target_compile_options(kernel PRIVATE
    $<$<COMPILE_LANGUAGE:C>:-m64>
    $<$<COMPILE_LANGUAGE:C>:-ffreestanding>
    $<$<COMPILE_LANGUAGE:C>:-fno-stack-protector>
    $<$<COMPILE_LANGUAGE:C>:-mno-red-zone>
    $<$<COMPILE_LANGUAGE:C>:-Wall>
    $<$<COMPILE_LANGUAGE:C>:-Wextra>
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