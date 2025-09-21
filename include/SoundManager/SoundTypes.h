/*
 * SoundTypes.h - Sound Manager Data Types and Structures
 *
 * Defines all data structures, constants, and type definitions used by
 * the Mac OS Sound Manager. This header provides complete compatibility
 * with the original Sound Manager data types.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 */

#ifndef _SOUNDTYPES_H_
#define _SOUNDTYPES_H_

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Basic Mac OS types compatibility */
/* OSErr is defined in MacTypes.h *//* OSType is defined in MacTypes.h */
/* Fixed is defined in MacTypes.h */
/* Fixed is defined in MacTypes.h *//* Boolean is defined in MacTypes.h *//* SInt16 is defined in MacTypes.h *//* SInt32 is defined in MacTypes.h *//* UInt16 is defined in MacTypes.h *//* UInt32 is defined in MacTypes.h *//* Handle is defined in MacTypes.h */
/* Str255 is defined in MacTypes.h */
#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

/* Four Character Code macro */
#define FOUR_CHAR_CODE(x) ((UInt32)(x))

/* Forward declarations */

/* Sound channel pointer types */
/* Ptr is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */
/* Ptr is defined in MacTypes.h */

/* Point structure (for recording dialogs) */
/* Point type defined in MacTypes.h */

/* Version information structure */

/* Sound Resource Formats */

/* Sound header format types */

/* Audio encoding types */

/* Standard Sound Header - Format 1 */

/* Extended Sound Header - Format 2 */

/* Compressed Sound Header */

/* Sound List Resource Header */

/* Sound Command Structure */

/* Sound Channel Status */

/* Sound Manager Status */

/* Sound Channel Structure */

/* Audio Selection Structure */

/* Ptr is defined in MacTypes.h */

/* File Play Completion UPP */

/* Sound Completion UPP */

/* Modal Filter UPP */

/* Sound Parameter Block for Sound Input */

/* Compression Information */

/* Ptr is defined in MacTypes.h */

/* MIDI Types */

/* Ptr is defined in MacTypes.h */

/* Sound Input Device Information Types */

/* Audio Quality Types */

/* Fixed Point Utilities */
#define Fixed2Long(f)       ((SInt32)((f) >> 16))
#define Long2Fixed(l)       ((Fixed)((l) << 16))
#define Fixed2Frac(f)       ((SInt16)(f))
#define Frac2Fixed(fr)      ((Fixed)(fr))
#define FixedRound(f)       ((SInt16)(((f) + 0x00008000) >> 16))
#define Fixed2X(f)          ((f) >> 16)
#define X2Fixed(x)          ((x) << 16)

/* Rate conversion macros */
#define Rate22khz           0x56220000UL    /* 22.254 kHz */
#define Rate11khz           0x2B110000UL    /* 11.127 kHz */
#define Rate44khz           0xAC440000UL    /* 44.100 kHz */
#define Rate48khz           0xBB800000UL    /* 48.000 kHz */

#ifdef __cplusplus
}
#endif

#endif /* _SOUNDTYPES_H_ */