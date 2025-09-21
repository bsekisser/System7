/*
 * QuickDraw Implementation
 * Provides actual drawing functionality for System 7.1
 */

#include "MacTypes.h"
#include "QuickDraw/QuickDraw.h"
#include <stdint.h>

/* External framebuffer from main.c */
extern void* framebuffer;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;
extern uint32_t pack_color(uint8_t r, uint8_t g, uint8_t b);

/* QuickDraw Globals */
QDGlobals qd = {0};

/* Current graphics port */
static GrafPort* currentPort = NULL;

/* Current pen state */
static Point penLocation = {0, 0};
static Pattern penPattern;
static short penMode = patCopy;

/* Initialize QuickDraw */
void InitGraf(void* globalPtr) {
    /* Initialize the QuickDraw globals */
    qd.thePort = NULL;
    qd.white.red = 0xFFFF;
    qd.white.green = 0xFFFF;
    qd.white.blue = 0xFFFF;
    qd.black.red = 0x0000;
    qd.black.green = 0x0000;
    qd.black.blue = 0x0000;
    qd.arrow.data[0] = 0x0000;  /* Standard arrow cursor */
    qd.dkGray.red = 0x4444;
    qd.dkGray.green = 0x4444;
    qd.dkGray.blue = 0x4444;
    qd.ltGray.red = 0xCCCC;
    qd.ltGray.green = 0xCCCC;
    qd.ltGray.blue = 0xCCCC;
    qd.gray.red = 0x8888;
    qd.gray.green = 0x8888;
    qd.gray.blue = 0x8888;

    /* Set screen bounds */
    qd.screenBits.bounds.left = 0;
    qd.screenBits.bounds.top = 0;
    qd.screenBits.bounds.right = fb_width;
    qd.screenBits.bounds.bottom = fb_height;
    qd.screenBits.rowBytes = fb_pitch;
    qd.screenBits.baseAddr = (Ptr)framebuffer;

    /* Initialize patterns */
    for (int i = 0; i < 8; i++) {
        qd.white.pat[i] = 0xFF;  /* Solid white */
        qd.black.pat[i] = 0x00;  /* Solid black */
        qd.gray.pat[i] = (i % 2) ? 0xAA : 0x55;  /* 50% gray */
        qd.ltGray.pat[i] = (i % 2) ? 0xEE : 0xBB;  /* Light gray */
        qd.dkGray.pat[i] = (i % 2) ? 0x44 : 0x11;  /* Dark gray */
    }
}

/* Set the current graphics port */
void SetPort(GrafPtr port) {
    currentPort = port;
    if (port) {
        qd.thePort = port;
    }
}

/* Get the current graphics port */
void GetPort(GrafPtr* port) {
    if (port) {
        *port = currentPort;
    }
}

/* Move the pen to a specific location */
void MoveTo(short h, short v) {
    penLocation.h = h;
    penLocation.v = v;
}

/* Move the pen relative to current position */
void Move(short dh, short dv) {
    penLocation.h += dh;
    penLocation.v += dv;
}

/* Draw a line from current pen position to a new position */
void LineTo(short h, short v) {
    if (!framebuffer) return;

    /* Simple line drawing using Bresenham's algorithm */
    int x0 = penLocation.h;
    int y0 = penLocation.v;
    int x1 = h;
    int y1 = v;

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    uint32_t color = pack_color(0, 0, 0);  /* Black for now */

    while (1) {
        /* Plot pixel */
        if (x0 >= 0 && x0 < fb_width && y0 >= 0 && y0 < fb_height) {
            uint32_t* pixel = (uint32_t*)((uint8_t*)framebuffer + y0 * fb_pitch + x0 * 4);
            *pixel = color;
        }

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }

    /* Update pen position */
    penLocation.h = h;
    penLocation.v = v;
}

/* Draw a line relative to current position */
void Line(short dh, short dv) {
    LineTo(penLocation.h + dh, penLocation.v + dv);
}

