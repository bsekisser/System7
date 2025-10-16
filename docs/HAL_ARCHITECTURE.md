# Hardware Abstraction Layer (HAL) Architecture

## Overview

The System 7 HAL provides a clean separation between platform-specific hardware operations and the core operating system implementation. This architecture enables porting to multiple hardware platforms (x86, ARM, PowerPC, etc.) without modifying the core System 7 code.

## Design Principles

1. **Complete Platform Abstraction**: Core System 7 code contains zero platform-specific code
2. **Pluggable Architecture**: New platforms are added by implementing the HAL interfaces
3. **Build-Time Selection**: Platform chosen via `make PLATFORM=<name>` at build time
4. **Interface Stability**: HAL interfaces designed to be stable across platform implementations

## HAL Interfaces

### 1. Boot Interface (`Platform/include/boot.h`)

Handles platform-specific initialization during system startup.

```c
void hal_boot_init(void);
```

**Responsibilities:**
- Early hardware initialization
- Platform-specific setup before kernel main
- Currently minimal (most init happens in kernel_main)

**x86 Implementation:** `Platform/x86/hal_boot.c`
- Stub implementation (serial and hardware init in kernel_main)
- Entry point via `Platform/x86/platform_boot.S` (Multiboot2)

### 2. I/O Interface (`Platform/include/io.h`)

Low-level I/O port operations for communicating with hardware.

```c
void     hal_outb(uint16_t port, uint8_t value);
uint8_t  hal_inb(uint16_t port);
void     hal_outw(uint16_t port, uint16_t value);
uint16_t hal_inw(uint16_t port);
void     hal_outl(uint16_t port, uint32_t value);
uint32_t hal_inl(uint16_t port);
```

**Responsibilities:**
- Hardware port I/O operations
- Platform may use memory-mapped I/O or port I/O as appropriate

**x86 Implementation:** `Platform/x86/io.c`
- Direct `inb`/`outb` x86 assembly instructions
- Used by: ATA driver, PS/2 controller, Sound Manager, QuickDraw (VGA)

**ARM Implementation Notes:**
- Will use memory-mapped I/O instead of port I/O
- Same function signatures, different underlying mechanism

### 3. Storage Interface (`Platform/include/storage.h`)

Block-level storage operations for file system access.

```c
typedef struct {
    uint32_t block_size;    // Typically 512 bytes
    uint64_t block_count;   // Total blocks on device
} hal_storage_info_t;

OSErr hal_storage_init(void);
OSErr hal_storage_shutdown(void);
int   hal_storage_get_drive_count(void);
OSErr hal_storage_get_drive_info(int drive_index, hal_storage_info_t* info);
OSErr hal_storage_read_blocks(int drive_index, uint64_t start_block,
                               uint32_t block_count, void* buffer);
OSErr hal_storage_write_blocks(int drive_index, uint64_t start_block,
                                uint32_t block_count, const void* buffer);
```

**Responsibilities:**
- Initialize storage subsystem
- Enumerate available drives
- Read/write blocks of data
- Abstract over ATA, SCSI, SD card, NVMe, etc.

**x86 Implementation:** `Platform/x86/ata.c`
- ATA/IDE PIO mode with LBA28 addressing
- Supports up to 4 devices (primary/secondary master/slave)
- Wraps ATA operations into HAL block interface

**ARM Implementation Notes:**
- Raspberry Pi: SD card controller
- Apple Silicon: NVMe or custom storage controller

### 4. Input Interface (`Platform/include/input.h`)

Generic input device polling and state queries.

```c
void    hal_input_poll(void);
void    hal_input_get_mouse(Point* mouse_loc);
Boolean hal_input_get_keyboard_state(KeyMap key_map);
```

**Responsibilities:**
- Poll input devices for events
- Query current mouse position
- Query keyboard state

**x86 Implementation:** `Platform/x86/hal_input.c`
- Wraps PS/2 keyboard and mouse driver
- Calls `PollPS2Input()`, `GetMouse()`, `GetPS2KeyboardState()`

**ARM Implementation Notes:**
- Raspberry Pi: USB HID via DWCOTG controller
- Apple Silicon: USB-C/Thunderbolt HID

## Directory Structure

```
src/Platform/
├── include/              # Platform-independent HAL interfaces
│   ├── boot.h
│   ├── io.h
│   ├── storage.h
│   └── input.h
├── x86/                  # x86 platform implementation
│   ├── platform_boot.S   # Multiboot2 entry point
│   ├── hal_boot.c        # Boot initialization
│   ├── io.c              # Port I/O operations
│   ├── ata.c             # ATA/IDE storage driver
│   ├── ps2.c             # PS/2 input driver
│   ├── hal_input.c       # Input HAL wrapper
│   └── linker.ld         # x86 linker script
├── arm/                  # ARM platform implementation (Raspberry Pi, QEMU virt)
└── ppc/                  # PowerPC platform (experimental scaffolding)
```

## Build System Integration

### Platform Selection

The `PLATFORM` variable determines which platform-specific code is compiled:

```makefile
PLATFORM ?= x86
HAL_DIR = src/Platform/$(PLATFORM)
```

### Source File Selection

Platform-specific sources are automatically included:

```makefile
C_SOURCES = src/boot.c \
            src/Platform/x86/io.c \
            src/Platform/x86/ata.c \
            src/Platform/x86/ps2.c \
            src/Platform/x86/hal_boot.c \
            src/Platform/x86/hal_input.c \
            # ... core sources ...

ASM_SOURCES = $(HAL_DIR)/platform_boot.S
```

### Linker Script

Each platform provides its own linker script:

