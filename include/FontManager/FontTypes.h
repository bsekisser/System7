/*
 * FontTypes.h - Font Manager Type Definitions
 *
 * Defines all data structures, constants, and types used by the Font Manager.
 * Derived from ROM Font Manager analysis.
 */

#ifndef FONT_TYPES_H
#define FONT_TYPES_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Font Constants */
enum {
    systemFont = 0,
    applFont = 1,
    newYork = 2,
    geneva = 3,
    monaco = 4,
    venice = 5,
    london = 6,
    athens = 7,
    sanFran = 8,
    toronto = 9,
    cairo = 11,
    losAngeles = 12,
    times = 20,
    helvetica = 21,
    courier = 22,
    symbol = 23,
    mobile = 24
};

/* Resource Type Constants */
enum {
    kFONTResourceType = 'FONT',
    kNFNTResourceType = 'NFNT'
};

/* Font Error Codes */
enum {
    fontNotFoundErr = -4960,
    fontCacheFullErr = -4961
};

/* Font Manager State */
typedef struct FontManagerState {
    Boolean initialized;
    Boolean fractEnable;
    Boolean scaleDisable;
    Boolean outlinePreferred;
    Boolean preserveGlyph;
    Boolean fontLock;
    void *cache;
    void *substitutions;
    short substitutionCount;
    Fixed fontGamma;
    Boolean hintingEnabled;
    Boolean smoothingEnabled;
} FontManagerState;

/* Font Manager Input Record */

/* Font Manager Output Record */
typedef struct FMOutput {
    SInt16 errNum;         /* Error number */
    Handle fontHandle;     /* Handle to font */
    unsigned char boldPixels;    /* Bold enhancement pixels */
    unsigned char italicPixels;  /* Italic slant pixels */
    unsigned char ulOffset;      /* Underline offset */
    unsigned char ulShadow;      /* Underline shadow */
    unsigned char ulThick;       /* Underline thickness */
    unsigned char shadowPixels;  /* Shadow enhancement pixels */
    char extra;            /* Extra pixels for style */
    unsigned char ascent;  /* Font ascent */
    unsigned char descent; /* Font descent */
    unsigned char widMax;  /* Maximum character width */
    char leading;          /* Leading between lines */
    char unused;           /* Reserved */
    Point numer;           /* Actual scale numerator */
    Point denom;           /* Actual scale denominator */
} FMOutput, *FMOutPtr;

/* Font Record - Bitmap Font Header */

/* Font Family Record - FOND Resource Header */

/* Font Metrics Record */
typedef struct FontMetrics {
    Fixed ascent;
    Fixed descent;
    Fixed leading;
    Fixed widMax;
    Fixed lineHeight;
} FontMetrics;

/* Character Metrics Structure */
typedef struct CharMetrics {
    Fixed width;
    Fixed height;
    Fixed advanceX;
    Fixed advanceY;
    Rect bounds;
} CharMetrics;

/* Text Metrics Structure */
typedef struct TextMetrics {
    Fixed width;
    Fixed height;
    Fixed ascent;
    Fixed descent;
    Fixed leading;
    short lineCount;
} TextMetrics;

/* Width Entry */
typedef struct WidthEntry {
    char character;
    Fixed width;
} WidthEntry;

/* Width Table */
typedef struct WidthTable {
    short count;
    WidthEntry entries[256];
} WidthTable;

/* Font Association Entry */

/* Font Association Table */

/* Style Table */

/* Name Table */

/* Kern Pair */
typedef struct KernPair {
    char first;
    char second;
    Fixed kerning;
} KernPair;

/* Kern Entry */

/* Kern Table */

/* Complete Width Table - Internal Font Manager Structure */

/* TrueType/Outline Font Support Structures */

/* Font Cache Structures */

/* Font Substitution Table */

/* Error Codes */

/* Font Format Types */

/* Modern Font Format Structures */

/* OpenType Font Structure */
typedef struct OpenTypeFont {
    UInt32 placeholder;
} OpenTypeFont;

/* WOFF Font Structure */
typedef struct WOFFFont {
    UInt32 placeholder;
} WOFFFont;

/* System Font Structure */
typedef struct SystemFont {
    UInt32 placeholder;
} SystemFont;

/* Font Collection Structure */
typedef struct FontCollection {
    UInt32 placeholder;
} FontCollection;

/* Modern Font Structure (Union of all types) */
typedef struct ModernFont {
    UInt32 placeholder; /* Placeholder field */
} ModernFont;

/* Web Font Metadata */
typedef struct WebFontMetadata {
    short familyID;
    Str255 familyName;
} WebFontMetadata;

/* Font Directory Entry */
typedef struct FontDirectoryEntry {
    short familyID;
    Str255 familyName;
} FontDirectoryEntry;

/* Font Directory */
typedef struct FontDirectory {
    short count;
    FontDirectoryEntry *entries;
} FontDirectory;

/* Font Match Criteria */
typedef struct FontMatchCriteria {
    Str255 familyName;
    short size;
    short style;
} FontMatchCriteria;

#ifdef __cplusplus
}
#endif

#endif /* FONT_TYPES_H */
