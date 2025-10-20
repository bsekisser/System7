#include "../../include/SystemTypes.h"
#include "QuickDraw/QuickDrawInternal.h"
#include "../../include/QuickDraw/QuickDraw.h"
#include "../../include/chicago_font.h"
#include <stddef.h>

// Global current port
GrafPtr g_currentPort = NULL;

// Current pen position (since GrafPort doesn't have h,v)
static Point g_penPosition = {0, 0};

// Font metrics cache
typedef struct {
    short fontNum;
    short fontSize;
    Style fontStyle;
    short ascent;
    short descent;
    short widMax;
    short leading;
    Boolean valid;
} FontMetricsCache;

static FontMetricsCache g_fontCache = {0};

// Forward declarations for functions we'll define
SInt16 CharWidth(SInt16 ch);
SInt16 StringWidth(ConstStr255Param s);
SInt16 TextWidth(const void *textBuf, SInt16 firstByte, SInt16 byteCount);

// Move pen position
void Move(SInt16 h, SInt16 v) {
    if (g_currentPort == NULL) return;
    g_penPosition.h += h;
    g_penPosition.v += v;
    /* Also update port's pen location for drawing operations */
    g_currentPort->pnLoc.h += h;
    g_currentPort->pnLoc.v += v;
}

void MoveTo(SInt16 h, SInt16 v) {
    if (g_currentPort == NULL) return;
    g_penPosition.h = h;
    g_penPosition.v = v;
    /* Also update port's pen location for drawing operations */
    g_currentPort->pnLoc.h = h;
    g_currentPort->pnLoc.v = v;
}

void GetPen(Point *pt) {
    if (pt == NULL || g_currentPort == NULL) return;
    /* Return the port's pen location (canonical source) */
    *pt = g_currentPort->pnLoc;
}

// Text measurement functions
SInt16 CharWidth(SInt16 ch) {
    if (g_currentPort == NULL) return 0;

    /* Use actual Chicago font metrics for printable ASCII */
    if (ch >= 0x20 && ch <= 0x7E) {
        const ChicagoCharInfo *info = &chicago_ascii[ch - 0x20];
        return info->advance;
    }

    /* Fallback for non-printable characters */
    short fontSize = g_currentPort->txSize;
    if (fontSize == 0) fontSize = 12;
    return (fontSize * 2) / 3;
}

SInt16 StringWidth(ConstStr255Param s) {
    if (s == NULL || s[0] == 0) return 0;

    SInt16 width = 0;
    for (int i = 1; i <= s[0]; i++) {
        width += CharWidth(s[i]);
    }
    return width;
}

SInt16 TextWidth(const void *textBuf, SInt16 firstByte, SInt16 byteCount) {
    if (textBuf == NULL || byteCount <= 0) return 0;

    const UInt8 *text = (const UInt8 *)textBuf;
    SInt16 width = 0;

    for (SInt16 i = firstByte; i < firstByte + byteCount && i < 255; i++) {
        width += CharWidth(text[i]);
    }

    return width;
}

// Text drawing functions
void DrawChar(SInt16 ch) {
    if (g_currentPort == NULL) return;

    /* Get current pen location in local coordinates */
    Point penLocal = g_currentPort->pnLoc;

    /* Default width for non-printable characters */
    SInt16 width = CharWidth(ch);

    /* Render the glyph if it's a printable ASCII character */
    if (ch >= 0x20 && ch <= 0x7E) {
        /* Get glyph info */
        const ChicagoCharInfo *info = &chicago_ascii[ch - 0x20];

        /* Use actual font metrics for advance width */
        width = info->advance;

        if (info->bit_width > 0) {
            /* Convert pen position to global coordinates for platform drawing */
            Point penGlobal = penLocal;
            LocalToGlobal(&penGlobal);

            /* Extract glyph bitmap from strike */
            uint8_t glyph_bitmap[CHICAGO_HEIGHT * 16];  /* Max 16 bytes per row */
            memset(glyph_bitmap, 0, sizeof(glyph_bitmap));

            for (int row = 0; row < CHICAGO_HEIGHT; row++) {
                uint16_t src_bit = info->bit_start;
                uint16_t src_byte = (row * CHICAGO_ROW_BYTES) + (src_bit / 8);
                uint8_t src_bit_offset = src_bit % 8;

                /* Extract bits for this glyph row */
                for (int bit = 0; bit < info->bit_width; bit++) {
                    uint8_t bit_val = (chicago_bitmap[src_byte + (src_bit_offset + bit) / 8]
                                      >> (7 - ((src_bit_offset + bit) % 8))) & 1;
                    if (bit_val) {
                        glyph_bitmap[row * 2 + (bit / 8)] |= (0x80 >> (bit % 8));
                    }
                }
            }

            /* Draw the glyph using platform drawing */
            extern void QDPlatform_DrawGlyphBitmap(GrafPtr port, Point pen,
                                                  const uint8_t *bitmap,
                                                  SInt16 width, SInt16 height,
                                                  const Pattern *pattern, SInt16 mode);

            QDPlatform_DrawGlyphBitmap(g_currentPort, penGlobal, glyph_bitmap,
                                      info->bit_width, CHICAGO_HEIGHT,
                                      &g_currentPort->pnPat, g_currentPort->pnMode);
        }
    }

    /* Advance pen position in local coordinates */
    g_currentPort->pnLoc.h += width;
    g_penPosition.h += width;  /* Keep global pen position in sync */
}

