add_executable(terminal
    $<TARGET_OBJECTS:crt0>
    src/main.c
    src/console.c
)

target_include_directories(terminal PRIVATE include)

target_compile_definitions(terminal PRIVATE
    VERSION="1.0.0"
)

target_link_libraries(terminal PRIVATE proto)

target_compile_options(terminal PRIVATE
    -ffreestanding
    -fno-stack-protector
    -march=x86-64
    -O1
)

target_link_options(terminal PRIVATE
    -nostdlib
    -nostartfiles
    -nodefaultlibs
    -no-pie
    -fno-stack-protector
    -T${CMAKE_SOURCE_DIR}/programs/terminal/linker.ld
)
