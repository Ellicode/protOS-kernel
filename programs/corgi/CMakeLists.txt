add_executable(corgi
    $<TARGET_OBJECTS:crt0>
    src/main.c
    src/split.c
    
    src/tools/cd.c
)

target_include_directories(corgi PRIVATE include)

target_compile_definitions(corgi PRIVATE
    CORGI_VERSION="1.0.0"
)

target_link_libraries(corgi PRIVATE proto)

target_compile_options(corgi PRIVATE
    -ffreestanding
    -fno-stack-protector
    -march=x86-64
    -O2
)

target_link_options(corgi PRIVATE
    -nostdlib
    -nostartfiles
    -nodefaultlibs
    -no-pie
    -fno-stack-protector
    -T${CMAKE_SOURCE_DIR}/programs/corgi/linker.ld
)
