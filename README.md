# System 7.1 Portable - Iteration 2

A portable implementation of Apple Macintosh System 7.1 for modern x86 hardware, bootable via GRUB2/Multiboot2.

## Features

- **Classic Mac OS Interface**: Authentic System 7.1 menu bar with rainbow Apple logo
- **Chicago Bitmap Font**: Pixel-perfect rendering of the classic Mac font
- **QuickDraw Graphics**: Core 2D graphics system implementation
- **Window Manager**: Foundation for classic Mac windowing system
- **Menu Manager**: Fully functional menu bar with File, Edit, View, and Label menus
- **PS/2 Input Support**: Keyboard and mouse input via PS/2 controller
- **Event Manager**: Classic Mac event handling system
- **Finder Integration**: Basic Finder desktop functionality

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
│   ├── ChicagoRealFont.c  # Chicago font rendering
│   ├── PS2Controller.c    # PS/2 input driver
│   ├── Finder/            # Finder implementation
│   ├── MenuManager/       # Menu system
│   ├── WindowManager/     # Window management
│   └── QuickDraw/         # Graphics primitives
├── include/               # Header files
├── System_Resources_Extracted/  # Original System 7.1 resources
│   ├── SICN/             # Small icons
│   ├── ICON/             # Icons
│   └── ...               # Other resources
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
✅ Chicago font rendering with proper spacing
✅ PS/2 controller initialized
✅ Event system framework in place

## Future Development

- [ ] Dropdown menu implementation
- [ ] Window dragging and resizing
- [ ] File system integration
- [ ] Application launching
- [ ] Dialog boxes
- [ ] Resource Manager with full .rsrc support
- [ ] Sound Manager
- [ ] AppleTalk networking

## License

This is a reimplementation project for educational and preservation purposes.
Original System 7.1 components are property of Apple Inc.

## Acknowledgments

Based on reverse engineering and analysis of original System 7.1 resources.
Chicago font extracted from authentic System 7.1 font resources.