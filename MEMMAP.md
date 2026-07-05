# Virtual memory layout

## Lower half

The lower half of the virtual memory of protOS contains user-space related data. It starts at the begginning of the virtual memory (`0x0000000000000000`) and ends at address `0x00007FFFFFFFFFFF`

| Start                | End                  | Description                       |
| -------------------- | -------------------- | --------------------------------- |
| `0x0000000000400000` | `         ?        ` | Elf program                       |
| `0x0000100000000000` | `0x00001FFFFFFFFFFF` | Memory map region (heap included) |
| `0x0000200000000000` | `         ?        ` | User Framebuffer                  |
| `0x0000300000000000` | `0x0000300000100000` | User Stack (1 MiB)                |

## Upper half

The upper half of the virtual memory contains all kernel-space related data. It starts at address `0xFFFF800000000000` and ends at the end of the virtual memory (`0xFFFFFFFFFFFFFFFF`)

| Start                    | End                  | Description                    |
| ------------------------ | -------------------- | ------------------------------ |
| `0xFFFF800000000000`[^1] | `0xFFFF900000000000` | Limine HHDM (e.g: framebuffer) |
| `0xFFFFA00000000000`     | `0xFFFFB00000000000` | Kernel Heap (max ~4GB)         |
| `0xFFFFFFFF80000000`[^1] | `0xFFFFFFFFFFFFFFFF` | ProtOS Kernel Image            |

[^1]: These memory addresses are provided by the Limine bootloader.
