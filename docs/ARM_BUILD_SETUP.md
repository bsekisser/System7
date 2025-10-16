# Raspberry Pi ARM Build Setup Guide

This document explains how to set up and build System 7.1 Portable for Raspberry Pi 3, 4, and 5.

## Quick Start

### 1. Install ARM Cross-Compiler

For Raspberry Pi 3/4 (ARMv7-A 32-bit):

```bash
# Ubuntu/Debian
sudo apt-get install gcc-arm-linux-gnueabihf binutils-arm-linux-gnueabihf

# Fedora/RHEL
sudo dnf install arm-linux-gnu-gcc arm-linux-gnu-binutils

# macOS (with Homebrew)
brew install arm-linux-gnueabihf-binutils
brew install arm-none-eabi-gcc  # Alternative
```

### 2. Verify Installation

```bash
arm-linux-gnueabihf-gcc --version
arm-linux-gnueabihf-ld --version
```

Expected output: ARM cross-compiler version 9.0 or later

### 3. Build for Raspberry Pi

```bash
# Build ARM kernel
make PLATFORM=arm clean all

# Create bootable SD image
make PLATFORM=arm sd

# Test in QEMU emulation
make PLATFORM=arm run

# Debug with GDB
make PLATFORM=arm debug
```

## Build System Details

### Platform Configuration

The Makefile has been extended to support ARM builds alongside x86:

```makefile
PLATFORM ?= x86                    # Default x86, set to 'arm' for Raspberry Pi

# For ARM builds:
make PLATFORM=arm                  # Build ARM kernel
make PLATFORM=arm sd               # Create SD image
make PLATFORM=arm run              # Run in QEMU
```

### Compiler Flags

**ARM-specific (ARMv7-A):**
```
-march=armv7-a                     # ARMv7-A architecture
-mfpu=neon                         # NEON SIMD support
-mfloat-abi=hard                   # Hardware floating-point ABI
```

**Common to both platforms:**
```
-ffreestanding                     # Freestanding environment (no libc)
-fno-builtin                       # No built-in functions
-nostdlib                          # No standard library
-fno-pic -fno-pie                  # Position-independent code disabled
```

### Directory Structure

```
src/Platform/
├── x86/                           # x86 implementation
│   ├── platform_boot.S            # x86 reset vector (Multiboot2)
│   ├── linker.ld                  # x86 linker script
│   ├── hal_boot.c                 # x86 boot initialization
│   ├── io.c                       # x86 port I/O
│   ├── ata.c                      # ATA/IDE driver
│   └── ps2.c                      # PS/2 keyboard/mouse
└── arm/                           # ARM implementation (NEW)
    ├── platform_boot.S            # ARM reset vector & DTB handling
    ├── linker.ld                  # ARM linker script
    ├── hal_boot.c                 # ARM boot initialization
    ├── io.c                       # ARM I/O (MMIO stubs)
    ├── mmio.h                     # Memory-mapped I/O operations
    ├── mmio.c                     # MMIO implementation
    └── device_tree.c              # Device Tree parsing
```

## Build Output

### x86 Output (unchanged)
```
make iso          → system71.iso (bootable CD image)
make run          → Run in QEMU/bochs
```

### ARM Output (new)
```
make sd           → system71.img (raw SD card image, 10MB)
make run          → Run in QEMU ARM emulation (raspi3)
make debug        → Debug with GDB on QEMU
```

## Testing

### 1. QEMU ARM Emulation (Fastest)

Test without hardware:

```bash
# Build and run in QEMU
make PLATFORM=arm run

# With debugging
make PLATFORM=arm debug

# In another terminal, connect GDB
arm-linux-gnueabihf-gdb build/arm/bin/kernel.elf
(gdb) target remote localhost:1234
(gdb) break boot_main
(gdb) continue
```

### 2. Real Hardware (Raspberry Pi)

#### Prerequisites
- Raspberry Pi 3/4/5
- microSD card (minimum 8GB, class 10 recommended)
- microSD card reader
- USB power supply (2.5A for Pi 3, 3A for Pi 4/5)

#### Installation
```bash
# Create bootable SD card
make PLATFORM=arm sd

# Write to SD card (Linux/macOS)
sudo dd if=system71.img of=/dev/sdX bs=4M conv=fsync
# Replace sdX with actual device (e.g., sda, sdb)

# Safely eject
sudo eject /dev/sdX
```

#### First Boot
1. Insert SD card into Raspberry Pi
2. Connect HDMI/USB peripherals (keyboard/mouse)
3. Power on
4. Watch serial console for boot messages:
   ```
   [ARM] Boot initialization complete
   Welcome to Macintosh System 7.1
   ```

## Troubleshooting

### Build Issues

**Error: `arm-linux-gnueabihf-gcc: command not found`**
- Install ARM cross-compiler (see Installation section above)
- Verify PATH includes cross-compiler directory

**Error: `Cannot open linker script`**
- Ensure `src/Platform/arm/linker.ld` exists
- Check Makefile `HAL_DIR` configuration

**Compilation errors in ARM files**
- ARM assembler syntax differs from x86
- Check `.align` vs `.balign`, register naming conventions
- Review ARM EABI documentation

### Runtime Issues