/* Set a pixel to a specific color */
void SetPixel(short h, short v, uint32_t color) {
    if (!framebuffer) return;
    if (h < 0 || h >= fb_width || v < 0 || v >= fb_height) return;

    uint32_t* pixel = (uint32_t*)((uint8_t*)framebuffer + v * fb_pitch + h * 4);
    *pixel = color;
}

/* Fill a rectangle with a solid color */
void PaintRect(const Rect* r) {
    if (!framebuffer || !r) return;

    uint32_t color = pack_color(0, 0, 0);  /* Black for now */

    /* Clip to screen bounds */
    int left = (r->left < 0) ? 0 : r->left;
    int top = (r->top < 0) ? 0 : r->top;
    int right = (r->right > fb_width) ? fb_width : r->right;
    int bottom = (r->bottom > fb_height) ? fb_height : r->bottom;

    for (int y = top; y < bottom; y++) {
        for (int x = left; x < right; x++) {
            uint32_t* pixel = (uint32_t*)((uint8_t*)framebuffer + y * fb_pitch + x * 4);
            *pixel = color;
        }
    }
}

/* Fill a rectangle with white */
void EraseRect(const Rect* r) {
    if (!framebuffer || !r) return;

    uint32_t color = pack_color(255, 255, 255);  /* White */

    /* Clip to screen bounds */
    int left = (r->left < 0) ? 0 : r->left;
    int top = (r->top < 0) ? 0 : r->top;
    int right = (r->right > fb_width) ? fb_width : r->right;
    int bottom = (r->bottom > fb_height) ? fb_height : r->bottom;

    for (int y = top; y < bottom; y++) {
        for (int x = left; x < right; x++) {
            uint32_t* pixel = (uint32_t*)((uint8_t*)framebuffer + y * fb_pitch + x * 4);
            *pixel = color;
        }
    }
}

/* Draw a rectangle outline */
void FrameRect(const Rect* r) {
    if (!r) return;

    /* Draw four lines */
    MoveTo(r->left, r->top);
    LineTo(r->right - 1, r->top);
    LineTo(r->right - 1, r->bottom - 1);
    LineTo(r->left, r->bottom - 1);
    LineTo(r->left, r->top);
}

/* Invert the pixels in a rectangle */
void InvertRect(const Rect* r) {
    if (!framebuffer || !r) return;

    /* Clip to screen bounds */
    int left = (r->left < 0) ? 0 : r->left;
    int top = (r->top < 0) ? 0 : r->top;
    int right = (r->right > fb_width) ? fb_width : r->right;
    int bottom = (r->bottom > fb_height) ? fb_height : r->bottom;

    for (int y = top; y < bottom; y++) {
        for (int x = left; x < right; x++) {
            uint32_t* pixel = (uint32_t*)((uint8_t*)framebuffer + y * fb_pitch + x * 4);
            /* Simple inversion - XOR with white */
            *pixel ^= 0x00FFFFFF;
        }
    }
}

/* Fill a rectangle with a pattern */
void FillRect(const Rect* r, const Pattern* pat) {
    if (!framebuffer || !r || !pat) return;

    /* Clip to screen bounds */
    int left = (r->left < 0) ? 0 : r->left;
    int top = (r->top < 0) ? 0 : r->top;
    int right = (r->right > fb_width) ? fb_width : r->right;
    int bottom = (r->bottom > fb_height) ? fb_height : r->bottom;

    for (int y = top; y < bottom; y++) {
        for (int x = left; x < right; x++) {
            /* Get pattern bit */
            int patY = (y - r->top) % 8;
            int patX = (x - r->left) % 8;
            int bit = (pat->pat[patY] >> (7 - patX)) & 1;

            uint32_t color = bit ? pack_color(0, 0, 0) : pack_color(255, 255, 255);
            uint32_t* pixel = (uint32_t*)((uint8_t*)framebuffer + y * fb_pitch + x * 4);
            *pixel = color;
        }
    }
}

