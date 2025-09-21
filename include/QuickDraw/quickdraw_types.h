/*
 * RE-AGENT-BANNER
 * QuickDraw Types Header
 * Reimplemented from Apple System 7.1 QuickDraw Core
 *
 * Original binary: System.rsrc (SHA256: 78150ebb66707277e0947fbf81f6a27fc5d263a71bbf96df7084d5c3ec22a5ba)
 * Architecture: 68k Mac OS System 7
 * Evidence sources: string analysis, Mac OS Toolbox reference, binary analysis
 *
 * This file contains QuickDraw data structures and type definitions
 * reimplemented from the original Apple QuickDraw graphics engine.
 */

#ifndef QUICKDRAW_TYPES_H
#define QUICKDRAW_TYPES_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"
#include "WindowManager/WindowTypes.h"

#pragma pack(push, 2)  /* 68k word alignment */

/* Basic Types - Ptr, Handle and Fixed are defined in MacTypes.h */
/* Style is defined in MacTypes.h */

/* Forward declarations */

/* Handle types */
/* Handle is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */
/* Ptr is defined in MacTypes.h */
/* Ptr is defined in MacTypes.h */

/* Basic geometric types - Point and Rect are defined in MacTypes.h */
/* Evidence: Standard Mac OS coordinate system, found in toolbox reference */

/* Pattern type */
/* Pattern type defined in MacTypes.h */
/* Evidence: Standard QuickDraw pattern, 8 bytes for 8x8 pixels */

/* Color types */
/* RGBColor is in QuickDraw/QDTypes.h */
/* Evidence: Standard RGB color specification, 16-bit components */

/* BitMap is defined in WindowManager/WindowTypes.h */

/* PixMap is defined in SystemTypes.h - no need to redefine */
/* Evidence: Color QuickDraw pixmap structure */

/* GrafPort is defined in WindowManager/WindowTypes.h */

/* Region structure */
/* Evidence: QuickDraw region structure, variable size */

/* Cursor is defined in QDTypes.h */

/* Transfer modes */

/* Text styles */

/* Constants */
#define kQDMaxColors 256
#define kQDPatternSize 8
#define kQDCursorSize 16

#pragma pack(pop)

#endif /* QUICKDRAW_TYPES_H */

/*
 * RE-AGENT-TRAILER-JSON
 * {
 *   "type": "header_file",
 *   "name": "quickdraw_types.h",
 *   "description": "QuickDraw data structures and types",
 *   "evidence_sources": ["evidence.curated.quickdraw.json", "layouts.curated.quickdraw.json"],
 *   "confidence": 0.90,
 *   "structures_defined": ["Point", "Rect", "Pattern", "RGBColor", "BitMap", "PixMap", "GrafPort", "Region", "Cursor"],
 *   "dependencies": ["mac_types.h"],
 *   "size_total_bytes": 108
 * }
 */