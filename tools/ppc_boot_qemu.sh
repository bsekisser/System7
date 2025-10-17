#!/bin/bash
# Automated PowerPC boot script for QEMU
# Sends boot commands to OpenBIOS to load and execute System 7 kernel

set -e

DISK_IMAGE="${1:-ppc_system71.img}"
KERNEL="${2:-kernel.elf}"

if [ ! -f "$DISK_IMAGE" ]; then
    echo "Error: Disk image not found: $DISK_IMAGE"
    exit 1
fi

if [ ! -f "$KERNEL" ]; then
    echo "Error: Kernel not found: $KERNEL"
    exit 1
fi

echo "================================================"
echo "System 7 PowerPC QEMU Boot"
echo "================================================"
echo "Disk:   $DISK_IMAGE"
echo "Kernel: $KERNEL"
echo ""
echo "Starting QEMU with OpenBIOS..."
echo "Boot commands will be sent automatically"
echo "================================================"
echo ""

# Create a named pipe for two-way communication
FIFO="/tmp/qemu_boot_$$.fifo"
mkfifo "$FIFO" 2>/dev/null || true

# Function to send commands to QEMU
send_command() {
    local cmd="$1"
    echo "$cmd" >> "$FIFO" &
    sleep 0.5
}

# Cleanup on exit
cleanup() {
    rm -f "$FIFO" 2>/dev/null || true
}
trap cleanup EXIT

# Start QEMU and feed commands via stdin
{
    # Wait for OpenBIOS prompt
    sleep 2

    # Send boot commands
    echo "load hd:,\\kernel.elf"
    sleep 1
    echo "go"
    sleep 5

    # Keep connection open for a while to see output
    sleep 10

} | qemu-system-ppc -M mac99 -m 512 -serial stdio -monitor none -nographic \
    -drive file="$DISK_IMAGE",format=raw,if=ide

echo ""
echo "Boot sequence completed"
