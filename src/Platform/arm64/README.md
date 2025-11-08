# ARM64 Platform Support

ARM64/AArch64 implementation for System 7.1 targeting Raspberry Pi 3/4/5 and QEMU.

## Features

- **Bootstrap**: EL2 to EL1 exception level transition
- **Serial I/O**: PL011 UART driver
- **Timing**: ARM Generic Timer with microsecond precision
- **Interrupts**: GICv2 Generic Interrupt Controller
- **Memory**: MMU with 4GB identity mapping
- **Cache**: Data and instruction cache management
- **Graphics**: Framebuffer driver via VideoCore mailbox
- **Hardware Detection**: Device Tree Blob parser

## Building

### Requirements

- `aarch64-none-elf-gcc` toolchain
- `qemu-system-aarch64` for testing

### Install Toolchain on macOS

```bash
brew install --cask gcc-aarch64-embedded
# or
brew install aarch64-elf-gcc
```

### Build

```bash
cd src/Platform/arm64
make
```

This produces `build/kernel8.img` (raw binary) and `build/kernel8.elf` (ELF format).

## Testing in QEMU

### Raspberry Pi 3 emulation

```bash
make qemu
```

### Generic ARM64 virtual machine

```bash
make qemu-virt
```

## Hardware Deployment

### Raspberry Pi 3

Copy `kernel8.img` to SD card boot partition.

### Raspberry Pi 4/5

Copy `kernel8.img` to SD card boot partition and add to `config.txt`:

```
arm_64bit=1
kernel=kernel8.img
```

## Memory Map

- `0x00000000 - 0x3FFFFFFF`: Normal cacheable memory (1GB)
- `0x40000000 - 0x7FFFFFFF`: Device memory - peripherals (1GB)
- `0x80000000 - 0xFFFFFFFF`: Normal cacheable memory (2GB)

Entry point: `0x80000`

## Architecture

- **platform_boot.S**: Assembly bootstrap, vector table
- **hal_boot.c**: Hardware initialization and detection
- **uart.c**: Serial console
- **timer.c**: System timer
- **gic.c**: Interrupt controller
- **mmu.c**: Virtual memory
- **cache.c**: Cache operations
- **mailbox.c**: VideoCore communication
- **framebuffer.c**: Graphics output
- **dtb.c**: Device Tree parsing
