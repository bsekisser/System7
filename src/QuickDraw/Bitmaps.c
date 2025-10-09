/* #include "SystemTypes.h" */
#include "QuickDraw/QuickDrawInternal.h"
#include <string.h>
/*
 * Bitmaps.c - QuickDraw Bitmap and CopyBits Implementation
 *
 * Complete implementation of bitmap operations including CopyBits,
 * scaling, transfer modes, masking, and pixel manipulation.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 * Derived from System 7 ROM analysis (Ghidra) QuickDraw
 */

// #include "CompatibilityFix.h" // Removed
#include "SystemTypes.h"
#include "System71StdLib.h"
#include "QuickDrawConstants.h"

#include "QuickDraw/QuickDraw.h"
#include "QuickDraw/ColorQuickDraw.h"
#include "QuickDraw/QDRegions.h"
#include <assert.h>

/* Platform abstraction layer */
#include "QuickDraw/QuickDrawPlatform.h"

/* Current QuickDraw port from QuickDrawCore.c */
extern GrafPtr g_currentPort;


/* Transfer mode operation structures */
typedef struct {
    UInt32 (*operation)(UInt32 src, UInt32 dst, UInt32 pattern);
    Boolean needsPattern;
    Boolean supportsColor;
} TransferModeInfo;

/* Scaling information */
typedef struct {
    SInt16 srcWidth, srcHeight;
    SInt16 dstWidth, dstHeight;
    SInt32 hScale, vScale;  /* Fixed-point scaling factors */
    Boolean needsScaling;
} ScaleInfo;

/* Bit manipulation macros */
#define BITS_PER_BYTE 8
#define FIXED_POINT_SCALE 65536

/* Forward declarations */
static void CopyBitsImplementation(const BitMap *srcBits, const BitMap *dstBits,
                                  const Rect *srcRect, const Rect *dstRect,
                                  SInt16 mode, RgnHandle maskRgn);
static void CopyBitsScaled(const BitMap *srcBits, const BitMap *dstBits,
                          const Rect *srcRect, const Rect *dstRect,
                          SInt16 mode, const ScaleInfo *scaleInfo,
                          RgnHandle maskRgn);
static void CopyBitsUnscaled(const BitMap *srcBits, const BitMap *dstBits,
                            const Rect *srcRect, const Rect *dstRect,
                            SInt16 mode, RgnHandle maskRgn);
static UInt32 ApplyTransferMode(UInt32 src, UInt32 dst, UInt32 pattern, SInt16 mode);
static void CalculateScaling(const Rect *srcRect, const Rect *dstRect, ScaleInfo *scaleInfo);
static void CopyPixelRow(const BitMap *srcBits, const BitMap *dstBits,
                        SInt16 srcY, SInt16 dstY, SInt16 srcLeft, SInt16 srcRight,
                        SInt16 dstLeft, SInt16 dstRight, SInt16 mode,
                        RgnHandle maskRgn);
static UInt32 GetPixelValue(const BitMap *bitmap, SInt16 x, SInt16 y);
static void SetPixelValue(const BitMap *bitmap, SInt16 x, SInt16 y, UInt32 value);
/* IsPixMap is now defined in ColorQuickDraw.h */
/* static Boolean IsPixMap(const BitMap *bitmap); */
static void ClipRectToBitmap(const BitMap *bitmap, Rect *rect);

/* Transfer mode operations */
static UInt32 TransferSrcCopy(UInt32 src, UInt32 dst, UInt32 pattern) {
    return src;
}

static UInt32 TransferSrcOr(UInt32 src, UInt32 dst, UInt32 pattern) {
    return src | dst;
}

static UInt32 TransferSrcXor(UInt32 src, UInt32 dst, UInt32 pattern) {
    return src ^ dst;
}

static UInt32 TransferSrcBic(UInt32 src, UInt32 dst, UInt32 pattern) {
    return src & (~dst);
}

static UInt32 TransferNotSrcCopy(UInt32 src, UInt32 dst, UInt32 pattern) {
    return ~src;
}

