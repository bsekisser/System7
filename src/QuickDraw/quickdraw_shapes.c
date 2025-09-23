/* #include "SystemTypes.h" */
/*
 * RE-AGENT-BANNER
 * QuickDraw Shape Functions
 * Reimplemented from Apple System 7.1 QuickDraw Core
 *
 * Original binary: System.rsrc (ROM disassembly)
 * Architecture: 68k Mac OS System 7
 * Evidence sources: trap analysis, Mac OS Toolbox reference
 *
 * This file implements QuickDraw rectangle and oval drawing functions.
 */

// #include "CompatibilityFix.h" // Removed
#include "QuickDraw/QuickDraw.h"
#include "QuickDraw/quickdraw_types.h"
#include "SystemTypes.h"
#include "QuickDrawConstants.h"

/* Global current port - from QuickDrawCore.c */
extern GrafPtr g_currentPort;
#define thePort g_currentPort

/*
 * FrameRect - Draw rectangle outline
 * Trap: 0xA8A1

 */
void FrameRect(const Rect* r)
{
    if (thePort == NULL || r == NULL) return;

    extern void serial_printf(const char* fmt, ...);
    serial_printf("FrameRect: Drawing rect (%d,%d,%d,%d)\n",
                  r->left, r->top, r->right, r->bottom);

    /* Get framebuffer info */
    extern void* framebuffer;
    extern uint32_t fb_width, fb_height, fb_pitch;

    if (!framebuffer) {
        serial_printf("FrameRect: No framebuffer available\n");
        return;
    }

    uint32_t* fb = (uint32_t*)framebuffer;
    int pitch = fb_pitch / 4;

    /* Clip to screen bounds */
    short left = r->left;
    short top = r->top;
    short right = r->right;
    short bottom = r->bottom;

    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (right > (short)fb_width) right = fb_width;
    if (bottom > (short)fb_height) bottom = fb_height;
    if (left >= right || top >= bottom) return;

    /* Draw rectangle frame - use black for now */
    uint32_t color = 0xFF000000;  /* Black */

    /* Top line */
    for (int x = left; x < right; x++) {
        fb[top * pitch + x] = color;
    }

    /* Bottom line */
    for (int x = left; x < right; x++) {
        fb[(bottom - 1) * pitch + x] = color;
    }

    /* Left line */
    for (int y = top; y < bottom; y++) {
        fb[y * pitch + left] = color;
    }

    /* Right line */
    for (int y = top; y < bottom; y++) {
        fb[y * pitch + (right - 1)] = color;
    }
}

/*
 * PaintRect - Fill rectangle with current pattern
 * Trap: 0xA8A2

 */
void PaintRect(const Rect* r)
{
    if (thePort == NULL || r == NULL) return;

    /* Fill rectangle with current pen pattern */
    FillRect(r, &thePort->pnPat);
}

/*
 * FillRect - Fill rectangle with specified pattern
 * Trap: 0xA8A3

 */
void FillRect(const Rect* r, const Pattern* pat)
{
    if (thePort == NULL || r == NULL || pat == NULL) return;

    extern void serial_printf(const char* fmt, ...);
    serial_printf("FillRect: Filling rect (%d,%d,%d,%d)\n",
                  r->left, r->top, r->right, r->bottom);

    /* Get framebuffer info */
    extern void* framebuffer;
    extern uint32_t fb_width, fb_height, fb_pitch;

    if (!framebuffer) {
        serial_printf("FillRect: No framebuffer available\n");
        return;
    }

    uint32_t* fb = (uint32_t*)framebuffer;
    int pitch = fb_pitch / 4;

    /* Clip to screen bounds */
    short left = r->left;
    short top = r->top;
    short right = r->right;
    short bottom = r->bottom;

    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (right > (short)fb_width) right = fb_width;
    if (bottom > (short)fb_height) bottom = fb_height;
    if (left >= right || top >= bottom) return;

    /* Fill rectangle with pattern */
    for (int y = top; y < bottom; y++) {
        for (int x = left; x < right; x++) {
            /* Apply 8x8 pattern */
            int patY = y & 7;  /* y mod 8 */
            int patX = x & 7;  /* x mod 8 */
            unsigned char patByte = pat->pat[patY];

            /* Check if pattern bit is set */
            if (patByte & (0x80 >> patX)) {
                fb[y * pitch + x] = 0xFF000000;  /* Black */
            } else {
                fb[y * pitch + x] = 0xFFFFFFFF;  /* White */
            }
        }
    }
}

/*
 * InvertRect - Invert rectangle contents
 * Trap: 0xA8A4

 */
void InvertRect(const Rect* r)
{
    if (thePort == NULL || r == NULL) return;

    extern void serial_printf(const char* fmt, ...);
    serial_printf("InvertRect: Inverting rect (%d,%d,%d,%d)\n",
                  r->left, r->top, r->right, r->bottom);

    /* Get framebuffer info */
    extern void* framebuffer;
    extern uint32_t fb_width, fb_height, fb_pitch;

    if (!framebuffer) {
        serial_printf("InvertRect: No framebuffer available\n");
        return;
    }

    uint32_t* fb = (uint32_t*)framebuffer;
    int pitch = fb_pitch / 4;

    /* Clip to screen bounds */
    short left = r->left;
    short top = r->top;
    short right = r->right;
    short bottom = r->bottom;

    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (right > (short)fb_width) right = fb_width;
    if (bottom > (short)fb_height) bottom = fb_height;
    if (left >= right || top >= bottom) return;

    /* Invert each pixel in rectangle */
    for (int y = top; y < bottom; y++) {
        for (int x = left; x < right; x++) {
            uint32_t pixel = fb[y * pitch + x];
            /* XOR with white to invert */
            fb[y * pitch + x] = pixel ^ 0xFFFFFFFF;
        }
    }
}

