# Cleanup Plan: Removing Confusing Markers

This document tracks specific instances where comments, strings, or identifiers might be confused for implementation, along with remediation steps.

## Audit Findings

### Scan Date: 2025-03-30

**Commands run:**
```bash
rg -i "copyright.*apple" src/ include/
rg -i "supermario" src/ include/
rg '\$Id:|\$Header:' src/ include/
rg -i "apple computer" src/ include/
```

**Results:** All scans returned zero matches ✅

## Hypothetical Examples (Template for Future Use)

### Example 1: Confusing Comment

**Finding:**
```
File: src/MemoryMgr/memory_manager_core.c
Line: 145
Content: /* Classic Mac heap algorithm from 1984 */
Issue: Vague attribution, could imply source code access
```

**Decision:** Replace with specific ROM derivation note

**Remediation:**
```diff
- /* Classic Mac heap algorithm from 1984 */
+ /* ROM $40B200: Heap compaction trigger (see Inside Mac Vol II-1 p.34)
+  * Evidence: JSR $40B200 from $40A8C0, compares free space ratio
+  * Disassembly: CMPI.W #$0064,D0 (100% threshold)
+  */
```

**Commit message:**
```
chore(provenance): clarify heap compaction comment with ROM evidence

- Remove vague "Classic Mac" attribution
- Add specific ROM address ($40B200) and disimplementation
- Cite Inside Macintosh Vol II-1 p.34 (public documentation)

This change improves provenance traceability without altering implementation.
```

## Commit Message Template

```
<type>(provenance): <short summary>

- <detailed change 1>
- <detailed change 2>

<justification: why this improves provenance traceability>

Evidence: <ROM address / public doc citation / none (our design)>
Impact: No functional change / Refactored implementation / Regenerated from disassembly
```

**Types:**
- `chore(provenance)`: Comment/string clarifications
- `refactor(provenance)`: Rename symbols, restructure code
- `fix(provenance)`: Remove contaminated code, regenerate

## Status

**Current audit status:** ✅ CLEAN (as of 2025-03-30)

No remediation needed at this time. This document serves as a template for future audits.

---
*Last audit: 2025-03-30*  
*Next audit: Before each major release*
