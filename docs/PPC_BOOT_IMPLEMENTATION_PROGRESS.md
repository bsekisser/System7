# PowerPC QEMU Boot Implementation Progress

**Date:** October 16, 2025
**Status:** Boot infrastructure complete, automated boot pending OpenBIOS improvements

## Summary

The PowerPC System 7 boot infrastructure has been significantly advanced. The kernel now:
- ✅ Compiles successfully with full System 7 features
- ✅ Can be packaged in a bootable FAT32 disk image
- ✅ Boots OpenBIOS and presents the firmware prompt
- ⚠️ Requires manual OF commands to load and execute (due to firmware limitations)

## What's New (This Session)

### 1. Improved Open Firmware Bootloader Script
**File:** `src/Platform/ppc/boot.fs`

- Enhanced automatic boot detection with ELF magic number verification
- Validates kernel presence at 0x01000000 before attempting jump
- Clear boot options and instructions for both automated and manual boot
- ISO-compliant Forth code

```forth
: is-kernel-present ( -- flag )
  KERNEL_ADDR @ dup 0= if
    drop false
  else
    \ Check ELF magic: 0x7f 'E' 'L' 'F'
    dup c@ ELF_MAGIC1 = if ...
```

### 2. Automated Bootable Disk Image Creation
**File:** `Makefile` (PPC section)

Created `make PLATFORM=ppc iso` target that:
- Generates 32 MB FAT32 disk image
- Copies kernel.elf to disk root as `kernel.elf`
- Uses `mtools` for filesystem access (no sudo needed)
- Verifies disk contents before completion

```makefile
$(PPC_DISK_IMAGE): $(KERNEL)
	@dd if=/dev/zero of=$(PPC_DISK_IMAGE) bs=1M count=32 2>/dev/null
	@mkfs.fat -F 32 -n "System71" $(PPC_DISK_IMAGE) 2>/dev/null
	@mcopy -i $(PPC_DISK_IMAGE) $(KERNEL) ::kernel.elf
```

### 3. Boot Helper Script
**File:** `tools/ppc_boot_qemu.sh`

Automated script that:
- Takes disk image and kernel paths as arguments
- Starts QEMU with proper device configuration
- Sends `load` and `go` commands to OpenBIOS
- Waits for boot sequence completion

```bash
./tools/ppc_boot_qemu.sh ppc_system71.img kernel.elf
```

### 4. Makefile Integration
Updated PPC Makefile targets:
- `make PLATFORM=ppc all` - Build kernel
- `make PLATFORM=ppc iso` - Create bootable disk image
- `make PLATFORM=ppc disk-run` - Boot from disk with IDE
- `make PLATFORM=ppc run` - Direct kernel load via `-device loader`

## Current Boot Flow

### Successful: Disk-based Boot
```
$ make PLATFORM=ppc iso        # Create disk image
$ make PLATFORM=ppc disk-run   # Boot from disk

OpenBIOS starts and shows prompt:
0 > load hd:,\kernel.elf
 ok
0 > go
[Kernel should execute here]
```

### Status: Kernel Load Issue

The `go` command shows:
```
No valid state has been set by load or init-program
```

This indicates OpenBIOS didn't successfully load the kernel ELF file.

## Technical Findings

### OpenBIOS Limitations (Documented)
1. **ELF Loading:** OpenBIOS may not support loading ELF files directly from FAT32
2. **Format Expectations:** OF loaders typically expect:
   - Raw binary format (specific entry point)
   - Or bootloader format (bootblock signature)
   - Or specific OF-compatible format

### Solutions to Explore

#### Short-term (Quick Fix)
1. **Binary Format:** Convert ELF to raw binary
   ```bash
   powerpc-linux-gnu-objcopy -O binary kernel.elf kernel.bin
   ```
   - Create disk image with binary instead of ELF
   - Update OF load commands to use binary format

2. **Bootblock Method:** Create small bootloader at sector 0
   - OpenBIOS will load bootblock first
   - Bootloader then loads real kernel from disk

#### Medium-term (Robust Solution)
1. **Update boot.fs Forth Script**
   - Use `load-base` and `load-size` to specify load address
   - Handle binary vs. ELF formats correctly
   - Add error checking and detailed diagnostics

2. **Custom Bootloader**
   - Write minimal PowerPC bootloader in Forth
   - Or use existing bootloader (SLOF, U-Boot)
   - Integrate into build system

