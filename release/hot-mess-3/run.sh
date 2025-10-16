#!/bin/bash
# System 7.1 Reimplementation - Hot Mess 3
# Quick launch script for QEMU (Linux)

set -e

echo "Starting System 7.1 Reimplementation (Hot Mess 3)..."
echo "Press Ctrl+Alt+G to release mouse capture"
echo "Serial debug output: /tmp/serial.log"
echo ""

qemu-system-i386 \
    -cdrom system71.iso \
    -drive file=test_disk.img,format=raw,if=ide \
    -m 1024 \
    -vga std \
    -serial file:/tmp/serial.log \
    -audiodev pa,id=snd0,server=/run/user/1000/pulse/native \
    -machine pcspk-audiodev=snd0
