#!/bin/bash
# Build script for reproducible ISO generation
# Run inside Docker container or on host

set -e

echo "=== System 7.1 Reimplementation - Reproducible Build ==="
echo "Build date: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
echo "Working directory: $(pwd)"
echo ""

# Clean previous build
echo "Cleaning previous build artifacts..."
make clean
rm -f kernel.elf system71.iso

# Build kernel
echo "Building kernel..."
make all

# Generate ISO
echo "Generating bootable ISO..."
make iso

# Compute hashes
echo ""
echo "=== Build Artifacts ==="
ls -lh kernel.elf system71.iso

echo ""
echo "=== SHA-256 Checksums ==="
sha256sum kernel.elf system71.iso | tee build_hashes.txt

echo ""
echo "=== Verification ==="
echo "Compare these hashes with PROVENANCE.md to verify reproducibility."
echo "Expected hashes (will be updated after first verified build):"
echo "  kernel.elf:    TBD"
echo "  system71.iso:  TBD"

# Optional: Extract symbols for analysis
if command -v nm &> /dev/null; then
    echo ""
    echo "Extracting symbols..."
    nm -C kernel.elf > kernel_symbols.txt
    echo "Symbols written to kernel_symbols.txt"
fi

echo ""
echo "✅ Build complete!"
