add_executable(ls
    $<TARGET_OBJECTS:crt0>
    src/main.c
)

target_include_directories(ls PRIVATE include)

target_compile_definitions(ls PRIVATE
    VERSION="1.0.0"
)

target_link_libraries(ls PRIVATE proto)

target_compile_options(ls PRIVATE
    -ffreestanding
    -fno-stack-protector
    -march=x86-64
    -O2
)

target_link_options(ls PRIVATE
    -nostdlib
    -nostartfiles
    -nodefaultlibs
    -no-pie
    -fno-stack-protector
    -T${CMAKE_SOURCE_DIR}/programs/ls/linker.ld
)
