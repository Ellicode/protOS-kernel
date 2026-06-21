add_library(proto STATIC
    io.c
    syscall.c
    graphics.c
    process.c
)

target_include_directories(proto PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(proto PRIVATE
    -m64
    -ffreestanding
    -nostdlib
    -fno-builtin
)
