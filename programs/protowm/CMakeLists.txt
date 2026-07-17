add_executable(protowm
    $<TARGET_OBJECTS:crt0>
    src/main.c

    include/graphics.h
    src/graphics.c
)

target_include_directories(protowm PRIVATE include)

target_compile_definitions(protowm PRIVATE
    VERSION="1.0.0"
)

target_link_libraries(protowm PRIVATE proto)

target_compile_options(protowm PRIVATE
    -m64
    -ffreestanding
    -fno-stack-protector
)

target_link_options(protowm PRIVATE
    -nostdlib
    -nostartfiles
    -nodefaultlibs
    -no-pie
    -fno-stack-protector
    -T${CMAKE_SOURCE_DIR}/programs/protowm/linker.ld
)
