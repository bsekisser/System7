/*
 * QuickDrawPlatform.h - Platform abstraction layer for QuickDraw
 *
 * This header provides platform-specific definitions and functions
 * needed by the QuickDraw implementation.
 */

#ifndef __QUICKDRAW_PLATFORM_H__
#define __QUICKDRAW_PLATFORM_H__

#include "SystemTypes.h"
#include "QuickDraw/QuickDraw.h"  /* For GrafPtr, Point, Pattern, etc. */

#ifdef __cplusplus
extern "C" {
#endif

/* Platform-specific pixel format definitions */

/* Platform framebuffer structure */
typedef struct PlatformFramebuffer {
    void* baseAddr;
    UInt32 width;
    UInt32 height;
    UInt32 pitch;
} PlatformFramebuffer;

/* Platform graphics context */

/* Platform initialization */
Boolean QDPlatform_Initialize(void);
void QDPlatform_Shutdown(void);

/* Framebuffer access */
PlatformFramebuffer* QDPlatform_GetFramebuffer(void);
void QDPlatform_LockFramebuffer(PlatformFramebuffer* fb);
void QDPlatform_UnlockFramebuffer(PlatformFramebuffer* fb);

/* Screen update */
void QDPlatform_UpdateScreen(SInt32 left, SInt32 top, SInt32 right, SInt32 bottom);
void QDPlatform_FlushScreen(void);

/* Pixel operations */
void QDPlatform_SetPixel(SInt32 x, SInt32 y, UInt32 color);
UInt32 QDPlatform_GetPixel(SInt32 x, SInt32 y);

/* Line drawing acceleration (optional) */
Boolean QDPlatform_DrawLineAccelerated(SInt32 x1, SInt32 y1, SInt32 x2, SInt32 y2, UInt32 color);

/* Rectangle operations (optional) */
Boolean QDPlatform_FillRectAccelerated(SInt32 left, SInt32 top, SInt32 right, SInt32 bottom, UInt32 color);

/* Blitting operations (optional) */
Boolean QDPlatform_BlitAccelerated(void* src, SInt32 srcX, SInt32 srcY,
                                void* dst, SInt32 dstX, SInt32 dstY,
                                SInt32 width, SInt32 height);

/* Color conversion */
UInt32 QDPlatform_RGBToPixel(UInt8 red, UInt8 green, UInt8 blue);
UInt32 QDPlatform_RGBToNative(UInt16 red, UInt16 green, UInt16 blue);
void QDPlatform_NativeToRGB(UInt32 native, UInt16* red, UInt16* green, UInt16* blue);

/* Drawing primitives - signatures matching QuickDrawCore calls */
void QDPlatform_DrawLine(GrafPtr port, Point startPt, Point endPt,
                        const Pattern* pat, SInt16 mode);
void QDPlatform_DrawShape(GrafPtr port, GrafVerb verb, const Rect* rect,
                         SInt16 shapeType, const Pattern* pat,
                         SInt16 ovalWidth, SInt16 ovalHeight);
void QDPlatform_DrawRegion(RgnHandle rgn, short mode, const Pattern* pat);

#ifdef __cplusplus
}
#endif

#endif /* __QUICKDRAW_PLATFORM_H__ */
