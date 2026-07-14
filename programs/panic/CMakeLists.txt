add_executable(panic
    $<TARGET_OBJECTS:crt0>
    src/main.c
)

target_include_directories(panic PRIVATE include)

target_compile_definitions(panic PRIVATE
    VERSION="1.0.0"
)

target_link_libraries(panic PRIVATE proto)

target_compile_options(panic PRIVATE
    -m64
    -ffreestanding
    -fno-stack-protector
)

target_link_options(panic PRIVATE
    -nostdlib
    -nostartfiles
    -nodefaultlibs
    -no-pie
    -fno-stack-protector
    -T${CMAKE_SOURCE_DIR}/programs/panic/linker.ld
)