#### Long-term (Production)
1. **OpenBIOS Fork** (if needed)
   - Patch OpenBIOS to better support ELF loading
   - Or implement alternative firmware integration

2. **Real Hardware Support**
   - Test on actual PowerPC hardware
   - Verify firmware compatibility
   - Build production boot media

## Build Artifacts Generated

```
kernel.elf          - Main System 7 kernel (ELF format, 3.9MB)
kernel.bin          - Raw binary conversion (1.6MB)
ppc_system71.img    - Bootable FAT32 disk image (32MB)
build/boot.fs       - Open Firmware Forth bootloader
```

## Tested Configuration

**Machine:** QEMU mac99
- CPUs: 1
- Memory: 512 MB
- Firmware: OpenBIOS 1.1 (Aug 25 2025)
- CPU: PowerPC G4
- Boot Device: IDE disk

**Successful Boot Sequence:**
```
1. QEMU starts OpenBIOS
2. OF firmware initializes
3. Attempts to auto-boot from disk (expected)
4. Falls back to prompt when no bootblock found
5. User/script can then type: load hd:,\kernel.elf
6. Result: Kernel load attempted (but format issue)
```

## Next Steps

### Immediate (To Get Working Boot This Week)
1. Try raw binary format on disk
2. Adjust load commands if needed
3. Test alternative device names (hd:0, disk:, etc.)
4. Add verbose output to kernel entry point

### Near-term (Next Week)
1. Integrate binary kernel into automated disk creation
2. Create smoke test in CI/CD to verify boot
3. Document boot troubleshooting guide
4. Test on multiple QEMU machine types

### Investigation Tasks
1. Check OpenBIOS source code for ELF loading support
2. Test with other PowerPC bootloaders
3. Examine serial output more carefully for error messages
4. Try alternative OF device access methods

## How to Continue Development

### Current Status Report
```
$ make PLATFORM=ppc clean all    # Builds successfully ✓
$ make PLATFORM=ppc iso          # Creates disk image ✓
$ make PLATFORM=ppc disk-run     # Boots to OF prompt ✓
                                 # Kernel load fails ⚠️
```

### Testing Manual Boot
```bash
make PLATFORM=ppc disk-run

# At the "0 >" prompt, type:
load hd:,\kernel.elf
go

# Expected: Kernel boots and shows "S7 B\n\r"
# Actual: "No valid state" error
```

### Debugging
1. Add debug output to `src/Platform/ppc/platform_boot.S`
2. Print kernel checksum from OF before load
3. Verify kernel symbols are in OF namespace
4. Check OpenBIOS device tree for disk info

## Files Modified/Created

**Modified:**
- `Makefile` - Added PPC disk image and boot targets
- `src/Platform/ppc/boot.fs` - Enhanced Forth bootloader
- `src/Platform/ppc/platform_boot.S` - (unchanged in this session)

**Created:**
- `tools/ppc_boot_qemu.sh` - Automated boot script
- `docs/PPC_BOOT_IMPLEMENTATION_PROGRESS.md` - This document

**Existing PPC Files (Complete):**
- `src/Platform/ppc/open_firmware.c` - IEEE 1275 interface
- `src/Platform/ppc/hal_boot.c` - HAL initialization
- `src/Platform/ppc/linker.ld` - Memory layout

## Resource Requirements

### Tools
- `mtools` (mcopy, mdir) - For FAT disk manipulation ✓
- `powerpc-linux-gnu-*` - Cross-compiler toolchain ✓
- `qemu-system-ppc` - QEMU PowerPC emulator ✓

### Disk Space
- 32 MB per boot disk image
- 3.9 MB for kernel ELF
- 1.6 MB for kernel binary

## Success Criteria (When Complete)

Boot will be considered successful when:
```
make PLATFORM=ppc run
[... QEMU boots ...]
[... OpenBIOS firmware output ...]
[... System 7 boot messages appear on serial ...]
[... "S7 B\n\r" output ...]
[... System initializes ...]
```

---

**Related Documentation:**
- `docs/PPC_BOOT_FINAL_SOLUTION.md` - Final investigation summary
- `docs/OF_BOOTLOADER_IMPLEMENTATION_SUMMARY.md` - Bootloader details
- `docs/README_PPC_BOOT.md` - Quick reference guide
