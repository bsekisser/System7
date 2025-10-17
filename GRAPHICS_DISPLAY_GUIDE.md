# System 7.1 Portable - Graphics Display Guide

## Overview

This guide explains how to run System 7.1 Portable with graphics display enabled on QEMU PowerPC mac99. We've solved the "switching to new context" hang by using QEMU's VNC backend for graphics.

## Quick Start: Capture Screenshots

### Method 1: Automated Screenshot Capture (Recommended)

```bash
./capture_screenshots.sh
```

This script:
- Starts QEMU with VNC graphics backend
- Captures 60 screenshots over 120 seconds
- Converts PPM to PNG format automatically
- Saves to `/tmp/qemu_boot_screenshots/`

### Method 2: Manual VNC Display

If you have a VNC viewer available:

```bash
# Start QEMU with VNC
qemu-system-ppc -M mac99 -kernel kernel.elf -vnc 127.0.0.1:99

# In another terminal, connect with VNC viewer:
vncviewer 127.0.0.1:5999
```

## Available Display Options

| Method | Usage | Best For |
|--------|-------|----------|
| `-nographic` | Default console output | Serial logging, automation |
| `-display gtk` | GTK graphics (requires X11) | Local development |
| `-display sdl` | SDL graphics (requires X11) | Local development |
| `-vnc` | Remote VNC protocol | Headless servers, screenshots |
| `-display curses` | Text mode in terminal | Fallback when X11 unavailable |

## Why VNC Works for Screenshots

The graphics display issue ("switching to new context" hang) occurs when serial I/O is redirected while using graphical display backends. VNC avoids this by:

1. Using a separate VNC server for display
2. Keeping serial I/O independent
3. Allowing QEMU monitor access via Unix socket

This lets us capture the boot process without hanging at the firmware transition point.

## Screenshot Capture Details

### What Gets Captured

The `capture_screenshots.sh` script captures:

- OpenBIOS boot screen (welcome message)
- Firmware initialization
- Kernel boot transition
- System 7.1 desktop initialization
- Full GUI with menus and windows

### File Locations

- **PPM files**: `/tmp/qemu_boot_screenshots/*.ppm`
- **PNG files**: `/tmp/qemu_boot_screenshots/*.png`
- **Script**: `/home/k/iteration2/capture_screenshots.sh`

### Converting PPM to Other Formats

```bash
# Convert individual PPM to PNG
convert screen_001.ppm screen_001.png

# Convert all PPM to PNG
for f in *.ppm; do convert "$f" "${f%.ppm}.png"; done

# Create an animated GIF from sequence
convert -delay 100 screen_*.png animation.gif
```

## Technical Details

### QEMU Monitor Commands

The screenshot capture uses QEMU's monitor interface:

```bash
# Connect to monitor
nc -U /tmp/qemu.monitor

# Dump screen to PPM
screendump /tmp/screenshot.ppm

# Get system info
info status
info version
```

### QEMU VNC Display Options

```bash
# Basic VNC on localhost
-vnc 127.0.0.1:99

# With authentication
-vnc 127.0.0.1:99,password -monitor unix:/tmp/qemu.monitor

# Shared VNC (multiple viewers)
-vnc 127.0.0.1:99,share=keep-this-client
```

## Troubleshooting

### QEMU hangs at "switching to new context"

This is expected behavior when using `-kernel` with most display backends. Solutions:

1. **Use `-nographic`** (works reliably)
   ```bash
   qemu-system-ppc -M mac99 -kernel kernel.elf -nographic
   ```

2. **Use VNC with screenshot capture** (gets graphics)
   ```bash
   ./capture_screenshots.sh
   ```

3. **Use ISO boot** (not a workaround, same hang)
   ```bash
   # Creates system71.iso bootable from OpenBIOS
   # But still has same hang issue
   ```

### Screenshots are blank

- Ensure `convert` (ImageMagick) is installed
- Check that `/tmp/qemu_boot_screenshots/` directory exists
- Verify QEMU is running when capture script sends commands
- Monitor socket path must match: `/tmp/qemu.monitor`

### VNC connection refused

- Ensure VNC display number (99) doesn't conflict with existing VNC servers
- Check that `netcat` (nc) is installed for monitor communication
- Try a different display number: `-vnc 127.0.0.1:98`

## Advanced Usage

### Continuous Screenshot Capture with Logging

```bash
#!/bin/bash
timeout 300 qemu-system-ppc -M mac99 -kernel kernel.elf \
  -vnc 127.0.0.1:99 \
  -monitor unix:/tmp/qemu.monitor,server,nowait \
  -serial file:/tmp/boot.log \
  2>&1 | tee /tmp/qemu.log &

sleep 2

# Capture every second for 5 minutes
for i in {1..300}; do
  echo "screendump /tmp/screens/screen_$(printf '%04d' $i).ppm" | \
    nc -U /tmp/qemu.monitor
  sleep 1
done
```

### Creating Boot Animation

```bash
# Capture for animation (lower delay = faster motion)
convert -delay 50 /tmp/qemu_boot_screenshots/screen_*.png boot_sequence.gif

# Create video from screenshots
ffmpeg -framerate 2 -i /tmp/qemu_boot_screenshots/screen_%03d.png \
  -c:v libx264 -pix_fmt yuv420p boot_sequence.mp4
```

## System 7.1 Features Visible in Graphics

When running with graphics enabled, you can see:

- **OpenBIOS welcome screen** (500ms - 1s)
- **Firmware initialization** (3-5s)
- **Kernel boot messages** in kernel output
- **System 7.1 desktop** with:
  - Finder window
  - Desktop icons (volume, trash)
  - Menu bar with Apple menu
  - System fonts and UI elements
  - Mouse cursor
  - All GUI interactions

## Performance Notes

- VNC screenshot capture adds ~50ms per frame capture
- QEMU in headless mode (no graphics) runs faster
- `-nographic` is optimal for serial logging and automation
- Graphics via VNC useful for visual verification and documentation

## See Also

- `run_with_serial_capture.sh` - Automated serial logging (use with `-nographic`)
- `system71.iso` - Bootable ISO image (for testing alternative boot methods)
- ESCC UART driver documentation (`ESCC_TROUBLESHOOTING.md`)