```makefile
LD = ld
LDFLAGS = -melf_i386 -nostdlib -T $(HAL_DIR)/linker.ld
```

## Migration from Old Architecture

### Before HAL (monolithic x86 code)

```c
// Direct hardware access scattered throughout codebase
outb(0x3F8, 'A');  // Serial port
uint8_t status = inb(0x1F7);  // ATA status
ATA_Init();  // Direct ATA driver call
```

### After HAL (platform-agnostic)

```c
// Core code uses HAL interfaces
hal_outb(0x3F8, 'A');  // Via HAL I/O interface
uint8_t status = hal_inb(0x1F7);  // Via HAL I/O interface
hal_storage_init();  // Via HAL storage interface
```

## Porting to a New Platform

### Step 1: Create Platform Directory

```bash
mkdir -p src/Platform/newplatform
```

### Step 2: Implement HAL Interfaces

Create the following files in `src/Platform/newplatform/`:

1. **Boot Entry** (`platform_boot.S` or `.c`)
   - Platform-specific bootloader entry point
   - Set up initial stack
   - Call `boot_main()`

2. **Boot Initialization** (`hal_boot.c`)
   ```c
   #include "Platform/include/boot.h"
   void hal_boot_init(void) {
       // Platform-specific early init
   }
   ```

3. **I/O Operations** (`io.c`)
   ```c
   #include "Platform/include/io.h"
   void hal_outb(uint16_t port, uint8_t value) {
       // Platform-specific implementation
   }
   // ... implement all I/O functions
   ```

4. **Storage Driver** (`storage.c`)
   ```c
   #include "Platform/include/storage.h"
   OSErr hal_storage_init(void) {
       // Initialize platform storage controller
   }
   // ... implement all storage functions
   ```

5. **Input Driver** (`input.c`)
   ```c
   #include "Platform/include/input.h"
   void hal_input_poll(void) {
       // Poll platform input devices
   }
   // ... implement all input functions
   ```

6. **Linker Script** (`linker.ld`)
   - Memory layout for platform
   - Entry point definition
   - Section placement

### Step 3: Update Build System

Add platform to Makefile (no changes needed if using `PLATFORM` variable):

```bash
make PLATFORM=newplatform
```

### Step 4: Test and Verify

1. Boot test: Verify kernel entry and early init
2. I/O test: Verify serial output
3. Storage test: Verify disk reads/writes
4. Input test: Verify keyboard/mouse events
5. Integration test: Boot full System 7 desktop

## Current Platform Support

### x86 (Complete ✅)

- **Boot**: Multiboot2 via GRUB
- **I/O**: x86 port I/O (`in`/`out` instructions)
- **Storage**: ATA/IDE PIO mode
- **Input**: PS/2 keyboard and mouse
- **Graphics**: VESA framebuffer
- **Status**: Fully functional, tested in QEMU

### ARM (Planned)

#### Raspberry Pi
- **Boot**: GPU bootloader → `kernel7.img`
- **I/O**: Memory-mapped I/O
- **Storage**: EMMC SD card controller
- **Input**: USB HID via DWCOTG
- **Graphics**: Mailbox framebuffer

#### Apple Silicon
- **Boot**: m1n1 bootloader (Asahi Linux)
- **I/O**: Memory-mapped I/O
- **Storage**: NVMe or custom controller
- **Input**: USB-C HID
- **Graphics**: Display controller (reverse-engineered)

### PowerPC (Planned)

- **Boot**: Open Firmware
- **I/O**: Memory-mapped I/O
- **Storage**: ATA or SCSI
- **Input**: ADB or USB
- **Graphics**: ATI/NVIDIA framebuffer
- **Console**: Open Firmware `stdout` (initial logging support via `open_firmware.c`)
- **Memory Discovery**: Query `/memory` `reg` property to size RAM for the HAL
- **HAL API**: `hal_ppc_get_memory_ranges()` returns cached OF memory map entries for platform diagnostics
- **Framebuffer**: `ofw_get_framebuffer_info()` attempts to pull base/stride/geometry from the firmware display node

## Performance Considerations

### HAL Overhead

The HAL adds minimal overhead:
- **Inline candidates**: `hal_inb`, `hal_outb` compile to single instructions
- **Function call overhead**: ~2-3 cycles for storage/input APIs
- **No runtime dispatch**: Platform selected at compile time

### Optimization

Platform implementations should:
1. Use inline functions for hot paths (`io.h`)
2. Minimize abstraction layers
3. Leverage platform-specific hardware features
4. Cache frequently-accessed values

## Future Enhancements

### Planned HAL Extensions

1. **Graphics Interface** (`hal_graphics.h`)
   - Framebuffer initialization
   - Mode setting
   - Hardware acceleration hooks

2. **Timer Interface** (`hal_timer.h`)
   - High-precision timestamp counter
   - Microsecond delays
   - Timer interrupt registration

3. **Interrupt Interface** (`hal_interrupt.h`)
   - Interrupt controller setup
   - IRQ handler registration
   - Interrupt masking

4. **Network Interface** (`hal_network.h`)
   - Ethernet controller
   - Packet send/receive
   - MAC address query

## References

- **HAL Implementation Plan**: `docs/future/REFACTORING_PLAN.md`
- **Multi-Platform Porting Guide**: `docs/future/PORTING_PLAN.md`
- **x86 Platform Code**: `src/Platform/x86/`
- **HAL Interfaces**: `src/Platform/include/`

## Related Documentation

- **Segment Loader**: Works across all platforms via CPU backend abstraction
- **Build System**: Platform selection and conditional compilation
- **Boot Process**: Platform-specific entry to platform-agnostic kernel
