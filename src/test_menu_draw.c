/* Test menu drawing directly */
#include "SystemTypes.h"
#include "QuickDraw/QuickDraw.h"

extern void* framebuffer;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;
extern uint32_t pack_color(uint8_t r, uint8_t g, uint8_t b);

void TestMenuDraw(void) {
    if (!framebuffer) return;

    /* Clear menu bar area */
    uint32_t *fb = (uint32_t*)framebuffer;
    uint32_t white = pack_color(255, 255, 255);
    uint32_t black = pack_color(0, 0, 0);

    /* Draw white background for menu bar */
    for (int y = 0; y < 20; y++) {
        for (int x = 0; x < fb_width; x++) {
            int fb_offset = y * (fb_pitch / 4) + x;
            fb[fb_offset] = white;
        }
    }

    /* Draw bottom line */
    for (int x = 0; x < fb_width; x++) {
        int fb_offset = 19 * (fb_pitch / 4) + x;
        fb[fb_offset] = black;
    }

    /* Test drawing menu titles at correct positions */
    int x_pos = 10;

    /* Apple menu - character 0x14 */
    DrawCharAt(x_pos, 13, 0x14);
    x_pos += 30;

    /* File menu */
    const char* file = "File";
    for (int i = 0; file[i]; i++) {
        DrawCharAt(x_pos, 13, file[i]);
        x_pos += CharWidth(file[i]);
    }
    x_pos += 20;

    /* Edit menu */
    const char* edit = "Edit";
    for (int i = 0; edit[i]; i++) {
        DrawCharAt(x_pos, 13, edit[i]);
        x_pos += CharWidth(edit[i]);
    }
    x_pos += 20;

    /* View menu */
    const char* view = "View";
    for (int i = 0; view[i]; i++) {
        DrawCharAt(x_pos, 13, view[i]);
        x_pos += CharWidth(view[i]);
    }
    x_pos += 20;

    /* Label menu */
    const char* label = "Label";
    for (int i = 0; label[i]; i++) {
        DrawCharAt(x_pos, 13, label[i]);
        x_pos += CharWidth(label[i]);
    }
    x_pos += 20;

    /* Special menu */
    const char* special = "Special";
    for (int i = 0; special[i]; i++) {
        DrawCharAt(x_pos, 13, special[i]);
        x_pos += CharWidth(special[i]);
    }
}