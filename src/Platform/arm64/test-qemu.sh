#!/bin/bash
# Quick test script for QEMU virtio-gpu
killall -9 qemu-system-aarch64 2>/dev/null
make clean
make QEMU=1
timeout 5 qemu-system-aarch64 -M virt -cpu cortex-a53 -m 1G \
    -kernel build/kernel8.elf \
    -device virtio-gpu-device \
    -serial stdio \
    -display none 2>&1 | head -100
