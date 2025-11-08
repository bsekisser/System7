/*
 * Simple Framebuffer
 */

#ifndef SIMPLE_FB_H
#define SIMPLE_FB_H

#include <stdbool.h>
#include <stdint.h>

bool simple_fb_init(void);
void simple_fb_clear(uint32_t color);
void simple_fb_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
uint32_t* simple_fb_get_buffer(void);
uint32_t simple_fb_get_width(void);
uint32_t simple_fb_get_height(void);

#endif
