/*
 * MenuTrack.c - Basic dropdown rendering and tracking
 *
 * Draws menu item list under the title and lets user select with mouse.
 */

#include "SystemTypes.h"
#include "MenuManager/MenuManager.h"
#include "MenuManager/MenuTypes.h"
#include "QuickDraw/QuickDraw.h"
#include "EventManager/EventTypes.h"  /* For mUpMask */

/* External functions */
extern int serial_printf(const char* format, ...);
extern void serial_puts(const char* str);
extern QDGlobals qd;
extern void DrawDesktop(void);
extern void DrawText(const void* textBuf, short firstByte, short byteCount);
extern void DrawVolumeIcon(void);
extern Boolean Button(void);  /* Check if mouse button is pressed */
extern void PollPS2Input(void);  /* Poll PS/2 devices for input */

/* Global framebuffer from main.c */
extern void* framebuffer;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;

/* Get title tracking data */
extern Boolean GetMenuTitleRectByID(short menuID, Rect* outRect);
extern void SetRect(Rect* rect, short left, short top, short right, short bottom);
extern void InvalRect(const Rect* rect);

/* Simple rectangle fill helper - renamed to avoid conflict */
static void DrawMenuRect(short left, short top, short right, short bottom, uint32_t color) {
    if (!framebuffer) {
        return;
    }

    uint32_t *fb = (uint32_t*)framebuffer;
    int pitch = fb_pitch / 4;  /* Convert from bytes to dwords */

    /* Clip to screen bounds */
    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (right > (int)fb_width) right = fb_width;
    if (bottom > (int)fb_height) bottom = fb_height;

    /* Additional safety checks */
    if (left >= right || top >= bottom) {
        return;
    }

    for (int y = top; y < bottom; y++) {
        for (int x = left; x < right; x++) {
            fb[y * pitch + x] = color;
        }
    }
}

/* Draw text at current pen position */
static void DrawMenuItemText(const char* text, short x, short y) {
    /* Move to position and draw text using QuickDraw */
    MoveTo(x, y);

    /* Calculate text length */
    short len = 0;
    while (text[len] != 0) len++;

    /* Draw the text using the real Chicago font */
    if (len > 0) {
        DrawText(text, 0, len);
    }

    serial_printf("Drawing menu item: %s at (%d,%d)\n", text, x, y);
}

/* Count menu items */
static short CountMenuItems(MenuHandle menu) {
    if (!menu || !*menu) return 0;
    MenuInfo* menuInfo = *menu;

    /* Count items in menuData - simplified for now */
    return 5; /* Hardcoded for testing */
}

/* Simple strcpy implementation to avoid crashes */
static void simple_strcpy(char* dst, const char* src) {
    if (!dst || !src) return;
    while (*src) {
        *dst++ = *src++;
    }
    *dst = 0;
}

/* Get menu item text - renamed to avoid conflict */
static void GetItemText(MenuHandle menu, short index, char* text) {
    if (!text) {
        return;
    }

    text[0] = 0;  /* Initialize to empty */

    if (!menu || !*menu) {
        return;
    }

    /* Simplified - return test strings */
    switch(index) {
        case 1: simple_strcpy(text, "New"); break;
        case 2: simple_strcpy(text, "Open..."); break;
        case 3: simple_strcpy(text, "Close"); break;
        case 4: simple_strcpy(text, "Save"); break;
        case 5: simple_strcpy(text, "Quit"); break;
        default: text[0] = 0;
    }
}

/* Draw menu items as a simple vertical list */
static void DrawMenuItems(MenuHandle menu, short left, short top) {
    if (!menu) return;

    short itemCount = CountMenuItems(menu);
    short lineHeight = 16;
    short menuWidth = 120;

    /* Draw menu background */
    DrawMenuRect(left, top, left + menuWidth, top + itemCount * lineHeight + 4, 0xFFFFFFFF);

    /* Draw border */
    DrawMenuRect(left, top, left + menuWidth, top + 1, 0xFF000000);
    DrawMenuRect(left, top + itemCount * lineHeight + 3, left + menuWidth, top + itemCount * lineHeight + 4, 0xFF000000);
    DrawMenuRect(left, top, left + 1, top + itemCount * lineHeight + 4, 0xFF000000);
    DrawMenuRect(left + menuWidth - 1, top, left + menuWidth, top + itemCount * lineHeight + 4, 0xFF000000);

    for (short i = 1; i <= itemCount; i++) {
        char itemText[256];
        GetItemText(menu, i, itemText);

        short itemTop = top + 2 + (i - 1) * lineHeight;

        /* Draw text */
        DrawMenuItemText(itemText, left + 4, itemTop + 12);
    }
}

/* Track menu: draw dropdown and wait for selection */
short TrackMenu(short menuID, Point startPt) {
    serial_printf("TrackMenu: Starting for menu %d\n", menuID);

    MenuHandle menu = GetMenuHandle(menuID);

    if (!menu) {
        serial_printf("TrackMenu: no handle for menu %d\n", menuID);
        return 0;
    }

    serial_printf("TrackMenu: Got menu handle\n");

    /* Get title position from tracking data */
    Rect titleRect;
    if (!GetMenuTitleRectByID(menuID, &titleRect)) {
        serial_printf("TrackMenu: no title rect for menu %d\n", menuID);
        return 0;
    }

    serial_printf("TrackMenu: Got title rect\n");

    short left = titleRect.left;
    short top = 20;  /* under menubar */

    serial_printf("TrackMenu: About to draw menu %d at (%d,%d)\n", menuID, left, top);
    DrawMenuItems(menu, left, top);
    serial_printf("TrackMenu: Finished drawing menu\n");

    /* Wait for mouse button release */
    serial_printf("TrackMenu: Waiting for mouse up...\n");

    /* Poll until mouse button is released */
    while (Button()) {
        /* Poll PS/2 to update mouse state */
        PollPS2Input();

        /* Small delay to avoid hogging CPU */
        for (volatile int i = 0; i < 10000; i++) {
            /* Brief delay */
        }
    }

    serial_printf("TrackMenu: Done waiting\n");

    /* Erase menu and redraw desktop to avoid corruption */
    short itemCount = CountMenuItems(menu);
    short lineHeight = 16;
    short menuWidth = 120;

    /* Properly redraw the desktop area where the menu was */
    /* First, we need to invalidate the menu area so it gets redrawn with the pattern */
    Rect menuRect;
    SetRect(&menuRect, left, top, left + menuWidth, top + itemCount * lineHeight + 4);

    /* Invalidate the menu area to trigger a proper redraw */
    InvalRect(&menuRect);

    /* Trigger desktop redraw which will use the pattern */
    DrawDesktop();
    DrawVolumeIcon();  /* Redraw the volume icon as well */

    serial_puts("TrackMenu: Returning 0 (test mode)\n");
    return 0;
}