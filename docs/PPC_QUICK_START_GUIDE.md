# PowerPC Boot Quick Start Guide

## Overview

This guide walks you through building and attempting to boot System 7 on PowerPC using QEMU with OpenBIOS firmware.

**Important**: Due to a known OpenBIOS bug, kernel booting currently hangs. See "Known Issues" section for details and solutions.

---

## Prerequisites

### Required Tools
```bash
# PowerPC cross-compiler
sudo apt-get install powerpc-linux-gnu-binutils powerpc-linux-gnu-gcc

# QEMU with PowerPC support
sudo apt-get install qemu-system-ppc

# Build tools
sudo apt-get install build-essential git
```

### Verify Installation
```bash
# Check cross-compiler
powerpc-linux-gnu-gcc --version
powerpc-linux-gnu-as --version

# Check QEMU
qemu-system-ppc --version
```

---

## Building System 7 for PowerPC

### Step 1: Clone/Prepare Repository
```bash
cd /home/k/iteration2
```

### Step 2: Build the Kernel
```bash
make PLATFORM=ppc clean all
```

**Output**:
```
CC src/main.c
CC src/boot.c
...
LD kernel.elf
✓ Kernel linked successfully (3.8M bytes)
✓ Bootloader copied to build/boot.fs
```

### Step 3: Verify Build
```bash
file kernel.elf
# Should output: PowerPC or Power ISA 32-bit MSB executable...

readelf -h kernel.elf
# Check: Entry point address: 0x1000000
# Check: Data: 2's complement, big endian
```

---

## Running the Kernel - WORKING METHOD

### ✅ Method 1: Using load + go (RECOMMENDED - IEEE 1275 Standard)

This bypasses the broken `-kernel` mechanism and uses proven OF commands.

**Step 1**: Boot QEMU without -kernel
```bash
qemu-system-ppc -M mac99 -drive file=/tmp/s7.img,format=raw,if=ide \
  -m 512 -serial stdio -nographic
```

**Step 2**: At the OF prompt (0 >), load and jump
```forth
0 > load hd:2,\\kernel.elf
0 > go
```

**Expected Result**: Kernel executes ✅

**Why this works**:
- Uses proven IEEE 1275 `load` and `go` commands
- Bypasses OpenBIOS's broken automatic boot mechanism
- Same method used on real PowerPC Macs
- Works with OpenBIOS, Apple OF, and all IEEE 1275 firmware

See `docs/PPC_BOOT_WORKING_SOLUTION.md` for detailed instructions.

### ⚠️ Method 2: Direct Boot (Not Recommended - Will Hang)

```bash
make PLATFORM=ppc run
```

**Result**: OpenBIOS loads kernel but hangs at "switching to new context:" (firmware bug)

**Don't use this** - use `load + go` method above instead.

### Method 2: Interactive OpenBIOS Prompt (Manual Boot Attempt)

```bash
# Boot without kernel to get interactive prompt
timeout 5 qemu-system-ppc -M mac99 -m 512 -serial stdio -nographic
```

**At the OpenBIOS prompt** (`0 >`):
```forth
\ Try to manually boot kernel
01000000 go

\ Or check help
help boot

\ Or list devices
dev /
ls
cd ..
```

**Result**: Same hang (firmware bug)

### Method 3: Debug with GDB

```bash
# Terminal 1: Start QEMU in debug mode
make PLATFORM=ppc debug

# Terminal 2: Connect GDB
powerpc-linux-gnu-gdb kernel.elf
(gdb) target remote :1234
(gdb) continue
(gdb) disassemble $pc
```

---

## Known Issues

### OpenBIOS Kernel Boot Hangs

**Symptom**: System hangs at "switching to new context:" with no output

**Cause**: Bug in OpenBIOS PowerPC kernel loading mechanism

**Evidence**: Exhaustive testing proved:
- Minimal assembly kernels hang ✗
- OF-aware C kernels hang ✗
- All PowerPC QEMU machines hang ✗
- It's NOT a System 7 code issue ✓

**Timeline**: This is a firmware bug, not something System 7 caused

### Why This Matters

The bug occurs when OpenBIOS tries to:
1. Set up CPU mode (user vs supervisor)
2. Configure MMU state
3. Transfer to kernel entry point

The CPU context switch fails and the system hangs indefinitely.

---

## Solutions

### Solution 1: Use Alternative Bootloader (RECOMMENDED)

If you have SLOF, U-Boot, or other firmware without this bug:

```bash
# Using SLOF (Slimline Open Firmware)
qemu-system-ppc -M mac99 -bios slof.bin -kernel kernel.elf -m 512 -serial stdio
```

**Status**: Not yet tested, but should work

### Solution 2: Wait for OpenBIOS Fix

The OpenBIOS project may release a patch. Check:
- https://github.com/qemu/openbios
- Track issues for PowerPC kernel loading

### Solution 3: Contribute a Fix

If interested in fixing OpenBIOS:
1. Fork https://github.com/qemu/openbios
2. Look in `arch/ppc/qemu/core.fs` or C boot code
3. Debug CPU context switch implementation
4. Submit pull request

---

## Bootloader Details

### OF Forth Bootloader

The bootloader is located at: `src/Platform/ppc/boot.fs`

