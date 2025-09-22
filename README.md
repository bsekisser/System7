# System 7.1 - Iteration 2

<img width="799" height="663" alt="working2" src="https://github.com/user-attachments/assets/0ca835e6-815f-4135-ab01-d7009c8c0d1a" />


An open implementation of Apple Macintosh System 7.1 for modern hardware, bootable via GRUB2/Multiboot2.

## Features

- **Classic Mac OS Interface**: Authentic System 7.1 menu bar with rainbow Apple logo
- **Desktop Icons**: Authentic Mac OS 7 hard drive icon with proper text rendering
- **Chicago Bitmap Font**: Pixel-perfect rendering of the classic Mac font
- **QuickDraw Graphics**: Core 2D graphics system implementation
- **Window Manager**: Foundation for classic Mac windowing system
- **Menu Manager**: Fully functional menu bar with File, Edit, View, and Label menus
- **PS/2 Input Support**: Keyboard and mouse input via PS/2 controller
- **Event Manager**: Classic Mac event handling system
- **Finder Integration**: Desktop functionality with volume icons

## Building

Requirements:
- GCC with 32-bit support
- GNU Make
- GRUB tools (grub-mkrescue)
- QEMU for testing

```bash
# Build the kernel
make

# Create bootable ISO
make iso

# Clean build artifacts
make clean
```

## Running

```bash
# Run in QEMU
qemu-system-i386 -cdrom system71.iso -serial stdio -display sdl -vga std -m 256M

# With debugging output to file
qemu-system-i386 -cdrom system71.iso -serial file:debug.log -display sdl -vga std -m 256M
```

## Project Structure

```
iteration2/
├── src/                    # Source code
│   ├── main.c             # Kernel entry point
│   ├── boot.S             # Multiboot2 boot assembly
│   ├── ChicagoRealFont.c  # Chicago font rendering
│   ├── PS2Controller.c    # PS/2 input driver
│   ├── Finder/            # Finder implementation
│   ├── MenuManager/       # Menu system
│   ├── WindowManager/     # Window management
│   ├── QuickDraw/         # Graphics primitives
│   ├── ResourceManager/   # Resource loading system
│   ├── EventManager/      # Event handling
│   ├── MemoryMgr/        # Memory management
│   ├── PatternManager/    # Pattern resources
│   ├── Resources/Icons/   # Icon resources (HD icon)
│   └── [30+ managers]     # Various system managers
├── include/               # Header files
├── System_Resources_Extracted/  # Original System 7.1 resources (69 types)
│   ├── SICN/             # Small icons
│   ├── ICON/             # Icons
│   ├── ppat/             # Pixel patterns
│   ├── PAT/              # Patterns
│   ├── MENU/             # Menu resources
│   ├── CURS/             # Cursors
│   └── [60+ folders]     # Other resource types
├── Makefile              # Build configuration
└── linker_mb2.ld         # Linker script for Multiboot2
```

## Technical Details

- **Architecture**: 32-bit x86
- **Boot Protocol**: Multiboot2
- **Graphics**: VESA framebuffer (800x600x32)
- **Font System**: Custom bitmap font renderer using authentic Chicago font data
- **Input**: PS/2 keyboard and mouse through port I/O
- **Memory Layout**: Starts at 1MB physical address

## Current Status

✅ Boots successfully via GRUB2
✅ Displays System 7.1 menu bar with rainbow Apple logo
✅ Authentic Mac OS 7 HD icon on desktop
✅ Chicago font rendering with proper spacing and kerning
✅ Desktop icon text with white background
✅ PS/2 keyboard and mouse support
✅ Event system framework in place
✅ Pattern resources (PAT) loaded from JSON
✅ Pixel pattern (ppat) backgrounds support
✅ Small icons (SICN) rendering support

## Future Development

- [ ] Dropdown menu implementation
- [ ] Window dragging and resizing
- [ ] File system integration
- [ ] Application launching
- [ ] Dialog boxes
- [ ] Resource Manager with full .rsrc support
- [ ] Sound Manager
- [ ] AppleTalk networking

## Legal

This is a reimplementation project for educational and preservation purposes.
Original System 7.1 and apple trademarks property of Apple Inc.

## Acknowledgments

Based on reverse engineering and analysis of original System 7.1 resources.