/* Set the pen pattern */
void PenPat(const Pattern* pat) {
    if (pat) {
        for (int i = 0; i < 8; i++) {
            penPattern.pat[i] = pat->pat[i];
        }
    }
}

/* Set pen to normal (black on white) */
void PenNormal(void) {
    PenPat(&qd.black);
    penMode = patCopy;
}

/* Draw an oval */
void FrameOval(const Rect* r) {
    if (!r) return;

    /* Simple approximation - draw a rectangle for now */
    FrameRect(r);
}

/* Fill an oval */
void PaintOval(const Rect* r) {
    if (!r) return;

    /* Simple approximation - fill a rectangle for now */
    PaintRect(r);
}

/* Draw a rounded rectangle */
void FrameRoundRect(const Rect* r, short ovalWidth, short ovalHeight) {
    if (!r) return;

    /* Simple approximation - draw a rectangle for now */
    FrameRect(r);
}

/* Fill a rounded rectangle */
void PaintRoundRect(const Rect* r, short ovalWidth, short ovalHeight) {
    if (!r) return;

    /* Simple approximation - fill a rectangle for now */
    PaintRect(r);
}

/* Scroll rectangle contents */
void ScrollRect(const Rect* r, short dh, short dv, RgnHandle updateRgn) {
    /* Complex operation - stub for now */
}

/* Copy bits from one bitmap to another */
void CopyBits(const BitMap* srcBits, const BitMap* dstBits,
              const Rect* srcRect, const Rect* dstRect,
              short mode, RgnHandle maskRgn) {
    /* Complex operation - stub for now */
}

/* Set the clipping region */
void SetClip(RgnHandle rgn) {
    /* Clipping not implemented yet */
}

/* Get the clipping region */
void GetClip(RgnHandle rgn) {
    /* Clipping not implemented yet */
}

/* Offset a rectangle */
void OffsetRect(Rect* r, short dh, short dv) {
    if (r) {
        r->left += dh;
        r->right += dh;
        r->top += dv;
        r->bottom += dv;
    }
}

/* Set a rectangle */
void SetRect(Rect* r, short left, short top, short right, short bottom) {
    if (r) {
        r->left = left;
        r->top = top;
        r->right = right;
        r->bottom = bottom;
    }
}

/* Check if a point is in a rectangle */
Boolean PtInRect(Point pt, const Rect* r) {
    if (!r) return false;
    return (pt.h >= r->left && pt.h < r->right &&
            pt.v >= r->top && pt.v < r->bottom);
}

/* Check if two rectangles intersect */
Boolean SectRect(const Rect* src1, const Rect* src2, Rect* dstRect) {
    if (!src1 || !src2 || !dstRect) return false;

    dstRect->left = (src1->left > src2->left) ? src1->left : src2->left;
    dstRect->top = (src1->top > src2->top) ? src1->top : src2->top;
    dstRect->right = (src1->right < src2->right) ? src1->right : src2->right;
    dstRect->bottom = (src1->bottom < src2->bottom) ? src1->bottom : src2->bottom;

    return (dstRect->left < dstRect->right && dstRect->top < dstRect->bottom);
}

/* Create a new region */
RgnHandle NewRgn(void) {
    /* Regions not fully implemented - return dummy handle */
    static MacRegion dummyRegion;
    dummyRegion.rgnSize = sizeof(MacRegion);
    dummyRegion.rgnBBox.left = 0;
    dummyRegion.rgnBBox.top = 0;
    dummyRegion.rgnBBox.right = fb_width;
    dummyRegion.rgnBBox.bottom = fb_height;
    return (RgnHandle)&dummyRegion;
}

/* Dispose of a region */
void DisposeRgn(RgnHandle rgn) {
    /* Stub */
}

/* Set a region to a rectangle */
void RectRgn(RgnHandle rgn, const Rect* r) {
    /* Stub */
}