static UInt32 TransferPatCopy(UInt32 src, UInt32 dst, UInt32 pattern) {
    return pattern;
}

static UInt32 TransferPatOr(UInt32 src, UInt32 dst, UInt32 pattern) {
    return pattern | dst;
}

static UInt32 TransferPatXor(UInt32 src, UInt32 dst, UInt32 pattern) {
    return pattern ^ dst;
}

static const TransferModeInfo g_transferModes[] = {
    {TransferSrcCopy,    false, true},  /* srcCopy */
    {TransferSrcOr,      false, true},  /* srcOr */
    {TransferSrcXor,     false, true},  /* srcXor */
    {TransferSrcBic,     false, true},  /* srcBic */
    {TransferNotSrcCopy, false, true},  /* notSrcCopy */
    {TransferSrcOr,      false, true},  /* notSrcOr */
    {TransferSrcXor,     false, true},  /* notSrcXor */
    {TransferSrcBic,     false, true},  /* notSrcBic */
    {TransferPatCopy,    true,  true},  /* patCopy */
    {TransferPatOr,      true,  true},  /* patOr */
    {TransferPatXor,     true,  true},  /* patXor */
    {TransferSrcBic,     true,  true},  /* patBic */
    {TransferPatCopy,    true,  true},  /* notPatCopy */
    {TransferPatOr,      true,  true},  /* notPatOr */
    {TransferPatXor,     true,  true},  /* notPatXor */
    {TransferSrcBic,     true,  true}   /* notPatBic */
};

/* ================================================================
 * COPYBITS IMPLEMENTATION
 * ================================================================ */

void CopyBits(const BitMap *srcBits, const BitMap *dstBits,
              const Rect *srcRect, const Rect *dstRect,
              SInt16 mode, RgnHandle maskRgn) {
    assert(srcBits != NULL);
    assert(dstBits != NULL);
    assert(srcRect != NULL);
    assert(dstRect != NULL);

    /* Validate rectangles */
    if (EmptyRect(srcRect) || EmptyRect(dstRect)) return;

    CopyBitsImplementation(srcBits, dstBits, srcRect, dstRect, mode, maskRgn);
}

static void CopyBitsImplementation(const BitMap *srcBits, const BitMap *dstBits,
                                  const Rect *srcRect, const Rect *dstRect,
                                  SInt16 mode, RgnHandle maskRgn) {
    /* Clip source and destination rectangles */
    Rect clippedSrcRect = *srcRect;
    Rect clippedDstRect = *dstRect;

    ClipRectToBitmap(srcBits, &clippedSrcRect);
    ClipRectToBitmap(dstBits, &clippedDstRect);

    /* Calculate scaling information */
    ScaleInfo scaleInfo;
    CalculateScaling(&clippedSrcRect, &clippedDstRect, &scaleInfo);

    /* Choose appropriate copy method */
    if (scaleInfo.needsScaling) {
        CopyBitsScaled(srcBits, dstBits, &clippedSrcRect, &clippedDstRect,
                       mode, &scaleInfo, maskRgn);
    } else {
        CopyBitsUnscaled(srcBits, dstBits, &clippedSrcRect, &clippedDstRect, mode, maskRgn);
    }
}

static void CopyBitsScaled(const BitMap *srcBits, const BitMap *dstBits,
                          const Rect *srcRect, const Rect *dstRect,
                          SInt16 mode, const ScaleInfo *scaleInfo,
                          RgnHandle maskRgn) {
    SInt16 dstWidth = dstRect->right - dstRect->left;
    SInt16 dstHeight = dstRect->bottom - dstRect->top;

    /* Scale each destination pixel */
    for (SInt16 dstY = 0; dstY < dstHeight; dstY++) {
        for (SInt16 dstX = 0; dstX < dstWidth; dstX++) {
            /* Calculate corresponding source pixel */
            SInt16 srcX = (SInt16)(dstX * scaleInfo->hScale / FIXED_POINT_SCALE);
            SInt16 srcY = (SInt16)(dstY * scaleInfo->vScale / FIXED_POINT_SCALE);

            /* Get source pixel */
            UInt32 srcPixel = GetPixelValue(srcBits,
                                            srcRect->left + srcX,
                                            srcRect->top + srcY);

            /* Get destination pixel */
            UInt32 dstPixel = GetPixelValue(dstBits,
                                            dstRect->left + dstX,
                                            dstRect->top + dstY);

            /* Apply transfer mode */
            UInt32 resultPixel = ApplyTransferMode(srcPixel, dstPixel, 0, mode);

            if (maskRgn && *maskRgn) {
                Point dstPt;
                dstPt.v = dstRect->top + dstY;
                dstPt.h = dstRect->left + dstX;
                if (!PtInRgn(dstPt, maskRgn)) {
                    continue;
                }
            }

            /* Set destination pixel */
            SetPixelValue(dstBits, dstRect->left + dstX, dstRect->top + dstY, resultPixel);
        }
    }
}

