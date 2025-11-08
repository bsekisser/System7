/*
 * Simple Framebuffer - Minimal Working Implementation
 * Allocates framebuffer in DATA section to avoid BSS issues
 */

#include <stdint.h>
#include <stdbool.h>
#include "simple_fb.h"

/* Framebuffer: 160x120 pixels = 76800 bytes */
#define FB_WIDTH 160
#define FB_HEIGHT 120

/* Framebuffer in BSS - uninitialized */
static uint32_t framebuffer[FB_WIDTH * FB_HEIGHT];
static bool initialized = false;

bool simple_fb_init(void) {
    initialized = true;
    return true;
}

void simple_fb_clear(uint32_t color) {
    if (!initialized) return;

    for (int i = 0; i < FB_WIDTH * FB_HEIGHT; i++) {
        framebuffer[i] = color;
    }
}

void simple_fb_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    if (!initialized) return;

    for (uint32_t py = y; py < y + h && py < FB_HEIGHT; py++) {
        for (uint32_t px = x; px < x + w && px < FB_WIDTH; px++) {
            framebuffer[py * FB_WIDTH + px] = color;
        }
    }
}

uint32_t* simple_fb_get_buffer(void) {
    return framebuffer;
}

uint32_t simple_fb_get_width(void) {
    return FB_WIDTH;
}

uint32_t simple_fb_get_height(void) {
    return FB_HEIGHT;
}
