#!/bin/bash
# Automated audit script: scan for leaked source markers
# Exit code: 0 = clean, 1 = findings need review

set -e

RED='\033[0;31m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

FINDINGS=0

echo "=== System 7.1 Provenance Audit ==="
echo "Scan date: $(date)"
echo ""

# Check 1: Apple copyright
echo "[1/6] Checking for Apple copyright notices..."
if rg -i "copyright.*apple" src/ include/ --no-ignore > /dev/null 2>&1; then
    echo -e "${RED}❌ FAIL: Apple copyright found${NC}"
    rg -i "copyright.*apple" src/ include/ --no-ignore
    FINDINGS=$((FINDINGS+1))
else
    echo -e "${GREEN}✅ PASS: No Apple copyright${NC}"
fi
echo ""

# Check 2: SuperMario references
echo "[2/6] Checking for SuperMario project references..."
if rg -i "supermario" src/ include/ --no-ignore > /dev/null 2>&1; then
    echo -e "${RED}❌ FAIL: SuperMario references found${NC}"
    rg -i "supermario" src/ include/ --no-ignore
    FINDINGS=$((FINDINGS+1))
else
    echo -e "${GREEN}✅ PASS: No SuperMario references${NC}"
fi
echo ""

# Check 3: SCCS/version control tags
echo "[3/6] Checking for SCCS/CVS version control tags..."
if rg '\$Id:|\$Header:|\$Log:' src/ include/ --no-ignore > /dev/null 2>&1; then
    echo -e "${RED}❌ FAIL: Version control tags found${NC}"
    rg '\$Id:|\$Header:|\$Log:' src/ include/ --no-ignore
    FINDINGS=$((FINDINGS+1))
else
    echo -e "${GREEN}✅ PASS: No version control tags${NC}"
fi
echo ""

# Check 4: Internal Apple comments
echo "[4/6] Checking for internal Apple engineering comments..."
if rg -i "see technote from|ask.*fred|radar.*bug|apple internal" src/ --no-ignore > /dev/null 2>&1; then
    echo -e "${YELLOW}⚠️  WARNING: Potential internal comments found${NC}"
    rg -i "see technote from|ask.*fred|radar.*bug|apple internal" src/ --no-ignore
    echo "   Review manually - may be benign references to public Technical Notes"
    FINDINGS=$((FINDINGS+1))
else
    echo -e "${GREEN}✅ PASS: No suspicious internal comments${NC}"
fi
echo ""

# Check 5: Verify ROM derivation comments exist
echo "[5/6] Verifying ROM derivation comments..."
ROM_COMMENTS=$(rg -c "ROM \$[0-9A-F]+" src/ 2>/dev/null | wc -l || echo 0)
if [ "$ROM_COMMENTS" -lt 5 ]; then
    echo -e "${YELLOW}⚠️  INFO: $ROM_COMMENTS files have ROM derivation comments${NC}"
    echo "   Consider adding more ROM address citations for traceability"
else
    echo -e "${GREEN}✅ PASS: $ROM_COMMENTS files have ROM derivation comments${NC}"
fi
echo ""

# Check 6: Build test
echo "[6/6] Testing build..."
if make clean > /dev/null 2>&1 && make all > /dev/null 2>&1; then
    echo -e "${GREEN}✅ PASS: Build succeeds${NC}"
    make clean > /dev/null 2>&1
else
    echo -e "${RED}❌ FAIL: Build broken${NC}"
    FINDINGS=$((FINDINGS+1))
fi
echo ""

# Summary
echo "=== Audit Summary ==="
if [ $FINDINGS -eq 0 ]; then
    echo -e "${GREEN}✅ CLEAN: No provenance issues detected${NC}"
    echo ""
    echo "Next steps:"
    echo "  1. Run similarity analysis: python3 prove/winnow.py src ../supermario"
    echo "  2. Build ISO: docker run --rm -v \$(pwd):/build system71-build"
    echo "  3. Publish hashes in PROVENANCE.md"
    exit 0
else
    echo -e "${RED}❌ REVIEW NEEDED: $FINDINGS issue(s) found${NC}"
    echo ""
    echo "Next steps:"
    echo "  1. Review findings above"
    echo "  2. Apply fixes per prove/CLEANUP_PLAN.md"
    echo "  3. Re-run: bash prove/scan_tree.sh"
    exit 1
fi
