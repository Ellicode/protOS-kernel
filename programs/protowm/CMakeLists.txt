add_executable(protowm
    $<TARGET_OBJECTS:crt0>
    src/main.c
    src/init.c
    src/cursor.c
    src/globals.c
    src/window.c
    src/hooks.c
)

target_include_directories(protowm PRIVATE include)

target_compile_definitions(protowm PRIVATE
    VERSION="1.0.0"
)

target_link_libraries(protowm PRIVATE proto)

target_compile_options(protowm PRIVATE
    -ffreestanding
    -fno-stack-protector
    -march=x86-64
    -O2
)

target_link_options(protowm PRIVATE
    -nostdlib
    -nostartfiles
    -nodefaultlibs
    -no-pie
    -fno-stack-protector
    -T${CMAKE_SOURCE_DIR}/programs/protowm/linker.ld
)
