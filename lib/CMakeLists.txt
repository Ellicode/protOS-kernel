add_library(proto STATIC
    src/proto/core/malloc.c
    src/proto/core/io.c
    src/proto/core/syscall.c
    src/proto/core/string.c
    src/proto/core/process.c

    src/proto/graphics/graphics.c
    src/proto/graphics/bmp.c
    src/proto/graphics/color.c
    src/proto/graphics/putpixel.c
    src/proto/graphics/draw_text.c
    src/proto/graphics/draw_img.c
    src/proto/graphics/draw_rect.c
    src/proto/graphics/draw_box.c

    src/proto/events/events.c
    
    # WM

    src/wm/wm.c
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

target_compile_options(crt0 PRIVATE
    -m64
    -ffreestanding
    -fno-stack-protector
    -fno-builtin
)

target_compile_options(proto PRIVATE
    -m64
    -ffreestanding
    -fno-stack-protector
    -fno-builtin
)