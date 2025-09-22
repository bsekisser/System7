/* Shared Label Renderer
 * Reuses perfected text rendering from HD icon
 */

#include "Finder/Icon/icon_label.h"
#include "Finder/Icon/icon_types.h"
#include <string.h>
#include <stdint.h>

/* Import Chicago font - using the chicago_font_data structures */
extern const uint8_t chicago_bitmap[];

/* Chicago font structures */
typedef struct {
    uint16_t bit_start;   /* Start bit offset in strike */
    uint16_t bit_width;   /* Width in bits (ink width) */
    int8_t left_offset;   /* Left side bearing */
    int8_t advance;       /* Logical advance width */
} ChicagoCharInfo;

/* Character info for ASCII printable (0x20 to 0x7E) */
static const ChicagoCharInfo chicago_ascii[] = {
    {   43,  0,   0,   4 },  /* ' ' */
    {   43,  2,   0,   3 },  /* '!' */
    {   45,  3,   0,   4 },  /* '"' */
    {   48,  8,   0,   9 },  /* '#' */
    {   56,  5,   0,   6 },  /* '$' */
    {   61,  9,   0,  10 },  /* '%' */
    {   70,  8,   0,   9 },  /* '&' */
    {   78,  1,   0,   2 },  /* '\'' */
    {   79,  3,   4,   4 },  /* '(' */
    {   82,  3,   2,   6 },  /* ')' */
    {   85,  5,   2,   7 },  /* '*' */
    {   90,  5,   1,  10 },  /* '+' */
    {   95,  2,   1,   7 },  /* ',' */
    {   97,  5,   1,  11 },  /* '-' */
    {  102,  2,   1,  10 },  /* '.' */
    {  104,  5,   1,   3 },  /* '/' */
    {  109,  6,   1,   5 },  /* '0' */
    {  115,  3,   1,   5 },  /* '1' */
    {  118,  6,   1,   7 },  /* '2' */
    {  124,  6,   1,   7 },  /* '3' */
    {  130,  7,   1,   4 },  /* '4' */
    {  137,  6,   1,   7 },  /* '5' */
    {  143,  6,   1,   4 },  /* '6' */
    {  149,  6,   1,   7 },  /* '7' */
    {  155,  6,   1,   8 },  /* '8' */
    {  161,  6,   2,   8 },  /* '9' */
    {  167,  2,   1,   8 },  /* ':' */
    {  169,  2,   1,   8 },  /* ';' */
    {  171,  5,   1,   8 },  /* '<' */
    {  176,  6,   1,   8 },  /* '=' */
    {  182,  5,   1,   8 },  /* '>' */
    {  187,  6,   1,   8 },  /* '?' */
    {  193,  9,   1,   8 },  /* '@' */
    {  202,  6,   1,   8 },  /* 'A' */
    {  208,  6,   1,   4 },  /* 'B' */
    {  214,  6,   1,   4 },  /* 'C' */
    {  220,  6,   0,   6 },  /* 'D' */
    {  226,  5,   1,   8 },  /* 'E' */
    {  231,  5,   0,   6 },  /* 'F' */
    {  236,  6,   1,   8 },  /* 'G' */
    {  242,  6,   1,  11 },  /* 'H' */
    {  248,  2,   1,   8 },  /* 'I' */
    {  250,  6,   1,   8 },  /* 'J' */
    {  256,  7,   1,   8 },  /* 'K' */
    {  263,  5,   1,   8 },  /* 'L' */
    {  268, 10,   1,   7 },  /* 'M' */
    {  278,  7,   1,   7 },  /* 'N' */
    {  285,  6,   1,   8 },  /* 'O' */
    {  291,  6,   1,   8 },  /* 'P' */
    {  297,  6,   2,   6 },  /* 'Q' */
    {  303,  6,   0,   7 },  /* 'R' */
    {  309,  5,   1,   9 },  /* 'S' */
    {  314,  6,   1,   7 },  /* 'T' */
    {  320,  6,   1,  12 },  /* 'U' */
    {  326,  6,   1,   9 },  /* 'V' */
    {  332, 10,   1,   8 },  /* 'W' */
    {  342,  6,   1,   8 },  /* 'X' */
    {  348,  6,   1,   8 },  /* 'Y' */
    {  354,  6,   1,   8 },  /* 'Z' */
    {  360,  3,   1,   7 },  /* '[' */
    {  363,  5,   0,   6 },  /* '\\' */
    {  368,  3,   1,   8 },  /* ']' */
    {  371,  5,   1,   8 },  /* '^' */
    {  376,  8,   1,  12 },  /* '_' */
    {  384,  3,   1,   8 },  /* '`' */
    {  387,  6,   1,   8 },  /* 'a' */
    {  393,  6,   1,   8 },  /* 'b' */
    {  399,  5,   1,   5 },  /* 'c' */
    {  404,  6,   1,   7 },  /* 'd' */
    {  410,  6,   1,   5 },  /* 'e' */
    {  416,  5,   2,   8 },  /* 'f' */
    {  421,  6,   0,   8 },  /* 'g' */
    {  427,  6,   1,   6 },  /* 'h' */
    {  433,  2,   1,   8 },  /* 'i' */
    {  435,  5,   1,   8 },  /* 'j' */
    {  440,  6,   1,   7 },  /* 'k' */
    {  446,  2,   1,   8 },  /* 'l' */
    {  448, 10,   1,   8 },  /* 'm' */
    {  458,  6,   1,   6 },  /* 'n' */
    {  464,  6,   1,   8 },  /* 'o' */
    {  470,  6,   1,   8 },  /* 'p' */
    {  476,  6,   1,   4 },  /* 'q' */
    {  482,  5,   0,   6 },  /* 'r' */
    {  487,  5,   1,   8 },  /* 's' */
    {  492,  4,   1,   4 },  /* 't' */
    {  496,  6,   1,  12 },  /* 'u' */
    {  502,  6,   1,   8 },  /* 'v' */
    {  508, 10,   1,   8 },  /* 'w' */
    {  518,  6,   1,   8 },  /* 'x' */
    {  524,  6,   1,   8 },  /* 'y' */
    {  530,  6,   1,   6 },  /* 'z' */
    {  536,  3,   1,   7 },  /* '{' */
    {  539,  1,   1,   6 },  /* '|' */
    {  540,  3,   1,   8 },  /* '}' */
    {  543,  6,   1,   8 },  /* '~' */
};

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

    for (int row = 0; row < 15; row++) {  /* Chicago font actual height is 15 */
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
    *outHeight = 15;  /* Chicago font actual height */
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
             textX + textWidth + 1, topY + 3, bgColor);  /* Extended right by 1 pixel */

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
    int labelTop = iconTopY + 27;  /* Moved down 2 pixels to hide bottom text */
    IconLabel_Draw(name, centerX, labelTop, selected);

    /* Return combined bounds for hit testing */
    int textWidth, textHeight;
    IconLabel_Measure(name, &textWidth, &textHeight);

    IconRect bounds;
    bounds.left = iconLeft;
    bounds.top = iconTopY;
    bounds.right = iconLeft + 32;
    bounds.bottom = labelTop + 5;  /* Include label area with adjusted position */

    /* Expand to include label width */
    int labelLeft = centerX - (textWidth / 2) - 2;
    int labelRight = centerX + (textWidth / 2) + 2;
    if (labelLeft < bounds.left) bounds.left = labelLeft;
    if (labelRight > bounds.right) bounds.right = labelRight;

    return bounds;
}