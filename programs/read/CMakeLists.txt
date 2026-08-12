add_executable(read
    $<TARGET_OBJECTS:crt0>
    src/main.c
)

target_include_directories(read PRIVATE include)

target_compile_definitions(read PRIVATE
    VERSION="1.0.0"
)

target_link_libraries(read PRIVATE proto)

target_compile_options(read PRIVATE
    -ffreestanding
    -fno-stack-protector
    -march=x86-64
    -O2
)

target_link_options(read PRIVATE
    -nostdlib
    -nostartfiles
    -nodefaultlibs
    -no-pie
    -fno-stack-protector
    -T${CMAKE_SOURCE_DIR}/programs/read/linker.ld
)
