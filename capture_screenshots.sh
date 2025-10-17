#!/bin/bash
# Capture QEMU desktop screenshots throughout boot sequence

OUTDIR="/tmp/qemu_boot_screenshots"
mkdir -p "$OUTDIR"
rm -f "$OUTDIR"/*.ppm "$OUTDIR"/*.png

echo "Starting QEMU with VNC for screenshot capture..."
echo "This will capture 60 screenshots over 120 seconds"
echo ""

# Start QEMU with VNC backend and monitor socket
timeout 120 qemu-system-ppc -M mac99 -kernel kernel.elf \
  -vnc 127.0.0.1:99 \
  -monitor unix:/tmp/qemu.monitor,server,nowait \
  2>&1 &
QEMU_PID=$!

sleep 2

# Capture screenshots every 2 seconds (60 total)
for i in {1..60}; do
  printf "Capturing screenshot %d... " "$i"
  
  # Send screendump command via monitor socket
  (echo "screendump $OUTDIR/screen_$(printf '%03d' $i).ppm"; sleep 0.5) | \
    nc -U /tmp/qemu.monitor 2>/dev/null
  
  echo "✓"
  sleep 1.5
done

echo ""
echo "Stopping QEMU..."
kill $QEMU_PID 2>/dev/null
wait 2>/dev/null

echo ""
echo "=== Converting PPM to PNG ==="
for ppm in "$OUTDIR"/*.ppm; do
  png="${ppm%.ppm}.png"
  convert "$ppm" "$png" 2>/dev/null
done

echo ""
echo "=== Screenshot Summary ==="
PPMCOUNT=$(ls "$OUTDIR"/*.ppm 2>/dev/null | wc -l)
PNGCOUNT=$(ls "$OUTDIR"/*.png 2>/dev/null | wc -l)
echo "PPM files: $PPMCOUNT"
echo "PNG files: $PNGCOUNT"
echo "Screenshot directory: $OUTDIR"
echo ""

if [ $PNGCOUNT -gt 0 ]; then
  echo "First screenshot (OpenBIOS):"
  ls -lh "$OUTDIR"/screen_001.png
  echo ""
  echo "Latest screenshot:"
  ls -lh "$OUTDIR"/screen_*.png | tail -1
fi
