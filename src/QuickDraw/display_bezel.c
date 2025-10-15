#include <stdint.h>

#include "QuickDraw/DisplayBezel.h"

extern void* framebuffer;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;

void QD_DrawCRTBezel(void) {
    if (!framebuffer || fb_width == 0 || fb_height == 0 || fb_pitch == 0) {
        return;
    }

    uint32_t* fb = (uint32_t*)framebuffer;
    int pitchPixels = fb_pitch / 4;
    const uint32_t black = 0xFF000000;

    int cornerRadius = (int)(fb_height / 55);
    if (cornerRadius < 4) {
        cornerRadius = 4;
    }
    if (cornerRadius > 12) {
        cornerRadius = 12;
    }

    int radiusSquared = cornerRadius * cornerRadius;
    int topCenterY = cornerRadius - 1;
    int bottomCenterY = (int)fb_height - cornerRadius;
    int leftCenterX = cornerRadius - 1;
    int rightCenterX = (int)fb_width - cornerRadius;

    for (int y = 0; y < cornerRadius; ++y) {
        uint32_t* row = fb + y * pitchPixels;
        int dyTop = y - topCenterY;
        int yBottom = (int)fb_height - 1 - y;
        int dyBottom = yBottom - bottomCenterY;

        for (int x = 0; x < cornerRadius; ++x) {
            int dx = x - leftCenterX;
            if (dx * dx + dyTop * dyTop >= radiusSquared) {
                row[x] = black;
            }
        }
        for (int x = (int)fb_width - cornerRadius; x < (int)fb_width; ++x) {
            int dx = x - rightCenterX;
            if (dx * dx + dyTop * dyTop >= radiusSquared) {
                row[x] = black;
            }
        }

        uint32_t* bottomRow = fb + yBottom * pitchPixels;
        for (int x = 0; x < cornerRadius; ++x) {
            int dx = x - leftCenterX;
            if (dx * dx + dyBottom * dyBottom >= radiusSquared) {
                bottomRow[x] = black;
            }
        }
        for (int x = (int)fb_width - cornerRadius; x < (int)fb_width; ++x) {
            int dx = x - rightCenterX;
            if (dx * dx + dyBottom * dyBottom >= radiusSquared) {
                bottomRow[x] = black;
            }
        }
    }
}
