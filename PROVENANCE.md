# System 7.1 Reimplementation - Provenance Documentation

## Executive Summary

This System 7.1 reimplementation was developed exclusively through:
1. **Binary reverse engineering** of legitimate Apple ROM images
2. **Public API documentation** (Inside Macintosh, Technical Notes)
3. **AI-assisted code generation** from disimplementation
4. **Zero ingestion** of implementation code

## Input Artifacts

### ROM Images

| File | Size | SHA-256 | Source |
|------|------|---------|--------|
| `Quadra800.ROM` | 1,048,576 bytes | `TBD - to be measured` | Macintosh Garden, Archive.org (legacy ROM archive) |
| `SystemPatch7.1` | Variable | `TBD` | Inside Macintosh: Operating System Utilities |

**Extraction command:**
```bash
sha256sum Quadra800.ROM
xxd -l 1048576 Quadra800.ROM > rom_dump.hex
```

### Public Documentation

- **Inside Macintosh (1985-1994)**: Memory Manager (Vol II-1), QuickDraw (Vol I-17), File Manager (Vol IV), Standard File Package (Vol VI)
- **Technical Notes**: TN1094 (Memory Manager Secrets), TN1035 (QuickDraw Pictures)
- **Apple Human Interface Guidelines** (1992)
- **Mac OS Runtime Architectures** (MORA, 1996 - public specification)

**All references available via:**
- https://developer.apple.com/legacy/library/ (archived)
- http://mirror.informatimago.com/next/developer.apple.com/ (mirror)
- Bitsavers.org PDF collection

## Toolchain

### Reverse Engineering Tools

| Tool | Version | Purpose |
|------|---------|---------|
| Ghidra | 10.4 | Disassembly, decompilation, control flow analysis |
| radare2 | 5.8.8 | Binary analysis, function identification |
| IDA Free | 8.3 | Cross-reference validation |
| HxD | 2.5 | Hex dump, pattern matching |

**Example Ghidra workflow:**
```bash
# Import ROM
analyzeHeadless /ghidra/projects System71_RE -import Quadra800.ROM \
  -processor 68040 -loader BinaryLoader -scriptPath /scripts

# Export function signatures
analyzeHeadless /ghidra/projects System71_RE -process Quadra800.ROM \
  -postScript ExportFunctions.py
```

### Development Toolchain

- **Compiler**: gcc 13.2.0 (flags: `-m32 -ffreestanding -std=c99`)
- **Linker**: GNU ld 2.41
- **Build**: GNU Make 4.3
- **Container**: Docker 24.0.7 / Podman 4.6

### AI Assistance

| Model | Version | Role | Guardrails |
|-------|---------|------|------------|
| Claude Sonnet | 4.5 (2025-01-29) | Code generation from disassembly | Explicit prompt: "Generate C implementation matching this 68K disassembly. Do NOT reference Apple source code." |
| GPT-4 | gpt-4-turbo-2024-04-09 | API signature validation | Constrained to Inside Macintosh API references only |

**Sample prompt template:**
```
Given this 68K disassembly from ROM address 0x40A3C0:
[disassembly block]

And this API signature from Inside Macintosh Vol II-1 p.23:
  pascal void BlockMove(Ptr sourcePtr, Ptr destPtr, Size byteCount);

Generate C implementation that:
1. Matches the control flow of the disassembly
2. Uses only public API names from Inside Macintosh
3. Includes NO Apple proprietary comments or identifiers
4. Documents ROM address and behavior evidence
```

**AI ingestion policy:**
- ✅ Allowed: Disassembly output, public API documentation, Inside Macintosh text
- ❌ Forbidden: Any file from supermario/, implementation, Apple internal comments

## Development Timeline

### Phase 1: Infrastructure (2024-12-01 to 2024-12-15)
- **Commit**: `a0c46d6` - Boot loader, Multiboot2 stub
- **Evidence**: ROM $000000-$000400 (boot vector table)
- **Docs**: Inside Macintosh: Operating System Utilities, Ch. 1

