# PowerPC System 7 Boot Testing Guide

## Quick Start

### Build Everything
```bash
cd /home/k/iteration2
make PLATFORM=ppc clean all    # Build kernel
make PLATFORM=ppc iso          # Create boot disk with ELF + binary
```

**Output:**
- `kernel.elf` - IEEE 1275 compliant ELF (3.9 MB)
- `kernel.bin` - Raw binary format (1.6 MB)
- `ppc_system71.img` - Bootable FAT32 disk (32 MB)

### Boot Methods Available

#### Method 1: Raw Binary Boot (Most Direct)
```bash
# Start QEMU
qemu-system-ppc -M mac99 -m 512 -serial stdio -monitor none -nographic \
    -drive file=ppc_system71.img,format=raw,if=ide

# At OpenBIOS "0 >" prompt, type:
hex
1000000 to load-base
" hd:,\kernel.bin" load
1000000 execute
```

#### Method 2: ELF Boot (IEEE 1275 Standard)
```bash
# At OpenBIOS "0 >" prompt:
boot hd:,\kernel.elf
```

#### Method 3: Via Makefile
```bash
# Test ELF boot
make PLATFORM=ppc disk-run

# Test binary boot (shows commands)
make PLATFORM=ppc boot-bin
```

## Boot Execution Flow

### Expected Output Sequence

**Stage 1: OpenBIOS Firmware**
```
>> =============================================================
>> OpenBIOS 1.1 [Aug 25 2025 18:10]
>> Configuration device id QEMU version 1 machine id 1
>> CPUs: 1
>> Memory: 512M
>> UUID: 00000000-0000-0000-0000-000000000000
>> CPU type PowerPC,G4
milliseconds isn't unique.
Welcome to OpenBIOS v1.1 built on Aug 25 2025 18:10
```

**Stage 2: Disk Detection**
```
Trying hd:,\\:tbxi...
Trying hd:,\ppc\bootinfo.txt...
Trying hd:,%BOOT...
No valid state has been set by load or init-program

0 >     # OpenBIOS prompt ready for commands
```

**Stage 3: Manual Boot Commands**
```
0 > hex
0 > 1000000 to load-base
0 > " hd:,\kernel.bin" load
0 > 1000000 execute
```

**Stage 4: Kernel Execution (Expected)**
```
S7 B        # Boot banner from platform_boot.S (direct UART output)
[System 7 initialization messages...]
```

## Diagnosing Boot Issues

### Issue: "Device needs media, but drive is empty"
- **Cause:** QEMU can't find the disk image file
- **Solution:**
  - Use absolute path: `/home/k/iteration2/ppc_system71.img`
  - Verify file exists: `ls -lh ppc_system71.img`
  - Check format: `file ppc_system71.img`

### Issue: Prompt doesn't appear
- **Cause:** OpenBIOS may be waiting on input or serial is misconfigured
- **Solution:**
  - Ensure `-serial stdio` is in QEMU command
  - Try with `-monitor none` explicitly
  - May take 10+ seconds to boot

### Issue: Load/execute returns "No valid state"
- **Expected Behavior:** This is a known OpenBIOS 1.1 limitation
- **Root Cause:** OpenBIOS doesn't initialize ELF client interface properly
- **Workaround:** Use raw binary method (kernel.bin)

### Issue: Kernel boots but stops/hangs
- **Possible Causes:**
  1. Kernel calling convention mismatch
  2. OF callback (r5) not set up correctly
  3. Stack or BSS not initialized
- **Debug:** Look for "S7 B\n\r" in serial output
  - Present = platform_boot.S executed ✓
  - Missing = never reached entry point ✗

## Detailed Command Reference

### OpenBIOS Forth Commands for Boot

**Set up load address:**
```forth
hex              \ Switch to hex number base
1000000 to load-base     \ Set where kernel will load
```

**Load kernel binary from disk:**
```forth
" hd:,\kernel.bin" load   \ Load kernel.bin from disk
```