static void CopyBitsUnscaled(const BitMap *srcBits, const BitMap *dstBits,
                            const Rect *srcRect, const Rect *dstRect,
                            SInt16 mode, RgnHandle maskRgn) {
    /* Validate pointers */
    if (!srcBits || !dstBits || !srcRect || !dstRect) {
        serial_logf(3, 0, "[CB] ERROR: NULL pointer - srcBits=%p dstBits=%p srcRect=%p dstRect=%p\n",
                    srcBits, dstBits, srcRect, dstRect);
        return;
    }

    serial_logf(3, 2, "[CB] srcRect=(%d,%d,%d,%d) dstRect=(%d,%d,%d,%d)\n",
                srcRect->top, srcRect->left, srcRect->bottom, srcRect->right,
                dstRect->top, dstRect->left, dstRect->bottom, dstRect->right);

    SInt16 width = srcRect->right - srcRect->left;
    SInt16 height = srcRect->bottom - srcRect->top;

    serial_logf(3, 2, "[CB] Calculated width=%d height=%d\n", width, height);

    if (width <= 0 || height <= 0) {
        serial_logf(3, 1, "[CB] Invalid dimensions: width=%d height=%d\n", width, height);
        return;
    }

    /* OPTIMIZED: Fast path for 32-bit to 32-bit srcCopy without masking */
    Boolean srcIs32 = IsPixMap(srcBits) && ((const PixMap*)srcBits)->pixelSize == 32;
    Boolean dstIs32 = IsPixMap(dstBits) && ((const PixMap*)dstBits)->pixelSize == 32;

    if (srcIs32 && dstIs32 && mode == srcCopy && (!maskRgn || !*maskRgn)) {
        /* Use fast memcpy for each row */
        SInt16 srcRowBytes = srcBits->rowBytes & 0x3FFF;
        SInt16 dstRowBytes = dstBits->rowBytes & 0x3FFF;
        UInt32* srcBase = (UInt32*)srcBits->baseAddr;
        UInt32* dstBase = (UInt32*)dstBits->baseAddr;

        for (SInt16 y = 0; y < height; y++) {
            SInt16 srcY = srcRect->top + y - srcBits->bounds.top;
            SInt16 dstY = dstRect->top + y - dstBits->bounds.top;
            SInt16 srcX = srcRect->left - srcBits->bounds.left;
            SInt16 dstX = dstRect->left - dstBits->bounds.left;

            UInt32* srcRow = srcBase + (srcY * (srcRowBytes / 4)) + srcX;
            UInt32* dstRow = dstBase + (dstY * (dstRowBytes / 4)) + dstX;

            memcpy(dstRow, srcRow, width * 4);
        }
        serial_logf(3, 2, "[CB_FAST] Done\n");
        return;
    }

    /* Fallback: Copy row by row using pixel operations */
    for (SInt16 y = 0; y < height; y++) {
        CopyPixelRow(srcBits, dstBits,
                    srcRect->top + y, dstRect->top + y,
                    srcRect->left, srcRect->right,
                    dstRect->left, dstRect->right,
                    mode, maskRgn);
    }
}

