#include <stdint.h>

#include "QuickDraw/DisplayBezel.h"

extern void* framebuffer;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;

static inline double qd_abs(double value) {
    return (value < 0.0) ? -value : value;
}

static double qd_sqrt(double value) {
    if (value <= 0.0) {
        return 0.0;
    }
    double x = value;
    if (x < 1.0) {
        x = 1.0;
    }
    for (int i = 0; i < 6; ++i) {
        x = 0.5 * (x + value / x);
    }
    return x;
}

static inline double superellipse_limit(double normalizedY) {
    double nyAbs = qd_abs(normalizedY);
    double nySquared = nyAbs * nyAbs;
    double nyFourth = nySquared * nySquared;
    if (nyFourth >= 1.0) {
        return 0.0;
    }
    double base = 1.0 - nyFourth;
    double root = qd_sqrt(qd_sqrt(base));
    return root;
}

void QD_DrawCRTBezel(void) {
    if (!framebuffer || fb_width == 0 || fb_height == 0 || fb_pitch == 0) {
        return;
    }

    uint32_t* fb = (uint32_t*)framebuffer;
    int pitchPixels = fb_pitch / 4;
    const double centerX = (fb_width - 1) * 0.5;
    const double centerY = (fb_height - 1) * 0.5;
    const double radiusX = centerX;
    const double radiusY = centerY;
    const uint32_t black = 0xFF000000;

    int rimThickness = (fb_height >= 90) ? (int)(fb_height / 90) : 4;
    if (rimThickness < 4) {
        rimThickness = 4;
    }

    for (uint32_t y = 0; y < fb_height; ++y) {
        double normY = (y - centerY) / radiusY;
        double maxNormX = superellipse_limit(normY);
        int interiorHalfWidth = (int)(maxNormX * radiusX);
        int leftBound = (int)(centerX - interiorHalfWidth);
        int rightBound = (int)(centerX + interiorHalfWidth);

        if (interiorHalfWidth <= 0) {
            uint32_t* row = fb + y * pitchPixels;
            int edgeDistanceY = (y < (uint32_t)rimThickness)
                                  ? (int)y
                                  : (int)(fb_height - 1 - y);
            if (edgeDistanceY >= rimThickness) {
                continue;
            }
            for (uint32_t x = 0; x < fb_width; ++x) {
                row[x] = black;
            }
            continue;
        }

        if (leftBound < 0) {
            leftBound = 0;
        }
        if (rightBound >= (int)fb_width) {
            rightBound = (int)fb_width - 1;
        }

        uint32_t* row = fb + y * pitchPixels;

        for (int x = 0; x < leftBound; ++x) {
            int distLeft = x;
            int distRight = (int)fb_width - 1 - x;
            int distTop = (int)y;
            int distBottom = (int)fb_height - 1 - (int)y;
            int edgeDist = distLeft;
            if (distRight < edgeDist) edgeDist = distRight;
            if (distTop < edgeDist) edgeDist = distTop;
            if (distBottom < edgeDist) edgeDist = distBottom;
            if (edgeDist < rimThickness) {
                row[x] = black;
            }
        }
        for (int x = rightBound + 1; x < (int)fb_width; ++x) {
            int distLeft = x;
            int distRight = (int)fb_width - 1 - x;
            int distTop = (int)y;
            int distBottom = (int)fb_height - 1 - (int)y;
            int edgeDist = distLeft;
            if (distRight < edgeDist) edgeDist = distRight;
            if (distTop < edgeDist) edgeDist = distTop;
            if (distBottom < edgeDist) edgeDist = distBottom;
            if (edgeDist < rimThickness) {
                row[x] = black;
            }
        }

        if ((int)y < rimThickness || (int)(fb_height - 1 - y) < rimThickness) {
            for (int x = leftBound; x <= rightBound; ++x) {
                if (x < 0 || x >= (int)fb_width) {
                    continue;
                }
                int distTop = (int)y;
                int distBottom = (int)fb_height - 1 - (int)y;
                int edgeDist = (distTop < distBottom) ? distTop : distBottom;
                if (edgeDist < rimThickness) {
                    row[x] = black;
                }
            }
        }
    }
}