**Execute kernel:**
```forth
1000000 execute          \ Jump to loaded kernel entry point
```

**Alternative boot (ELF, if OF supports it):**
```forth
boot hd:,\kernel.elf    \ Uses OF's ELF loader
```

## Verification Checklist

Before assuming boot succeeded, verify:

- [ ] QEMU starts without "Device needs media" error
- [ ] OpenBIOS displays boot banner within 15 seconds
- [ ] OF prompt "0 >" appears
- [ ] Commands execute without "unknown word" errors
- [ ] Kernel binary loads: " hd:,\kernel.bin" load returns "ok"
- [ ] Kernel executes: 1000000 execute is sent
- [ ] Serial output shows "S7 B\n\r" (or custom boot message)

## Understanding Boot Architecture

### Why Two Formats?

1. **kernel.elf** - Complete ELF with symbols, sections
   - Preferred by IEEE 1275 standard
   - Requires proper client interface setup
   - OpenBIOS 1.1 has incomplete support

2. **kernel.bin** - Raw binary (no headers)
   - Direct memory dump at load address
   - Works with simple loaders (Forth, custom)
   - More reliable on incomplete firmware

### Boot Flow (With Binary)

```
QEMU starts
    ↓
OpenBIOS initializes
    ↓
OpenBIOS shows prompt (0 >)
    ↓
User enters Forth commands
    ↓
Load command: kernel.bin → memory at 0x01000000
    ↓
Execute command: jump to 0x01000000
    ↓
platform_boot.S:_start executes
    ↓
UART writes "S7 B\n\r"
    ↓
OF early init runs
    ↓
boot_main() called
    ↓
System 7 kernel_main() starts
```

## Troubleshooting With Detailed Logging

### Capture Full Boot Output
```bash
# Save complete boot output
qemu-system-ppc -M mac99 -m 512 -serial file:/tmp/serial.log \
    -monitor none -nographic \
    -drive file=ppc_system71.img,format=raw,if=ide

# Send commands via stdin
cat << EOF | qemu-system-ppc ...
hex
1000000 to load-base
" hd:,\kernel.bin" load
1000000 execute
EOF

# Examine log
cat /tmp/serial.log
```

### QEMU Debug Mode
```bash
# Boot with GDB support
qemu-system-ppc -M mac99 -m 512 -serial stdio -s -S \
    -drive file=ppc_system71.img,format=raw,if=ide

# In another terminal
powerpc-linux-gnu-gdb kernel.elf
(gdb) target remote :1234
(gdb) continue     # or 'b _start' for breakpoint
```

## Known Firmware Issues

### OpenBIOS 1.1 (Current QEMU)
- ✗ Incomplete ELF client interface support
- ✓ Works with raw binary boot
- ✓ FAT32 file system support
- ✓ Device tree support

### Workarounds for OpenBIOS
1. Use raw binary (kernel.bin) - implemented
2. Upgrade to newer OpenBIOS
3. Use SLOF or alternative firmware
4. Create XCOFF wrapper

## Next Steps After Boot

Once System 7 kernel boots:
1. Observe startup messages on serial console
2. Verify system initialization completes
3. Check for expected services (desktop, finder, etc.)
4. Test with minimal System 7 application
5. Profile performance characteristics

## Reference Files

**Related Documentation:**
- `/home/k/iteration2/docs/PPC_BOOT_IEEE1275_ANALYSIS.md` - Technical analysis
- `/home/k/iteration2/src/Platform/ppc/boot.fs` - ELF boot script
- `/home/k/iteration2/src/Platform/ppc/boot_binary.fs` - Binary boot script
- `/home/k/iteration2/src/Platform/ppc/platform_boot.S` - Bootstrap assembly

**Build Artifacts:**
- `/home/k/iteration2/Makefile` - Build system
- `/home/k/iteration2/kernel.elf` - Built kernel
- `/home/k/iteration2/kernel.bin` - Binary conversion
- `/home/k/iteration2/ppc_system71.img` - Boot disk image
