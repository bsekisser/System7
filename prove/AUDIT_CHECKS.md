# Audit Checks: Scanning for implementation Markers

## Quick Audit Commands

### 1. Check for Apple Copyright/Identifiers

```bash
# Scan for Apple copyright notices
rg -i "copyright.*apple" src/ include/ --no-ignore

# Scan for SuperMario project name
rg -i "supermario" src/ include/ --no-ignore

# Scan for SCCS version control tags
rg '\$Id:|\$Header:|\$Log:' src/ include/ --no-ignore

# Scan for Apple Computer mentions
rg -i "apple computer" src/ include/ --no-ignore
```

**Expected output:** `No matches found` (or only in this AUDIT_CHECKS.md file)

**If found:**
- ❌ **STOP**: Potential implementation contamination
- **Action**: Remove file, regenerate from disassembly, document in `prove/CLEANUP_PLAN.md`

### 2. Check for Suspicious Comments

```bash
# Look for internal Apple engineering comments
rg -i "see technote from|ask.*fred|radar.*bug|apple internal" src/ --no-ignore

# Look for version control artifacts
rg "Revision.*Date.*Author" src/ include/ --no-ignore

# Look for old Mac source code patterns (Pascal comments)
rg '\(\*.*\*\)|\{.*\}' src/ --type c --no-ignore
```

**Expected output:** `No matches found`

**If found:**
- ⚠️  **Review**: May be benign (e.g., "see TN1094" referring to public doc)
- **Action**: Replace with neutral citation (e.g., "Reference: Technical Note 1094, public doc")

### 3. Check for Identical String Literals (Benign Overlap Expected)

```bash
# Extract all string literals from our code
rg -oI '"[^"]{10,}"' src/ | sort -u > /tmp/our_strings.txt

# Extract from supermario
rg -oI '"[^"]{10,}"' ../supermario/ | sort -u > /tmp/their_strings.txt

# Find common strings
comm -12 /tmp/our_strings.txt /tmp/their_strings.txt > /tmp/common_strings.txt

# Count
wc -l /tmp/common_strings.txt
```

**Expected output:** 
- Common strings: 50-200 (mostly debug messages, error strings)
- **Benign examples:** `"Invalid handle"`, `"Memory full"`, `"File not found"`
- **Suspicious examples:** `""`, `"SuperMario Project Build 42"`

### 4. Symbol Name Overlap (Expected for Public APIs)

```bash
# Extract function names from our code
ctags -R --c-kinds=f -o - src/ | awk '{print $1}' | sort -u > /tmp/our_funcs.txt

# Extract from supermario (if available)
ctags -R --c-kinds=f -o - ../supermario/ | awk '{print $1}' | sort -u > /tmp/their_funcs.txt

# Find common function names
comm -12 /tmp/our_funcs.txt /tmp/their_funcs.txt > /tmp/common_funcs.txt

# Classify as public vs. internal
echo "=== PUBLIC API (expected overlap) ==="
grep -E '^(New|Dispose|Get|Set|Block|Standard|Draw|Open|Close|Read|Write)' /tmp/common_funcs.txt

echo "=== INTERNAL HELPERS (unexpected overlap) ==="
grep -vE '^(New|Dispose|Get|Set|Block|Standard|Draw|Open|Close|Read|Write)' /tmp/common_funcs.txt
```

**Expected:**
- Public API overlap: 200-500 functions (all documented in Inside Macintosh)
- Internal helper overlap: 0-10 (investigate if >10)

**If internal helpers match:**
- ⚠️  **Review context**: May be common patterns (e.g., `parse_header`, `validate_input`)
- **Action**: Rename our internal helpers to be more distinctive

## Remediation Checklist

If suspicious content found:

### Level 1: Comments/Strings (Low Risk)
- [ ] Identify exact lines with `rg -n <pattern> src/`
- [ ] Verify context (is it a public doc citation or internal comment?)
- [ ] Replace or remove
- [ ] Add derivation note referencing ROM address or public doc
- [ ] Commit with message: `chore(provenance): remove confusing comment; cite ROM $ADDR`

### Level 2: Internal Symbol Names (Medium Risk)
- [ ] List all occurrences: `rg -l <symbol> src/`
- [ ] Verify symbol is not in Inside Macintosh index
- [ ] Rename to project-specific name (e.g., `helper_parse_foo` → `sys71_parse_foo_internal`)
- [ ] Update all references
- [ ] Commit: `refactor(provenance): rename internal helper to avoid namespace collision`

### Level 3: Code Structure Match (High Risk)
- [ ] Extract control flow graph: `cflow src/file.c > cfg_ours.txt`
- [ ] Compare with supermario equivalent
- [ ] If match >70%: **REIMPLEMENT from disassembly using different algorithm**
- [ ] Document new approach in file header
- [ ] Commit: `refactor(provenance): reimplement <module> with alternate algorithm`

### Level 4: File Contamination (Critical)
- [ ] **DELETE FILE IMMEDIATELY**
- [ ] Regenerate from ROM disassembly with different AI prompt
- [ ] Add explicit guard in prompt: "Do not use any Apple source code patterns"
- [ ] Review new implementation with winnowing
- [ ] Commit: `chore(provenance): regenerate <file> from clean-room RE`

## Automated Audit Script

Run all checks:
```bash
bash prove/scan_tree.sh
```

See `prove/scan_tree.sh` for implementation.

---
*Audit protocol version: 1.0*  
*Last updated: 2025-03-30*
