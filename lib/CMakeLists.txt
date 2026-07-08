add_library(proto STATIC
    src/malloc.c
    src/io.c
    src/syscall.c
    src/graphics.c
    src/string.c
    src/process.c
    src/ipc.c
)

target_include_directories(proto PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(proto PRIVATE
    -m64
    -ffreestanding
    -nostdlib
    -fno-stack-protector
    -fno-builtin
)

add_library(crt0 OBJECT
    crt0.c
)

target_compile_options(crt0 PRIVATE
    -m64
    -ffreestanding
    -fno-stack-protector
    -fno-builtin
)