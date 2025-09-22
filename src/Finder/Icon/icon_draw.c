/* Icon Drawing with QuickDraw-faithful Compositing
 * Handles ICN# (1-bit) and cicn (color) icons
 */

#include "Finder/Icon/icon_types.h"
#include <stdint.h>
#include <stddef.h>

/* Import framebuffer access */
extern uint32_t* g_vram;
extern int g_screenWidth;

/* Helper: Get bit from bitmap */
static inline uint8_t GetBit(const uint8_t* row, int x) {
    return (row[x >> 3] >> (7 - (x & 7))) & 1;
}

/* Draw pixel to framebuffer */
static void SetPixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < 800 && y >= 0 && y < 600) {
        g_vram[y * g_screenWidth + x] = color;
    }
}

/* Draw 1-bit ICN# icon */
static void DrawICN32(const IconBitmap* ib, int dx, int dy) {
    uint32_t black = 0xFF000000;
    uint32_t white = 0xFFFFFFFF;

    /* First pass: Draw white fill where mask is set */
    for (int y = 0; y < 32; ++y) {
        if (dy + y >= 600) break;
        const uint8_t* mrow = ib->mask1b + y * 4;  /* 32px = 4 bytes */

        for (int x = 0; x < 32; ++x) {
            if (dx + x >= 800) break;
            if (GetBit(mrow, x)) {
                SetPixel(dx + x, dy + y, white);
            }
        }
    }

    /* Second pass: Draw black pixels where image is set */
    for (int y = 0; y < 32; ++y) {
        if (dy + y >= 600) break;
        const uint8_t* mrow = ib->mask1b + y * 4;
        const uint8_t* irow = ib->img1b + y * 4;

        for (int x = 0; x < 32; ++x) {
            if (dx + x >= 800) break;
            if (GetBit(mrow, x) && GetBit(irow, x)) {
                SetPixel(dx + x, dy + y, black);
            }
        }
    }
}

/* Draw color cicn icon */
static void DrawCICN32(const IconBitmap* ib, int dx, int dy) {
    for (int y = 0; y < 32; ++y) {
        if (dy + y >= 600) break;
        const uint32_t* src = ib->argb32 + y * 32;

        for (int x = 0; x < 32; ++x) {
            if (dx + x >= 800) break;
            uint32_t pixel = src[x];
            uint8_t alpha = (pixel >> 24) & 0xFF;

            if (alpha > 0) {
                /* Simple alpha blend if needed */
                SetPixel(dx + x, dy + y, pixel);
            }
        }
    }
}

/* Public API: Draw 32x32 icon */
void Icon_Draw32(const IconHandle* h, int x, int y) {
    if (!h || !h->fam) return;

    const IconBitmap* b = &h->fam->large;

    if (b->depth == kIconColor32 && b->argb32) {
        DrawCICN32(b, x, y);
    } else if (b->img1b && b->mask1b) {
        DrawICN32(b, x, y);
    }
}

/* Draw 16x16 icon (for list views) */
void Icon_Draw16(const IconHandle* h, int x, int y) {
    /* TODO: Implement SICN drawing when we have small icons */
    if (!h || !h->fam || !h->fam->hasSmall) return;
    /* For now, just use the large icon */
    Icon_Draw32(h, x, y);
}