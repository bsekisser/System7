/*
 * Copyright (c) 2024 System7 Project
 * MIT License - See LICENSE file
 */

/*
 * ColorTypes.h - Color Manager Types and Structures
 * System 7.1 color management types
 */

#ifndef COLORTYPES_H
#define COLORTYPES_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"
#include "WindowManager/WindowTypes.h"

/* Forward declarations */
/* Ptr is defined in MacTypes.h */

/* RGB Color - 6 bytes, 16-bit components */
/* RGBColor is in QuickDraw/QDTypes.h */

/* Color Specification - 8 bytes */
/* ColorSpec is already defined in MacTypes.h, include guard for safety */
#ifndef __COLOR_SPEC_DEFINED__
#define __COLOR_SPEC_DEFINED__

#endif

/* Color Table - variable size */

/* Palette Entry - 8 bytes */

/* Palette - variable size */

/* Inverse Color Table - variable size */

/* Graphics Device - variable size */

    SInt32 gdMode;           /* Device mode */
    SInt16 gdCCBytes;        /* Color correction bytes */
    SInt16 gdCCDepth;        /* Color correction depth */
    void** gdCCXData;         /* Color correction data */
    void** gdCCXMask;         /* Color correction mask */
    void** gdExt;             /* Device extension */
} GDevice;

/* Request List Record */

/* Handle Types */
/* Handle is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */

/* Color Manager Constants */
#define pixPurge         0x8000   /* Color table can be purged */
#define noUpdates        0x4000   /* Don't update color table */
#define pixNotPurgeable  0x0000   /* Color table not purgeable */

/* Palette Usage Flags */
#define pmCourteous      0x0000   /* Courteous palette usage */
#define pmDithered       0x0001   /* Use dithering */
#define pmTolerant       0x0002   /* Use tolerance matching */
#define pmAnimated       0x0004   /* Animated palette entries */
#define pmExplicit       0x0008   /* Explicit palette matching */

/* Graphics Device Flags */
#define gdDevType        0x0000   /* Standard device type */
#define burstDevice      0x0001   /* Burst device */
#define ext32Device      0x0002   /* 32-bit clean device */
#define ramInit          0x0004   /* RAM initialization */
#define mainScreen       0x0008   /* Main screen device */
#define allInit          0x0010   /* All initialization complete */
#define screenDevice     0x0020   /* Screen device */
#define noDriver         0x0040   /* No driver present */
#define screenActive     0x0080   /* Screen is active */
#define hiliteBit        0x0080   /* Highlight bit */
#define roundedDevice    0x0020   /* Rounded corners device */
#define hasAuxMenuBar    0x0008   /* Has auxiliary menu bar */

/* Core Color Drawing Functions */
void RGBForeColor(const RGBColor* rgb);
void RGBBackColor(const RGBColor* rgb);
void GetForeColor(RGBColor* rgb);
void GetBackColor(RGBColor* rgb);
void GetCPixel(SInt16 h, SInt16 v, RGBColor* cPix);
void SetCPixel(SInt16 h, SInt16 v, const RGBColor* cPix);

/* Color Table Management */
CTabHandle GetCTable(SInt16 ctID);
void DisposeCTable(CTabHandle cTabH);
CTabHandle CopyColorTable(CTabHandle srcTab);
void MakeRGBPat(PixPatHandle pp, const RGBColor* myColor);

/* Color Conversion Functions */
void Index2Color(SInt32 index, RGBColor* aColor);
SInt32 Color2Index(const RGBColor* myColor);
void InvertColor(RGBColor* myColor);
Boolean RealColor(const RGBColor* rgb);
void GetSubTable(CTabHandle myColors, SInt16 iTabRes, CTabHandle targetTbl);

/* Inverse Table Functions */
OSErr MakeITable(CTabHandle cTabH, ITabHandle iTabH, SInt16 res);

/* Palette Management Functions */
PaletteHandle NewPalette(SInt16 entries, CTabHandle srcColors,
                        SInt16 srcUsage, SInt16 srcTolerance);
PaletteHandle GetNewPalette(SInt16 PaletteID);
void DisposePalette(PaletteHandle srcPalette);
void ActivatePalette(WindowPtr srcWindow);
void SetPalette(WindowPtr dstWindow, PaletteHandle srcPalette, Boolean cUpdates);
PaletteHandle GetPalette(WindowPtr srcWindow);
void CopyPalette(PaletteHandle srcPalette, PaletteHandle dstPalette,
                SInt16 srcEntry, SInt16 dstEntry, SInt16 dstLength);

/* Palette Color Functions */
void PmForeColor(SInt16 dstEntry);
void PmBackColor(SInt16 dstEntry);
void AnimateEntry(WindowPtr dstWindow, SInt16 dstEntry, const RGBColor* srcRGB);
void AnimatePalette(WindowPtr dstWindow, CTabHandle srcCTab, SInt16 srcIndex,
                   SInt16 dstEntry, SInt16 dstLength);
void GetEntryColor(PaletteHandle srcPalette, SInt16 srcEntry, RGBColor* dstRGB);
void SetEntryColor(PaletteHandle dstPalette, SInt16 dstEntry, const RGBColor* srcRGB);
void GetEntryUsage(PaletteHandle srcPalette, SInt16 srcEntry,
                  SInt16* dstUsage, SInt16* dstTolerance);
void SetEntryUsage(PaletteHandle dstPalette, SInt16 dstEntry,
                  SInt16 srcUsage, SInt16 srcTolerance);
SInt32 Palette2CTab(PaletteHandle srcPalette, CTabHandle dstCTab);
SInt32 CTab2Palette(CTabHandle srcCTab, PaletteHandle dstPalette,
                    SInt16 srcUsage, SInt16 srcTolerance);

/* Extended Color Table Functions */
void ProtectEntry(SInt16 index, Boolean protect);
void ReserveEntry(SInt16 index, Boolean reserve);
void SetEntries(SInt16 start, SInt16 count, CTabHandle srcTable);
void RestoreEntries(CTabHandle srcTable, CTabHandle dstTable, ReqListRec* selection);
void SaveEntries(CTabHandle srcTable, CTabHandle resultTable, ReqListRec* selection);

/* Error Handling */
SInt16 QDError(void);

/* Color Manager Initialization */
OSErr ColorManager_Init(void);
void ColorManager_Cleanup(void);

/* Global State Access */
RGBColor* ColorManager_GetForeColorPtr(void);
RGBColor* ColorManager_GetBackColorPtr(void);
CTabHandle ColorManager_GetDeviceColors(void);

#endif /* COLORTYPES_H */