static void CopyPixelRow(const BitMap *srcBits, const BitMap *dstBits,
                        SInt16 srcY, SInt16 dstY, SInt16 srcLeft, SInt16 srcRight,
                        SInt16 dstLeft, SInt16 dstRight, SInt16 mode,
                        RgnHandle maskRgn) {
    SInt16 width = srcRight - srcLeft;

    for (SInt16 x = 0; x < width; x++) {
        if (maskRgn && *maskRgn) {
            Point dstPt;
            dstPt.v = dstY;
            dstPt.h = dstLeft + x;
            if (!PtInRgn(dstPt, maskRgn)) {
                continue;
            }
        }

        UInt32 srcPixel = GetPixelValue(srcBits, srcLeft + x, srcY);
        UInt32 dstPixel = GetPixelValue(dstBits, dstLeft + x, dstY);
        UInt32 resultPixel = ApplyTransferMode(srcPixel, dstPixel, 0, mode);
        SetPixelValue(dstBits, dstLeft + x, dstY, resultPixel);
    }
}

/* ================================================================
 * MASKING OPERATIONS
 * ================================================================ */

void CopyMask(const BitMap *srcBits, const BitMap *maskBits,
              const BitMap *dstBits, const Rect *srcRect,
              const Rect *maskRect, const Rect *dstRect) {
    assert(srcBits != NULL);
    assert(maskBits != NULL);
    assert(dstBits != NULL);
    assert(srcRect != NULL);
    assert(maskRect != NULL);
    assert(dstRect != NULL);

    /* Validate rectangles */
    if (EmptyRect(srcRect) || EmptyRect(maskRect) || EmptyRect(dstRect)) return;

    SInt16 width = srcRect->right - srcRect->left;
    SInt16 height = srcRect->bottom - srcRect->top;

    /* Copy pixels where mask is non-zero */
    for (SInt16 y = 0; y < height; y++) {
        for (SInt16 x = 0; x < width; x++) {
            UInt32 maskPixel = GetPixelValue(maskBits,
                                             maskRect->left + x,
                                             maskRect->top + y);

            if (maskPixel != 0) {
                UInt32 srcPixel = GetPixelValue(srcBits,
                                                srcRect->left + x,
                                                srcRect->top + y);
                SetPixelValue(dstBits,
                            dstRect->left + x,
                            dstRect->top + y,
                            srcPixel);
            }
        }
    }
}

void CopyDeepMask(const BitMap *srcBits, const BitMap *maskBits,
                  const BitMap *dstBits, const Rect *srcRect,
                  const Rect *maskRect, const Rect *dstRect,
                  SInt16 mode, RgnHandle maskRgn) {
    assert(srcBits != NULL);
    assert(maskBits != NULL);
    assert(dstBits != NULL);
    assert(srcRect != NULL);
    assert(maskRect != NULL);
    assert(dstRect != NULL);

    /* Validate rectangles */
    if (EmptyRect(srcRect) || EmptyRect(maskRect) || EmptyRect(dstRect)) return;

    SInt16 width = srcRect->right - srcRect->left;
    SInt16 height = srcRect->bottom - srcRect->top;

    /* Copy pixels using mask and transfer mode */
    for (SInt16 y = 0; y < height; y++) {
        for (SInt16 x = 0; x < width; x++) {
            UInt32 maskPixel = GetPixelValue(maskBits,
                                             maskRect->left + x,
                                             maskRect->top + y);

            if (maskPixel != 0) {
                UInt32 srcPixel = GetPixelValue(srcBits,
                                                srcRect->left + x,
                                                srcRect->top + y);
                UInt32 dstPixel = GetPixelValue(dstBits,
                                                dstRect->left + x,
                                                dstRect->top + y);

                UInt32 resultPixel = ApplyTransferMode(srcPixel, dstPixel, 0, mode);
                SetPixelValue(dstBits,
                            dstRect->left + x,
                            dstRect->top + y,
                            resultPixel);
            }
        }
    }
}

/* ================================================================
 * RECTANGLE SCROLLING
 * ================================================================ */

