```
                             dP   MMP"""""YMM MP""""""`MM
                             88   M' .mmm. `M M  mmmmm..M
88d888b. 88d888b. .d8888b. d8888P M  MMMMM  M M.      `YM
88'  `88 88'  `88 88'  `88   88   M  MMMMM  M MMMMMMM.  M
88.  .88 88       88.  .88   88   M. `MMM' .M M. .MMM'  M
88Y888P' dP       `88888P'   dP   MMb     dMM Mb.     .dM
88                                MMMMMMMMMMM MMMMMMMMMMM
dP
```

# Welcome to ProtOS! (kernel)

ProtOS is a simple but powerful---

Hold up.

ProtOS is yet another hobby kernel made by an incompetent web developer wanting to suffer from the hell of osdev. This kernel is nowhere near as powerful, optimized, efficent, readable (and the list goes on) as any other operating system, but... it's working I guess?

- No I won't add super secret dark magic optimization stuff

- No I won't compress the code to the absolute limit to save 10MB of storage space

- Yes I will have a lot of fun learning operating system developement (and suffering)!

## Roadmap / File structure

- [x] Basic console output <span style="float:right">([`console.c`](src/graphics/console.c))</span>

**INTERRUPTS**

- [x] Global Descriptor Table <span style="float:right">([`gdt.c`](src/gdt.c))</span>
- [x] Interrupt Descriptor Table <span style="float:right">([`idt.c`](src/interrupts/idt.c))</span>
- [x] Simple PIC driver <span style="float:right">([`pic.c`](src/interrupts/pic.c))</span>
- [x] System panic and IRQ handling <span style="float:right">([`interrupts.c`](src/interrupts/interrupts.c))</span>
- [x] PS2 keyboard driver <span style="float:right">([`ps2.c`](src/drivers/ps2.c))</span>
- [x] Programmable Interrupt Timer <span style="float:right">([`pit.c`](src/drivers/pit.c))</span>

**MEMORY MANAGEMENT**

- [x] Bitmap PMM <span style="float:right">([`pmm.c`](src/memory/pmm.c))</span>
- [x] Freelist PMM <span style="float:right">([`freelist_pmm.c`](src/memory/freelist_pmm.c))</span>
- [x] Page tables setup & VMM <span style="float:right">([`vmm.c`](src/memory/vmm.c))</span>
- [x] Linked-List kernel heap <span style="float:right">([`heap.c`](src/memory/heap.c))</span>

**FILESYSTEMS**

- [x] Virtual Filesystem <span style="float:right">([`vfs.c`](src/filesystems/vfs.c))</span>
- [x] TAR filesystem

**USERSPACE**

- [x] Scheduler <span style="float:right">([`heap.c`](src/userspace/scheduler.c))</span>
- [x] ELF Loading
- [x] Syscalls

_after that i can finally work on stuff_

## Policy against AI-generated content

AI-generated code is not accepted within the OS. The challenge of this project is to learn, not to ship as fast as possible, so please don't.

## Contributing

I don't like working it teams, but i'm always happy to chat when i need help!

## Debugging

> [!NOTE]
> I use arch btw, so i didn't make any scripts to run for non-linux OSes like Windows and MacOS (macOS might work but yea). Good luck figuring it out!

### Dependencies

- QEMU
- OVMF (x86_64 UEFI emulator)
- CMake

To build and debug the kernel, you can either

1. Press the Start Debugging button in Visual Studio Code (`F5` key)
2. Run the following command in your terminal

```sh
./scripts/build-and-run.sh
```

3. Manually build the kernel and then run it in your emulator of choice

```sh
cmake -S . -B build
cmake --build build
```
