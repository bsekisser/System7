#!/bin/bash
# Fix stray \2 escape sequences in source files

cd /home/k/iteration2

# Fix all instances of )->\\2 to just )
find src -name "*.c" -type f | while read file; do
    if grep -q '\\2' "$file"; then
        # Replace )->\\2-> with )->
        sed -i 's/)->\\2->/)->/g' "$file"
        # Replace )->\\2. with )->
        sed -i 's/)->\\2\./)->/g' "$file"
        # Replace any remaining (something)->\\2 patterns
        sed -i 's/\([a-zA-Z_][a-zA-Z0-9_]*\))->\\2/\1)/g' "$file"
        echo "Fixed: $file"
    fi
done

echo "All stray sequences fixed"