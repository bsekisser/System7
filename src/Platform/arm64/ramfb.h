/*
 * QEMU ramfb (RAM Framebuffer) Driver
 */

#ifndef RAMFB_H
#define RAMFB_H

#include <stdbool.h>
#include <stdint.h>

bool ramfb_init(void);
void ramfb_clear(uint32_t color);
void ramfb_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
uint32_t* ramfb_get_buffer(void);
uint32_t ramfb_get_width(void);
uint32_t ramfb_get_height(void);

#endif
