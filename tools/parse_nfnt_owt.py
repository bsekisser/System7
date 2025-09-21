#!/usr/bin/env python3
"""
NFNT parser using OWT (Offset/Width Table) data
Since the bitmap location table is missing, we'll use the OWT data
and the known working offsets from chicago_fixed.h
"""
import struct

def parse_nfnt_owt(data):
    """Parse NFNT using OWT for widths"""

    # FontRec header (26 bytes total with row_words)
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

    print(f"Font metrics:")
    print(f"  Height: {f_rect_height}")
    print(f"  Ascent: {ascent}, Descent: {descent}")
    print(f"  Row words: {row_words}")
    print(f"  KernMax: {kern_max}")

    num_chars = last_char - first_char + 1

    # Strike bitmap starts at offset 26
    bitmap_start = 26
    bitmap_size = row_words * 2 * f_rect_height
    bitmap = data[bitmap_start:bitmap_start + bitmap_size]

    print(f"\nBitmap: {bitmap_size} bytes at 0x{bitmap_start:04X}")

    # OWT at byte offset
    owt_byte_offset = owt_loc * 2
    print(f"OWT at byte offset: 0x{owt_byte_offset:04X}")

    # Parse OWT - packed offset/width pairs
    char_info = []
    for i in range(num_chars):
        pos = owt_byte_offset + (i * 2)
        if pos + 1 < len(data):
            entry = struct.unpack('>H', data[pos:pos+2])[0]
            if entry == 0xFFFF:
                char_info.append(None)  # Missing glyph
            else:
                offset = (entry >> 8) & 0xFF  # High byte
                width = entry & 0xFF          # Low byte
                char_info.append((offset, width))
        else:
            char_info.append(None)

    return {
        'height': f_rect_height,
        'ascent': ascent,
        'descent': descent,
        'first_char': first_char,
        'last_char': last_char,
        'row_words': row_words,
        'kern_max': kern_max,
        'bitmap': bitmap,
        'char_info': char_info
    }

def generate_header(font_data, output_file):
    """Generate C header with OWT-based data and manual bit offsets"""
    with open(output_file, 'w') as f:
        f.write("/* Chicago font from NFNT with OWT data */\n\n")
        f.write("#ifndef CHICAGO_FONT_H\n")
        f.write("#define CHICAGO_FONT_H\n\n")
        f.write("#include <stdint.h>\n\n")

        # Font metrics
        f.write(f"#define CHICAGO_HEIGHT {font_data['height']}\n")
        f.write(f"#define CHICAGO_ASCENT {font_data['ascent']}\n")
        f.write(f"#define CHICAGO_DESCENT {font_data['descent']}\n")
        f.write(f"#define CHICAGO_ROW_WORDS {font_data['row_words']}\n")
        f.write(f"#define CHICAGO_KERN_MAX {font_data['kern_max']}\n\n")

        # Character info structure
        f.write("typedef struct {\n")
        f.write("    uint16_t bit_offset;  /* Bit offset in strike */\n")
        f.write("    int8_t left_offset;   /* Left side bearing (offset - kernMax) */\n")
        f.write("    uint8_t advance;      /* Logical advance width */\n")
        f.write("} ChicagoCharInfo;\n\n")

        # Bitmap data
        f.write("/* Strike bitmap */\n")
        f.write("static const uint8_t chicago_bitmap[] = {\n")
        for i in range(0, len(font_data['bitmap']), 16):
            f.write("    ")
            for j in range(min(16, len(font_data['bitmap']) - i)):
                f.write(f"0x{font_data['bitmap'][i+j]:02X}")
                if i+j < len(font_data['bitmap']) - 1:
                    f.write(", ")
            f.write("\n")
        f.write("};\n\n")

        # Manual bit offsets from chicago_fixed.h for ASCII printable
        # These are the working offsets we know are correct
        manual_offsets = {
            32: 344,  # space
            33: 360,  # !
            34: 384,  # "
            35: 448,  # #
            36: 488,  # $
            37: 560,  # %
            38: 624,  # &
            39: 632,  # '
            40: 656,  # (
            41: 680,  # )
            42: 720,  # *
            43: 760,  # +
            44: 776,  # ,
            45: 816,  # -
            46: 832,  # .
            47: 872,  # /
            48: 920,  # 0
            49: 944,  # 1
            50: 992,  # 2
            51: 1040, # 3
            52: 1096, # 4
            53: 1144, # 5
            54: 1192, # 6
            55: 1240, # 7
            56: 1288, # 8
            57: 1336, # 9
            58: 1352, # :
            59: 1368, # ;
            60: 1408, # <
            61: 1456, # =
            62: 1496, # >
            63: 1544, # ?
            64: 1616, # @
            65: 1664, # A
            66: 1712, # B
            67: 1760, # C
            68: 1808, # D
            69: 1848, # E
            70: 1888, # F
            71: 1936, # G
            72: 1984, # H
            73: 2000, # I
            74: 2040, # J
            75: 2088, # K
            76: 2104, # L
            77: 2144, # M
            78: 2200, # N
            79: 2248, # O
            80: 2296, # P
            81: 2344, # Q
            82: 2392, # R
            83: 2432, # S
            84: 2472, # T
            85: 2520, # U
            86: 2568, # V
            87: 2616, # W
            88: 2672, # X
            89: 2720, # Y
            90: 2768, # Z
            91: 2808, # [
            92: 2848, # \
            93: 2888, # ]
            94: 2928, # ^
            95: 3008, # _
            96: 3072, # `
            97: 3096, # a
            98: 3144, # b
            99: 3192, # c
            100: 3232, # d
            101: 3280, # e
            102: 3328, # f
            103: 3368, # g
            104: 3416, # h
            105: 3464, # i
            106: 3480, # j
            107: 3520, # k
            108: 3568, # l
            109: 3584, # m
            110: 3640, # n
            111: 3688, # o
            112: 3736, # p
            113: 3784, # q
            114: 3832, # r
            115: 3856, # s
            116: 3896, # t
            117: 3936, # u
            118: 3984, # v
            119: 4032, # w
            120: 4088, # x
            121: 4136, # y
            122: 4184, # z
            123: 4224, # {
            124: 4256, # |
            125: 4280, # }
            126: 4312, # ~
        }

        # Character info array for ASCII printable
        f.write("/* Character info for ASCII printable (space to ~) */\n")
        f.write("static const ChicagoCharInfo chicago_chars[] = {\n")

        for ch in range(32, 127):
            char_idx = ch - font_data['first_char']

            # Get bit offset from manual table
            bit_offset = manual_offsets.get(ch, 0)

            # Get OWT info if available
            if 0 <= char_idx < len(font_data['char_info']) and font_data['char_info'][char_idx]:
                offset, width = font_data['char_info'][char_idx]
                left_offset = offset - font_data['kern_max']
            else:
                # Default for missing chars
                left_offset = 0
                width = 8

            if ch == ord('\\'):
                f.write(f"    {{ {bit_offset:4}, {left_offset:2}, {width:2} }}, /* '\\\\' */\n")
            else:
                f.write(f"    {{ {bit_offset:4}, {left_offset:2}, {width:2} }}, /* '{chr(ch)}' */\n")

        f.write("};\n\n")
        f.write("#endif /* CHICAGO_FONT_H */\n")

        print(f"Generated {output_file}")

if __name__ == "__main__":
    with open("NFNT_5478.bin", "rb") as f:
        data = f.read()

    font_data = parse_nfnt_owt(data)
    generate_header(font_data, "/home/k/iteration2/include/chicago_font.h")