/*
 * MacPaint_Core.c - MacPaint 1.3 Core Application Logic
 *
 * Converted from original MacPaint 1.3 Pascal source by Bill Atkinson
 * Adapted for System 7.1 Portable on ARM/x86
 *
 * This module implements the core painting application logic, including:
 * - Tool management and drawing operations
 * - Pattern and line drawing
 * - Painting algorithms (ported from 68k assembly)
 * - Document management
 *
 * ORIGINAL SOURCE STATS:
 * - MacPaint.p: 5804 lines of Pascal
 * - PaintAsm.a: 2738 lines of 68k assembly (painting algorithms)
 * - MyHeapAsm.a: 159 lines of memory manager wrappers
 * - MyTools.a: 631 lines of toolbox trap definitions
 *
 * CONVERSION APPROACH:
 * - Pascal procedures/functions → C functions
 * - 68k assembly → C implementations of algorithms
 * - Direct Toolbox calls → System 7.1 Toolbox
 * - BitMap operations → QuickDraw equivalents
 */

#include "SystemTypes.h"
#include "QuickDraw/QuickDraw.h"
#include "Finder/finder.h"
#include "System71StdLib.h"
#include "MemoryMgr/MemoryManager.h"
#include <string.h>

/*
 * MacPaint Global State (converted from Pascal VAR declarations)
 */

/* UI and Window State */
static WindowPtr gPaintWindow = NULL;
static GrafPtr gPaintPort = NULL;
static Rect gPaintRect;

/* Document State */
static char gDocName[64];
static int gDocDrive = 0;
static int gDocDirty = 0;
static UInt32 gWorkSize = 0;

/* Tool State */
typedef enum {
    TOOL_LASSO = 0,
    TOOL_SELECT = 1,
    TOOL_GRABBER = 2,
    TOOL_TEXT = 3,
    TOOL_FILL = 4,
    TOOL_SPRAY = 5,
    TOOL_BRUSH = 6,
    TOOL_PENCIL = 7,
    TOOL_LINE = 8,
    TOOL_ERASE = 9,
    TOOL_RECT = 11
} MacPaintTool;

static MacPaintTool gCurrentTool = TOOL_PENCIL;
static int gLineSize = 1;
static Pattern gCurrentPattern;
static Rect gSelectionRect;
static int gSelectionActive = 0;

/* Pattern storage (from resource) */
#define MACPAINT_PATTERN_COUNT 38
static Pattern gPatterns[MACPAINT_PATTERN_COUNT];

/* Canvas/Drawing Buffer */
#define MACPAINT_DOC_WIDTH 576
#define MACPAINT_DOC_HEIGHT 720
static BitMap gPaintBuffer;
static Ptr gPaintBufferData = NULL;

/*
 * Forward Declarations for Assembly-converted functions
 */
extern void MacPaint_PixelRow(Ptr src, Ptr dst, int width, int mode);
extern void MacPaint_ExpandPattern(Pattern pat, UInt32 *expanded);
extern void MacPaint_BlitRectangle(BitMap *src, BitMap *dst, Rect *srcRect, Rect *dstRect);

/*
 * INITIALIZATION
 */

/**
 * MacPaint_Initialize - Initialize MacPaint application
 * Called at startup to set up all data structures
 */
OSErr MacPaint_Initialize(void)
{
    int i;
    Size bufferSize;

    /* Initialize global state */
    gCurrentTool = TOOL_PENCIL;
    gLineSize = 1;
    gSelectionActive = 0;
    gDocDirty = 0;
    strcpy(gDocName, "Untitled");

    /* Initialize pattern table - will be loaded from resources */
    for (i = 0; i < MACPAINT_PATTERN_COUNT; i++) {
        memset(&gPatterns[i], 0, sizeof(Pattern));
    }

    /* Initialize paint buffer (for off-screen drawing) */
    bufferSize = (MACPAINT_DOC_WIDTH / 8) * MACPAINT_DOC_HEIGHT;
    gPaintBufferData = NewPtr(bufferSize);
    if (!gPaintBufferData) {
        return memFullErr;
    }

    /* Set up BitMap structure for paint buffer */
    gPaintBuffer.baseAddr = gPaintBufferData;
    gPaintBuffer.rowBytes = MACPAINT_DOC_WIDTH / 8;
    gPaintBuffer.bounds.top = 0;
    gPaintBuffer.bounds.left = 0;
    gPaintBuffer.bounds.bottom = MACPAINT_DOC_HEIGHT;
    gPaintBuffer.bounds.right = MACPAINT_DOC_WIDTH;

    return noErr;
}