/*
 * FrameOval - Draw oval outline
 * Trap: 0xA8B7

 */
void FrameOval(const Rect* r)
{
    if (thePort == NULL || r == NULL) return;

    extern void serial_printf(const char* fmt, ...);
    serial_printf("FrameOval: Drawing oval in rect (%d,%d,%d,%d)\n",
                  r->left, r->top, r->right, r->bottom);

    /* Get framebuffer info */
    extern void* framebuffer;
    extern uint32_t fb_width, fb_height, fb_pitch;

    if (!framebuffer) {
        serial_printf("FrameOval: No framebuffer available\n");
        return;
    }

    /* For now, just draw the bounding rectangle */
    /* Full ellipse algorithm would use Bresenham's method */
    FrameRect(r);

    /* Mark center to show it's an oval placeholder */
    uint32_t* fb = (uint32_t*)framebuffer;
    int pitch = fb_pitch / 4;

    short centerX = (r->left + r->right) / 2;
    short centerY = (r->top + r->bottom) / 2;

    if (centerX >= 0 && centerX < (short)fb_width &&
        centerY >= 0 && centerY < (short)fb_height) {
        fb[centerY * pitch + centerX] = 0xFF000000;
    }
}

/*
 * PaintOval - Fill oval with current pattern
 * Trap: 0xA8B8

 */
void PaintOval(const Rect* r)
{
    if (thePort == NULL || r == NULL) return;

    /* Fill oval with current pen pattern */
    FillOval(r, &thePort->pnPat);
}

/*
 * FillOval - Fill oval with specified pattern
 * Trap: 0xA8B9

 */
void FillOval(const Rect* r, const Pattern* pat)
{
    if (thePort == NULL || r == NULL || pat == NULL) return;

    extern void serial_printf(const char* fmt, ...);
    serial_printf("FillOval: Filling oval in rect (%d,%d,%d,%d)\n",
                  r->left, r->top, r->right, r->bottom);

    /* For now, just fill the bounding rectangle */
    /* Full implementation would check if each point is inside ellipse */
    FillRect(r, pat);
}

/*
 * InvertOval - Invert oval contents
 * Trap: 0xA8BA

 */
void InvertOval(const Rect* r)
{
    if (thePort == NULL || r == NULL) return;

    extern void serial_printf(const char* fmt, ...);
    serial_printf("InvertOval: Inverting oval in rect (%d,%d,%d,%d)\n",
                  r->left, r->top, r->right, r->bottom);

    /* For now, just invert the bounding rectangle */
    /* Full implementation would check if each point is inside ellipse */
    InvertRect(r);
}

/* Rectangle utility functions */

/*
 * SetRect - Set rectangle coordinates

 */
void SetRect(Rect* r, short left, short top, short right, short bottom)
{
    if (r == NULL) return;

    r->left = left;
    r->top = top;
    r->right = right;
    r->bottom = bottom;
}

/*
 * InsetRect - Inset rectangle by specified amounts

 */
void InsetRect(Rect* r, short dh, short dv)
{
    if (r == NULL) return;

    r->left += dh;
    r->top += dv;
    r->right -= dh;
    r->bottom -= dv;
}

/*
 * OffsetRect - Offset rectangle by specified amounts

 */
void OffsetRect(Rect* r, short dh, short dv)
{
    if (r == NULL) return;

    r->left += dh;
    r->top += dv;
    r->right += dh;
    r->bottom += dv;
}

/*
 * SectRect - Calculate intersection of two rectangles

 */
Boolean SectRect(const Rect* src1, const Rect* src2, Rect* dst)
{
    if (src1 == NULL || src2 == NULL || dst == NULL) return false;

    short left = (src1->left > src2->left) ? src1->left : src2->left;
    short top = (src1->top > src2->top) ? src1->top : src2->top;
    short right = (src1->right < src2->right) ? src1->right : src2->right;
    short bottom = (src1->bottom < src2->bottom) ? src1->bottom : src2->bottom;

    if (left < right && top < bottom) {
        SetRect(dst, left, top, right, bottom);
        return true;
    } else {
        SetRect(dst, 0, 0, 0, 0);
        return false;
    }
}

/*
 * UnionRect - Calculate union of two rectangles

 */
void UnionRect(const Rect* src1, const Rect* src2, Rect* dst)
{
    if (src1 == NULL || src2 == NULL || dst == NULL) return;

    short left = (src1->left < src2->left) ? src1->left : src2->left;
    short top = (src1->top < src2->top) ? src1->top : src2->top;
    short right = (src1->right > src2->right) ? src1->right : src2->right;
    short bottom = (src1->bottom > src2->bottom) ? src1->bottom : src2->bottom;

    SetRect(dst, left, top, right, bottom);
}

/*
 * EmptyRect - Test if rectangle is empty

 */
Boolean EmptyRect(const Rect* r)
{
    if (r == NULL) return true;

    return (r->left >= r->right) || (r->top >= r->bottom);
}

/*
 * EqualRect - Test if two rectangles are equal

 */
Boolean EqualRect(const Rect* rect1, const Rect* rect2)
{
    if (rect1 == NULL || rect2 == NULL) return false;

    return (rect1->left == rect2->left) &&
           (rect1->top == rect2->top) &&
           (rect1->right == rect2->right) &&
           (rect1->bottom == rect2->bottom);
}

/*
 * PtInRect - Test if point is inside rectangle

 */
Boolean PtInRect(Point pt, const Rect* r)
{
    if (r == NULL) return false;

    return (pt.h >= r->left) && (pt.h < r->right) &&
           (pt.v >= r->top) && (pt.v < r->bottom);
}

/*
