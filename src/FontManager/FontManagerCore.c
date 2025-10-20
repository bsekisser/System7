/*
 * FontManagerCore.c - Core Font Manager Implementation
 *
 * System 7.1-compatible Font Manager with bitmap font support
 * Integrates with existing Chicago font implementation
 */

#include "FontManager/FontInternal.h"
#include "FontManager/FontManager.h"
#include "FontManager/FontTypes.h"
#include "QuickDraw/ColorQuickDraw.h"
#include "QuickDraw/QuickDraw.h"
#include "SystemTypes.h"
#include "chicago_font.h"
#include <string.h>
#include "FontManager/FontLogging.h"

/* Debug logging */
#define FM_DEBUG 1

#if FM_DEBUG
#define FM_LOG(...) FONT_LOG_DEBUG("FM: " __VA_ARGS__)
#else
#define FM_LOG(...)
#endif

/* Ensure boolean constants are defined */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* External dependencies */
extern GrafPtr g_currentPort;

/* External framebuffer from multiboot */
extern void* framebuffer;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;
extern uint32_t pack_color(uint8_t r, uint8_t g, uint8_t b);

/* Global Font Manager state */
static FontManagerState g_fmState = {0};

/* Font stack for nested font operations (push/pop semantics) */
#define MAX_FONT_STACK 16
typedef struct FontStackEntry {
    SInt16 fontNum;    /* Font family ID */
    SInt16 fontSize;   /* Size in points */
    Style  fontFace;   /* Style flags (bold, italic, etc.) */
    SInt32 fgColor;    /* Foreground color */
} FontStackEntry;

static FontStackEntry g_fontStack[MAX_FONT_STACK];
static SInt16 g_fontStackDepth = 0;

/* ============================================================================
 * Internal Low-Level Character Drawing
 * ============================================================================ */

/* Helper to get a bit from MSB-first bitmap */
static inline uint8_t get_bit(const uint8_t *row, int bitOff) {
    return (row[bitOff >> 3] >> (7 - (bitOff & 7))) & 1;
}

/*
 * FM_DrawChicagoCharInternal - Internal function to draw a Chicago character at pixel level
 * This is the core drawing function used by all Font Manager rendering
 */
void FM_DrawChicagoCharInternal(short x, short y, char ch, uint32_t color) {
    static int debug_count = 0;
    extern void serial_puts(const char* str);

    if (ch < 32 || ch > 126) return;

    ChicagoCharInfo info = chicago_ascii[ch - 32];
    x += info.left_offset;

    Ptr destBase = NULL;
    SInt16 destRowBytes = 0;
    SInt32 destWidth = fb_width;
    SInt32 destHeight = fb_height;
    SInt32 destXOrigin = 0;
    SInt32 destYOrigin = 0;

    if (g_currentPort && g_currentPort->portBits.baseAddr) {
        destBase = g_currentPort->portBits.baseAddr;
        destRowBytes = g_currentPort->portBits.rowBytes & 0x3FFF;

        /* For About window with direct framebuffer rendering:
         * - baseAddr points to window position in framebuffer (framebuffer + offset)
         * - bounds is (0,0,width,height) in local coordinates
         * - We need destXOrigin=0, destYOrigin=0 since baseAddr is already offset
         * This is the "direct framebuffer" case, which is actually correct! */

        /* Use window dimensions from portRect */
        destWidth = g_currentPort->portRect.right - g_currentPort->portRect.left;
        destHeight = g_currentPort->portRect.bottom - g_currentPort->portRect.top;
        destXOrigin = g_currentPort->portBits.bounds.left;  /* Should be 0 for offset baseAddr */
        destYOrigin = g_currentPort->portBits.bounds.top;   /* Should be 0 for offset baseAddr */

        if (debug_count < 3) {
            extern void serial_printf(const char* fmt, ...);
            serial_printf("[CHICAGO] x=%d y=%d destWidth=%d destHeight=%d destXOrigin=%d destYOrigin=%d\n",
                         x, y, destWidth, destHeight, destXOrigin, destYOrigin);
            debug_count++;
        }
    } else if (framebuffer) {
        destBase = (Ptr)framebuffer;
        destRowBytes = fb_pitch;
    }

    if (!destBase || destRowBytes <= 0 || destWidth <= 0 || destHeight <= 0) {
        if (debug_count < 3) {
            serial_puts("[CHICAGO] Invalid buffer parameters, returning\n");
        }
        return;
    }

    int pixels_drawn = 0;
    int first_pixel_x = -1, first_pixel_y = -1;

    for (int row = 0; row < CHICAGO_HEIGHT; row++) {
        int destY = y + row - destYOrigin;
        if (destY < 0 || destY >= destHeight) {
            continue;
        }

        const uint8_t *strike_row = chicago_bitmap + (row * CHICAGO_ROW_BYTES);

        for (int col = 0; col < info.bit_width; col++) {
            int destX = x + col - destXOrigin;
            if (destX < 0 || destX >= destWidth) {
                continue;
            }
            int bit_position = info.bit_start + col;
            if (get_bit(strike_row, bit_position)) {
                uint8_t* dstRow = (uint8_t*)destBase + destY * destRowBytes;
                uint32_t* dstPixels = (uint32_t*)dstRow;
                dstPixels[destX] = color;
                if (first_pixel_x < 0) {
                    first_pixel_x = destX;
                    first_pixel_y = destY;
                }
                pixels_drawn++;
            }
        }
    }

    if (debug_count < 3 && pixels_drawn > 0) {
        extern void serial_printf(const char* fmt, ...);
        serial_printf("[CHICAGO] Drew %d pixels, first at (%d,%d)\n", pixels_drawn, first_pixel_x, first_pixel_y);
    }
}

