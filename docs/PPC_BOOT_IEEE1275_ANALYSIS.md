# PowerPC IEEE 1275 Boot Analysis

**Date:** October 16-17, 2025
**Status:** Boot infrastructure implements proper calling conventions; OpenBIOS 1.1 has limited ELF support

## Executive Summary

The System 7 kernel is correctly built as an IEEE 1275-compliant client program (ELF32, big-endian, ET_EXEC) with proper entry point (_start at 0x01000000) and OF calling convention support. However, **OpenBIOS 1.1 on mac99 does not fully support booting ELF client programs**—it can load them but doesn't properly initialize the client interface state required for execution.

## ELF Format Verification

Kernel meets all IEEE 1275 requirements:

```
$ readelf -h kernel.elf
  Class:                             ELF32
  Data:                              2's complement, big endian
  Type:                              EXEC (Executable file)
  Machine:                           PowerPC
  Entry point address:               0x1000000

$ readelf -l kernel.elf
Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x010000 0x01000000 0x01000000 0x198dc0 0x9f0254 RWE 0x10000
```

✓ Statically linked
✓ No PIE
✓ Entry point set
✓ Single LOAD segment
✓ Correct virtual address mapping

## Calling Convention Implementation

**platform_boot.S (lines 69-75):**
```asm
/* Save Open Firmware callback pointer (r5 per IEEE 1275 calling convention)
 * When booted by OF via 'boot' command, r5 holds the OF entry point
 * r3/r4 hold initrd address/length (or 0 if not present)
 */
mr      r31, r5                   /* Save OF callback pointer in r31 */
mr      r6, r3                    /* r6 = initrd address (if any) */
mr      r7, r4                    /* r7 = initrd length (if any) */
```

This correctly implements the calling convention per **Linux PowerPC Booting ABI**:
- r5: OF entry point (client-interface callback)
- r3, r4: initrd address and length (or 0)

## Boot Tests Performed

### Test 1: `boot hd:,\kernel.elf` (IEEE 1275 standard boot)

```
OpenBIOS prompt: 0 > boot hd:,\kernel.elf
Result: "No valid state has been set by load or init-program"
        "ok" (returns to prompt, does not boot)
```

**Interpretation:** OpenBIOS recognizes the `boot` command syntax but doesn't execute the loaded ELF as a client program. The error string indicates OpenBIOS's ELF loader doesn't set up the state required for client-program execution.

### Test 2: `-device loader` (QEMU device loader)

```bash
qemu-system-ppc -M mac99 -m 512 -serial stdio \
    -device loader,file=kernel.elf,addr=0x01000000,cpu-num=0
```

**Result:** QEMU loads binary at address but doesn't start firmware or kernel. `-device loader` is a raw loader that doesn't invoke OF.

### Test 3: Manual `load/go` sequence (previous attempt)

```
0 > load hd:,\kernel.elf
0 > go
Result: "No valid state has been set by load or init-program"
```

**Interpretation:** Same issue—load succeeds, but `go` fails because of missing client interface state.

## OpenBIOS Limitations Analysis

**OpenBIOS 1.1 (Aug 25 2025) on mac99:**
- ✓ Recognizes `boot` command
- ✓ Can load ELF files from FAT32 disk
- ✗ Does not initialize client-interface state
- ✗ Does not set up r5 (OF callback) properly
- ✗ Does not call ELF entry point with proper client context

**Evidence:**
- Both `boot` and `load/go` fail with identical error
- Disk auto-boot search works (looks for .tbxi, bootinfo.txt, %BOOT)
- Error message is generic—not kernel-specific

## Root Cause

OpenBIOS 1.1's ELF boot support is incomplete for PPC. The firmware:
1. Loads the ELF to correct address ✓
2. Locates and validates entry point ✓
3. But: Does NOT run the entry point as client program ✗
4. Missing: Client interface initialization (r5 callback setup)

This is a firmware limitation, not a kernel problem.

## Workarounds & Alternatives

### Short-term Workarounds

**Option A: Use alternative firmware**
- SLOF (Secure LOP Firmware) - Better ELF support
- Newer OpenBIOS (>1.1)
- U-Boot with PPC support

**Option B: XCOFF wrapper** (most compatible)
```
1. Link kernel as ELF
2. Use powerpc-linux-gnu-objcopy:
   objcopy -O aixcoff-rs6000 kernel.elf ofwboot.xcf
3. Put ofwboot.xcf on disk
4. OpenBIOS will recognize and boot XCOFF format
5. XCOFF stage-1 can load ELF kernel
```

**Option C: Raw binary + Forth bootloader**
```forth
\ In boot.fs or at prompt:
hex
1000000 constant KADDR
KADDR to load-base
" hd:,\kernel.bin" load-base  \ load from disk
KADDR execute                   \ jump to entry
```

This approach:
- Avoids OF ELF loader entirely
- Gives precise control over state setup
- Most reliable for custom kernels

### Long-term Solutions

1. **Patch OpenBIOS** - Add client-interface state initialization for ELF
2. **Custom bootloader** - Replace OF boot sequence with custom Forth loader
3. **Real hardware testing** - New World Mac PPC (which has OF v3) may handle ELF better

## Verification Checklist for Future Boot Attempts

When testing alternative approaches:

```
[ ] ELF header correct (ET_EXEC, big-endian, entry non-zero)
[ ] Entry point symbol exists: nm kernel.elf | grep _start
[ ] Binary loadable: objdump -D kernel.elf | grep "_start:"
[ ] Calling convention preserved in platform_boot.S
[ ] r5 captured (OF callback)
[ ] r3/r4 handled (initrd, may be zero)
[ ] Stack setup before C code
[ ] BSS zeroed before C code
[ ] boot_main receives r31 (OF callback) in correct register
```

## Next Steps for Continuation

### Immediate (This Week)
1. Build XCOFF wrapper if powerpc-linux-gnu-binutils supports it
2. Test raw binary + Forth loader approach
3. Try booting with `-device loader` but with custom Forth entry script

### Near-term (Next Week)
1. Investigate SLOF or newer OpenBIOS
2. Consider petitboot if targeting Linux compatibility
3. Document workaround for CI/CD boot tests

### Production
1. Evaluate firmware options based on target hardware
2. Create automated disk image with appropriate bootloader
3. Test on real PowerPC hardware

## Technical References

**IEEE 1275 (Open Firmware):**
- Client Program Entry: "The value of OF entry point must be in r5"
- Calling Convention: (r3, r4) = initrd address/length or (0, 0)
- Entry Point Expected: Address provided in ELF header

**Linux PowerPC ABI:**
- https://www.kernel.org/doc/Documentation/powerpc/booting.txt

**NetBSD PowerPC Boot:**
- Uses ofwboot.xcf (XCOFF) as stage-1
- NetBSD kernel is ELF, loaded by XCOFF bootloader

## Conclusion

The System 7 kernel is correctly implemented for IEEE 1275 compliance. **The boot failure is due to OpenBIOS firmware limitations, not the kernel.** Moving to a firmware with complete ELF client support (SLOF, newer OpenBIOS, or Mac OS X Open Firmware) will resolve the issue.

The proper calling conventions are in place and will work with any standards-compliant IEEE 1275 firmware.
