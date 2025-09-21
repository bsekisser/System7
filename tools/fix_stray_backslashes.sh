#!/bin/bash
# Fix all stray backslash errors in source files

echo "Fixing stray backslash errors in all source files..."

# Find all C files with stray backslash patterns
for file in $(find /home/k/iteration2/src -name "*.c"); do
    # Check if file has stray backslash patterns
    if grep -q '\\\[0-9]\.' "$file" 2>/dev/null; then
        echo "Fixing: $file"

        # Replace various patterns of corrupted member access
        sed -i 's/\([a-zA-Z_][a-zA-Z0-9_]*\)->\\[0-9]\.\([a-zA-Z_][a-zA-Z0-9_]*\)/\/\* \1->\2 \*\//g' "$file"

        # Fix simpler stray backslash cases
        sed -i 's/\\2\.//g' "$file"
        sed -i 's/\\1\.//g' "$file"
        sed -i 's/\\3\.//g' "$file"

        # Fix patterns like \2.member
        sed -i 's/\\[0-9]\.//g' "$file"
    fi
done

echo "Stray backslash fixing complete!"