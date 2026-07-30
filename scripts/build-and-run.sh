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

ALLOCATED_MEMORY=1G
EXTRA_QEMU_ARGS="" # You can add extra arguments for QEMU here if needed

# Foreground colors =================================================================
T_BLACK='\033[0;30m'
T_RED='\033[0;31m'
T_GREEN='\033[0;32m'
T_YELLOW='\033[0;33m'
T_BLUE='\033[0;34m'
T_MAGENTA='\033[0;35m'
T_CYAN='\033[0;36m'
T_WHITE='\033[0;37m'
T_DEFAULT='\033[0;39m'

# Background colors =================================================================
B_BLACK='\033[0;40m'
B_RED='\033[0;41m'
B_GREEN='\033[0;42m'
B_YELLOW='\033[0;43m'
B_BLUE='\033[0;44m'
B_MAGENTA='\033[0;45m'
B_CYAN='\033[0;46m'
B_WHITE='\033[0;47m'
B_DEFAULT='\033[0;49m'

# Attributes =================================================================
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

# Cross-compilation: on aarch64 hosts (e.g. Asahi Linux on Apple Silicon) use
# the x86_64 cross-compiler toolchain automatically.
# Install the required packages first:
#   sudo dnf install gcc-x86_64-linux-gnu binutils-x86_64-linux-gnu nasm
TOOLCHAIN_ARG=""
HOST_ARCH="$(uname -m)"
CROSS_TOOLCHAIN="$PROJECT_ROOT/cmake/toolchains/x86_64-cross.cmake"
if [ "$HOST_ARCH" = "aarch64" ]; then
    echo -e "${B_BLUE} INFO ${A_RESET} Detected aarch64 host — using x86_64 cross-compilation toolchain"
    TOOLCHAIN_ARG="--toolchain $CROSS_TOOLCHAIN"
fi

LOCAL_OVMF_CODE_PATH="ignore-scripts/ovmf/OVMF_CODE.fd"
REMOTE_OVMF_CODE_PATH="/usr/share/edk2/x64/OVMF_CODE.4m.fd"
BOOT_DIRECTORY="ignore-scripts/esp/"
BUILD_FILE_NAME="kernel.elf"

echo -e "${B_BLUE} INFO ${A_RESET} Starting build process..."

# CLEAN THIS SHIT BEFORE BUILDING BC IT FUCKING CRASHES EVERY TIME
# rm -r build/

cd "$PROJECT_ROOT" || error_exit "${B_RED} ERR! ${A_RESET} Failed to change directory"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug $TOOLCHAIN_ARG
cmake --build build

cp build/programs/corgi/corgi initramfs/System/Programs/corgi
cp build/programs/protofetch/protofetch initramfs/System/Programs/protofetch
cp build/programs/ls/ls initramfs/System/Programs/ls
cp build/programs/protowm/protowm initramfs/System/Programs/protowm
cp build/programs/edit/edit initramfs/System/Programs/edit
cp build/programs/panic/panic initramfs/System/Programs/panic
cp build/programs/read/read initramfs/System/Programs/read

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

tar -cf ignore-scripts/esp/boot/initramfs.tar -C initramfs .
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

case $1 in
    * ) qemu-system-x86_64 \
            -m $ALLOCATED_MEMORY \
            -drive if=pflash,format=raw,readonly=on,file=$LOCAL_OVMF_CODE_PATH \
            -drive if=ide,format=raw,file=fat:rw:ignore-scripts/esp \
            -serial stdio \
            -display gtk,zoom-to-fit=on \
            # -device virtio-vga-gl \
            # -no-reboot \
            # -no-shutdown \
            # $EXTRA_QEMU_ARGS
esac

echo -e "\n"