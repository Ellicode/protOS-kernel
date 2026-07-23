add_library(proto STATIC
    src/core/malloc.c
    src/core/io.c
    src/core/syscall.c
    src/core/string.c
    src/core/process.c

    src/graphics/graphics.c
    src/graphics/bmp.c

    src/events/events.c
)

target_include_directories(proto PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

set_target_properties(proto PROPERTIES
    PUBLIC_HEADER "include/proto/proto.h"
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

target_link_libraries(crt0 PRIVATE proto)

target_compile_options(proto PRIVATE
    -m64
    -ffreestanding
    -fno-stack-protector
    -fno-builtin
)