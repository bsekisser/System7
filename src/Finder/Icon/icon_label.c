/* Shared Label Renderer
 * Reuses perfected text rendering from HD icon
 */

#include "Finder/Icon/icon_label.h"
#include "Finder/Icon/icon_types.h"
#include <string.h>
#include <stdint.h>

/* Import Chicago font - using the chicago_font_data structures */
extern const uint8_t chicago_bitmap[];
extern const uint8_t chicago_widths[];
#include "chicago_font.h"  /* For ChicagoCharInfo */

/* Import framebuffer - using proper names */
extern void* framebuffer;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;
extern uint32_t pack_color(uint8_t r, uint8_t g, uint8_t b);

/* Draw rectangle */
static void FillRect(int left, int top, int right, int bottom, uint32_t color) {
    uint32_t* fb = (uint32_t*)framebuffer;
    for (int y = top; y < bottom && y < fb_height; y++) {
        for (int x = left; x < right && x < fb_width; x++) {
            if (x >= 0 && y >= 0) {
                fb[y * (fb_pitch/4) + x] = color;
            }
        }
    }
}

/* Draw character using direct bitmap rendering (perfected from HD icon) */
static void DrawChar(char ch, int x, int y, uint32_t color) {
    if (ch < 32 || ch > 126) return;

    uint32_t* fb = (uint32_t*)framebuffer;
    ChicagoCharInfo info = chicago_ascii[ch - 32];

    for (int row = 0; row < 16; row++) {  /* Chicago font height */
        const uint8_t* strike_row = chicago_bitmap + (row * 140);  /* 140 bytes per row */

        for (int col = 0; col < info.bit_width; col++) {
            int bit_position = info.bit_start + col;
            int byte_index = bit_position >> 3;
            int bit_offset = 7 - (bit_position & 7);

            if (strike_row[byte_index] & (1 << bit_offset)) {
                if (x + col < fb_width && y + row < fb_height) {
                    fb[(y + row) * (fb_pitch/4) + (x + col)] = color;
                }
            }
        }
    }
}

/* Measure text using exact character bit widths */
void IconLabel_Measure(const char* name, int* outWidth, int* outHeight) {
    int width = 0;
    int len = strlen(name);

    for (int i = 0; i < len; i++) {
        char ch = name[i];
        if (ch >= 32 && ch <= 126) {
            ChicagoCharInfo info = chicago_ascii[ch - 32];
            width += info.bit_width + 1;  /* bit width + 1 for spacing */
            if (ch == ' ') width += 3;  /* Extra space width (perfected value) */
        }
    }

    *outWidth = width;
    *outHeight = 16;  /* Chicago font height */
}

/* Draw label with white background */
void IconLabel_Draw(const char* name, int cx, int topY, bool selected) {
    int textWidth, textHeight;
    IconLabel_Measure(name, &textWidth, &textHeight);

    int textX = cx - (textWidth / 2);
    int padding = 2;

    /* Draw white background rectangle behind text */
    uint32_t bgColor = selected ? 0xFF000080 : 0xFFFFFFFF;  /* Blue if selected, white otherwise */
    uint32_t fgColor = selected ? 0xFFFFFFFF : 0xFF000000;  /* White text if selected, black otherwise */

    /* Adjusted background rectangle (perfected from HD icon) */
    FillRect(textX - padding, topY - textHeight + 2,
             textX + textWidth - 2, topY + 3, bgColor);  /* Reduced right padding by 4 pixels */

    /* Draw text using direct bitmap rendering */
    int currentX = textX;
    int len = strlen(name);

    for (int i = 0; i < len; i++) {
        char ch = name[i];
        if (ch >= 32 && ch <= 126) {
            ChicagoCharInfo info = chicago_ascii[ch - 32];
            DrawChar(ch, currentX, topY - textHeight + 3, fgColor);
            currentX += info.bit_width + 1;
            if (ch == ' ') currentX += 3;  /* Extra space between words */
        }
    }
}

/* Draw icon with label - main entry point for icon+label rendering */
IconRect Icon_DrawWithLabel(const IconHandle* h, const char* name,
                            int centerX, int iconTopY, bool selected) {
    /* Draw icon centered at centerX */
    int iconLeft = centerX - 16;  /* 32x32 icon */
    Icon_Draw32(h, iconLeft, iconTopY);

    /* Draw label below icon (using perfected positioning from HD icon) */
    int labelTop = iconTopY + 25;  /* Final positioning from HD icon */
    IconLabel_Draw(name, centerX, labelTop, selected);

    /* Return combined bounds for hit testing */
    int textWidth, textHeight;
    IconLabel_Measure(name, &textWidth, &textHeight);

    IconRect bounds;
    bounds.left = iconLeft;
    bounds.top = iconTopY;
    bounds.right = iconLeft + 32;
    bounds.bottom = labelTop + 3;  /* Include label area */

    /* Expand to include label width */
    int labelLeft = centerX - (textWidth / 2) - 2;
    int labelRight = centerX + (textWidth / 2) + 2;
    if (labelLeft < bounds.left) bounds.left = labelLeft;
    if (labelRight > bounds.right) bounds.right = labelRight;

    return bounds;
}