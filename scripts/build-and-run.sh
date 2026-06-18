#!/bin/bash
set -e

DEBUG_MODE=0
for arg in "$@"; do
    case $arg in
        --debug) DEBUG_MODE=1 ;;
    esac
done

error_exit() {
    echo -e "$1"
    exit 1
}

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

ALLOCATED_MEMORY=256M
EXTRA_QEMU_ARGS="" # You can add extra arguments for QEMU here if needed

# Foreground colors
T_BLACK='\033[0;30m'
T_RED='\033[0;31m'
T_GREEN='\033[0;32m'
T_YELLOW='\033[0;33m'
T_BLUE='\033[0;34m'
T_MAGENTA='\033[0;35m'
T_CYAN='\033[0;36m'
T_WHITE='\033[0;37m'
T_DEFAULT='\033[0;39m'

# Background colors
B_BLACK='\033[0;40m'
B_RED='\033[0;41m'
B_GREEN='\033[0;42m'
B_YELLOW='\033[0;43m'
B_BLUE='\033[0;44m'
B_MAGENTA='\033[0;45m'
B_CYAN='\033[0;46m'
B_WHITE='\033[0;47m'
B_DEFAULT='\033[0;49m'

# Attributes
A_RESET='\033[0m'
A_BOLD='\033[1m'
A_DIM='\033[2m'
A_ITALIC='\033[3m'
A_UNDERLINE='\033[4m'

# Build QEMU debug args
if [ $DEBUG_MODE -eq 1 ]; then
    echo -e "${B_MAGENTA} DBG  ${A_RESET} Debug mode is on"
    EXTRA_QEMU_ARGS="$EXTRA_QEMU_ARGS -s -S"
fi

LOCAL_OVMF_CODE_PATH="ignore-scripts/ovmf/OVMF_CODE.fd"
REMOTE_OVMF_CODE_PATH="/usr/share/edk2/x64/OVMF_CODE.4m.fd"
BOOT_DIRECTORY="ignore-scripts/esp/"
BUILD_FILE_NAME="kernel.elf"

echo -e "${B_BLUE} INFO ${A_RESET} Starting build process..."

# CLEAN THIS SHIT BEFORE BUILDING BC IT FUCKING CRASHES EVERY TIME
# rm -r build/

cd "$PROJECT_ROOT" || error_exit "${B_RED} ERR! ${A_RESET} Failed to change directory"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# build standard c library
mkdir -p build/lib

gcc -m64 -ffreestanding -fno-builtin -nostdlib -c lib/io.c -o build/lib/io.o
gcc -m64 -ffreestanding -fno-builtin -nostdlib -c lib/syscall.c -o build/lib/syscall.o
gcc -m64 -ffreestanding -fno-builtin -nostdlib -c lib/graphics.c -o build/lib/graphics.o

ar rcs build/libproto.a     \
    build/lib/io.o          \
    build/lib/graphics.o          \
    build/lib/syscall.o

gcc -m64 -ffreestanding -fno-stack-protector -Ilib -no-pie -c tests/elf-loading/main.c -o build/executable.o
ld -T tests/elf-loading/linker.ld build/executable.o -Lbuild -lproto -o build/executable.elf

cp build/executable.elf tests/initramfs/System/Programs/executable.elf

mkdir -p ignore-scripts

echo -e "${B_GREEN}  OK  ${A_RESET} Build successful. Preparing boot directory..."

if [ -d "$BOOT_DIRECTORY" ]; then
    rm "$BOOT_DIRECTORY/$BUILD_FILE_NAME" 2>/dev/null || true # Remove existing BOOTX64.EFI if it exists, ignore error if it doesn't
    cp build/$BUILD_FILE_NAME $BOOT_DIRECTORY/$BUILD_FILE_NAME
else
    echo -e "${B_YELLOW} WARN ${A_RESET}: Boot directory not found. Creating required directories..."
    mkdir -p $BOOT_DIRECTORY
    cp build/$BUILD_FILE_NAME $BOOT_DIRECTORY/$BUILD_FILE_NAME
fi

mkdir -p ignore-scripts/esp

tar -cf ignore-scripts/esp/boot/initramfs.tar -C tests/initramfs .
echo -e "${B_GREEN}  OK  ${A_RESET} Initramfs generated successfully"

echo -e "${B_BLUE} INFO ${A_RESET} Build completed. Attempting to launch QEMU..."

if [ -f "$LOCAL_OVMF_CODE_PATH" ]; then
    echo -e "${B_BLUE} INFO ${A_RESET} Launching QEMU with local OVMF..."
else
    echo -e "${B_YELLOW} WARN ${A_RESET} OVMF_CODE.fd not found in the '.ovmf' directory"
    read -p "       Do you wish to move automatically $REMOTE_OVMF_CODE_PATH to the '.ovmf' directory? (y/n): " yn
    case $yn in
        [Yy]* ) mkdir -p ignore-scripts/ovmf && cp $REMOTE_OVMF_CODE_PATH $LOCAL_OVMF_CODE_PATH; echo -e "${B_GREEN}  OK  ${A_RESET} Moved.";;
        * ) echo -e "${B_YELLOW} WARN ${A_RESET} Cancelled. Run 'cp $REMOTE_OVMF_CODE_PATH $LOCAL_OVMF_CODE_PATH' manually if needed."; exit;;
    esac
fi

if [ $DEBUG_MODE -eq 1 ]; then
    echo "QEMU: waiting for GDB on port 1234"
fi

qemu-system-x86_64 \
    -m $ALLOCATED_MEMORY \
    -machine q35 \
    -cpu qemu64 \
    -drive if=pflash,format=raw,readonly=on,file=$LOCAL_OVMF_CODE_PATH \
    -drive if=ide,format=raw,file=fat:rw:ignore-scripts/esp \
    -net none \
    -serial stdio \
    -vga std \
    -no-reboot \
    -no-shutdown \
    $EXTRA_QEMU_ARGS \
    # -d int

echo -e "\n"