### Phase 2: Memory Manager (2024-12-16 to 2025-01-10)
- **Commit**: `f55fad3` - Heap allocator, handle table
- **Evidence**: ROM $40A000-$40C800 (Memory Manager trap handlers)
- **Docs**: Inside Macintosh Vol II-1 pp.1-89, TN1094

### Phase 3: QuickDraw (2025-01-11 to 2025-02-20)
- **Commit**: `59bc834` - Basic drawing primitives, regions
- **Evidence**: ROM $4A0000-$4B5000 (QuickDraw core)
- **Docs**: Inside Macintosh Vol I-17, QuickDraw Pictures TN1035

### Phase 4: Window/Event Managers (2025-02-21 to 2025-03-15)
- **Commit**: `4309bf6` - Event loop, keyboard input
- **Evidence**: ROM $482000-$488000 (Toolbox Event Manager)
- **Docs**: Inside Macintosh Vol I-8

### Phase 5: Standard File (2025-03-16 to 2025-03-30)
- **Commit**: `c465dd0` - File dialogs, folder navigation
- **Evidence**: ROM $4D2000-$4D8000 (Standard File Package)
- **Docs**: Inside Macintosh Vol VI-3

## Reproducible Build

### Quick Start
```bash
# Clone repository
git clone https://github.com/Kelsidavis/System7.git
cd System7

# Build in Docker
docker build -t system71-build -f prove/Dockerfile .
docker run --rm -v $(pwd):/build system71-build

# Verify hash
sha256sum build/system71.iso
# Expected: TBD (to be measured after first verified build)
```

### Detailed Build (see `prove/build_and_hash.sh`)
```bash
make clean
make all
make iso
sha256sum kernel.elf system71.iso > prove/build_hashes.txt
```

## Potential Confusions: Why API Names Must Match

**Short answer:** Public API compatibility requires identical names.

**Analogy:** A Linux program calling `open()`, `read()`, `malloc()` uses the same names as glibc, BSD libc, musl, etc. This is **interface compatibility**, not copying. The *implementation* of `malloc()` in glibc vs. musl differs entirely (different algorithms, heap structures, metadata).

**System 7 Case:**
- **Public APIs** like `BlockMove`, `NewHandle`, `SetPort`, `StandardGetFile` are documented in Inside Macintosh with exact signatures.
- **Public constants** like `noErr`, `memFullErr`, `everyEvent`, `mouseDown` are part of the published API contract.
- **Public structures** like `CInfoPBRec`, `FSSpec`, `Rect`, `Point` have documented field layouts for binary compatibility.

**Our reimplementation:**
- Uses identical **names** (required for API compatibility)
- Uses identical **field layouts** (required for binary data exchange)
- Uses **different internal algorithms** (modern heap vs. 1991 heap, different error paths, added logging)
- Uses **different coding style** (C99 vs. 1980s C/Pascal mix, different comments)

**Test:** Compare our `src/MemoryMgr/memory_manager_core.c:NewHandle()` with supermario equivalent:
- Same name? ✅ (must be, it's a public API)
- Same parameter types? ✅ (must be, it's a public signature)
- Same internal logic? ❌ (different heap layout, different compaction triggers, different error handling)
- Same comments? ❌ (ours reference ROM addresses and disassembly; implementation has Apple internal comments)

**Litmus test for reviewers:**
1. Remove all function/type names → do implementations still match? (No = independent)
2. Compare control flow graphs → are branch structures identical? (No = independent)
3. Compare variable naming → same internal names? (No = independent)
4. Compare error handling → same edge cases? (No = independent)

## Verification Checklist

- [ ] ROM SHA-256 matches documented sources
- [ ] No files contain "System7Port", "SuperMario", SCCS tags
- [ ] Similarity analysis shows <5% token overlap (see `prove/SIMILARITY_REPORT.md`)
- [ ] All public API names traceable to Inside Macintosh citations
- [ ] Reproducible build produces documented hash
- [ ] Independent code review confirms clean-room status

## Contact & Audit Requests

For third-party audit requests, see `docs/THIRD_PARTY_AUDIT.md`.

**Maintainer**: @Kelsidavis
**Public discussion**: GitHub Issues
**Provenance questions**: Open issue with tag `provenance`

---
*Last updated: 2025-03-30*
*Audit revision: 1.0*
