#!/usr/bin/env python3
"""
Convert Mac NFNT resource to C header file
Properly parse the character offset/width table
"""
import struct
import sys

def parse_nfnt(data):
    """Parse an NFNT resource"""
    # NFNT header structure (FontRec)
    font_type = struct.unpack('>H', data[0:2])[0]
    first_char = struct.unpack('>H', data[2:4])[0]
    last_char = struct.unpack('>H', data[4:6])[0]
    widmax = struct.unpack('>H', data[6:8])[0]
    kern_max = struct.unpack('>h', data[8:10])[0]
    n_descent = struct.unpack('>h', data[10:12])[0]
    rect_width = struct.unpack('>H', data[12:14])[0]
    height = struct.unpack('>H', data[14:16])[0]
    ow_tLoc = struct.unpack('>H', data[16:18])[0]  # Offset to offset/width table
    ascent = struct.unpack('>H', data[18:20])[0]
    descent = struct.unpack('>H', data[20:22])[0]
    leading = struct.unpack('>H', data[22:24])[0]
    row_words = struct.unpack('>H', data[24:26])[0]

    print(f"Font type: 0x{font_type:04X}")
    print(f"First char: {first_char}")
    print(f"Last char: {last_char}")
    print(f"Max width: {widmax}")
    print(f"Height: {height}")
    print(f"Ascent: {ascent}")
    print(f"Descent: {descent}")
    print(f"Row words: {row_words}")

    # Bitmap starts at offset 26
    bitmap_size = row_words * 2 * height
    bitmap = data[26:26+bitmap_size]

    # The offset table starts after the bitmap
    offset_table_start = 26 + bitmap_size
    num_chars = last_char - first_char + 2  # +1 for range, +1 for missing char

    # Read the offset table (one word per character + 1 extra for end)
    offsets = []
    for i in range(num_chars + 1):
        pos = offset_table_start + (i * 2)
        if pos + 1 < len(data):
            offset = struct.unpack('>H', data[pos:pos+2])[0]
            offsets.append(offset)
        else:
            break

    # Width table follows the offset table
    width_table_start = offset_table_start + (num_chars + 1) * 2

    # Read widths (one byte per character)
    widths = []
    for i in range(num_chars):
        pos = width_table_start + i
        if pos < len(data):
            widths.append(data[pos])
        else:
            widths.append(0)

    # Combine offsets and widths
    chars = []
    for i in range(num_chars):
        if i < len(offsets) and i < len(widths):
            chars.append((offsets[i], widths[i]))
        else:
            chars.append((0, 0))

    return {
        'height': height,
        'ascent': ascent,
        'descent': descent,
        'first_char': first_char,
        'last_char': last_char,
        'row_words': row_words,
        'bitmap': bitmap,
        'chars': chars
    }

def generate_header(font_data, output_file):
    """Generate C header file"""
    with open(output_file, 'w') as f:
        f.write("/* Generated from NFNT resource */\n\n")
        f.write("#ifndef CHICAGO_FONT_H\n")
        f.write("#define CHICAGO_FONT_H\n\n")
        f.write("#include <stdint.h>\n\n")

        # Font metrics
        f.write(f"#define CHICAGO_HEIGHT {font_data['height']}\n")
        f.write(f"#define CHICAGO_FIRST_CHAR {font_data['first_char']}\n")
        f.write(f"#define CHICAGO_LAST_CHAR {font_data['last_char']}\n")
        f.write(f"#define CHICAGO_ASCENT {font_data['ascent']}\n")
        f.write(f"#define CHICAGO_DESCENT {font_data['descent']}\n")
        f.write(f"#define CHICAGO_ROW_WORDS {font_data['row_words']}\n\n")

        # Character info structure
        f.write("typedef struct {\n")
        f.write("    uint16_t offset;  /* Bit offset from start of row */\n")
        f.write("    uint16_t width;   /* Character width in pixels */\n")
        f.write("} CharInfo;\n\n")

        # Bitmap data
        f.write("static const uint8_t chicago_bitmap[] = {\n")
        for i in range(0, len(font_data['bitmap']), 16):
            f.write("    ")
            for j in range(min(16, len(font_data['bitmap']) - i)):
                f.write(f"0x{font_data['bitmap'][i+j]:02X}")
                if i+j < len(font_data['bitmap']) - 1:
                    f.write(", ")
            f.write("\n")
        f.write("};\n\n")

        # Character info table
        f.write("static const CharInfo chicago_chars[] = {\n")
        for i, (offset, width) in enumerate(font_data['chars']):
            if i == 0:
                f.write(f"    {{ {offset:3}, {width:3} }},  /* Missing char */\n")
            else:
                char_code = font_data['first_char'] + i - 1
                if 32 <= char_code <= 126:
                    f.write(f"    {{ {offset:3}, {width:3} }},  /* '{chr(char_code)}' */\n")
                else:
                    f.write(f"    {{ {offset:3}, {width:3} }},  /* 0x{char_code:02X} */\n")
        f.write("};\n\n")

        f.write("#endif /* CHICAGO_FONT_H */\n")

        print(f"Generated {output_file}")

if __name__ == "__main__":
    # Read the NFNT resource file
    with open("NFNT_5478.bin", "rb") as f:
        data = f.read()

    font_data = parse_nfnt(data)
    generate_header(font_data, "/home/k/iteration2/include/chicago_font.h")