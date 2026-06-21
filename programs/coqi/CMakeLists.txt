add_executable(coqi.elf
    main.c
)

target_link_libraries(coqi.elf PRIVATE proto)

target_compile_options(coqi.elf PRIVATE
    -m64
    -ffreestanding
    -fno-stack-protector
)

target_link_options(coqi.elf PRIVATE
    -nostdlib
    -nostartfiles
    -nodefaultlibs
    -no-pie
    -T${CMAKE_SOURCE_DIR}/programs/coqi/linker.ld
)
