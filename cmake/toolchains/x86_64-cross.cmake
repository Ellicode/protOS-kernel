set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Cross-compiler binaries provided by the gcc-x86_64-linux-gnu package on Fedora.
set(CMAKE_C_COMPILER   x86_64-linux-gnu-gcc)
set(CMAKE_AR           x86_64-linux-gnu-ar)
set(CMAKE_RANLIB       x86_64-linux-gnu-ranlib)
set(CMAKE_LINKER       x86_64-linux-gnu-ld)

# NASM is architecture-independent; the same binary works on any host.
set(CMAKE_ASM_NASM_COMPILER nasm)

# Skip the CMake compiler sanity-check link step. A freestanding / bare-metal
# binary can never pass the default hosted-C link test.
set(CMAKE_C_COMPILER_WORKS   TRUE CACHE BOOL "Cross-compiler assumed working" FORCE)
set(CMAKE_CXX_COMPILER_WORKS TRUE CACHE BOOL "Cross-compiler assumed working" FORCE)

# Do not look for host-native programs, libraries or headers when resolving
# find_program / find_library / find_path calls made for the target.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)