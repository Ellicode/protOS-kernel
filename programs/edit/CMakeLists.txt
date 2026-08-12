add_executable(edit
    $<TARGET_OBJECTS:crt0>
    src/main.c
)

target_include_directories(edit PRIVATE include)

target_compile_definitions(edit PRIVATE
    VERSION="1.0.0"
)

target_link_libraries(edit PRIVATE proto)

target_compile_options(edit PRIVATE
    -ffreestanding
    -fno-stack-protector
    -march=x86-64
    -O2
)

target_link_options(edit PRIVATE
    -nostdlib
    -nostartfiles
    -nodefaultlibs
    -no-pie
    -fno-stack-protector
    -T${CMAKE_SOURCE_DIR}/programs/edit/linker.ld
)
