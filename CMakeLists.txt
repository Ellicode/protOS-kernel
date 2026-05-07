# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 17)

project(kernel
        VERSION 0.1.0
        DESCRIPTION "A kernel for protOS"
        LANGUAGES C
)


set(KERNEL_SOURCES
        src/main.c
        include/main.h

        src/string.c
        include/string.h

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
)

add_executable(kernel ${KERNEL_SOURCES})

target_compile_options(kernel PRIVATE
        -m64
        -ffreestanding
        -fpie
        -fno-stack-protector
        -mno-red-zone
        -Wall
        -Wextra
)

target_include_directories(kernel PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_options(kernel PRIVATE
        -m64
        -pie
        -nostdlib
        -nostartfiles
        -nodefaultlibs
        "-Wl,-T,${CMAKE_SOURCE_DIR}/linker.ld"
        -Wl,--build-id=none
        -Wl,-z,max-page-size=0x1000
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)