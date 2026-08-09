add_executable(testapp
    $<TARGET_OBJECTS:crt0>
    src/main.c
)

target_include_directories(testapp PRIVATE include)

target_compile_definitions(testapp PRIVATE
    VERSION="1.0.0"
)

target_link_libraries(testapp PRIVATE proto)

target_compile_options(testapp PRIVATE
    -m64
    -ffreestanding
    -fno-stack-protector
)

target_link_options(testapp PRIVATE
    -nostdlib
    -nostartfiles
    -nodefaultlibs
    -no-pie
    -fno-stack-protector
    -T${CMAKE_SOURCE_DIR}/programs/testapp/linker.ld
)