/* Built-in Chicago font strike (from chicago_font.h) */
static FontStrike g_chicagoStrike12 = {
    chicagoFont,      /* familyID */
    12,               /* size */
    normal,           /* face */
    FALSE,            /* synthetic */
    CHICAGO_ASCENT,   /* ascent */
    CHICAGO_DESCENT,  /* descent */
    CHICAGO_LEADING,  /* leading */
    16,               /* widMax - maximum width of Chicago chars */
    32,               /* firstChar */
    126,              /* lastChar */
    0,                /* rowWords - calculated from CHICAGO_ROW_BYTES */
    CHICAGO_HEIGHT,   /* fRectHeight */
    NULL,             /* bitmapData - use chicago_bitmap directly */
    NULL,             /* locTable */
    NULL,             /* widthTable */
    NULL,             /* next */
    NULL,             /* prev */
    0                 /* lastUsed */
};

/* Built-in font families */
static FontFamily g_chicagoFamily = {
    chicagoFont,
    {7, 'C','h','i','c','a','g','o'},  /* Pascal string */
    NULL,   /* fondHandle */
    TRUE,   /* hasNFNT */
    FALSE,  /* hasTrueType */
    NULL    /* next */
};

static FontFamily g_genevaFamily = {
    genevaFont,
    {6, 'G','e','n','e','v','a'},  /* Pascal string */
    NULL,   /* fondHandle */
    TRUE,   /* hasNFNT */
    FALSE,  /* hasTrueType */
    NULL    /* next */
};

static FontFamily g_monacoFamily = {
    monacoFont,
    {6, 'M','o','n','a','c','o'},  /* Pascal string */
    NULL,   /* fondHandle */
    TRUE,   /* hasNFNT */
    FALSE,  /* hasTrueType */
    NULL    /* next */
};

/* Helper to compare pascal strings */
static Boolean EqualString(const unsigned char *s1, const unsigned char *s2, Boolean caseSensitive, Boolean diacSensitive) {
    short len1 = s1[0];
    short len2 = s2[0];
    if (len1 != len2) return FALSE;
    for (short i = 1; i <= len1; i++) {
        unsigned char c1 = s1[i];
        unsigned char c2 = s2[i];
        if (!caseSensitive) {
            if (c1 >= 'a' && c1 <= 'z') c1 -= 32;
            if (c2 >= 'a' && c2 <= 'z') c2 -= 32;
        }
        if (c1 != c2) return FALSE;
    }
    return TRUE;
}

/* ============================================================================
 * Initialization
 * ============================================================================ */

