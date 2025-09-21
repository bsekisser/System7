#!/usr/bin/env python3
"""
Final NFNT parser with correct understanding of the format:
- Strike bitmap at 0x0018
- Location table (bit offsets) at 0x084C
- Width/Offset table (packed) at 0x0A24
"""
import struct

def parse_nfnt_final(data):
    """Parse NFNT with correct table locations"""

    # FontRec header
    font_type = struct.unpack('>H', data[0:2])[0]
    first_char = struct.unpack('>H', data[2:4])[0]
    last_char = struct.unpack('>H', data[4:6])[0]
    wid_max = struct.unpack('>H', data[6:8])[0]
    kern_max = struct.unpack('>h', data[8:10])[0]
    n_descent = struct.unpack('>h', data[10:12])[0]
    f_rect_width = struct.unpack('>H', data[12:14])[0]
    f_rect_height = struct.unpack('>H', data[14:16])[0]
    owt_loc = struct.unpack('>H', data[16:18])[0]
    ascent = struct.unpack('>H', data[18:20])[0]
    descent = struct.unpack('>H', data[20:22])[0]
    leading = struct.unpack('>H', data[22:24])[0]
    row_words = struct.unpack('>H', data[24:26])[0]

    num_chars = last_char - first_char + 1
    row_bytes = row_words * 2
    row_bits = row_words * 16

    print(f"Font metrics:")
    print(f"  Characters: 0x{first_char:02X} to 0x{last_char:02X} ({num_chars} glyphs)")
    print(f"  Height: {f_rect_height}, Ascent: {ascent}, Descent: {descent}")
    print(f"  Row: {row_words} words, {row_bytes} bytes, {row_bits} bits")

    # Strike bitmap at 0x0018 (note: not 0x001A!)
    strike_start = 0x0018
    strike_size = row_bytes * f_rect_height
    strike_end = strike_start + strike_size
    bitmap = data[strike_start:strike_end]

    print(f"\nStrike bitmap: 0x{strike_start:04X} to 0x{strike_end-1:04X} ({strike_size} bytes)")

    # Location table right after strike at 0x084C
    location_start = 0x084C
    locations = []
    for i in range(num_chars + 1):
        pos = location_start + (i * 2)
        if pos + 1 < len(data):
            bit_offset = struct.unpack('>H', data[pos:pos+2])[0]
            locations.append(bit_offset)

    print(f"\nLocation table: 0x{location_start:04X}")
    print(f"  {len(locations)} entries, last = {locations[-1]} bits (expected ~{row_bits})")

    # Width/Offset table at 0x0A24 (packed)
    owt_start = 0x0A24
    char_info = []
    for i in range(num_chars):
        pos = owt_start + (i * 2)
        if pos + 1 < len(data):
            packed = struct.unpack('>H', data[pos:pos+2])[0]
            if packed == 0xFFFF:
                char_info.append(None)  # Missing glyph
            else:
                left_offset = struct.unpack('b', bytes([(packed >> 8) & 0xFF]))[0]  # signed
                advance = struct.unpack('b', bytes([packed & 0xFF]))[0]  # signed
                char_info.append((left_offset, advance))

    # Check sentinel
    sentinel_pos = owt_start + (num_chars * 2)
    sentinel = struct.unpack('>H', data[sentinel_pos:sentinel_pos+2])[0]
    print(f"\nWidth/Offset table: 0x{owt_start:04X} to 0x{sentinel_pos-1:04X}")
    print(f"  Sentinel at 0x{sentinel_pos:04X}: 0x{sentinel:04X}")

    return {
        'height': f_rect_height,
        'ascent': ascent,
        'descent': descent,
        'first_char': first_char,
        'last_char': last_char,
        'row_words': row_words,
        'row_bytes': row_bytes,
        'kern_max': kern_max,
        'bitmap': bitmap,
        'locations': locations,
        'char_info': char_info
    }

