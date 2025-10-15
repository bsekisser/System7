#include <math.h>
#include <stdint.h>

#include "QuickDraw/DisplayBezel.h"

extern void* framebuffer;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;

static inline double superellipse_limit(double normalizedY, double exponent) {
    double nyExp = pow(fabs(normalizedY), exponent);
    if (nyExp >= 1.0) {
        return 0.0;
    }
    return pow(1.0 - nyExp, 1.0 / exponent);
}

void QD_DrawCRTBezel(void) {
    if (!framebuffer || fb_width == 0 || fb_height == 0 || fb_pitch == 0) {
        return;
    }

    uint32_t* fb = (uint32_t*)framebuffer;
    int pitchPixels = fb_pitch / 4;
    const double exponent = 4.0;  /* Superellipse exponent to approximate Macintosh CRT */
    const double centerX = (fb_width - 1) * 0.5;
    const double centerY = (fb_height - 1) * 0.5;
    const double radiusX = centerX;
    const double radiusY = centerY;
    const uint32_t black = 0xFF000000;

    for (uint32_t y = 0; y < fb_height; ++y) {
        double normY = (y - centerY) / radiusY;
        double maxNormX = superellipse_limit(normY, exponent);
        int interiorHalfWidth = (int)(maxNormX * radiusX);
        int leftBound = (int)(centerX - interiorHalfWidth);
        int rightBound = (int)(centerX + interiorHalfWidth);

        if (interiorHalfWidth <= 0) {
            /* Entire row outside superellipse, paint whole row black */
            for (uint32_t x = 0; x < fb_width; ++x) {
                fb[y * pitchPixels + x] = black;
            }
            continue;
        }

        if (leftBound < 0) leftBound = 0;
        if (rightBound >= (int)fb_width) rightBound = fb_width - 1;

        uint32_t* row = fb + y * pitchPixels;

        for (int x = 0; x < leftBound; ++x) {
            row[x] = black;
        }
        for (int x = rightBound + 1; x < (int)fb_width; ++x) {
            row[x] = black;
        }
    }
}