void InitFonts(void) {
    if (g_fmState.initialized) {
        FM_LOG("InitFonts: Already initialized\n");
        return;
    }

    FM_LOG("InitFonts: Initializing Font Manager\n");

    /* Set up family list */
    g_chicagoFamily.next = &g_genevaFamily;
    g_genevaFamily.next = &g_monacoFamily;
    g_monacoFamily.next = NULL;
    g_fmState.familyList = &g_chicagoFamily;

    /* Set up strike cache with Chicago 12 */
    g_chicagoStrike12.rowWords = CHICAGO_ROW_BYTES / 2;
    g_fmState.strikeCache = &g_chicagoStrike12;
    g_fmState.strikeCacheSize = 1;
    g_fmState.currentStrike = &g_chicagoStrike12;

    /* Initialize state flags */
    g_fmState.fractEnable = FALSE;
    g_fmState.scaleDisable = FALSE;
    g_fmState.outlinePreferred = FALSE;
    g_fmState.preserveGlyph = FALSE;
    g_fmState.fontLock = FALSE;

    /* Set default font in current port if available */
    if (g_currentPort) {
        g_currentPort->txFont = chicagoFont;
        g_currentPort->txFace = normal;
        g_currentPort->txSize = 12;
        FM_LOG("InitFonts: Set port font to Chicago 12\n");
    }

    g_fmState.initialized = TRUE;
    FM_LOG("InitFonts: Font Manager initialized with %d families\n", 3);
}

OSErr FlushFonts(void) {
    FM_LOG("FlushFonts: Flushing font caches\n");
    /* For now, just reset to Chicago 12 */
    g_fmState.currentStrike = &g_chicagoStrike12;
    return noErr;
}

/* ============================================================================
 * Font Family Management
 * ============================================================================ */

void GetFontName(short familyID, Str255 name) {
    FontFamily* family = g_fmState.familyList;

    while (family) {
        if (family->familyID == familyID) {
            /* Copy Pascal string */
            short len = family->familyName[0];
            for (short i = 0; i <= len; i++) {
                name[i] = family->familyName[i];
            }
            FM_LOG("GetFontName: ID %d -> %.*s\n", familyID, len, &name[1]);
            return;
        }
        family = family->next;
    }

    /* Not found - return empty string */
    name[0] = 0;
    FM_LOG("GetFontName: ID %d not found\n", familyID);
}

void GetFNum(ConstStr255Param name, short *familyID) {
    if (!name || !familyID) return;

    FontFamily* family = g_fmState.familyList;

    while (family) {
        if (EqualString(name, family->familyName, FALSE, FALSE)) {
            *familyID = family->familyID;
            FM_LOG("GetFNum: %.*s -> ID %d\n", name[0], &name[1], *familyID);
            return;
        }
        family = family->next;
    }

    /* Not found */
    *familyID = -1;
    FM_LOG("GetFNum: %.*s not found\n", name[0], &name[1]);
}

Boolean RealFont(short fontNum, short size) {
    /* For now, only Chicago 12 is "real" */
    if (fontNum == chicagoFont && size == 12) {
        FM_LOG("RealFont: Chicago %d is real\n", size);
        return TRUE;
    }

    FM_LOG("RealFont: Font %d size %d is synthetic\n", fontNum, size);
    return FALSE;
}

/* ============================================================================
 * Font Selection
 * ============================================================================ */

void TextFont(short font) {
    if (!g_currentPort) return;

    g_currentPort->txFont = font;
    FM_LOG("TextFont: Set to %d\n", font);

    /* Update current strike - for now always use Chicago 12 */
    g_fmState.currentStrike = &g_chicagoStrike12;
}

void TextFace(Style face) {
    if (!g_currentPort) return;

    g_currentPort->txFace = face;
    FM_LOG("TextFace: Set to 0x%02x\n", face);

    /* Would update strike here for style synthesis */
}

void TextSize(short size) {
    if (!g_currentPort) return;

    g_currentPort->txSize = size;
    FM_LOG("TextSize: Set to %d\n", size);

    /* Update current strike based on size */
    if (size == 12) {
        g_fmState.currentStrike = &g_chicagoStrike12;
    } else {
        /* For other sizes, we'll use scaling */
        /* Keep Chicago 12 as base but remember requested size in txSize */
        g_fmState.currentStrike = &g_chicagoStrike12;
        FM_LOG("TextSize: Will use scaling for %dpt\n", size);
    }
}

void TextMode(short mode) {
    if (!g_currentPort) return;

    g_currentPort->txMode = mode;
    FM_LOG("TextMode: Set to %d\n", mode);
}

/* ============================================================================
 * Metrics
 * ============================================================================ */

