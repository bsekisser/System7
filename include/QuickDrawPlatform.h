#ifndef QUICKDRAW_PLATFORM_H
#define QUICKDRAW_PLATFORM_H

/* QuickDraw Platform Abstraction Layer */

#include "SystemTypes.h"

/* Platform-specific includes */
#ifdef __unix__
    #include <stdint.h>
    #include <stddef.h>
#endif

/* Platform-specific graphics context */
typedef struct PlatformGraphicsContext {
    void* context;      /* Platform-specific context */
    int width;
    int height;
    int depth;
    void* buffer;
} PlatformGraphicsContext;

/* Platform graphics functions */
void PlatformInitGraphics(void);
void PlatformShutdownGraphics(void);
void PlatformDrawPixel(int x, int y, UInt32 color);
void PlatformDrawLine(int x1, int y1, int x2, int y2, UInt32 color);
void PlatformFillRect(int x, int y, int width, int height, UInt32 color);
void PlatformBitBlt(void* src, int srcX, int srcY, void* dst, int dstX, int dstY, int width, int height);
void PlatformFlush(void);

#endif /* QUICKDRAW_PLATFORM_H */