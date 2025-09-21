#!/bin/bash
# Progressive compilation fixer for System 7.1

echo "=========================================="
echo "System 7.1 Progressive Compilation Fixer"
echo "=========================================="

cd /home/k/iteration2

# Step 1: Count total files
TOTAL_FILES=$(find src -name '*.c' | wc -l)
echo "Total source files: $TOTAL_FILES"

# Step 2: Clean build
make clean > /dev/null 2>&1

# Step 3: Try to compile and capture errors
echo "Scanning for compilation errors..."
make -j1 2>&1 | tee build.log > /dev/null

# Step 4: Extract and categorize errors
echo ""
echo "Error Analysis:"
echo "---------------"

# Count different error types
UNKNOWN_TYPE=$(grep -c "unknown type name" build.log)
UNDECLARED=$(grep -c "undeclared" build.log)
NO_MEMBER=$(grep -c "has no member named" build.log)
STRAY=$(grep -c "stray '\\\\' in program" build.log)
REDEF=$(grep -c "redefinition of\|conflicting types" build.log)

echo "Unknown types: $UNKNOWN_TYPE"
echo "Undeclared symbols: $UNDECLARED"
echo "Missing members: $NO_MEMBER"
echo "Stray backslashes: $STRAY"
echo "Redefinitions: $REDEF"

# Step 5: Show which files compile successfully
echo ""
echo "Successfully compiling files:"
grep "^CC src" build.log | awk '{print "  ✓", $2}'

# Step 6: Show first failing file
echo ""
echo "First failing file:"
grep "error:" build.log | head -1 | sed 's/:.*error:/ - Error:/'

# Step 7: Count successful compilations
SUCCESS=$(grep -c "^gcc.*-c.*-o.*\.o$" build.log)
echo ""
echo "=========================================="
echo "Progress: $SUCCESS / $TOTAL_FILES files compiled"
PERCENT=$((SUCCESS * 100 / TOTAL_FILES))
echo "Completion: $PERCENT%"
echo "=========================================="

# Step 8: Generate fix recommendations
if [ $UNKNOWN_TYPE -gt 0 ]; then
    echo ""
    echo "RECOMMENDATION: Add missing type definitions to SystemTypes.h"
    grep "unknown type name" build.log | sed "s/.*unknown type name '//" | sed "s/'.*//" | sort -u | head -10
fi

if [ $STRAY -gt 0 ]; then
    echo ""
    echo "RECOMMENDATION: Fix corrupted member access in these files:"
    grep "stray '\\\\' in program" build.log | cut -d: -f1 | sort -u | head -5
fi

# Cleanup
rm -f build.log