**QEMU: `Illegal instruction` or CPU mismatch**
- Ensure QEMU supports ARM: `qemu-system-arm --version`
- Use compatible machine type: `raspi3`, `raspi2`, or `versatileab`

**Hardware: System doesn't boot**
1. Check serial console output (UART0 on GPIO pins 14/15)
2. Verify SD card is bootable (proper DTB/bootloader)
3. Try different power supply (may be insufficient)
4. Check for overclocking settings in `config.txt`

**Hardware: Freezes during boot**
- May be missing framebuffer initialization
- Check `device_tree_init()` is called with DTB from firmware
- Verify GPIO/UART initialization completed

## Platform-Specific Notes

### Raspberry Pi 3
- **CPU**: Broadcom BCM2835, ARMv7-A (4x 1.2GHz)
- **RAM**: 1GB LPDDR2
- **GPU**: VideoCore IV
- **Framebuffer**: Via GPU mailbox interface
- **Storage**: Micro SD card via SDHCI

### Raspberry Pi 4
- **CPU**: Broadcom BCM2711, ARMv7-A (4x 1.5GHz)
- **RAM**: 1-8GB LPDDR4
- **GPU**: VideoCore VI
- **Framebuffer**: Via GPU mailbox interface (higher resolution)
- **Storage**: Micro SD or eMMC via SDHCI

### Raspberry Pi 5
- **CPU**: Broadcom BCM2712, ARMv8-A (4x 2.4GHz)
- **RAM**: 4-8GB LPDDR5
- **GPU**: VideoCore VII
- **Framebuffer**: Via GPU mailbox interface (4K support)
- **Storage**: Micro SD or NVMe via PCIe

## Architecture Details

### Boot Sequence

1. **GPU Firmware** (Proprietary, closed-source)
   - Initializes DDR memory
   - Loads kernel from SD card at 0x8000
   - Parses bootloader config (`config.txt`)
   - Passes Device Tree Blob (DTB) in r2

2. **ARM Bootstrap** (`platform_boot.S`)
   - Disable interrupts (GIC)
   - Set up stack (64KB at high memory)
   - Clear BSS section
   - Call `boot_main()` with DTB address

3. **Boot Main** (`boot.c`)
   - Parse Device Tree (memory, GPIO, framebuffer)
   - Initialize HAL (hal_boot_init)
   - Set up platform (timer, serial, storage)
   - Jump to kernel main

4. **Kernel** (`main.c`)
   - Initialize memory manager
   - Start event system
   - Launch Finder desktop

### Memory Layout

```
0x00000000  ├─ GPU Firmware (unmapped to ARM)
            │
0x00008000  ├─ Kernel Image Start
            │   ├─ .boot (reset vector, DTB parsing)
            │   ├─ .text (code)
            │   ├─ .rodata (constants)
            │   ├─ .data (initialized globals)
            │   └─ .bss (uninitialized, cleared by bootstrap)
            │
~100MB      ├─ System Zone (memory manager)
            │   ├─ Master pointers
            │   ├─ Trap dispatcher
            │   └─ Low-memory globals
            │
~200MB      ├─ Application Zone
            │   ├─ Finder heap
            │   ├─ Application heaps
            │   └─ Free space
            │
~512MB      ├─ Stack grows downward
            │
0xFC000000  ├─ MMIO Peripherals (Pi 5 only)
            │
0x3F000000  └─ MMIO Peripherals (Pi 3/4)
            │   ├─ 0x3F200000 GPIO
            │   ├─ 0x3F201000 UART0
            │   ├─ 0x3F300000 SD controller
            │   ├─ 0x3F980000 USB HCD
            │   └─ ...
```

## Next Steps

### Phase 1 (Current)
- ✅ Build system integration (Makefile)
- ✅ Platform HAL stubs (boot, I/O)
- ✅ Linker script and bootstrap
- ✅ Device Tree parsing skeleton
- Status: **Infrastructure complete, ready for testing**

### Phase 2 (Next)
- [ ] VideoCore GPU framebuffer initialization
- [ ] SDHCI SD card driver
- [ ] USB HCD for keyboard/mouse
- [ ] ARM generic timer backend
- [ ] GPIO-based input fallback

### Phase 3 (Future)
- [ ] Performance tuning
- [ ] Pi 5 ARMv8-A 64-bit support
- [ ] Graphics acceleration (QPU shaders)
- [ ] Audio output via HDMI
- [ ] Network (Ethernet/WiFi)

## References

- **ARM EABI**: https://github.com/ARM-software/abi-aa/releases/download/2023Q3/abi.pdf
- **Broadcom BCM2835 Datasheet**: https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2835/
- **Device Tree Specification**: https://www.devicetree.org/specifications/
- **Raspberry Pi Firmware**: https://github.com/raspberrypi/firmware
- **QEMU ARM Emulation**: https://wiki.qemu.org/Documentation/Platforms/ARM
- **OSDev ARM**: https://wiki.osdev.org/ARM_architecture

## Support

For issues:
1. Check this guide's Troubleshooting section
2. Review build logs in `build/arm/`
3. Consult OSDev wiki for ARM architecture questions
4. Inspect Device Tree: `sudo dtdump /sys/firmware/devicetree/`