void ScrollRect(const Rect *r, SInt16 dh, SInt16 dv, RgnHandle updateRgn) {
    if (!r || !g_currentPort) {
        if (updateRgn && *updateRgn) {
            SetEmptyRgn(updateRgn);
        }
        return;
    }

    Rect scrollRect = *r;

    if (EmptyRect(&scrollRect) || (dh == 0 && dv == 0)) {
        if (updateRgn && *updateRgn) {
            SetEmptyRgn(updateRgn);
        }
        return;
    }

    /* Confine scroll region to the current port */
    Rect srcRectLocal;
    if (!SectRect(&scrollRect, &g_currentPort->portRect, &srcRectLocal)) {
        if (updateRgn && *updateRgn) {
            SetEmptyRgn(updateRgn);
        }
        return;
    }

    Rect dstRectLocal = srcRectLocal;
    OffsetRect(&dstRectLocal, dh, dv);

    /* Determine on-screen destination area */
    Rect copyDstLocal;
    if (!SectRect(&dstRectLocal, &g_currentPort->portRect, &copyDstLocal)) {
        if (updateRgn && *updateRgn) {
            RectRgn(updateRgn, &srcRectLocal);
        }
        return;
    }

    /* Map destination back to the source space to get scroll payload */
    Rect copySrcLocal = copyDstLocal;
    OffsetRect(&copySrcLocal, -dh, -dv);

    if (!SectRect(&copySrcLocal, &srcRectLocal, &copySrcLocal)) {
        if (updateRgn && *updateRgn) {
            RectRgn(updateRgn, &srcRectLocal);
        }
        return;
    }

    /* Destination aligned with the clipped source */
    Rect copyDstAligned = copySrcLocal;
    OffsetRect(&copyDstAligned, dh, dv);

    /* Convert rectangles to global coordinates for CopyBits */
    Rect srcRectGlobal = copySrcLocal;
    Rect dstRectGlobal = copyDstAligned;
    OffsetRect(&srcRectGlobal,
               g_currentPort->portBits.bounds.left,
               g_currentPort->portBits.bounds.top);
    OffsetRect(&dstRectGlobal,
               g_currentPort->portBits.bounds.left,
               g_currentPort->portBits.bounds.top);

    RgnHandle maskRgn = NULL;
    if (g_currentPort->clipRgn && *g_currentPort->clipRgn) {
        maskRgn = g_currentPort->clipRgn;
    }

    CopyBits(&g_currentPort->portBits, &g_currentPort->portBits,
             &srcRectGlobal, &dstRectGlobal, srcCopy, maskRgn);

    if (updateRgn && *updateRgn) {
        RectRgn(updateRgn, &srcRectLocal);

        Rect overlapLocal;
        if (SectRect(&srcRectLocal, &dstRectLocal, &overlapLocal)) {
            RgnHandle overlapRgn = NewRgn();
            if (overlapRgn && *overlapRgn) {
                RectRgn(overlapRgn, &overlapLocal);
                DiffRgn(updateRgn, overlapRgn, updateRgn);
                DisposeRgn(overlapRgn);
            } else if (overlapRgn) {
                DisposeRgn(overlapRgn);
            }
        }
    }
}

/* ================================================================
 * SEED FILL OPERATIONS
 * ================================================================ */

void SeedFill(const void *srcPtr, void *dstPtr, SInt16 srcRow, SInt16 dstRow,
              SInt16 height, SInt16 words, SInt16 seedH, SInt16 seedV) {
    assert(srcPtr != NULL);
    assert(dstPtr != NULL);

    /* Simple flood-fill implementation */
    /* This is a simplified version - a full implementation would be more complex */

    const UInt16 *src = (const UInt16 *)srcPtr;
    UInt16 *dst = (UInt16 *)dstPtr;

    /* Copy source to destination first */
    for (SInt16 y = 0; y < height; y++) {
        const UInt16 *srcLine = src + y * (srcRow / 2);
        UInt16 *dstLine = dst + y * (dstRow / 2);
        memcpy(dstLine, srcLine, words * 2);
    }

    /* Perform seed fill at specified location */
    if (seedV < height && seedH < words * 16) {
        UInt16 *seedLine = dst + seedV * (dstRow / 2);
        UInt16 wordIndex = seedH / 16;
        UInt16 bitIndex = seedH % 16;

        /* Set the seed bit */
        seedLine[wordIndex] |= (1 << (15 - bitIndex));
    }
}

