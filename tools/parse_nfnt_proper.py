#!/usr/bin/env python3
"""
Proper NFNT parser following the exact specification
"""
import struct

def parse_nfnt(data):
    """Parse NFNT following the exact specification"""

    # FontRec header (24 bytes, big-endian) at offset 0
    font_type = struct.unpack('>H', data[0:2])[0]
    first_char = struct.unpack('>H', data[2:4])[0]
    last_char = struct.unpack('>H', data[4:6])[0]
    wid_max = struct.unpack('>H', data[6:8])[0]
    kern_max = struct.unpack('>h', data[8:10])[0]
    n_descent = struct.unpack('>h', data[10:12])[0]
    f_rect_width = struct.unpack('>H', data[12:14])[0]
    f_rect_height = struct.unpack('>H', data[14:16])[0]
    owt_loc = struct.unpack('>H', data[16:18])[0]  # WORD offset from start of FontRec
    ascent = struct.unpack('>H', data[18:20])[0]
    descent = struct.unpack('>H', data[20:22])[0]
    leading = struct.unpack('>H', data[22:24])[0]
    row_words = struct.unpack('>H', data[24:26])[0]

    print(f"Font type: 0x{font_type:04X}")
    print(f"First char: {first_char}")
    print(f"Last char: {last_char}")
    print(f"Height: {f_rect_height}")
    print(f"Ascent: {ascent}, Descent: {descent}")
    print(f"Row words: {row_words}")

    # Calculate number of chars
    num_chars = last_char - first_char + 1

    # BitImage starts immediately after FontRec at byte 26
    bitmap_start = 26
    bitmap_size = row_words * 2 * f_rect_height
    bitmap = data[bitmap_start:bitmap_start + bitmap_size]

    print(f"\nBitmap at offset 0x{bitmap_start:04X}, size {bitmap_size} bytes")

    # The offset/width table (OWT) is at WORD offset owt_loc from FontRec start
    # FontRec starts at byte 0, so OWT byte offset = owt_loc * 2
    owt_start = owt_loc * 2  # Convert word offset to byte offset

    print(f"OWT starts at byte offset 0x{owt_start:04X} (word offset 0x{owt_loc:04X})")

    # Read offset table (N+1 WORD offsets)
    offsets = []
    for i in range(num_chars + 1):
        pos = owt_start + (i * 2)
        if pos + 1 < len(data):
            offset = struct.unpack('>H', data[pos:pos+2])[0]
            offsets.append(offset)

    print(f"Read {len(offsets)} offsets")
    print(f"First few offsets: {offsets[:10]}")

    # Width table immediately follows offset table
    width_table_start = owt_start + (num_chars + 1) * 2

    # Determine if widths are 8-bit or 16-bit
    # Check how much space remains in the file
    remaining = len(data) - width_table_start

    widths = []
    if remaining >= num_chars * 2:
        # Likely 16-bit widths
        print("Using 16-bit widths")
        for i in range(num_chars):
            pos = width_table_start + (i * 2)
            if pos + 1 < len(data):
                width = struct.unpack('>h', data[pos:pos+2])[0]
                widths.append(width)
    else:
        # 8-bit widths
        print("Using 8-bit widths")
        for i in range(num_chars):
            pos = width_table_start + i
            if pos < len(data):
                width = struct.unpack('b', data[pos:pos+1])[0]
                widths.append(width)

    print(f"Read {len(widths)} widths")

    return {
        'height': f_rect_height,
        'ascent': ascent,
        'descent': descent,
        'first_char': first_char,
        'last_char': last_char,
        'row_words': row_words,
        'bitmap': bitmap,
        'offsets': offsets,
        'widths': widths
    }

def generate_header(font_data, output_file):
    """Generate C header file"""
    with open(output_file, 'w') as f:
        f.write("/* Chicago font from NFNT resource */\n\n")
        f.write("#ifndef CHICAGO_FONT_H\n")
        f.write("#define CHICAGO_FONT_H\n\n")
        f.write("#include <stdint.h>\n\n")

        # Font metrics
        f.write(f"#define CHICAGO_HEIGHT {font_data['height']}\n")
        f.write(f"#define CHICAGO_FIRST_CHAR {font_data['first_char']}\n")
        f.write(f"#define CHICAGO_LAST_CHAR {font_data['last_char']}\n")
        f.write(f"#define CHICAGO_ASCENT {font_data['ascent']}\n")
        f.write(f"#define CHICAGO_DESCENT {font_data['descent']}\n")
        f.write(f"#define CHICAGO_ROW_WORDS {font_data['row_words']}\n")
        f.write(f"#define CHICAGO_NUM_CHARS {len(font_data['widths'])}\n\n")

        # Bitmap data
        f.write("/* Strike bitmap - all glyphs laid out horizontally */\n")
        f.write("static const uint8_t chicago_bitmap[] = {\n")
        for i in range(0, len(font_data['bitmap']), 16):
            f.write("    ")
            for j in range(min(16, len(font_data['bitmap']) - i)):
                f.write(f"0x{font_data['bitmap'][i+j]:02X}")
                if i+j < len(font_data['bitmap']) - 1:
                    f.write(", ")
            f.write("\n")
        f.write("};\n\n")

        # Offset table (WORD offsets into strike)
        f.write("/* Offset table - WORD offsets into strike bitmap */\n")
        f.write("static const uint16_t chicago_offsets[] = {\n")
        for i, offset in enumerate(font_data['offsets']):
            if i % 8 == 0:
                f.write("    ")
            f.write(f"{offset:5}")
            if i < len(font_data['offsets']) - 1:
                f.write(", ")
            if (i + 1) % 8 == 0:
                f.write("\n")
        if len(font_data['offsets']) % 8 != 0:
            f.write("\n")
        f.write("};\n\n")

        # Width table (logical advances)
        f.write("/* Width table - logical advance for each character */\n")
        f.write("static const int8_t chicago_widths[] = {\n")
        for i, width in enumerate(font_data['widths']):
            if i % 16 == 0:
                f.write("    ")
            f.write(f"{width:3}")
            if i < len(font_data['widths']) - 1:
                f.write(", ")
            if (i + 1) % 16 == 0:
                f.write("\n")
        if len(font_data['widths']) % 16 != 0:
            f.write("\n")
        f.write("};\n\n")

        f.write("#endif /* CHICAGO_FONT_H */\n")

        print(f"Generated {output_file}")

if __name__ == "__main__":
    with open("NFNT_5478.bin", "rb") as f:
        data = f.read()

    font_data = parse_nfnt(data)
    generate_header(font_data, "/home/k/iteration2/include/chicago_font.h")