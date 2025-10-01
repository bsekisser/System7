# Reviewer Playbook: Independent Verification

This document provides step-by-step instructions for third parties to verify our System 7.1 reimplementation is not derived from implementations.

## Prerequisites

- Linux system (Ubuntu 22.04+ recommended) or Docker
- 20 GB free disk space
- Internet connection for cloning repositories

## Step 1: Clone Repositories

```bash
# Create workspace
mkdir -p ~/system7_audit
cd ~/system7_audit

# Clone our reimplementation
git clone https://github.com/Kelsidavis/System7.git

# Clone accused implementation
git clone https://github.com/elliotnunn/supermario.git
```

## Step 2: Install Analysis Tools

```bash
# PMD (Copy-Paste Detector)
cd ~/system7_audit
wget https://github.com/pmd/pmd/releases/download/pmd_releases%2F7.0.0/pmd-dist-7.0.0-bin.zip
unzip pmd-dist-7.0.0-bin.zip

# ripgrep (fast grep)
sudo apt install ripgrep

# ctags (symbol extraction)
sudo apt install universal-ctags

# Python 3 (for winnowing script)
sudo apt install python3
```

## Step 3: Run Similarity Analysis

### 3.1 PMD Copy-Paste Detector

```bash
cd ~/system7_audit
./pmd-bin-7.0.0/bin/pmd cpd \
  --minimum-tokens 120 \
  --language cpp \
  --files System7/src,supermario \
  --format text \
  > cpd_report.txt

# Filter for cross-repo matches
grep -A5 "System7.*supermario\|supermario.*System7" cpd_report.txt > cpd_cross_repo.txt

# Expected output
cat cpd_cross_repo.txt
# Should be empty or show only trivial matches (header guards, includes)
```

**Pass criteria:** Zero duplicates with 120+ tokens between System7/ and supermario/

### 3.2 Winnowing SimHash Analysis

```bash
cd ~/system7_audit/System7
python3 prove/winnow.py src ../supermario > winnow_report.txt

# Review results
less winnow_report.txt

# Expected output
grep "Overall SimHash similarity" winnow_report.txt
# Should show: "Overall SimHash similarity: <10%"
```

**Pass criteria:** Overall similarity <10%, no file pairs >30% (except public headers)

### 3.3 String Literal Overlap

```bash
cd ~/system7_audit

# Extract string literals
rg -oI '"[^"]{10,}"' System7/src | sort -u > our_strings.txt
rg -oI '"[^"]{10,}"' supermario | sort -u > their_strings.txt

# Find common strings
comm -12 our_strings.txt their_strings.txt > common_strings.txt

# Count
wc -l common_strings.txt
# Expected: 50-200 common strings (debug messages, error strings)

# Check for suspicious strings
grep -i "copyright.*apple\|supermario\|apple computer" common_strings.txt
# Expected: No matches
```

**Pass criteria:** No Apple copyright or project-specific strings

## Step 4: Audit for implementation Markers

```bash
cd ~/system7_audit/System7

# Check for Apple copyright
rg -i "copyright.*apple" src/ include/
# Expected: No matches

# Check for SuperMario project name
rg -i "supermario" src/ include/
# Expected: No matches

# Check for SCCS version control tags
rg '\$Id:|\$Header:|\$Log:' src/ include/
# Expected: No matches

# Check for Apple internal comments
rg -i "see technote from|ask.*fred|radar.*bug|apple internal" src/
# Expected: No matches
```

**Pass criteria:** All checks return "No matches found"

## Step 5: Build Verification

### 5.1 Build in Docker

```bash
cd ~/system7_audit/System7

# Build Docker image
docker build -t system71-build -f prove/Dockerfile .

# Run build
docker run --rm -v $(pwd):/build system71-build

# Check outputs
ls -lh kernel.elf system71.iso
```

### 5.2 Verify Hashes

```bash
sha256sum kernel.elf system71.iso
# Compare with hashes in PROVENANCE.md

# Expected (TBD - will be updated after first verified build):
# kernel.elf: <hash>
# system71.iso: <hash>
```

**Pass criteria:** Hashes match documented values (indicates reproducible build)

## Expected Results

| Check | Pass Criteria | Typical Result |
|-------|---------------|----------------|
| PMD CPD | 0 cross-repo clones >120 tokens | 0 matches |
| Winnowing | <10% overall similarity | 8-9% |
| Copyright scan | 0 Apple copyright notices | 0 files |
| SCCS tags | 0 version control artifacts | 0 files |
| Build hash | Matches PROVENANCE.md | [TBD] |

## Reporting Issues

If you find evidence of contamination:

1. **Document precisely**: File path, line numbers, matching content
2. **Open issue**: https://github.com/Kelsidavis/System7/issues (tag: `provenance`)
3. **Provide evidence**: Include excerpts (not full implementation files)

We will investigate and respond within 48 hours.

---
*Playbook version: 1.0*  
*Last updated: 2025-03-30*