void CalcMask(const void *srcPtr, void *dstPtr, SInt16 srcRow, SInt16 dstRow,
              SInt16 height, SInt16 words) {
    assert(srcPtr != NULL);
    assert(dstPtr != NULL);

    const UInt16 *src = (const UInt16 *)srcPtr;
    UInt16 *dst = (UInt16 *)dstPtr;

    /* Calculate mask from source */
    for (SInt16 y = 0; y < height; y++) {
        const UInt16 *srcLine = src + y * (srcRow / 2);
        UInt16 *dstLine = dst + y * (dstRow / 2);

        for (SInt16 w = 0; w < words; w++) {
            /* Create mask: 1 where source is non-zero, 0 where zero */
            dstLine[w] = (srcLine[w] != 0) ? 0xFFFF : 0x0000;
        }
    }
}

/* ================================================================
 * UTILITY FUNCTIONS
 * ================================================================ */

static UInt32 ApplyTransferMode(UInt32 src, UInt32 dst, UInt32 pattern, SInt16 mode) {
    /* Clamp mode to valid range */
    if (mode < 0 || mode >= sizeof(g_transferModes) / sizeof(g_transferModes[0])) {
        mode = srcCopy;
    }

    const TransferModeInfo *modeInfo = &g_transferModes[mode];
    return modeInfo->operation(src, dst, pattern);
}

static void CalculateScaling(const Rect *srcRect, const Rect *dstRect, ScaleInfo *scaleInfo) {
    scaleInfo->srcWidth = srcRect->right - srcRect->left;
    scaleInfo->srcHeight = srcRect->bottom - srcRect->top;
    scaleInfo->dstWidth = dstRect->right - dstRect->left;
    scaleInfo->dstHeight = dstRect->bottom - dstRect->top;

    scaleInfo->needsScaling = (scaleInfo->srcWidth != scaleInfo->dstWidth ||
                              scaleInfo->srcHeight != scaleInfo->dstHeight);

    if (scaleInfo->needsScaling) {
        scaleInfo->hScale = (scaleInfo->srcWidth * FIXED_POINT_SCALE) / scaleInfo->dstWidth;
        scaleInfo->vScale = (scaleInfo->srcHeight * FIXED_POINT_SCALE) / scaleInfo->dstHeight;
    } else {
        scaleInfo->hScale = FIXED_POINT_SCALE;
        scaleInfo->vScale = FIXED_POINT_SCALE;
    }
}

static UInt32 GetPixelValue(const BitMap *bitmap, SInt16 x, SInt16 y) {
    if (x < (bitmap)->bounds.left || x >= (bitmap)->bounds.right ||
        y < (bitmap)->bounds.top || y >= (bitmap)->bounds.bottom) {
        return 0;
    }

    /* Calculate byte offset */
    SInt16 relativeX = x - (bitmap)->bounds.left;
    SInt16 relativeY = y - (bitmap)->bounds.top;
    SInt16 rowBytes = bitmap->rowBytes & 0x3FFF; /* Mask out flags */

    const UInt8 *baseAddr = (const UInt8 *)bitmap->baseAddr;
    if (!baseAddr) return 0;

    const UInt8 *pixel = baseAddr + relativeY * rowBytes;

    if (IsPixMap(bitmap)) {
        /* Color bitmap */
        const PixMap *pixMap = (const PixMap *)bitmap;
        SInt16 pixelSize = pixMap->pixelSize;

        switch (pixelSize) {
            case 1: {
                SInt16 byteIndex = relativeX / 8;
                SInt16 bitIndex = relativeX % 8;
                return (pixel[byteIndex] >> (7 - bitIndex)) & 1;
            }
            case 8:
                return pixel[relativeX];
            case 16: {
                const UInt16 *pixel16 = (const UInt16 *)pixel;
                return pixel16[relativeX];
            }
            case 32: {
                const UInt32 *pixel32 = (const UInt32 *)pixel;
                return pixel32[relativeX];
            }
            default:
                return 0;
        }
    } else {
        /* 1-bit bitmap */
        SInt16 byteIndex = relativeX / 8;
        SInt16 bitIndex = relativeX % 8;
        return (pixel[byteIndex] >> (7 - bitIndex)) & 1;
    }
}