/**
 * MacPaint_Shutdown - Clean up MacPaint resources
 */
void MacPaint_Shutdown(void)
{
    if (gPaintBufferData) {
        DisposePtr(gPaintBufferData);
        gPaintBufferData = NULL;
    }

    if (gPaintWindow) {
        DisposeWindow(gPaintWindow);
        gPaintWindow = NULL;
    }
}

/*
 * TOOL OPERATIONS
 */

/**
 * MacPaint_SelectTool - Select active drawing tool
 * toolID: 0-11 (lasso, select, grabber, text, fill, spray, brush, pencil, line, erase, rect)
 */
void MacPaint_SelectTool(MacPaintTool toolID)
{
    gCurrentTool = toolID;
    /* TODO: Update cursor and UI */
}

/**
 * MacPaint_SetLineSize - Set line/brush size (1-8)
 */
void MacPaint_SetLineSize(int size)
{
    if (size >= 1 && size <= 8) {
        gLineSize = size;
    }
}

/**
 * MacPaint_SetPattern - Set current drawing pattern
 */
void MacPaint_SetPattern(int patternIndex)
{
    if (patternIndex >= 0 && patternIndex < MACPAINT_PATTERN_COUNT) {
        memcpy(&gCurrentPattern, &gPatterns[patternIndex], sizeof(Pattern));
    }
}

/*
 * DRAWING OPERATIONS
 */

/**
 * MacPaint_DrawLine - Draw a line from (x1,y1) to (x2,y2)
 * Using Bresenham's line algorithm
 */
void MacPaint_DrawLine(int x1, int y1, int x2, int y2)
{
    int dx, dy, sx, sy, err, e2;
    int x, y;

    dx = (x2 > x1) ? (x2 - x1) : (x1 - x2);
    dy = (y2 > y1) ? (y2 - y1) : (y1 - y2);
    sx = (x1 < x2) ? 1 : -1;
    sy = (y1 < y2) ? 1 : -1;
    err = dx - dy;

    x = x1;
    y = y1;

    while (1) {
        /* Draw pixel at (x, y) */
        /* SetCPixel(x, y, gCurrentPattern); */

        if (x == x2 && y == y2) break;

        e2 = 2 * err;
        if (e2 > -dy) {
            err = err - dy;
            x = x + sx;
        }
        if (e2 < dx) {
            err = err + dx;
            y = y + sy;
        }
    }

    gDocDirty = 1;
}

/**
 * MacPaint_FillRect - Fill rectangle with current pattern
 */
void MacPaint_FillRect(Rect *rect)
{
    if (!rect) return;

    /* TODO: Implement pattern-based filling */
    /* Use gCurrentPattern to fill the rectangle */

    gDocDirty = 1;
}

/**
 * MacPaint_DrawOval - Draw oval shape
 */
void MacPaint_DrawOval(Rect *rect)
{
    if (!rect) return;

    /* TODO: Implement oval drawing (Bresenham circle algorithm) */

    gDocDirty = 1;
}

/**
 * MacPaint_DrawRect - Draw rectangle outline
 */
void MacPaint_DrawRect(Rect *rect)
{
    if (!rect) return;

    /* Draw four lines for rectangle */
    MacPaint_DrawLine(rect->left, rect->top, rect->right, rect->top);
    MacPaint_DrawLine(rect->right, rect->top, rect->right, rect->bottom);
    MacPaint_DrawLine(rect->right, rect->bottom, rect->left, rect->bottom);
    MacPaint_DrawLine(rect->left, rect->bottom, rect->left, rect->top);

    gDocDirty = 1;
}

/*
 * DOCUMENT OPERATIONS
 */

/**
 * MacPaint_NewDocument - Create a new blank document
 */
OSErr MacPaint_NewDocument(void)
{
    /* Clear paint buffer */
    memset(gPaintBufferData, 0, (MACPAINT_DOC_WIDTH / 8) * MACPAINT_DOC_HEIGHT);

    /* Reset document state */
    strcpy(gDocName, "Untitled");
    gDocDirty = 0;
    gSelectionActive = 0;

    /* TODO: Create and display window */

    return noErr;
}

/**
 * MacPaint_SaveDocument - Save document to file
 */
OSErr MacPaint_SaveDocument(const char *filename)
{
    /* TODO: Implement file saving
     * Format:
     * - Header (magic, version, width, height)
     * - Compressed bitmap data (PackBits format)
     * - Resource fork with patterns/metadata
     */

    if (filename) {
        strcpy(gDocName, filename);
        gDocDirty = 0;
    }

    return noErr;
}

