/*
 * MacPaint.h - MacPaint Application Interface
 *
 * Public API for MacPaint 1.3 running on System 7.1 Portable
 * Adapted from original MacPaint by Bill Atkinson
 */

#ifndef MACPAINT_H
#define MACPAINT_H

#include "SystemTypes.h"
#include "QuickDraw/QuickDraw.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Tool Enumeration
 */
typedef enum {
    MacPaintTool_Lasso = 0,
    MacPaintTool_Select = 1,
    MacPaintTool_Grabber = 2,
    MacPaintTool_Text = 3,
    MacPaintTool_Fill = 4,
    MacPaintTool_Spray = 5,
    MacPaintTool_Brush = 6,
    MacPaintTool_Pencil = 7,
    MacPaintTool_Line = 8,
    MacPaintTool_Erase = 9,
    MacPaintTool_Rect = 11
} MacPaintTool;

/*
 * Constants
 */
#define MACPAINT_DOC_WIDTH 576
#define MACPAINT_DOC_HEIGHT 720
#define MACPAINT_PATTERN_COUNT 38

/*
 * Initialization and Shutdown
 */
OSErr MacPaint_Initialize(void);
void MacPaint_Shutdown(void);

/*
 * Tool Operations
 */
void MacPaint_SelectTool(MacPaintTool toolID);
void MacPaint_SetLineSize(int size);
void MacPaint_SetPattern(int patternIndex);

/*
 * Drawing Operations
 */
void MacPaint_DrawLine(int x1, int y1, int x2, int y2);
void MacPaint_FillRect(Rect *rect);
void MacPaint_DrawOval(Rect *rect);
void MacPaint_DrawRect(Rect *rect);

/*
 * Document Operations
 */
OSErr MacPaint_NewDocument(void);
OSErr MacPaint_SaveDocument(const char *filename);
OSErr MacPaint_OpenDocument(const char *filename);

/*
 * Low-level Bit Operations (ported from 68k assembly)
 */
int MacPaint_PixelTrue(int h, int v, BitMap *bits);
void MacPaint_SetPixel(int h, int v, BitMap *bits);
void MacPaint_ClearPixel(int h, int v, BitMap *bits);
void MacPaint_InvertBuf(BitMap *buf);
void MacPaint_ZeroBuf(BitMap *buf);
void MacPaint_ExpandPattern(Pattern pat, UInt32 *expanded);

/*
 * Rendering
 */
void MacPaint_Render(void);
void MacPaint_InvalidateRect(Rect *rect);

#ifdef __cplusplus
}
#endif

#endif /* MACPAINT_H */
