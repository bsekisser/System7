/* Test drawing a single character */
#include "SystemTypes.h"
#include "chicago_font.h"

extern void* framebuffer;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;
extern uint32_t pack_color(uint8_t r, uint8_t g, uint8_t b);

void TestSingleChar(void) {
    if (!framebuffer) return;

    uint32_t *fb = (uint32_t*)framebuffer;
    uint32_t white = pack_color(255, 255, 255);
    uint32_t black = pack_color(0, 0, 0);

    /* Clear area */
    for (int y = 50; y < 100; y++) {
        for (int x = 50; x < 200; x++) {
            fb[y * (fb_pitch/4) + x] = white;
        }
    }

    /* Draw character 'A' (0x41) at position 60,60 */
    /* 'A' is at index 0x41 - 0 + 1 = 66 in the array */
    /* From the header: { 208, 6 } for 'A' */
    int char_idx = 66;  /* Index for 'A' */
    int offset = 208;   /* Bit offset for 'A' */
    int width = 6;      /* Width of 'A' */

    /* Draw the character bit by bit */
    for (int row = 0; row < CHICAGO_HEIGHT; row++) {
        /* Each row in the bitmap starts at this byte offset */
        int row_byte_start = row * CHICAGO_ROW_WORDS * 2;

        for (int col = 0; col < width; col++) {
            /* The bit we want is at (offset + col) within this row */
            int bit_index = offset + col;
            int byte_index = row_byte_start + (bit_index / 8);
            int bit_in_byte = 7 - (bit_index % 8);  /* MSB first */

            if (byte_index < sizeof(chicago_bitmap)) {
                if (chicago_bitmap[byte_index] & (1 << bit_in_byte)) {
                    /* Set pixel */
                    int x = 60 + col;
                    int y = 60 + row;
                    if (x < fb_width && y < fb_height) {
                        fb[y * (fb_pitch/4) + x] = black;
                    }
                }
            }
        }
    }

    /* Also draw 'F' at 80,60 */
    /* 'F' is at index 0x46 - 0 + 1 = 71 */
    /* From header: { 236, 6 } for 'F' */
    offset = 236;
    width = 6;

    for (int row = 0; row < CHICAGO_HEIGHT; row++) {
        int row_byte_start = row * CHICAGO_ROW_WORDS * 2;

        for (int col = 0; col < width; col++) {
            int bit_index = offset + col;
            int byte_index = row_byte_start + (bit_index / 8);
            int bit_in_byte = 7 - (bit_index % 8);

            if (byte_index < sizeof(chicago_bitmap)) {
                if (chicago_bitmap[byte_index] & (1 << bit_in_byte)) {
                    int x = 80 + col;
                    int y = 60 + row;
                    if (x < fb_width && y < fb_height) {
                        fb[y * (fb_pitch/4) + x] = black;
                    }
                }
            }
        }
    }

    /* Draw 'i' at 100,60 */
    /* 'i' is at index 0x69 - 0 + 1 = 106 */
    /* From header: { 435, 5 } for 'i' */
    offset = 435;
    width = 5;

    for (int row = 0; row < CHICAGO_HEIGHT; row++) {
        int row_byte_start = row * CHICAGO_ROW_WORDS * 2;

        for (int col = 0; col < width; col++) {
            int bit_index = offset + col;
            int byte_index = row_byte_start + (bit_index / 8);
            int bit_in_byte = 7 - (bit_index % 8);

            if (byte_index < sizeof(chicago_bitmap)) {
                if (chicago_bitmap[byte_index] & (1 << bit_in_byte)) {
                    int x = 100 + col;
                    int y = 60 + row;
                    if (x < fb_width && y < fb_height) {
                        fb[y * (fb_pitch/4) + x] = black;
                    }
                }
            }
        }
    }

    /* Draw 'l' at 110,60 */
    /* 'l' is at index 0x6C - 0 + 1 = 109 */
    /* From header: { 448, 10 } for 'l' */
    offset = 448;
    width = 10;

    for (int row = 0; row < CHICAGO_HEIGHT; row++) {
        int row_byte_start = row * CHICAGO_ROW_WORDS * 2;

        for (int col = 0; col < width; col++) {
            int bit_index = offset + col;
            int byte_index = row_byte_start + (bit_index / 8);
            int bit_in_byte = 7 - (bit_index % 8);

            if (byte_index < sizeof(chicago_bitmap)) {
                if (chicago_bitmap[byte_index] & (1 << bit_in_byte)) {
                    int x = 110 + col;
                    int y = 60 + row;
                    if (x < fb_width && y < fb_height) {
                        fb[y * (fb_pitch/4) + x] = black;
                    }
                }
            }
        }
    }

    /* Draw 'e' at 125,60 */
    /* 'e' is at index 0x65 - 0 + 1 = 102 */
    /* From header: { 416, 5 } for 'e' */
    offset = 416;
    width = 5;

    for (int row = 0; row < CHICAGO_HEIGHT; row++) {
        int row_byte_start = row * CHICAGO_ROW_WORDS * 2;

        for (int col = 0; col < width; col++) {
            int bit_index = offset + col;
            int byte_index = row_byte_start + (bit_index / 8);
            int bit_in_byte = 7 - (bit_index % 8);

            if (byte_index < sizeof(chicago_bitmap)) {
                if (chicago_bitmap[byte_index] & (1 << bit_in_byte)) {
                    int x = 125 + col;
                    int y = 60 + row;
                    if (x < fb_width && y < fb_height) {
                        fb[y * (fb_pitch/4) + x] = black;
                    }
                }
            }
        }
    }
}