void DrawString(ConstStr255Param s) {
    if (g_currentPort == NULL || s == NULL) return;

    for (int i = 1; i <= s[0]; i++) {
        DrawChar(s[i]);
    }
}

void DrawText(const void *textBuf, SInt16 firstByte, SInt16 byteCount) {
    if (g_currentPort == NULL || textBuf == NULL) return;

    const UInt8 *text = (const UInt8 *)textBuf;

    for (SInt16 i = firstByte; i < firstByte + byteCount && i < 255; i++) {
        DrawChar(text[i]);
    }
}

// Text style functions
void TextFont(SInt16 fontNum) {
    if (g_currentPort == NULL) return;
    g_currentPort->txFont = fontNum;
    g_fontCache.valid = false;
}

void TextFace(Style face) {
    if (g_currentPort == NULL) return;
    g_currentPort->txFace = face;
    g_fontCache.valid = false;
}

void TextMode(SInt16 mode) {
    if (g_currentPort == NULL) return;
    g_currentPort->txMode = mode;
}

void TextSize(SInt16 size) {
    if (g_currentPort == NULL) return;
    g_currentPort->txSize = size;
    g_fontCache.valid = false;
}

void SpaceExtra(Fixed extra) {
    if (g_currentPort == NULL) return;
    g_currentPort->spExtra = extra;
}

// Font metrics functions
void GetFontInfo(FontInfo *info) {
    if (info == NULL || g_currentPort == NULL) return;

    // Update cache if needed
    if (!g_fontCache.valid) {
        g_fontCache.fontNum = g_currentPort->txFont;
        g_fontCache.fontSize = g_currentPort->txSize;
        g_fontCache.fontStyle = g_currentPort->txFace;

        // Calculate metrics based on font size
        short size = g_fontCache.fontSize;
        if (size == 0) size = 12;

        g_fontCache.ascent = (size * 3) / 4;
        g_fontCache.descent = size / 4;
        g_fontCache.widMax = size;
        g_fontCache.leading = size / 6;
        g_fontCache.valid = true;
    }

    info->ascent = g_fontCache.ascent;
    info->descent = g_fontCache.descent;
    info->widMax = g_fontCache.widMax;
    info->leading = g_fontCache.leading;
}

// Implementation helpers
static void UpdateFontCache(void) {
    if (g_currentPort == NULL) return;

    short fontNum = g_currentPort->txFont;
    short fontSize = g_currentPort->txSize;
    Style fontStyle = g_currentPort->txFace;

    // Apply style modifications
    if (fontStyle & bold) {
        // Bold increases width
    }
    if (fontStyle & italic) {
        // Italic adjusts angle
    }
    if (fontStyle & extend) {
        // Extended increases width
    }
    if (fontStyle & condense) {
        // Condensed decreases width
    }

    g_fontCache.valid = true;
}

static void DrawTextString(const UInt8 *text, SInt16 length, Point pos, Style style) {
    if (text == NULL || g_currentPort == NULL) return;

    // Save position
    Point startPos = pos;

    // Draw main text
    for (SInt16 i = 0; i < length; i++) {
        // Draw character
        // (Actual implementation would render glyph)
        pos.h += CharWidth(text[i]);
    }

    // Apply underline if needed
    if (g_currentPort->txFace & underline) {
        // Draw underline from startPos to pos
        MoveTo(startPos.h, startPos.v + 2);
        LineTo(pos.h, startPos.v + 2);
    }

    // Apply shadow if needed
    if (g_currentPort->txFace & shadow) {
        // Draw shadow offset by 1,1
        Point shadowPos = startPos;
        shadowPos.h++;
        shadowPos.v++;
        // Draw shadow text
    }
}