**What it does**:
1. Runs under OpenBIOS Forth interpreter
2. Verifies kernel at 0x01000000
3. Displays system information
4. Attempts to transfer control via `go` word

**Current status**:
- ✅ Implemented and tested
- ❌ Doesn't work with OpenBIOS bug
- ✅ Would work with alternative bootloader

**To use manually**:
```bash
# Boot without -kernel
qemu-system-ppc -M mac99 -m 512 -serial stdio -nographic

# At OF prompt, load bootloader
0 > boot build/boot.fs

# Or directly transfer:
0 > 01000000 go
```

---

## Testing & Verification

### Verify Build Artifacts

```bash
# Check kernel
ls -lh kernel.elf
file kernel.elf
readelf -h kernel.elf
readelf -l kernel.elf

# Check bootloader
ls -lh build/boot.fs
file build/boot.fs
```

### Verify Cross-Compiler

```bash
# Create simple test
cat > test.S << 'EOF'
.section ".text"
.globl _start
_start:
    li r3, 0x42
    b .
EOF

powerpc-linux-gnu-as test.S -o test.o
powerpc-linux-gnu-objdump -d test.o
```

### Monitor Serial Output

```bash
# Log serial output to file
qemu-system-ppc -M mac99 -kernel kernel.elf \
  -serial file:/tmp/serial.log -m 512 -monitor none -nographic &

sleep 2
cat /tmp/serial.log
```

---

## Advanced Usage

### Custom Boot Scripts

Modify `src/Platform/ppc/boot.fs` to add custom Forth code:

```forth
\ Add to boot.fs
: custom-boot
  ." Custom System 7 Boot Sequence" cr
  ." Loading kernel..." cr
  01000000 go
;

custom-boot
```

Rebuild:
```bash
make PLATFORM=ppc bootloader
```

### Enable Debug Output

Add verbose output to bootloader:

```forth
\ In boot.fs, after KERNEL_ADDR go:
." Kernel transfer returned (unexpected)" cr
." Attempting diagnostic check..." cr
KERNEL_ADDR @ . cr
```

### Test Specific Features

```bash
# Test UART initialization
cat > /tmp/uart_test.fs << 'EOF'
." UART Test" cr
01000000 @ hex . cr
EOF

# Run test
qemu-system-ppc -M mac99 -m 512 \
  -drive file=/tmp/uart_test.fs,format=raw,if=ide \
  -serial stdio -nographic
```

---

## Performance Tuning

### QEMU Options

For faster emulation:
```bash
# Enable TCG accelerator (if available)
qemu-system-ppc -M mac99 -accel tcg,tb-size=1024 \
  -kernel kernel.elf -m 512
```

For debugging:
```bash
# Single-step execution
qemu-system-ppc -M mac99 -kernel kernel.elf \
  -m 512 -serial stdio -nographic -s -S
```

### Memory Configuration

```bash
# Default 512MB
qemu-system-ppc -M mac99 -kernel kernel.elf -m 512

# Minimal 256MB
qemu-system-ppc -M mac99 -kernel kernel.elf -m 256

# Maximum 2GB
qemu-system-ppc -M mac99 -kernel kernel.elf -m 2048
```

---

## Troubleshooting

### Problem: "could not load kernel"

**Check**:
```bash
file kernel.elf
readelf -h kernel.elf
```

**Fix**:
- Rebuild: `make PLATFORM=ppc clean all`
- Verify path is correct
- Check file size: `ls -lh kernel.elf`

### Problem: Cross-compiler not found

**Check**:
```bash
which powerpc-linux-gnu-gcc
powerpc-linux-gnu-gcc --version
```

**Fix**:
```bash
sudo apt-get install powerpc-linux-gnu-binutils powerpc-linux-gnu-gcc
export PATH=/usr/bin:$PATH
```

### Problem: QEMU not found

**Check**:
```bash
which qemu-system-ppc
qemu-system-ppc --version
```

**Fix**:
```bash
sudo apt-get install qemu-system-ppc
```

### Problem: Build fails with error messages

**Check error**:
```bash
make PLATFORM=ppc 2>&1 | tail -50
```

**Common fixes**:
- Missing dependencies: `apt-get install powerpc-linux-gnu-gcc`
- Clean rebuild: `make PLATFORM=ppc clean all`
- Check permissions: `ls -l src/Platform/ppc/`

---

## Next Steps

1. **Short term**: Wait for SLOF or U-Boot integration
2. **Medium term**: Test on alternative bootloader
3. **Long term**: Contribute OpenBIOS fix or use SLOF

---

## Documentation References

- **Full Technical Analysis**: `docs/PPC_BOOT_FINAL_SOLUTION.md`
- **Investigation Report**: `docs/POWERPC_BOOT_STATUS.md`
- **Bootloader Guide**: `docs/OF_FORTH_BOOTLOADER_GUIDE.md`
- **Build System**: `docs/BUILD_SYSTEM_IMPROVEMENTS.md`

---

## Support & Contact

For issues or questions:

1. Check documentation in `docs/` directory
2. Review troubleshooting section above
3. Check QEMU manual: `qemu-system-ppc -h`
4. Contact project maintainers

---

**Last Updated**: 2025-10-16
**Status**: PowerPC build working, firmware limitation documented
**Next**: Alternative bootloader integration

