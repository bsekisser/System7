/* #include "SystemTypes.h" */
// #include "CompatibilityFix.h" // Removed
/*
 * RE-AGENT-BANNER
 * QuickDraw Bitmap Operations
 * Clean-room reimplementation from ROM analysis
 *
 * SOURCE: Quadra 800 ROM (1MB, 1993 release)
 * Architecture: 68k Mac OS System 7
 * Evidence sources: trap analysis, Mac OS Toolbox reference
 *
 * This file implements QuickDraw bitmap operations, particularly CopyBits.
 */

#include "QuickDraw/quickdraw.h"
#include "QuickDraw/quickdraw_types.h"
/* #include "SystemTypes.h" */
#include "SystemTypes.h"
#include "QuickDrawConstants.h"


/*
 * CopyBits - Copy bits from source to destination bitmap
 * Trap: 0xA8EC

 */
void CopyBits(const BitMap* srcBits, const BitMap* dstBits,
              const Rect* srcRect, const Rect* dstRect,
              short mode, RgnHandle maskRgn)
{
    if (srcBits == NULL || dstBits == NULL ||
        srcRect == NULL || dstRect == NULL) return;

    /* Validate bitmaps have valid base addresses */
    if (srcBits->baseAddr == NULL || dstBits->baseAddr == NULL) return;

    /* TODO: Implement actual bitmap copying algorithm */
    /* This is a complex function that involves:
     * 1. Clipping source and destination rectangles to bitmap bounds
     * 2. Handling scaling if source and destination rectangles differ in size
     * 3. Applying the specified transfer mode
     * 4. Respecting the mask region if provided
     * 5. Handling different pixel formats and bit depths
     */

    /* Calculate actual copy rectangles after clipping */
    Rect clippedSrc = *srcRect;
    Rect clippedDst = *dstRect;

    /* Clip source rectangle to source bitmap bounds */
    if (!SectRect(&clippedSrc, &srcBits->bounds, &clippedSrc)) {
        return;  /* No intersection */
    }

    /* Clip destination rectangle to destination bitmap bounds */
    if (!SectRect(&clippedDst, &dstBits->bounds, &clippedDst)) {
        return;  /* No intersection */
    }

    /* Calculate scaling factors */
    short srcWidth = clippedSrc.right - clippedSrc.left;
    short srcHeight = clippedSrc.bottom - clippedSrc.top;
    short dstWidth = clippedDst.right - clippedDst.left;
    short dstHeight = clippedDst.bottom - clippedDst.top;

    /* For now, only handle 1:1 copying (no scaling) */
    if (srcWidth != dstWidth || srcHeight != dstHeight) {
        /* TODO: Implement scaling algorithm */
        return;
    }

    /* Simple byte copying for 1:1 transfers */
    /* This is a simplified implementation - real CopyBits is much more complex */
    const unsigned char* srcBase = (const unsigned char*)srcBits->baseAddr;
    unsigned char* dstBase = (unsigned char*)dstBits->baseAddr;

    short srcRowBytes = srcBits->rowBytes;
    short dstRowBytes = dstBits->rowBytes;

    /* Copy pixel data row by row */
    for (short y = 0; y < srcHeight; y++) {
        short srcY = clippedSrc.top + y;
        short dstY = clippedDst.top + y;

        const unsigned char* srcRow = srcBase + (srcY * srcRowBytes);
        unsigned char* dstRow = dstBase + (dstY * dstRowBytes);

        /* Copy bytes for this row */
        short bytesToCopy = srcWidth / 8;  /* Assuming 1-bit pixels */
        if (bytesToCopy > 0) {
            /* Apply transfer mode */
            switch (mode) {
                case srcCopy:
                    for (short x = 0; x < bytesToCopy; x++) {
                        dstRow[clippedDst.left / 8 + x] = srcRow[clippedSrc.left / 8 + x];
                    }
                    break;
                case srcOr:
                    for (short x = 0; x < bytesToCopy; x++) {
                        dstRow[clippedDst.left / 8 + x] |= srcRow[clippedSrc.left / 8 + x];
                    }
                    break;
                case srcXor:
                    for (short x = 0; x < bytesToCopy; x++) {
                        dstRow[clippedDst.left / 8 + x] ^= srcRow[clippedSrc.left / 8 + x];
                    }
                    break;
                case srcBic:
                    for (short x = 0; x < bytesToCopy; x++) {
                        dstRow[clippedDst.left / 8 + x] &= ~srcRow[clippedSrc.left / 8 + x];
                    }
                    break;
                /* TODO: Implement other transfer modes */
                default:
                    /* Default to source copy */
                    for (short x = 0; x < bytesToCopy; x++) {
                        dstRow[clippedDst.left / 8 + x] = srcRow[clippedSrc.left / 8 + x];
                    }
                    break;
            }
        }
    }

    /* TODO: Handle mask region if provided */
    /* The mask region would clip the copy operation further */
}

/*
 * ScrollRect - Scroll rectangle contents

 */
void ScrollRect(const Rect* r, short dh, short dv, RgnHandle updateRgn)
{
    if (thePort == NULL || r == NULL) return;

    /* TODO: Implement rectangle scrolling */
    /* This involves:
     * 1. Moving bitmap data within the rectangle
     * 2. Calculating the update region (areas that need redrawing)
     * 3. Using CopyBits to move the existing pixels
     */

    /* Calculate destination rectangle */
    Rect dstRect = *r;
    OffsetRect(&dstRect, dh, dv);

    /* Use CopyBits to move the existing content */
    CopyBits(&thePort->portBits, &thePort->portBits,
             r, &dstRect, srcCopy, NULL);

    /* TODO: Calculate and set update region */
    if (updateRgn != NULL) {
        /* Set update region to areas that were uncovered by the scroll */
        SetEmptyRgn(updateRgn);
    }
}

/*
 * Pattern utility functions
 */

/*
 * GetIndPattern - Get pattern from pattern list resource

 */
void GetIndPattern(Pattern* thePat, short patternListID, short index)
{
    if (thePat == NULL) return;

    /* TODO: Implement pattern resource loading */
    /* For now, return a default pattern based on index */
    switch (index) {
        case 1:  /* Black */
            *thePat = (Pattern){{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
            break;
        case 2:  /* Dark gray */
            *thePat = (Pattern){{0x77, 0xDD, 0x77, 0xDD, 0x77, 0xDD, 0x77, 0xDD}};
            break;
        case 3:  /* Gray */
            *thePat = (Pattern){{0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55}};
            break;
        case 4:  /* Light gray */
            *thePat = (Pattern){{0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22}};
            break;
        default:  /* White */
            *thePat = (Pattern){{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
            break;
    }
}

/*
 */
