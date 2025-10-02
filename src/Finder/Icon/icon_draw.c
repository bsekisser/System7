/* Icon Drawing with QuickDraw-faithful Compositing
 * Handles ICN# (1-bit) and cicn (color) icons
 */

#include "Finder/Icon/icon_types.h"
#include <stdint.h>
#include <stddef.h>

/* Import framebuffer access */
extern void* framebuffer;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;
extern uint32_t pack_color(uint8_t r, uint8_t g, uint8_t b);

/* Helper: Get bit from bitmap */
static inline uint8_t GetBit(const uint8_t* row, int x) {
    return (row[x >> 3] >> (7 - (x & 7))) & 1;
}

/* Draw pixel to framebuffer */
static void SetPixel(int x, int y, uint32_t color) {
    uint32_t* fb = (uint32_t*)framebuffer;
    if (x >= 0 && x < fb_width && y >= 0 && y < fb_height) {
        fb[y * (fb_pitch/4) + x] = color;
    }
}

/* Draw 1-bit ICN# icon
 *
 * Classic Mac icons use a two-layer system:
 * 1. Mask layer: Defines which pixels are part of the icon (1 = opaque, 0 = transparent)
 * 2. Image layer: Defines the color of opaque pixels (1 = black, 0 = white)
 *
 * This allows icons to have white bodies with black outlines and details.
 * The mask creates the shape, while the image adds the black pixels for details.
 *
 * Example: A trash can icon would have:
 * - Mask: All 1s for the entire trash can shape
 * - Image: 1s only for the outline and vertical stripes, 0s for the white body
 */
static void DrawICN32(const IconBitmap* ib, int dx, int dy) {
    extern void serial_printf(const char* fmt, ...);
    uint32_t black = 0xFF000000;
    uint32_t white = 0xFFFFFFFF;

    serial_printf("[ICON_DRAW] DrawICN32 at (%d,%d)\n", dx, dy);

    /* Draw the icon using proper Mac mask/image compositing:
     * - Mask defines the shape (1 = opaque, 0 = transparent)
     * - Image defines the colors (1 = black, 0 = white)
     */
    for (int y = 0; y < 32; ++y) {
        if (dy + y >= 600) break;
        const uint8_t* mrow = ib->mask1b + y * 4;  /* 32px = 4 bytes */
        const uint8_t* irow = ib->img1b + y * 4;

        for (int x = 0; x < 32; ++x) {
            if (dx + x >= 800) break;
            /* Only draw where mask bit is set (opaque area) */
            if (GetBit(mrow, x)) {
                /* If image bit is set, draw black; otherwise draw white */
                if (GetBit(irow, x)) {
                    SetPixel(dx + x, dy + y, black);
                } else {
                    SetPixel(dx + x, dy + y, white);
                }
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