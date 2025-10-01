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

/* Width Entry */

/* Width Table */

/* Font Association Entry */

/* Font Association Table */

/* Style Table */

/* Name Table */

/* Kern Pair */

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

/* WOFF Font Structure */

/* System Font Structure */

/* Font Collection Structure */

/* Modern Font Structure (Union of all types) */
typedef struct ModernFont {
    UInt32 placeholder; /* Placeholder field */
} ModernFont;

/* Web Font Metadata */

/* Font Directory Entry */

/* Font Directory */

#ifdef __cplusplus
}
#endif

#endif /* FONT_TYPES_H */
