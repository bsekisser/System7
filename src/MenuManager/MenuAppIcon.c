/*
 * MenuAppIcon.c - Draw Application (top-right) menu icon
 * Uses 1-bit 16x16 glyph from system7_resources.h
 */
#include "MenuManager/MenuAppIcon.h"
#include "QuickDraw/QuickDraw.h"
#include "Resources/system7_resources.h"

#define MENU_APP_ICON_WIDTH  20
#define MENU_APP_ICON_HEIGHT 18

extern UInt32 pack_color(uint8_t r, uint8_t g, uint8_t b);

/* application_icon_16 is 16x16, 1-bit, 2 bytes per row */
short MenuAppIcon_Draw(GrafPtr port, short left, short top, Boolean highlighted)
{
    if (!port || !port->portBits.baseAddr) {
        return MENU_APP_ICON_WIDTH;
    }

    const SInt16 rowBytes = port->portBits.rowBytes & 0x3FFF;
    if (rowBytes <= 0) return MENU_APP_ICON_WIDTH;

    const SInt16 width = port->portBits.bounds.right - port->portBits.bounds.left;
    const SInt16 height = port->portBits.bounds.bottom - port->portBits.bounds.top;
    if (width <= 0 || height <= 0) return MENU_APP_ICON_WIDTH;

    UInt32* pixels = (UInt32*)port->portBits.baseAddr;
    const int stride = rowBytes / 4; /* 32-bit pixels */

    const int iconLeft = left + (MENU_APP_ICON_WIDTH - 16) / 2;
    const int iconTop  = top  + (MENU_APP_ICON_HEIGHT - 16) / 2;

    const UInt32 onColor  = highlighted ? pack_color(255, 255, 255) : pack_color(0, 0, 0);

    for (int y = 0; y < 16; y++) {
        int dstY = iconTop + y;
        if (dstY < port->portBits.bounds.top || dstY >= port->portBits.bounds.bottom) continue;

        /* two bytes per row, MSB first */
        unsigned char b0 = application_icon_16[y * 2 + 0];
        unsigned char b1 = application_icon_16[y * 2 + 1];
        unsigned short rowBits = (unsigned short)((b0 << 8) | b1);

        for (int x = 0; x < 16; x++) {
            if (rowBits & (1u << (15 - x))) {
                int dstX = iconLeft + x;
                if (dstX < port->portBits.bounds.left || dstX >= port->portBits.bounds.right) continue;
                pixels[(dstY - port->portBits.bounds.top) * stride +
                       (dstX - port->portBits.bounds.left)] = onColor;
            }
        }
    }

    return MENU_APP_ICON_WIDTH;
}