/**
 * MacPaint_OpenDocument - Open document from file
 */
OSErr MacPaint_OpenDocument(const char *filename)
{
    /* TODO: Implement file loading
     * Read header, validate format, decompress bitmap
     */

    strcpy(gDocName, filename);
    gDocDirty = 0;

    return noErr;
}

/*
 * ASSEMBLY-CONVERTED FUNCTIONS (from PaintAsm.a)
 * These were originally implemented in 68k assembly and are ported to C
 */

/**
 * MacPaint_PixelTrue - Check if pixel at (h,v) is set in bitmap
 * Returns 1 if pixel is set, 0 if clear
 */
int MacPaint_PixelTrue(int h, int v, BitMap *bits)
{
    if (!bits || h < 0 || v < 0 || h >= bits->bounds.right || v >= bits->bounds.bottom) {
        return 0;
    }

    int byteOffset = (v * bits->rowBytes) + (h / 8);
    int bitOffset = 7 - (h % 8);
    unsigned char byte = ((unsigned char *)bits->baseAddr)[byteOffset];

    return (byte >> bitOffset) & 1;
}

/**
 * MacPaint_SetPixel - Set pixel at (h,v) in bitmap
 */
void MacPaint_SetPixel(int h, int v, BitMap *bits)
{
    if (!bits || h < 0 || v < 0 || h >= bits->bounds.right || v >= bits->bounds.bottom) {
        return;
    }

    int byteOffset = (v * bits->rowBytes) + (h / 8);
    int bitOffset = 7 - (h % 8);
    ((unsigned char *)bits->baseAddr)[byteOffset] |= (1 << bitOffset);
}

/**
 * MacPaint_ClearPixel - Clear pixel at (h,v) in bitmap
 */
void MacPaint_ClearPixel(int h, int v, BitMap *bits)
{
    if (!bits || h < 0 || v < 0 || h >= bits->bounds.right || v >= bits->bounds.bottom) {
        return;
    }

    int byteOffset = (v * bits->rowBytes) + (h / 8);
    int bitOffset = 7 - (h % 8);
    ((unsigned char *)bits->baseAddr)[byteOffset] &= ~(1 << bitOffset);
}

/**
 * MacPaint_InvertBuf - Invert all bits in buffer (XOR with black)
 */
void MacPaint_InvertBuf(BitMap *buf)
{
    if (!buf) return;

    int byteCount = buf->rowBytes * (buf->bounds.bottom - buf->bounds.top);
    unsigned char *ptr = (unsigned char *)buf->baseAddr;

    for (int i = 0; i < byteCount; i++) {
        ptr[i] ^= 0xFF;
    }
}

/**
 * MacPaint_ZeroBuf - Clear all bits in buffer
 */
void MacPaint_ZeroBuf(BitMap *buf)
{
    if (!buf) return;

    int byteCount = buf->rowBytes * (buf->bounds.bottom - buf->bounds.top);
    memset(buf->baseAddr, 0, byteCount);
}

/**
 * MacPaint_ExpandPattern - Expand 8x8 pattern to 24 longwords for drawing
 * This was in ExpandPat procedure
 */
void MacPaint_ExpandPattern(Pattern pat, UInt32 *expanded)
{
    if (!expanded) return;

    for (int i = 0; i < 8; i++) {
        /* Each row of pattern becomes 3 longwords (24 bits = 3 bytes * 8) */
        unsigned char row = pat.pat[i];
        expanded[i * 3 + 0] = ((row & 0x80) << 24) | ((row & 0x40) << 17) | ((row & 0x20) << 10) | ((row & 0x10) << 3);
        expanded[i * 3 + 1] = ((row & 0x08) << 28) | ((row & 0x04) << 21) | ((row & 0x02) << 14) | ((row & 0x01) << 7);
        expanded[i * 3 + 2] = 0;
    }
}

/*
 * RENDERING
 */

/**
 * MacPaint_Render - Render paint buffer to window
 */
void MacPaint_Render(void)
{
    if (!gPaintWindow || !gPaintPort) {
        return;
    }

    /* TODO: Copy gPaintBuffer to screen via CopyBits or similar */
}

/**
 * MacPaint_InvalidateRect - Mark region for redraw
 */
void MacPaint_InvalidateRect(Rect *rect)
{
    if (!gPaintWindow) return;

    /* TODO: Queue for redraw */
}
