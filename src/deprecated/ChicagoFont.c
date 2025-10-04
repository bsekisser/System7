/*
 * ChicagoFont.c - Chicago bitmap font implementation
 * Uses the real Chicago font data from System 7.1
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

/* Global pen position - defined here since Text.c not compiled */
Point g_penPosition = {0, 0};

/* Current port from QuickDrawCore.c */
extern GrafPtr g_currentPort;

/* Draw a single Chicago character at given position */
static void DrawChicagoCharAt(short x, short y, char ch, uint32_t color) {
    if (!framebuffer) return;

    /* Handle only printable ASCII */
    if (ch < CHICAGO_FIRST_CHAR || ch > CHICAGO_LAST_CHAR) return;

    int charIndex = ch - CHICAGO_FIRST_CHAR;
    const uint8_t* bitmap = chicago_bitmaps[charIndex];
    uint32_t* fb = (uint32_t*)framebuffer;

    /* Draw each row of the character */
    for (int row = 0; row < CHICAGO_CHAR_HEIGHT; row++) {
        if (y + row >= fb_height) break;

        uint8_t rowData = bitmap[row];

        /* Draw each pixel in the row */
        for (int col = 0; col < 8; col++) {
            if (x + col >= fb_width) break;

            if (rowData & (0x80 >> col)) {
                /* Set pixel */
                int offset = (y + row) * (fb_pitch / 4) + (x + col);
                fb[offset] = color;
            }
        }
    }
}

/* QuickDraw text functions using Chicago font */
void DrawChar(short ch) {
    if (!g_currentPort || !framebuffer) return;

    /* Use thePort's pen position */
    Point penPos = g_currentPort->pnLoc;

    /* Draw at current pen position with black */
    DrawChicagoCharAt(penPos.h, penPos.v, (char)ch, pack_color(0, 0, 0));

    /* Advance pen position by character width */
    if (ch >= CHICAGO_FIRST_CHAR && ch <= CHICAGO_LAST_CHAR) {
        g_currentPort->pnLoc.h += chicago_widths[ch - CHICAGO_FIRST_CHAR];
    }
}

void DrawString(ConstStr255Param s) {
    if (!s || s[0] == 0) return;

    for (int i = 1; i <= s[0]; i++) {
        DrawChar(s[i]);
    }
}

void DrawText(const void* textBuf, short firstByte, short byteCount) {
    if (!textBuf || byteCount <= 0) return;

    const char* text = (const char*)textBuf;
    for (short i = 0; i < byteCount; i++) {
        DrawChar(text[firstByte + i]);
    }
}

/* Text measurement functions for Chicago font */
short CharWidth(short ch) {
    if (ch < CHICAGO_FIRST_CHAR || ch > CHICAGO_LAST_CHAR) {
        return 7;  /* Default width */
    }
    return chicago_widths[ch - CHICAGO_FIRST_CHAR];
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

/* Helper functions for drawing text at specific positions */
void DrawStringAt(short x, short y, const char* str, uint32_t color) {
    if (!str) return;

    short currentX = x;
    while (*str) {
        DrawChicagoCharAt(currentX, y, *str, color);
        currentX += CharWidth(*str);
        str++;
    }
}

void DrawPStringAt(short x, short y, ConstStr255Param pstr, uint32_t color) {
    if (!pstr || pstr[0] == 0) return;

    short currentX = x;
    for (int i = 1; i <= pstr[0]; i++) {
        DrawChicagoCharAt(currentX, y, pstr[i], color);
        currentX += CharWidth(pstr[i]);
    }
}

/* Initialize Chicago font (called from InitFonts) */
void InitChicagoFont(void) {
    /* Nothing to initialize for built-in font */
}