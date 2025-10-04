/*
 * FontManagerCore.c - Core Font Manager Implementation
 *
 * System 7.1-compatible Font Manager with bitmap font support
 * Integrates with existing Chicago font implementation
 */

#include "FontManager/FontManager.h"
#include "FontManager/FontTypes.h"
#include "QuickDraw/QuickDraw.h"
#include "SystemTypes.h"
#include "chicago_font.h"
#include <string.h>

/* Debug logging */
#define FM_DEBUG 1

#if FM_DEBUG
extern void serial_printf(const char* fmt, ...);
#define FM_LOG(...) serial_printf("FM: " __VA_ARGS__)
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
extern void DrawString(ConstStr255Param s);
extern short StringWidth(ConstStr255Param s);

/* Global Font Manager state */
static FontManagerState g_fmState = {0};

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
            extern short FM_GetScaledCharWidth(char ch, short targetSize);
            short scaledWidth = FM_GetScaledCharWidth(ch, g_currentPort->txSize);

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
        extern void DrawChar(short ch);
        DrawChar(bytes[i]);
    }

    /* Restore pen (DrawChar advances it) */
    /* Actually leave pen advanced for proper text flow */
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