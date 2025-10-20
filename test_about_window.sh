#!/bin/bash
# Test script for About This Macintosh text rendering

echo "=== Starting System 7.1 test ==="
echo "Please follow these steps:"
echo "1. Wait for the system to boot (desktop appears)"
echo "2. Click on the Apple menu (top-left)"
echo "3. Click on 'About This Macintosh'"
echo "4. Press Ctrl+C to stop"
echo ""
echo "The serial output will be saved to /tmp/serial_about_test.log"
echo "Press Enter to start..."
read

# Clean up old log
rm -f /tmp/serial_about_test.log

# Start QEMU
qemu-system-i386 \
    -cdrom system71.iso \
    -drive file=test_disk.img,format=raw,if=ide \
    -m 1024 \
    -vga std \
    -serial file:/tmp/serial_about_test.log \
    -audiodev pa,id=snd0,server=/run/user/1000/pulse/native \
    -machine pcspk-audiodev=snd0 \
    -device sb16,audiodev=snd0

echo ""
echo "=== System stopped ==="
echo "Checking for About window messages..."
echo ""

if grep -q "WINDOW CREATED" /tmp/serial_about_test.log; then
    echo "✓ About window was created"
    echo ""
    echo "Window configuration:"
    grep -A15 "WINDOW CREATED" /tmp/serial_about_test.log
    echo ""
else
    echo "✗ About window was NOT created"
    echo ""
    echo "Possible causes:"
    echo "- Menu item not clicked"
    echo "- Menu handler not called"
    echo "- Window creation failed"
    echo ""
    echo "Recent menu activity:"
    tail -50 /tmp/serial_about_test.log | grep -i "menu\|about"
fi
