add_executable(protofetch
    $<TARGET_OBJECTS:crt0>
    src/main.c
)

target_include_directories(protofetch PRIVATE include)

target_compile_definitions(protofetch PRIVATE
    VERSION="1.0.0"
)

target_link_libraries(protofetch PRIVATE proto)

target_compile_options(protofetch PRIVATE
    -m64
    -ffreestanding
    -fno-stack-protector
)

target_link_options(protofetch PRIVATE
    -nostdlib
    -nostartfiles
    -nodefaultlibs
    -no-pie
    -fno-stack-protector
    -T${CMAKE_SOURCE_DIR}/programs/protofetch/linker.ld
)
