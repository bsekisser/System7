# ARM64 Platform Testing

## Build Status

✅ **Build successful** - 14KB kernel image

## Hardware Addresses

The current implementation uses Raspberry Pi hardware addresses:

### UART (PL011)
- Pi 3: `0x3F201000`
- Pi 4/5: `0xFE201000`
- QEMU virt: `0x09000000` (requires address adjustment)

### Mailbox
- Pi 3: `0x3F00B880`
- Pi 4/5: `0xFE00B880`

### GIC
- Pi 3: `0x40041000` (GICD), `0x40042000` (GICC)
- Pi 4/5: `0xFF841000` (GICD), `0xFF842000` (GICC)

## QEMU Testing Notes

### Raspberry Pi 3 Emulation

```bash
qemu-system-aarch64 -M raspi3b -kernel build/kernel8.img -serial stdio -display none
```

**Status**: UART auto-detection should work for this configuration.

### Generic ARM64 (virt machine)

For testing with QEMU's virt machine, UART address needs adjustment:

```bash
qemu-system-aarch64 \
  -M virt \
  -cpu cortex-a53 \
  -m 1G \
  -kernel build/kernel8.elf \
  -serial stdio \
  -display none
```

**Status**: Requires UART address override for virt machine (`0x09000000`).

## Real Hardware Testing

Copy `build/kernel8.img` to SD card boot partition.

For Raspberry Pi 4/5, add to `config.txt`:
```
arm_64bit=1
kernel=kernel8.img
```

## Build Output

- `build/kernel8.elf`: ELF format with symbols (83KB)
- `build/kernel8.img`: Raw binary for hardware (14KB)

## Implemented Features

- ✅ EL2→EL1 exception level transition
- ✅ UART auto-detection (Pi 3/4/5)
- ✅ ARM Generic Timer
- ✅ GICv2 interrupt controller
- ✅ MMU with 4GB identity mapping
- ✅ Cache management
- ✅ Device Tree Blob parser
- ✅ Mailbox communication
- ✅ Framebuffer initialization
- ✅ Hardware detection (CPU, memory, board)

## Next Steps for QEMU Support

1. Add UART address override for virt machine
2. Add device tree support for dynamic hardware detection
3. Create boot configuration for both raspi3b and virt machines
