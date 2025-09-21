/*
 * parse_nfnt.c - Parse Mac NFNT (bitmap font) resources
 * Converts to C header file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#pragma pack(push, 1)

/* NFNT font resource structure */
typedef struct {
    uint16_t fontType;      /* font type */
    uint16_t firstChar;     /* ASCII code of first character */
    uint16_t lastChar;      /* ASCII code of last character */
    uint16_t widMax;        /* maximum character width */
    int16_t  kernMax;       /* negative of maximum character kern */
    int16_t  nDescent;      /* negative of descent */
    uint16_t fRectWidth;    /* width of font rectangle */
    uint16_t fRectHeight;   /* height of font rectangle */
    uint16_t owTLoc;        /* offset to offset/width table */
    uint16_t ascent;        /* ascent */
    uint16_t descent;       /* descent */
    uint16_t leading;       /* leading */
    uint16_t rowWords;      /* row width of bit image in words */
} NFNTHeader;

#pragma pack(pop)

uint16_t swap16(uint16_t val) {
    return ((val & 0xFF) << 8) | ((val >> 8) & 0xFF);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <NFNT_file> <output.h>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        printf("Cannot open file: %s\n", argv[1]);
        return 1;
    }

    /* Read NFNT header */
    NFNTHeader header;
    fread(&header, sizeof(NFNTHeader), 1, f);

    /* Convert from big-endian */
    header.fontType = swap16(header.fontType);
    header.firstChar = swap16(header.firstChar);
    header.lastChar = swap16(header.lastChar);
    header.widMax = swap16(header.widMax);
    header.kernMax = (int16_t)swap16((uint16_t)header.kernMax);
    header.nDescent = (int16_t)swap16((uint16_t)header.nDescent);
    header.fRectWidth = swap16(header.fRectWidth);
    header.fRectHeight = swap16(header.fRectHeight);
    header.owTLoc = swap16(header.owTLoc);
    header.ascent = swap16(header.ascent);
    header.descent = swap16(header.descent);
    header.leading = swap16(header.leading);
    header.rowWords = swap16(header.rowWords);

    printf("NFNT Font Information:\n");
    printf("Font Type: 0x%04X\n", header.fontType);
    printf("First Char: %d\n", header.firstChar);
    printf("Last Char: %d\n", header.lastChar);
    printf("Max Width: %d\n", header.widMax);
    printf("Font Height: %d\n", header.fRectHeight);
    printf("Font Width: %d\n", header.fRectWidth);
    printf("Ascent: %d\n", header.ascent);
    printf("Descent: %d\n", header.descent);
    printf("Row Words: %d\n", header.rowWords);
    printf("\n");

    int num_chars = header.lastChar - header.firstChar + 1 + 1; /* +1 for missing char */

    /* Read bitmap data */
    int bitmap_size = header.rowWords * 2 * header.fRectHeight;
    uint8_t *bitmap_data = malloc(bitmap_size);
    fread(bitmap_data, 1, bitmap_size, f);

    /* Read offset/width table */
    fseek(f, header.owTLoc, SEEK_SET);
    uint16_t *ow_table = malloc((num_chars + 1) * sizeof(uint16_t));
    fread(ow_table, sizeof(uint16_t), num_chars + 1, f);

    /* Convert offset/width table from big-endian */
    for (int i = 0; i <= num_chars; i++) {
        ow_table[i] = swap16(ow_table[i]);
    }

    /* Generate C header file */
    FILE *out = fopen(argv[2], "w");
    if (!out) {
        printf("Cannot create output file: %s\n", argv[2]);
        fclose(f);
        return 1;
    }

    fprintf(out, "/* Generated from %s */\n\n", argv[1]);
    fprintf(out, "#ifndef CHICAGO_REAL_FONT_H\n");
    fprintf(out, "#define CHICAGO_REAL_FONT_H\n\n");

    fprintf(out, "#define CHICAGO_REAL_HEIGHT %d\n", header.fRectHeight);
    fprintf(out, "#define CHICAGO_REAL_FIRST_CHAR %d\n", header.firstChar);
    fprintf(out, "#define CHICAGO_REAL_LAST_CHAR %d\n", header.lastChar);
    fprintf(out, "#define CHICAGO_REAL_ASCENT %d\n", header.ascent);
    fprintf(out, "#define CHICAGO_REAL_DESCENT %d\n", header.descent);
    fprintf(out, "#define CHICAGO_REAL_ROW_WORDS %d\n\n", header.rowWords);

    /* Output bitmap data */
    fprintf(out, "static const uint8_t chicago_real_bitmap[] = {\n");
    for (int i = 0; i < bitmap_size; i++) {
        if (i % 16 == 0) fprintf(out, "    ");
        fprintf(out, "0x%02X", bitmap_data[i]);
        if (i < bitmap_size - 1) fprintf(out, ",");
        if (i % 16 == 15) fprintf(out, "\n");
        else if (i < bitmap_size - 1) fprintf(out, " ");
    }
    if ((bitmap_size - 1) % 16 != 15) fprintf(out, "\n");
    fprintf(out, "};\n\n");

    /* Output character widths and offsets */
    fprintf(out, "typedef struct {\n");
    fprintf(out, "    uint16_t offset;  /* Bit offset in row */\n");
    fprintf(out, "    uint8_t width;    /* Character width */\n");
    fprintf(out, "} CharInfo;\n\n");

    fprintf(out, "static const CharInfo chicago_real_chars[] = {\n");
    for (int i = 0; i < num_chars; i++) {
        uint16_t ow = ow_table[i];
        uint16_t next_ow = ow_table[i + 1];

        /* Offset is the full 16-bit value */
        uint16_t offset = ow;
        uint16_t next_offset = next_ow;
        uint8_t width = next_offset - offset;

        fprintf(out, "    {%3d, %2d}", offset, width);
        if (i < num_chars - 1) fprintf(out, ",");

        if (i == 0) {
            fprintf(out, "  /* Missing char */");
        } else {
            int ch = header.firstChar + i - 1;
            if (ch >= 32 && ch <= 126) {
                fprintf(out, "  /* '%c' */", ch);
            } else {
                fprintf(out, "  /* 0x%02X */", ch);
            }
        }
        fprintf(out, "\n");
    }
    fprintf(out, "};\n\n");

    /* Helper function to get character bitmap */
    fprintf(out, "/* Get character bitmap row */\n");
    fprintf(out, "static inline uint32_t get_chicago_real_char_row(int ch, int row) {\n");
    fprintf(out, "    if (ch < CHICAGO_REAL_FIRST_CHAR || ch > CHICAGO_REAL_LAST_CHAR) {\n");
    fprintf(out, "        ch = 0; /* Use missing char glyph */\n");
    fprintf(out, "    } else {\n");
    fprintf(out, "        ch = ch - CHICAGO_REAL_FIRST_CHAR + 1;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    \n");
    fprintf(out, "    CharInfo info = chicago_real_chars[ch];\n");
    fprintf(out, "    int byte_offset = row * CHICAGO_REAL_ROW_WORDS * 2;\n");
    fprintf(out, "    int bit_offset = info.offset;\n");
    fprintf(out, "    \n");
    fprintf(out, "    /* Extract bits from bitmap */\n");
    fprintf(out, "    uint32_t bits = 0;\n");
    fprintf(out, "    int byte_idx = byte_offset + (bit_offset / 8);\n");
    fprintf(out, "    int bit_idx = bit_offset %% 8;\n");
    fprintf(out, "    \n");
    fprintf(out, "    for (int i = 0; i < info.width; i++) {\n");
    fprintf(out, "        if (chicago_real_bitmap[byte_idx] & (0x80 >> bit_idx)) {\n");
    fprintf(out, "            bits |= (1 << (info.width - 1 - i));\n");
    fprintf(out, "        }\n");
    fprintf(out, "        bit_idx++;\n");
    fprintf(out, "        if (bit_idx >= 8) {\n");
    fprintf(out, "            bit_idx = 0;\n");
    fprintf(out, "            byte_idx++;\n");
    fprintf(out, "        }\n");
    fprintf(out, "    }\n");
    fprintf(out, "    \n");
    fprintf(out, "    return bits;\n");
    fprintf(out, "}\n\n");

    fprintf(out, "#endif /* CHICAGO_REAL_FONT_H */\n");

    fclose(out);
    fclose(f);
    free(bitmap_data);
    free(ow_table);

    printf("Generated %s\n", argv[2]);
    return 0;
}