void GetFontMetrics(FMetricRec *theMetrics) {
    if (!theMetrics || !g_fmState.currentStrike) return;

    FontStrike* strike = g_fmState.currentStrike;

    /* FMetricRec uses SInt32, not Fixed */
    theMetrics->ascent = strike->ascent;
    theMetrics->descent = strike->descent;
    theMetrics->widMax = strike->widMax;
    theMetrics->leading = strike->leading;
    theMetrics->wTabHandle = NULL;  /* Width table handle */

    FM_LOG("GetFontMetrics: ascent=%d descent=%d widMax=%d leading=%d\n",
           strike->ascent, strike->descent, strike->widMax, strike->leading);
}

/* ============================================================================
 * Width Measurement
 * ============================================================================ */

short CharWidth(short ch) {
    if (ch >= 32 && ch <= 126 && g_fmState.currentStrike == &g_chicagoStrike12) {
        /* Check if we need scaling for different size */
        if (g_currentPort && g_currentPort->txSize != 12) {
            /* Use scaled width for non-12pt sizes */
            short scaledWidth = FM_GetScaledCharWidth(g_currentPort->txFont, g_currentPort->txSize, ch);

            /* Apply style on top of scaling if needed */
            if (g_currentPort->txFace != normal) {
                extern short FM_GetStyledCharWidth(char ch, Style face);
                /* Style adjustments are relative, apply to scaled width */
                return scaledWidth + (FM_GetStyledCharWidth(ch, g_currentPort->txFace) - scaledWidth);
            }

            return scaledWidth;
        }

        /* Use actual Chicago font metrics for 12pt */
        ChicagoCharInfo info = chicago_ascii[ch - 32];
        short width = info.bit_width + 2;  /* Corrected spacing */
        if (ch == ' ') width += 3;  /* Extra space width */

        /* Apply full style synthesis if we have styles */
        if (g_currentPort && g_currentPort->txFace != normal) {
            /* Use style synthesis for complex styles */
            extern short FM_GetStyledCharWidth(char ch, Style face);
            return FM_GetStyledCharWidth(ch, g_currentPort->txFace);
        }

        return width;
    }

    /* Default width for unknown chars */
    return 8;
}

