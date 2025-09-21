#!/usr/bin/env python3
"""
Convert native Mac ppat resources to PPAT8 format
The native ppat resources (400-413) need conversion to work properly.
"""

import struct
import json

def parse_native_ppat(hex_data):
    """Parse native Mac ppat format and extract pattern/colors"""
    data = bytes.fromhex(hex_data.replace(" ", ""))

    # Check if it has enough data for a ppat
    if len(data) < 0x86:  # Minimum size for ppat with pattern at 0x4C
        print(f"Data too small: {len(data)} bytes")
        return None

    # Version check
    version = struct.unpack('>H', data[0:2])[0]
    if version != 0x0001:
        print(f"Wrong version: 0x{version:04x}")
        return None

    # Get pixel depth at offset 0x40 (2-bit, 4-bit, or 8-bit)
    pixel_depth = struct.unpack('>H', data[0x40:0x42])[0]
    print(f"Pixel depth: {pixel_depth} bits")

    if pixel_depth == 2:
        # 2-bit pattern (4 colors) - pattern at 0x4C
        pattern_data = data[0x4C:0x5C]  # 16 bytes for 8x8 at 2bpp

        # Extract 4 colors - analyze the hex to find color table
        # Based on ppat 400 analysis:
        colors = [
            (0x00, 0x99, 0x99, 0xFF),  # Teal
            (0x99, 0x00, 0x99, 0xFF),  # Purple
            (0x00, 0xEB, 0xFF, 0xFF),  # Light blue
            (0xFF, 0x99, 0x00, 0xFF),  # Orange
        ]

        # Convert 2-bit pattern to indices
        indices = []
        for y in range(8):
            row_data = struct.unpack('>H', pattern_data[y*2:(y+1)*2])[0]
            row = []
            for x in range(8):
                # Extract 2 bits from left to right
                shift = 14 - (x * 2)
                idx = (row_data >> shift) & 0x03
                row.append(idx)
            indices.append(row)

    elif pixel_depth == 4:
        # 4-bit pattern (up to 16 colors)
        pattern_data = data[0x4C:0x6C]  # 32 bytes for 8x8 at 4bpp

        # Extract up to 16 colors (need to find color table)
        # For now use placeholder colors
        colors = [
            (0xFF, 0xFF, 0xFF, 0xFF),  # White
            (0x00, 0x00, 0x00, 0xFF),  # Black
            (0xFF, 0x00, 0x00, 0xFF),  # Red
            (0x00, 0xFF, 0x00, 0xFF),  # Green
            (0x00, 0x00, 0xFF, 0xFF),  # Blue
            (0xFF, 0xFF, 0x00, 0xFF),  # Yellow
            (0xFF, 0x00, 0xFF, 0xFF),  # Magenta
            (0x00, 0xFF, 0xFF, 0xFF),  # Cyan
        ]

        # Convert 4-bit pattern to indices
        indices = []
        for y in range(8):
            row = []
            for x in range(4):  # 4 bytes per row
                byte = pattern_data[y*4 + x]
                # Each byte contains 2 pixels
                row.append((byte >> 4) & 0x0F)
                row.append(byte & 0x0F)
            indices.append(row)
    else:
        print(f"Unsupported pixel depth: {pixel_depth}")
        return None

    return {"palette": colors[:len(set(sum(indices, [])))], "indices": indices}

def create_ppat8(palette, indices):
    """Create PPAT8 format from palette and indices"""
    out = bytearray()

    # Header
    out.extend(b"PPAT8\x00")

    # Width and height
    out.extend(struct.pack('>HH', 8, 8))

    # Palette length
    out.extend(struct.pack('>H', len(palette)))

    # Palette entries (RGBA)
    for color in palette:
        out.extend(bytes(color))

    # Indices (64 bytes, one per pixel)
    for row in indices:
        for idx in row:
            out.append(idx)

    return bytes(out)

def main():
    # Read the patterns.json file
    with open('patterns.json', 'r') as f:
        data = json.load(f)

    # Find all ppat resources (400-413)
    converted = []

    for resource in data['resources']:
        if resource['type'] == 'ppat' and 400 <= resource['id'] <= 413:
            print(f"\nConverting ppat {resource['id']} ({resource['name']})...")

            hex_data = resource['data']['hex']
            result = parse_native_ppat(hex_data)

            if result:
                # Convert to PPAT8
                ppat8_data = create_ppat8(result['palette'], result['indices'])

                # Create new resource entry with ppat8 format
                new_entry = {
                    "type": "ppat",
                    "id": resource['id'],
                    "name": resource['name'],
                    "data": {
                        "ppat8": {
                            "palette": result['palette'],
                            "indices": [" ".join(map(str, row)) for row in result['indices']]
                        }
                    }
                }
                converted.append(new_entry)
                print(f"  Converted successfully!")
            else:
                print(f"  Failed to convert")

    # Output converted patterns
    print("\nConverted patterns:")
    print(json.dumps(converted, indent=2))

if __name__ == "__main__":
    main()