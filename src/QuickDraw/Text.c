#include "../../include/SystemTypes.h"
#include "../../include/QuickDraw/QuickDraw.h"
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
}

void MoveTo(SInt16 h, SInt16 v) {
    if (g_currentPort == NULL) return;
    g_penPosition.h = h;
    g_penPosition.v = v;
}

void GetPen(Point *pt) {
    if (pt == NULL) return;
    *pt = g_penPosition;
}

// Text measurement functions
SInt16 CharWidth(SInt16 ch) {
    // Simple fixed-width approximation
    if (g_currentPort == NULL) return 0;

    short fontSize = g_currentPort->txSize;
    if (fontSize == 0) fontSize = 12;

    // Approximate character width
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

    // Draw character at current position
    // (Implementation would draw actual glyph)

    // Advance pen position
    g_penPosition.h += CharWidth(ch);
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
