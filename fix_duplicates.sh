#!/bin/bash

echo "=== FIXING DUPLICATE DEFINITIONS ==="

# Remove the duplicate multiboot definitions we just added
sed -i '/^\/\/ Multiboot definitions$/,/^} multiboot_tag_basic_meminfo_t;$/d' include/SystemTypes.h

# Check if multiboot.h exists and include it properly
if [ ! -f "include/multiboot.h" ]; then
    echo "Creating multiboot.h..."
    cat > include/multiboot.h << 'EOF'
#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

// Multiboot2 definitions
#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289
#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6

typedef struct multiboot_tag {
    uint32_t type;
    uint32_t size;
} multiboot_tag_t;

typedef struct multiboot_tag_framebuffer {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint16_t reserved;
} multiboot_tag_framebuffer_t;

typedef struct multiboot_tag_module {
    uint32_t type;
    uint32_t size;
    uint32_t mod_start;
    uint32_t mod_end;
    char cmdline[1];
} multiboot_tag_module_t;

typedef struct multiboot_tag_basic_meminfo {
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
} multiboot_tag_basic_meminfo_t;

typedef struct multiboot_info {
    uint32_t size;
    uint32_t reserved;
} multiboot_info_t;

typedef struct multiboot_memory_map {
    uint32_t size;
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
} multiboot_memory_map_t;

#endif // MULTIBOOT_H
EOF
fi

# Now update files that need multiboot.h
echo "Updating files to include multiboot.h..."
for file in src/main.c src/SystemInit.c; do
    if [ -f "$file" ]; then
        # Add include at the top if not present
        if ! grep -q "#include \"multiboot.h\"" "$file"; then
            sed -i '1a#include "multiboot.h"' "$file"
        fi
    fi
done

echo "Duplicate definitions fixed!"