def generate_header(font_data, output_file):
    """Generate C header with complete font data"""
    with open(output_file, 'w') as f:
        f.write("/* Chicago font from NFNT - complete parser */\n\n")
        f.write("#ifndef CHICAGO_FONT_H\n")
        f.write("#define CHICAGO_FONT_H\n\n")
        f.write("#include <stdint.h>\n\n")

        # Font metrics
        f.write(f"#define CHICAGO_HEIGHT {font_data['height']}\n")
        f.write(f"#define CHICAGO_ASCENT {font_data['ascent']}\n")
        f.write(f"#define CHICAGO_DESCENT {font_data['descent']}\n")
        f.write(f"#define CHICAGO_ROW_WORDS {font_data['row_words']}\n")
        f.write(f"#define CHICAGO_ROW_BYTES {font_data['row_bytes']}\n")
        f.write(f"#define CHICAGO_KERN_MAX {font_data['kern_max']}\n")
        f.write(f"#define CHICAGO_FIRST_CHAR {font_data['first_char']}\n")
        f.write(f"#define CHICAGO_LAST_CHAR {font_data['last_char']}\n\n")

        # Character info structure
        f.write("typedef struct {\n")
        f.write("    uint16_t bit_start;   /* Start bit offset in strike */\n")
        f.write("    uint16_t bit_width;   /* Width in bits (ink width) */\n")
        f.write("    int8_t left_offset;   /* Left side bearing */\n")
        f.write("    int8_t advance;       /* Logical advance width */\n")
        f.write("} ChicagoCharInfo;\n\n")

        # External bitmap declaration
        f.write("/* Strike bitmap data (defined in chicago_font_data.c) */\n")
        f.write("extern const uint8_t chicago_bitmap[2100];\n\n")

        # Character info for ASCII printable range
        f.write("/* Character info for ASCII printable (0x20 to 0x7E) */\n")
        f.write("static const ChicagoCharInfo chicago_ascii[] = {\n")

        for ch in range(0x20, 0x7F):
            char_idx = ch - font_data['first_char']

            # Get bit offsets from location table
            if char_idx < len(font_data['locations']) - 1:
                bit_start = font_data['locations'][char_idx]
                bit_end = font_data['locations'][char_idx + 1]
                bit_width = bit_end - bit_start
            else:
                bit_start = 0
                bit_width = 0

            # Get left offset and advance from OWT
            if 0 <= char_idx < len(font_data['char_info']) and font_data['char_info'][char_idx]:
                left_offset, advance = font_data['char_info'][char_idx]
            else:
                left_offset = 0
                advance = 0

            if ch == ord('\\'):
                f.write(f"    {{ {bit_start:4}, {bit_width:2}, {left_offset:3}, {advance:2} }},  /* '\\\\' */\n")
            else:
                f.write(f"    {{ {bit_start:4}, {bit_width:2}, {left_offset:3}, {advance:2} }},  /* '{chr(ch)}' */\n")

        f.write("};\n\n")
        f.write("#endif /* CHICAGO_FONT_H */\n")

    print(f"\nGenerated {output_file}")

def generate_bitmap_c(bitmap, output_file):
    """Generate C file with bitmap data"""
    with open(output_file, 'w') as f:
        f.write("/* Chicago font bitmap data from NFNT resource */\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write("/* Strike bitmap - 2100 bytes */\n")
        f.write("const uint8_t chicago_bitmap[2100] = {\n")

        for i in range(0, len(bitmap), 16):
            f.write("    ")
            for j in range(min(16, len(bitmap) - i)):
                f.write(f"0x{bitmap[i+j]:02X}")
                if i+j < len(bitmap) - 1:
                    f.write(", ")
            f.write("\n")
        f.write("};\n")

    print(f"Generated {output_file}")

if __name__ == "__main__":
    with open("NFNT_5478.bin", "rb") as f:
        data = f.read()

    font_data = parse_nfnt_final(data)

    # Show some ASCII characters for verification
    print("\nSample ASCII characters:")
    for ch_code in [0x20, 0x41, 0x61]:  # space, 'A', 'a'
        idx = ch_code - font_data['first_char']
        if idx < len(font_data['locations']) - 1:
            start = font_data['locations'][idx]
            end = font_data['locations'][idx + 1]
            width = end - start

            if font_data['char_info'][idx]:
                left, adv = font_data['char_info'][idx]
                char_str = f"'{chr(ch_code)}'" if 32 <= ch_code <= 126 else f"0x{ch_code:02X}"
                print(f"  {char_str}: bits {start}-{end} (width={width}), left={left}, advance={adv}")

    generate_header(font_data, "/home/k/iteration2/include/chicago_font.h")
    generate_bitmap_c(font_data['bitmap'], "/home/k/iteration2/src/chicago_font_data.c")