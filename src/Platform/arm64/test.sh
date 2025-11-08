#!/bin/bash
# ARM64 Build and Test Script for macOS

set -e

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "ARM64 Platform Build and Test"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Check for ARM64 toolchain
echo ""
echo "Checking for ARM64 toolchain..."

if command -v aarch64-none-elf-gcc &> /dev/null; then
    TOOLCHAIN="aarch64-none-elf-"
    echo "✓ Found: aarch64-none-elf-gcc"
elif command -v aarch64-elf-gcc &> /dev/null; then
    TOOLCHAIN="aarch64-elf-"
    echo "✓ Found: aarch64-elf-gcc"
    export PREFIX="aarch64-elf-"
else
    echo "✗ ARM64 toolchain not found"
    echo ""
    echo "Install with:"
    echo "  brew install --cask gcc-aarch64-embedded"
    echo "or:"
    echo "  brew install aarch64-elf-gcc"
    exit 1
fi

${TOOLCHAIN}gcc --version | head -1

# Check for QEMU
echo ""
echo "Checking for QEMU..."

if command -v qemu-system-aarch64 &> /dev/null; then
    echo "✓ Found: qemu-system-aarch64"
    qemu-system-aarch64 --version | head -1
else
    echo "✗ QEMU not found"
    echo ""
    echo "Install with:"
    echo "  brew install qemu"
    exit 1
fi

# Build
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Building ARM64 kernel..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

make clean
make

if [ -f build/kernel8.img ]; then
    SIZE=$(stat -f%z build/kernel8.img)
    echo ""
    echo "✓ Build successful!"
    echo "  Output: build/kernel8.img (${SIZE} bytes)"
else
    echo "✗ Build failed"
    exit 1
fi

# Test
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Testing in QEMU (Raspberry Pi 3 emulation)"
echo "Press Ctrl-C to exit"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

sleep 1
make qemu
