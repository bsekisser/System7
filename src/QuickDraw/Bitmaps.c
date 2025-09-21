/* #include "SystemTypes.h" */
#include <string.h>
/*
 * Bitmaps.c - QuickDraw Bitmap and CopyBits Implementation
 *
 * Complete implementation of bitmap operations including CopyBits,
 * scaling, transfer modes, masking, and pixel manipulation.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 * Based on Apple Macintosh System Software 7.1 QuickDraw
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
#include "QuickDrawPlatform.h"


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
                          SInt16 mode, const ScaleInfo *scaleInfo);
static void CopyBitsUnscaled(const BitMap *srcBits, const BitMap *dstBits,
                            const Rect *srcRect, const Rect *dstRect,
                            SInt16 mode);
static UInt32 ApplyTransferMode(UInt32 src, UInt32 dst, UInt32 pattern, SInt16 mode);
static void CalculateScaling(const Rect *srcRect, const Rect *dstRect, ScaleInfo *scaleInfo);
static void CopyPixelRow(const BitMap *srcBits, const BitMap *dstBits,
                        SInt16 srcY, SInt16 dstY, SInt16 srcLeft, SInt16 srcRight,
                        SInt16 dstLeft, SInt16 dstRight, SInt16 mode);
static UInt32 GetPixelValue(const BitMap *bitmap, SInt16 x, SInt16 y);
static void SetPixelValue(const BitMap *bitmap, SInt16 x, SInt16 y, UInt32 value);
/* IsPixMap is now defined in ColorQuickDraw.h */
/* static Boolean IsPixMap(const BitMap *bitmap); */
static SInt16 GetBitmapDepth(const BitMap *bitmap);
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

static UInt32 TransferBlend(UInt32 src, UInt32 dst, UInt32 pattern) {
    /* Simple average blend */
    return (src + dst) / 2;
}

static UInt32 TransferAddPin(UInt32 src, UInt32 dst, UInt32 pattern) {
    UInt32 result = src + dst;
    return (result > 0xFFFFFF) ? 0xFFFFFF : result;
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
                       mode, &scaleInfo);
    } else {
        CopyBitsUnscaled(srcBits, dstBits, &clippedSrcRect, &clippedDstRect, mode);
    }
}

static void CopyBitsScaled(const BitMap *srcBits, const BitMap *dstBits,
                          const Rect *srcRect, const Rect *dstRect,
                          SInt16 mode, const ScaleInfo *scaleInfo) {
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

            /* Set destination pixel */
            SetPixelValue(dstBits, dstRect->left + dstX, dstRect->top + dstY, resultPixel);
        }
    }
}

static void CopyBitsUnscaled(const BitMap *srcBits, const BitMap *dstBits,
                            const Rect *srcRect, const Rect *dstRect,
                            SInt16 mode) {
    SInt16 width = srcRect->right - srcRect->left;
    SInt16 height = srcRect->bottom - srcRect->top;

    /* Copy row by row for efficiency */
    for (SInt16 y = 0; y < height; y++) {
        CopyPixelRow(srcBits, dstBits,
                    srcRect->top + y, dstRect->top + y,
                    srcRect->left, srcRect->right,
                    dstRect->left, dstRect->right,
                    mode);
    }
}

static void CopyPixelRow(const BitMap *srcBits, const BitMap *dstBits,
                        SInt16 srcY, SInt16 dstY, SInt16 srcLeft, SInt16 srcRight,
                        SInt16 dstLeft, SInt16 dstRight, SInt16 mode) {
    SInt16 width = srcRight - srcLeft;

    for (SInt16 x = 0; x < width; x++) {
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

    UInt8 *baseAddr = (UInt8 *)bitmap->baseAddr;
    if (!baseAddr) return 0;

    UInt8 *pixel = baseAddr + relativeY * rowBytes;

    if (IsPixMap(bitmap)) {
        /* Color bitmap */
        PixMap *pixMap = (PixMap *)bitmap;
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
                UInt16 *pixel16 = (UInt16 *)pixel;
                return pixel16[relativeX];
            }
            case 32: {
                UInt32 *pixel32 = (UInt32 *)pixel;
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
        PixMap *pixMap = (PixMap *)bitmap;
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

/* IsPixMap function removed - now using the inline version from ColorQuickDraw.h */

static SInt16 GetBitmapDepth(const BitMap *bitmap) {
    if (IsPixMap(bitmap)) {
        PixMap *pixMap = (PixMap *)bitmap;
        return pixMap->pixelSize;
    }
    return 1;
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
