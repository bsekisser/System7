# System 7.1 Reimplementation - Hot Mess 1

A clean room reimplementation of classic Macintosh System 7.1 for modern hardware.

## Release: Hot Mess 1

This release includes the double-click detection fix that resolves coordinate system handling issues.

### What's Included

- `system71.iso` - Bootable ISO image with System 7.1 reimplementation
- `test_disk.img` - 100MB HFS-formatted virtual disk with sample files
- `run.sh` - Quick launch script for QEMU

### Requirements

- QEMU (qemu-system-i386)
- 1GB RAM allocation
- Linux host (tested on Ubuntu/Debian-based systems)

### Quick Start

```bash
chmod +x run.sh
./run.sh
```

### Manual Launch

If you prefer to run QEMU manually:

```bash
qemu-system-i386 \
    -cdrom system71.iso \
    -drive file=test_disk.img,format=raw,if=ide \
    -m 1024 \
    -vga std \
    -serial file:/tmp/serial.log
```

### Usage Tips

- **Mouse capture**: Click inside the QEMU window to capture mouse
- **Release mouse**: Press `Ctrl+Alt+G` to release mouse capture
- **Debug output**: Check `/tmp/serial.log` for system debug messages
- **Shut down**: Use Apple Menu > Shut Down or Special > Shut Down

### Features

- Classic Mac OS Finder interface with desktop icons
- Window management with drag, close, and minimize
- Menu bar with Apple, File, Edit, View, Label, and Special menus
- HFS filesystem support with automatic volume mounting
- ATA/IDE disk driver for virtual disks
- PS/2 mouse and keyboard input
- TextEdit integration for .txt files
- About This Macintosh window
- Double-click to open folders and files

### Known Issues

This is an early development release ("Hot Mess 1"):
- Limited application support (mainly Finder and TextEdit)
- No scroll bars yet
- Some menu items are not fully implemented
- Graphics are basic VGA mode

### Development

Source code: https://github.com/Kelsidavis/System7
Git tag: `hot-mess-1`

### License

This is a clean room reimplementation created without access to original Apple source code.
