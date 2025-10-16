# System 7.1 Reimplementation – Hot Mess 3

A clean-room reimplementation of classic Macintosh System 7.1 for modern hardware.

## Release: Hot Mess 3

This drop focuses on two big quality-of-life improvements:

- **Reliable typing everywhere** – the PS/2 translation path now mirrors the original ADB key codes, so SimpleText shows the right characters and the caret updates instantly.
- **Early Raspberry Pi plumbing** – the build now ships with portable halt helpers and ARM HAL stubs, laying the groundwork for Pi-class boards while keeping the x86 flow unchanged.

### What's Included

- `system71.iso` – Bootable ISO image with the latest System 7.1 reimplementation
- `test_disk.img` – 100 MB HFS-formatted virtual disk with sample files
- `run.sh` – Quick launch script for QEMU on Linux hosts

### Requirements

- QEMU (tested with `qemu-system-i386`)
- 1 GB RAM allocation
- Linux host (Ubuntu/Debian-based systems recommended)

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
    -serial file:/tmp/serial.log \
    -audiodev pa,id=snd0,server=/run/user/1000/pulse/native \
    -machine pcspk-audiodev=snd0
```

### Usage Tips

- **Mouse capture**: Click inside the QEMU window to capture the mouse
- **Release mouse**: Press `Ctrl+Alt+G`
- **Debug output**: Check `/tmp/serial.log` for system debug messages
- **Shut down**: Apple Menu → Shut Down

### Highlights in Hot Mess 3

- Correct PS/2 → Mac key translation with left/right modifiers, keypad, and function keys
- SimpleText and other Toolbox clients now redraw the caret immediately on every key press
- Portable `platform_halt()` helper used across x86 and ARM builds
- ARM build pipeline gains boot/init stubs so cross-compiling with `make PLATFORM=arm` is possible

### Known Issues

This is still an early development snapshot:
- USB input isn’t hooked up yet on ARM builds (stubs return neutral state)
- Many menu items remain placeholders
- Graphics stay in classic VGA mode

### Development Notes

Source code: https://github.com/Kelsidavis/System7
Git tag: `hot-mess-3`

### License

This project is a clean-room reimplementation created without access to Apple’s original source code. Use at your own risk.