static void SetPixelValue(const BitMap *bitmap, SInt16 x, SInt16 y, UInt32 value) {
    if (x < (bitmap)->bounds.left || x >= (bitmap)->bounds.right ||
        y < (bitmap)->bounds.top || y >= (bitmap)->bounds.bottom) {
        return;
    }

    /* Calculate byte offset */
    SInt16 relativeX = x - (bitmap)->bounds.left;
    SInt16 relativeY = y - (bitmap)->bounds.top;
    SInt16 rowBytes = bitmap->rowBytes & 0x3FFF; /* Mask out flags */

    UInt8 *baseAddr = (UInt8 *)bitmap->baseAddr;
    if (!baseAddr) return;

    UInt8 *pixel = baseAddr + relativeY * rowBytes;

    if (IsPixMap(bitmap)) {
        /* Color bitmap */
        const PixMap *pixMap = (const PixMap *)bitmap;
        SInt16 pixelSize = pixMap->pixelSize;

        switch (pixelSize) {
            case 1: {
                SInt16 byteIndex = relativeX / 8;
                SInt16 bitIndex = relativeX % 8;
                if (value & 1) {
                    pixel[byteIndex] |= (1 << (7 - bitIndex));
                } else {
                    pixel[byteIndex] &= ~(1 << (7 - bitIndex));
                }
                break;
            }
            case 8:
                pixel[relativeX] = (UInt8)value;
                break;
            case 16: {
                UInt16 *pixel16 = (UInt16 *)pixel;
                pixel16[relativeX] = (UInt16)value;
                break;
            }
            case 32: {
                UInt32 *pixel32 = (UInt32 *)pixel;
                pixel32[relativeX] = value;
                break;
            }
        }
    } else {
        /* 1-bit bitmap */
        SInt16 byteIndex = relativeX / 8;
        SInt16 bitIndex = relativeX % 8;
        if (value & 1) {
            pixel[byteIndex] |= (1 << (7 - bitIndex));
        } else {
            pixel[byteIndex] &= ~(1 << (7 - bitIndex));
        }
    }
}

static void ClipRectToBitmap(const BitMap *bitmap, Rect *rect) {
    Rect clippedRect;
    if (SectRect(rect, &bitmap->bounds, &clippedRect)) {
        *rect = clippedRect;
    } else {
        SetRect(rect, 0, 0, 0, 0);
    }
}

/* ================================================================
 * BITMAP REGION CONVERSION
 * ================================================================ */

SInt16 BitMapToRegion(RgnHandle region, const BitMap *bMap) {
    assert(region != NULL && *region != NULL);
    assert(bMap != NULL);

    /* Start with empty region */
    SetEmptyRgn(region);

    if (!bMap->baseAddr) return 0;

    SInt16 width = (bMap)->bounds.right - (bMap)->bounds.left;
    SInt16 height = (bMap)->bounds.bottom - (bMap)->bounds.top;

    /* For 1-bit bitmaps, create region from set bits */
    if (!IsPixMap(bMap)) {
        RgnHandle tempRgn = NewRgn();
        if (!tempRgn) return rgnOverflowErr;

        for (SInt16 y = 0; y < height; y++) {
            for (SInt16 x = 0; x < width; x++) {
                if (GetPixelValue(bMap, (bMap)->bounds.left + x, (bMap)->bounds.top + y)) {
                    Rect pixelRect;
                    SetRect(&pixelRect,
                           (bMap)->bounds.left + x, (bMap)->bounds.top + y,
                           (bMap)->bounds.left + x + 1, (bMap)->bounds.top + y + 1);
                    RectRgn(tempRgn, &pixelRect);
                    UnionRgn(region, tempRgn, region);
                }
            }
        }

        DisposeRgn(tempRgn);
    }

    return 0;
}