short StringWidth(ConstStr255Param s) {
    if (!s || s[0] == 0) return 0;

    short width = 0;
    for (int i = 1; i <= s[0]; i++) {
        width += CharWidth(s[i]);
    }

    FM_LOG("StringWidth: \"%.*s\" = %d pixels\n", s[0], &s[1], width);
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

/* ============================================================================
 * Font Manager State Access
 * ============================================================================ */

FontManagerState *GetFontManagerState(void) {
    return &g_fmState;
}

FontStrike *FM_GetCurrentStrike(void) {
    return g_fmState.currentStrike;
}

/* ============================================================================
 * Font Drawing Integration
 * ============================================================================ */

short FM_MeasureRun(const unsigned char* bytes, short len) {
    if (!bytes || len <= 0) return 0;

    short width = 0;
    for (short i = 0; i < len; i++) {
        width += CharWidth(bytes[i]);
    }
    return width;
}

void FM_DrawRun(const unsigned char* bytes, short len, Point baseline) {
    /* For now, delegate to existing ChicagoRealFont implementation */
    /* This would be expanded to handle different fonts/sizes */

    if (!bytes || len <= 0 || !g_currentPort) return;

    /* Save and set pen location */
    Point savePen = g_currentPort->pnLoc;
    g_currentPort->pnLoc = baseline;

    /* Draw each character */
    for (short i = 0; i < len; i++) {
        DrawChar(bytes[i]);
    }

    /* Restore pen (DrawChar advances it) */
    /* Actually leave pen advanced for proper text flow */
}

/* ============================================================================
 * QuickDraw Coordinate Conversion
 * ============================================================================ */

/*
 * QD_LocalToPixel - Convert QuickDraw local coordinates to pixel coordinates
 */
void QD_LocalToPixel(short localX, short localY, short* pixelX, short* pixelY) {
    if (!g_currentPort || !pixelX || !pixelY) return;

    /* Convert from QuickDraw coordinates to framebuffer pixels */
    /* Account for the port's origin */
    *pixelX = localX - g_currentPort->portRect.left + g_currentPort->portBits.bounds.left;
    *pixelY = localY - g_currentPort->portRect.top + g_currentPort->portBits.bounds.top;
}

/* ============================================================================
 * QuickDraw Text Drawing Functions
 * ============================================================================ */

/*
 * DrawChar - Draw a single character at the current pen location
 */
void DrawChar(short ch) {
    extern void serial_puts(const char* str);
    static int char_count = 0;

    if (char_count < 5) {
        serial_puts("[DRAWCHAR-FM] DrawChar called\n");
        char_count++;
    }

    if (!g_currentPort) return;

    Style face = g_currentPort->txFace;
    Boolean hasBold = (face & bold) != 0;
    Boolean hasItalic = (face & italic) != 0;

    /* Get current font strike */
    FontStrike *strike = FM_GetCurrentStrike();

    /* Check if strike has valid bitmap data */
    if (!strike || !strike->locTable || !strike->bitmapData) {
        /* Fallback to Chicago bitmap (direct framebuffer drawing) */
        extern UInt32 QDPlatform_MapQDColor(SInt32 qdColor);
        Point pen = g_currentPort->pnLoc;
        short px, py;
        QD_LocalToPixel(pen.h, pen.v - CHICAGO_ASCENT, &px, &py);
        UInt32 color = QDPlatform_MapQDColor(g_currentPort->fgColor);

        static int coord_debug = 0;
        if (coord_debug < 3) {
            extern void serial_printf(const char* fmt, ...);
            serial_printf("[DRAWCHAR] pen=(%d,%d) pixel=(%d,%d) bounds=(%d,%d,%d,%d) portRect=(%d,%d,%d,%d)\n",
                         pen.h, pen.v, px, py,
                         g_currentPort->portBits.bounds.left, g_currentPort->portBits.bounds.top,
                         g_currentPort->portBits.bounds.right, g_currentPort->portBits.bounds.bottom,
                         g_currentPort->portRect.left, g_currentPort->portRect.top,
                         g_currentPort->portRect.right, g_currentPort->portRect.bottom);
            coord_debug++;
        }

        /* Draw character with style synthesis */
        if (hasBold) {
            /* Bold: draw twice with 1 pixel offset */
            FM_DrawChicagoCharInternal(px, py, (char)ch, color);
            FM_DrawChicagoCharInternal(px + 1, py, (char)ch, color);
        } else {
            FM_DrawChicagoCharInternal(px, py, (char)ch, color);
        }

        if (hasItalic) {
            /* Italic: draw with slight right offset for shear effect */
            FM_DrawChicagoCharInternal(px + 1, py, (char)ch, color);
        }

        g_currentPort->pnLoc.h += CharWidth(ch);
        return;
    }

    /* Get foreground color from port */
    extern UInt32 QDPlatform_MapQDColor(SInt32 qdColor);
    UInt32 color = QDPlatform_MapQDColor(g_currentPort->fgColor);

    /* Calculate glyph position (baseline - ascent) */
    Point pen = g_currentPort->pnLoc;
    short glyphX = pen.h;
    short glyphY = pen.v - strike->ascent;

    /* Draw the glyph using platform layer */
    extern SInt16 QDPlatform_DrawGlyph(FontStrike *strike, UInt8 ch, SInt16 x, SInt16 y,
                                       GrafPtr port, UInt32 color);

    if (hasBold) {
        /* Bold: draw twice with 1 pixel offset */
        SInt16 advance = QDPlatform_DrawGlyph(strike, (UInt8)ch, glyphX, glyphY,
                                              g_currentPort, color);
        QDPlatform_DrawGlyph(strike, (UInt8)ch, glyphX + 1, glyphY,
                             g_currentPort, color);
        /* Advance pen by character width + bold offset */
        if (advance > 0) {
            g_currentPort->pnLoc.h += advance + 1;
        } else {
            g_currentPort->pnLoc.h += CharWidth(ch) + 1;
        }
    } else {
        SInt16 advance = QDPlatform_DrawGlyph(strike, (UInt8)ch, glyphX, glyphY,
                                              g_currentPort, color);
        /* Advance pen by character width */
        if (advance > 0) {
            g_currentPort->pnLoc.h += advance;
        } else {
            g_currentPort->pnLoc.h += CharWidth(ch);
        }
    }
}

/*
 * DrawString - Draw a Pascal string at the current pen location
 */
void DrawString(ConstStr255Param s) {
    extern void serial_puts(const char* str);
    serial_puts("[DRAWSTR-FM] DrawString called\n");

    if (!s || s[0] == 0 || !g_currentPort) {
        serial_puts("[DRAWSTR-FM] Early return\n");
        return;
    }

    serial_puts("[DRAWSTR-FM] Drawing characters\n");

    unsigned char len = s[0];
    Style face = g_currentPort->txFace;
    short startX = g_currentPort->pnLoc.h;  /* Save start for underline */

    /* Draw each character using DrawChar (bold/italic handled there) */
    for (int i = 1; i <= len; i++) {
        DrawChar(s[i]);
    }

    /* Draw underline if needed */
    if (face & underline) {
        Point oldPen = g_currentPort->pnLoc;
        short underlineY = oldPen.v + 2;  /* 2 pixels below baseline */
        MoveTo(startX, underlineY);
        LineTo(oldPen.h, underlineY);
        g_currentPort->pnLoc = oldPen;  /* Restore pen position */
    }
}

/*
 * DrawText - Draw text buffer at current pen location
 * Different from DrawString - takes a buffer pointer and length
 */
void DrawText(const void* textBuf, short firstByte, short byteCount) {
    if (!textBuf || byteCount <= 0 || !g_currentPort) return;

    const unsigned char* text = (const unsigned char*)textBuf;

    /* Draw each character using DrawChar */
    for (short i = 0; i < byteCount; i++) {
        DrawChar(text[firstByte + i]);
    }
}

/* ============================================================================
 * Font Swapping
 * ============================================================================ */

static FMOutput g_fmOutput;  /* Static output record */

FMOutPtr FMSwapFont(const FMInput *inRec) {
    if (!inRec) return NULL;

    FM_LOG("FMSwapFont: family=%d size=%d face=0x%02x\n",
           inRec->family, inRec->size, inRec->face);

    /* For now, always return Chicago 12 metrics */
    g_fmOutput.errNum = noErr;
    g_fmOutput.fontHandle = NULL;  /* Would be handle to NFNT resource */
    g_fmOutput.boldPixels = (inRec->face & bold) ? 1 : 0;
    g_fmOutput.italicPixels = (inRec->face & italic) ? 2 : 0;  /* ~14 degree slant */
    g_fmOutput.ulOffset = 2;  /* Underline offset from baseline */
    g_fmOutput.ulThick = 1;   /* Underline thickness */
    g_fmOutput.ulShadow = 0;  /* Underline shadow */
    g_fmOutput.shadowPixels = (inRec->face & shadow) ? 1 : 0;
    g_fmOutput.extra = 0;
    g_fmOutput.ascent = CHICAGO_ASCENT;
    g_fmOutput.descent = CHICAGO_DESCENT;
    g_fmOutput.widMax = 16;
    g_fmOutput.leading = CHICAGO_LEADING;
    g_fmOutput.unused = 0;
    g_fmOutput.numer.h = 1;
    g_fmOutput.numer.v = 1;
    g_fmOutput.denom.h = 1;
    g_fmOutput.denom.v = 1;

    /* Update current strike */
    g_fmState.currentStrike = &g_chicagoStrike12;

    return &g_fmOutput;
}

/* ============================================================================
 * Font Locking and Options
 * ============================================================================ */

void SetFontLock(Boolean lockFlag) {
    g_fmState.fontLock = lockFlag;
    FM_LOG("SetFontLock: %s\n", lockFlag ? "locked" : "unlocked");
}

void SetFScaleDisable(Boolean fscaleDisable) {
    g_fmState.scaleDisable = fscaleDisable;
    FM_LOG("SetFScaleDisable: %s\n", fscaleDisable ? "disabled" : "enabled");
}

void SetFractEnable(Boolean fractEnable) {
    g_fmState.fractEnable = fractEnable;
    FM_LOG("SetFractEnable: %s\n", fractEnable ? "enabled" : "disabled");
}

void SetOutlinePreferred(Boolean outlinePreferred) {
    g_fmState.outlinePreferred = outlinePreferred;
    FM_LOG("SetOutlinePreferred: %s\n", outlinePreferred ? "yes" : "no");
}

Boolean GetOutlinePreferred(void) {
    return g_fmState.outlinePreferred;
}

void SetPreserveGlyph(Boolean preserveGlyph) {
    g_fmState.preserveGlyph = preserveGlyph;
    FM_LOG("SetPreserveGlyph: %s\n", preserveGlyph ? "yes" : "no");
}

Boolean GetPreserveGlyph(void) {
    return g_fmState.preserveGlyph;
}

/* ============================================================================
 * System Font Access
 * ============================================================================ */

short GetDefFontSize(void) {
    return 12;  /* System 7 default */
}

short GetSysFont(void) {
    return chicagoFont;
}

short GetAppFont(void) {
    return genevaFont;  /* Application font */
}

/* ============================================================================
 * Error Handling
 * ============================================================================ */

static OSErr g_lastFontError = noErr;

OSErr GetLastFontError(void) {
    return g_lastFontError;
}

void SetFontErrorCallback(void (*callback)(OSErr error, const char *message)) {
    /* Store callback for error notifications */
}

/* ============================================================================
 * Font Stack (Push/Pop for nested font operations)
 * ============================================================================ */

/*
 * FMPushFont - Save current font state on stack
 * Allows nested font changes (e.g., bold text within normal text)
 */
void FMPushFont(void) {
    if (g_fontStackDepth >= MAX_FONT_STACK) {
        FM_LOG("FMPushFont: Stack overflow (depth=%d)\n", g_fontStackDepth);
        return;
    }

    FontStackEntry *entry = &g_fontStack[g_fontStackDepth++];

    /* Save current port state */
    if (g_currentPort) {
        entry->fontNum = g_currentPort->txFont;
        entry->fontSize = g_currentPort->txSize;
        entry->fontFace = g_currentPort->txFace;
        entry->fgColor = g_currentPort->fgColor;
    } else {
        entry->fontNum = systemFont;
        entry->fontSize = 12;
        entry->fontFace = normal;
        entry->fgColor = 0;  /* Black */
    }

    FM_LOG("FMPushFont: Saved font state (depth=%d, font=%d, size=%d, face=0x%02X)\n",
           g_fontStackDepth, entry->fontNum, entry->fontSize, entry->fontFace);
}

/*
 * FMPopFont - Restore font state from stack
 * Returns to previous font settings
 */
void FMPopFont(void) {
    if (g_fontStackDepth <= 0) {
        FM_LOG("FMPopFont: Stack underflow\n");
        return;
    }

    FontStackEntry *entry = &g_fontStack[--g_fontStackDepth];

    /* Restore font state */
    if (g_currentPort) {
        TextFont(entry->fontNum);
        TextSize(entry->fontSize);
        TextFace(entry->fontFace);
        RGBForeColor((RGBColor*)&entry->fgColor);  /* Approximate - would need proper RGB conversion */
    }

    FM_LOG("FMPopFont: Restored font state (depth=%d, font=%d, size=%d, face=0x%02X)\n",
           g_fontStackDepth, entry->fontNum, entry->fontSize, entry->fontFace);
}

/*
 * FMGetFontStackDepth - Get current stack depth (for debugging)
 */
SInt16 FMGetFontStackDepth(void) {
    return g_fontStackDepth;
}

/*
 * FMSetFontSize - Change current font size (with common sizes 9, 12, 14, 18)
 */
void FMSetFontSize(SInt16 size) {
    if (!g_currentPort) return;

    /* Map requested size to available sizes */
    SInt16 mappedSize = 12;  /* Default to 12pt */

    if (size <= 9) {
        mappedSize = 9;
    } else if (size <= 12) {
        mappedSize = 12;
    } else if (size <= 14) {
        mappedSize = 14;
    } else {
        mappedSize = 18;
    }

    TextSize(mappedSize);

    FM_LOG("FMSetFontSize: Set font size from %d to %d\n", size, mappedSize);
}

/* ============================================================================
 * Stub Implementations (for linking)
 * ============================================================================ */

Boolean IsOutline(Point numer, Point denom) {
    /* No outline fonts yet */
    return FALSE;
}

OSErr OutlineMetrics(short byteCount, const void *textPtr, Point numer,
                     Point denom, short *yMax, short *yMin, Fixed* awArray,
                     Fixed* lsbArray, Rect* boundsArray) {
    /* Not implemented for bitmap fonts */
    return -1;  /* fontNotFoundErr */
}

/* C-style font name functions */
void getfnum(char *theName, short *familyID) {
    if (!theName || !familyID) return;

    /* Convert C string to Pascal string */
    unsigned char pstr[256];
    short len = 0;
    while (theName[len] && len < 255) {
        pstr[len + 1] = theName[len];
        len++;
    }
    pstr[0] = len;

    GetFNum(pstr, familyID);
}

void getfontname(short familyID, char *theName) {
    if (!theName) return;

    unsigned char pstr[256];
    GetFontName(familyID, pstr);

    /* Convert Pascal string to C string */
    short len = pstr[0];
    for (short i = 0; i < len; i++) {
        theName[i] = pstr[i + 1];
    }
    theName[len] = '\0';
}
