#!/usr/bin/env python3
"""
Correct NFNT parser based on the actual format specification
"""
import struct
import sys

def parse_nfnt(data):
    """Parse an NFNT resource correctly"""

    # FontRec header (24 bytes, big-endian)
    font_type = struct.unpack('>H', data[0:2])[0]
    first_char = struct.unpack('>H', data[2:4])[0]
    last_char = struct.unpack('>H', data[4:6])[0]
    wid_max = struct.unpack('>H', data[6:8])[0]
    kern_max = struct.unpack('>h', data[8:10])[0]  # signed
    n_descent = struct.unpack('>h', data[10:12])[0]  # signed
    f_rect_width = struct.unpack('>H', data[12:14])[0]
    f_rect_height = struct.unpack('>H', data[14:16])[0]
    owt_loc = struct.unpack('>H', data[16:18])[0]  # WORD offset to OWT
    ascent = struct.unpack('>H', data[18:20])[0]
    descent = struct.unpack('>H', data[20:22])[0]
    leading = struct.unpack('>H', data[22:24])[0]
    row_words = struct.unpack('>H', data[24:26])[0]

    print(f"Font type: 0x{font_type:04X}")
    print(f"First char: {first_char} (0x{first_char:02X})")
    print(f"Last char: {last_char} (0x{last_char:02X})")
    print(f"Max width: {wid_max}")
    print(f"Height: {f_rect_height}")
    print(f"Ascent: {ascent}")
    print(f"Descent: {descent}")
    print(f"Row words: {row_words}")
    print(f"OWT location: {owt_loc} words from start")

    # Calculate sizes
    num_chars = last_char - first_char + 1
    row_bytes = row_words * 2
    bitmap_size = row_bytes * f_rect_height

    # OWT is at word offset owt_loc from start of FontRec (which starts at 0)
    owt_byte_offset = owt_loc * 2

    print(f"\nOWT at byte offset: 0x{owt_byte_offset:04X}")

    # Read offset table (N+1 word offsets)
    offsets = []
    for i in range(num_chars + 1):
        offset_pos = owt_byte_offset + (i * 2)
        if offset_pos + 1 < len(data):
            offset = struct.unpack('>H', data[offset_pos:offset_pos+2])[0]
            offsets.append(offset)

    print(f"Read {len(offsets)} offsets")

    # Width table follows offsets
    width_table_start = owt_byte_offset + (num_chars + 1) * 2

    # BitImage typically starts at offset 26 (right after header)
    # But let's calculate where it should be
    bit_image_offset = 26  # Right after 24-byte header + 2 bytes padding often

    # Check if we have space for 8-bit or 16-bit widths
    space_before_bitmap = bit_image_offset - width_table_start
    if space_before_bitmap < 0:
        # OWT comes after bitmap
        bit_image_offset = 26
        # Widths follow offsets
        widths_are_16bit = False  # Try 8-bit first
    else:
        widths_are_16bit = (space_before_bitmap >= num_chars * 2)

    # Read widths
    widths = []
    if widths_are_16bit:
        print("Using 16-bit widths")
        for i in range(num_chars):
            pos = width_table_start + (i * 2)
            if pos + 1 < len(data):
                width = struct.unpack('>h', data[pos:pos+2])[0]  # signed
                widths.append(width)
    else:
        print("Using 8-bit widths")
        for i in range(num_chars):
            pos = width_table_start + i
            if pos < len(data):
                width = struct.unpack('b', data[pos:pos+1])[0]  # signed byte
                widths.append(width)

    print(f"Read {len(widths)} widths")

    # Extract bitmap
    bitmap = data[bit_image_offset:bit_image_offset + bitmap_size]

    # Combine data
    chars = []
    for i in range(num_chars):
        if i < len(offsets) and i < len(widths):
            # Offset is in WORDS, width is logical advance
            chars.append((offsets[i], widths[i]))
        else:
            chars.append((0, 0))

    return {
        'height': f_rect_height,
        'ascent': ascent,
        'descent': descent,
        'first_char': first_char,
        'last_char': last_char,
        'row_words': row_words,
        'bitmap': bitmap,
        'chars': chars,
        'offsets': offsets
    }

def generate_header(font_data, output_file):
    """Generate C header file"""
    with open(output_file, 'w') as f:
        f.write("/* Generated from NFNT resource - corrected parser */\n\n")
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
        f.write("    uint16_t offset;  /* WORD offset into strike bitmap */\n")
        f.write("    int16_t width;    /* Logical advance width (signed) */\n")
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

        # Add missing char (using first entry)
        if font_data['chars']:
            f.write(f"    {{ {font_data['chars'][0][0]:4}, {font_data['chars'][0][1]:3} }},  /* Missing char */\n")

        for i, (offset, width) in enumerate(font_data['chars']):
            char_code = font_data['first_char'] + i
            if 32 <= char_code <= 126:
                if char_code == ord('\\'):
                    f.write(f"    {{ {offset:4}, {width:3} }},  /* '\\\\' */\n")
                else:
                    f.write(f"    {{ {offset:4}, {width:3} }},  /* '{chr(char_code)}' */\n")
            else:
                f.write(f"    {{ {offset:4}, {width:3} }},  /* 0x{char_code:02X} */\n")
        f.write("};\n\n")

        # Also export the offset array for ink width calculations
        f.write("/* Offset table for calculating ink widths */\n")
        f.write("static const uint16_t chicago_offsets[] = {\n")
        for i, offset in enumerate(font_data['offsets']):
            if i % 8 == 0:
                f.write("    ")
            f.write(f"{offset:4}")
            if i < len(font_data['offsets']) - 1:
                f.write(", ")
            if (i + 1) % 8 == 0:
                f.write("\n")
        if len(font_data['offsets']) % 8 != 0:
            f.write("\n")
        f.write("};\n\n")

        f.write("#endif /* CHICAGO_FONT_H */\n")

        print(f"Generated {output_file}")

if __name__ == "__main__":
    # Read the NFNT resource file
    with open("NFNT_5478.bin", "rb") as f:
        data = f.read()

    font_data = parse_nfnt(data)
    generate_header(font_data, "/home/k/iteration2/include/chicago_font.h")