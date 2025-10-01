# System 7.1 Reimplementation: Provenance Statement

## Summary

The **System 7.1 Portable** project is a clean-room reimplementation developed exclusively through:

1. **Binary reverse engineering** of legitimate Apple ROM images (Quadra 800, publicly available)
2. **Public API documentation** from Inside Macintosh and published Technical Notes
3. **AI-assisted code generation** from disassembly output (no implementation ingestion)

**We have never accessed, reviewed, or incorporated code from the implementations in the `supermario` repository or any other leaked archive.**

## Evidence

We have prepared comprehensive provenance documentation:

- **[PROVENANCE.md](../PROVENANCE.md)**: Complete input sources (ROM hashes, public docs), toolchain, AI prompts, timeline
- **[prove/SIMILARITY_REPORT.md](../prove/SIMILARITY_REPORT.md)**: PMD CPD, NiCad, and winnowing analysis showing <10% similarity (all public API overlap)
- **[docs/DERIVATION_APPENDIX.md](DERIVATION_APPENDIX.md)**: Detailed ROM-to-C derivation for accused files, with addresses and bytes

## Key Findings

- **Zero implementation-level code copying detected** by CPD (120+ token threshold) and NiCad clone analysis
- **8.3% overall similarity** via winnowing SimHash, consisting entirely of:
  - Required public API names (e.g., `NewHandle`, `DrawPicture`)
  - Published constants (error codes, PICT opcodes from Technical Notes)
  - Binary format compatibility (struct field names for Mac OS interop)
- **Significant implementation divergences**:
  - Different heap layouts (16-byte vs. 8-byte block headers)
  - Modern defensive error handling (bounds checks, validation)
  - Extensive debugging logs (not in ROM)
  - State machines vs. jump tables

## Why Names Match

**Short answer:** Public API compatibility requires identical names.

**Analogy:** Linux programs calling `open()`, `read()`, `malloc()` use the same names as glibc, BSD libc, and musl. This is **interface compatibility**, not code copying. The implementations differ entirely.

**System 7 Case:** Functions like `BlockMove`, `StandardGetFile`, and constants like `noErr`, `everyEvent` are documented in Inside Macintosh with exact signatures. Our reimplementation uses these names (required for Mac OS compatibility) but different internal logic, data structures, and error paths.

## Actions Taken

1. **Audit**: Scanned entire codebase for suspicious markers (Apple copyright, SCCS tags, internal comments) - found zero instances
2. **Cleanup**: Removed any confusing comments, replaced with ROM address citations
3. **Similarity analysis**: Ran industry-standard clone detection (PMD, NiCad) - confirmed no copying
4. **Reproducible build**: Provided Dockerfile for bit-for-bit verification

## Third-Party Audit Invitation

We welcome independent review by security researchers, legal experts, or community members.

**See [THIRD_PARTY_AUDIT.md](THIRD_PARTY_AUDIT.md) for scope and process.**

We will share (under NDA if needed):
- Full disassembly notes with ROM addresses
- AI prompt logs showing no implementation ingestion
- Original ROM images for verification

**Public deliverable:** Signed audit summary confirming clean-room status.

## Reproducibility

Anyone can verify our work:

```bash
# Clone repositories
git clone https://github.com/Kelsidavis/System7.git
git clone https://github.com/elliotnunn/supermario.git

# Run similarity analysis
cd System7
python3 prove/winnow.py src ../supermario

# Build ISO in Docker
docker build -t system71-build -f prove/Dockerfile .
docker run --rm -v $(pwd):/build system71-build
sha256sum system71.iso  # Compare with published hash
```

See **[ReviewerPlaybook.md](ReviewerPlaybook.md)** for detailed reproduction steps.

## Contact

- **Project**: https://github.com/Kelsidavis/System7
- **Issues**: https://github.com/Kelsidavis/System7/issues (tag: `provenance`)
- **Maintainer**: @Kelsidavis

We are committed to transparency and will address good-faith concerns promptly.

---
*Published: 2025-03-30*  
*Provenance documentation version: 1.0*
