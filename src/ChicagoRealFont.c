/*
 * ChicagoRealFont.c - Real Chicago font from System 7.1 resources
 * Uses the actual NFNT resource data
 */

#include "SystemTypes.h"
#include "QuickDraw/QuickDraw.h"
#include "chicago_font.h"

/* External framebuffer from multiboot */
extern void* framebuffer;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;
extern uint32_t pack_color(uint8_t r, uint8_t g, uint8_t b);

/* Current port from QuickDrawCore.c */
extern GrafPtr g_currentPort;

/* Helper to get a bit from MSB-first bitmap */
static inline uint8_t get_bit(const uint8_t *row, int bitOff) {
    /* MSB-first within byte */
    return (row[bitOff >> 3] >> (7 - (bitOff & 7))) & 1;
}

/* Draw a character from the real Chicago font */
static void DrawRealChicagoChar(short x, short y, char ch, uint32_t color) {
    if (!framebuffer) return;

    /* Bounds check */
    if (x < 0 || y < 0) return;
    if (y + CHICAGO_HEIGHT > fb_height) return;

    /* Only handle ASCII printable range */
    if (ch < 32 || ch > 126) return;

    /* Get character info */
    ChicagoCharInfo info = chicago_ascii[ch - 32];

    /* Apply left side bearing */
    x += info.left_offset;

    /* Draw the character from the strike bitmap */
    uint32_t *fb = (uint32_t*)framebuffer;

    /* The strike is a 1-bpp bitmap with CHICAGO_HEIGHT rows */
    /* Each row is CHICAGO_ROW_BYTES (140) bytes wide */
    /* info.bit_start is the bit offset where this glyph starts */

    for (int row = 0; row < CHICAGO_HEIGHT; row++) {
        if (y + row >= fb_height) break;

        /* Get pointer to start of this row in the strike */
        const uint8_t *strike_row = chicago_bitmap + (row * CHICAGO_ROW_BYTES);

        /* Extract the glyph bits for this row */
        for (int col = 0; col < info.bit_width; col++) {
            if (x + col >= fb_width) break;

            /* The bit we want is at horizontal position (bit_start + col) in this row */
            int bit_position = info.bit_start + col;

            /* Get the bit using MSB-first extraction */
            if (get_bit(strike_row, bit_position)) {
                /* Set pixel in framebuffer */
                int fb_offset = (y + row) * (fb_pitch / 4) + (x + col);
                fb[fb_offset] = color;
            }
        }
    }
}

/* QuickDraw text functions using real Chicago font */
void DrawChar(short ch) {
    extern void serial_printf(const char* fmt, ...);

    /* serial_printf("DrawChar called: ch='%c' (0x%02x), fb=%p\n", ch, ch, framebuffer); */

    /* Don't check g_currentPort for menu drawing - just check framebuffer */
    if (!framebuffer) {
        /* serial_printf("DrawChar: no framebuffer!\n"); */
        return;
    }

    /* Use thePort's pen position if available, otherwise use default */
    Point penPos = {0, 0};
    if (g_currentPort) {
        penPos = g_currentPort->pnLoc;
    }

    /* Draw at current pen position with black */
    DrawRealChicagoChar(penPos.h, penPos.v - CHICAGO_ASCENT, (char)ch, pack_color(0, 0, 0));

    /* Advance pen position by character advance width */
    if (g_currentPort && ch >= 32 && ch <= 126) {
        g_currentPort->pnLoc.h += chicago_ascii[ch - 32].advance;
    }
}

void DrawString(ConstStr255Param s) {
    if (!s || s[0] == 0) return;

    for (int i = 1; i <= s[0]; i++) {
        DrawChar(s[i]);
    }
}

void DrawText(const void* textBuf, short firstByte, short byteCount) {
    extern void serial_printf(const char* fmt, ...);
    extern void serial_puts(const char* str);
    extern void MoveTo(short h, short v);

    /* serial_printf("DrawText called: firstByte=%d, byteCount=%d\n", firstByte, byteCount); */

    if (!textBuf || byteCount <= 0) {
        /* serial_puts("DrawText early return\n"); */
        return;
    }

    const char* text = (const char*)textBuf;

    /* Save initial pen position */
    Point startPos = {0, 0};
    if (g_currentPort) {
        startPos = g_currentPort->pnLoc;
    }

    /* Draw each character with proper spacing */
    int accumulated_width = 0;
    for (short i = 0; i < byteCount; i++) {
        char ch = text[firstByte + i];
        /* serial_printf("Drawing char %d: '%c' (0x%02x) at h=%d\n", i, ch, ch, startPos.h + accumulated_width); */

        /* Move to position for this character */
        MoveTo(startPos.h + accumulated_width, startPos.v);

        /* Draw the character (DrawChar won't advance since we're managing position) */
        DrawChar(ch);

        /* Accumulate width for next character */
        /* Use bit_width + 1 for spacing instead of the incorrect advance field */
        if (ch >= 32 && ch <= 126) {
            accumulated_width += chicago_ascii[ch - 32].bit_width + 1;
        }
    }

    /* Set final pen position */
    if (g_currentPort) {
        g_currentPort->pnLoc.h = startPos.h + accumulated_width;
    }

    /* serial_puts("DrawText done\n"); */
}

/* Font measurement functions */
short CharWidth(short ch) {
    if (ch >= 32 && ch <= 126) {
        /* Use bit_width + 1 for spacing instead of incorrect advance */
        return chicago_ascii[ch - 32].bit_width + 1;
    }
    return 0;
}

short StringWidth(ConstStr255Param s) {
    if (!s || s[0] == 0) return 0;

    short width = 0;
    for (int i = 1; i <= s[0]; i++) {
        width += CharWidth(s[i]);
    }
    return width;
}

short TextWidth(const void* textBuf, short firstByte, short byteCount) {
    if (!textBuf || byteCount <= 0) return 0;

    const char* text = (const char*)textBuf;
    short width = 0;
    for (short i = 0; i < byteCount; i++) {
        width += CharWidth(text[firstByte + i]);
    }
    return width;
}

/* Utility function for debugging */
void DrawCharAt(short x, short y, char ch) {
    DrawRealChicagoChar(x, y, ch, pack_color(0, 0, 0));
}