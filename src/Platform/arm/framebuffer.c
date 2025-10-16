/*
 * ARM Framebuffer Integration
 * Bridges VideoCore GPU with QuickDraw graphics system
 * Sets up display memory and initializes graphics environment
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "System71StdLib.h"
#include "QuickDraw.h"
#include "videocore.h"

/* Global framebuffer info (shared with QuickDraw) */
static videocore_fb_t arm_framebuffer = {0};
static int framebuffer_ready = 0;

/*
 * Initialize framebuffer from VideoCore GPU
 * Called during platform boot before QuickDraw initialization
 */
int arm_framebuffer_init(void) {
    Serial_WriteString("[FB] Initializing ARM framebuffer\n");

    /* Initialize VideoCore mailbox */
    if (videocore_init() != 0) {
        Serial_WriteString("[FB] Failed to initialize VideoCore\n");
        return -1;
    }

    /* Request framebuffer from GPU
     * Default to 1024x768 @ 32-bit RGBA
     * (same as x86 default for compatibility)
     */
    arm_framebuffer.width = 1024;
    arm_framebuffer.height = 768;
    arm_framebuffer.depth = 32;

    if (videocore_allocate_fb(&arm_framebuffer) != 0) {
        Serial_WriteString("[FB] Failed to allocate framebuffer from GPU\n");
        return -1;
    }

    framebuffer_ready = 1;

    Serial_Printf("[FB] Framebuffer ready: %ux%u @ %u-bit\n",
                  arm_framebuffer.width, arm_framebuffer.height, arm_framebuffer.depth);
    Serial_Printf("[FB] Physical address: 0x%x, Pitch: %u bytes\n",
                  arm_framebuffer.fb_address, arm_framebuffer.pitch);

    return 0;
}

/*
 * Get framebuffer info for QuickDraw
 * QuickDraw needs: address, width, height, pitch, depth
 */
int arm_get_framebuffer_info(void *fb_info_ptr) {
    if (!framebuffer_ready) {
        return -1;
    }

    if (!fb_info_ptr) {
        return -1;
    }

    /* Return framebuffer address and metadata
     * Caller is responsible for interpreting this based on their structure
     */
    videocore_fb_t *info = (videocore_fb_t *)fb_info_ptr;
    memcpy(info, &arm_framebuffer, sizeof(videocore_fb_t));

    return 0;
}

/*
 * Set framebuffer resolution
 * Called if runtime resolution change requested
 */
int arm_set_framebuffer_size(uint32_t width, uint32_t height, uint32_t depth) {
    if (!videocore_mbox_base) {
        Serial_WriteString("[FB] VideoCore not initialized\n");
        return -1;
    }

    Serial_Printf("[FB] Requesting framebuffer resize: %ux%u @ %u-bit\n",
                  width, height, depth);

    if (videocore_set_fb_size(width, height, depth) != 0) {
        Serial_WriteString("[FB] Failed to resize framebuffer\n");
        return -1;
    }

    videocore_get_fb_info(&arm_framebuffer);
    return 0;
}

/*
 * Clear framebuffer to a solid color
 * Useful for testing and splash screens
 */
void arm_clear_framebuffer(uint32_t color) {
    if (!framebuffer_ready) {
        return;
    }

    uint32_t *fb = (uint32_t *)arm_framebuffer.fb_address;
    uint32_t pixel_count = (arm_framebuffer.pitch / 4) * arm_framebuffer.height;

    for (uint32_t i = 0; i < pixel_count; i++) {
        fb[i] = color;
    }
}

/*
 * Draw a test pattern on framebuffer
 * Useful for early boot diagnostics
 */
void arm_draw_test_pattern(void) {
    if (!framebuffer_ready) {
        return;
    }

    uint32_t *fb = (uint32_t *)arm_framebuffer.fb_address;
    uint32_t pitch_pixels = arm_framebuffer.pitch / 4;

    /* Four color quadrants: red, green, blue, white */
    uint32_t colors[] = {
        0xFF0000FF,  /* Red (BGRA format) */
        0xFF00FF00,  /* Green */
        0xFFFF0000,  /* Blue */
        0xFFFFFFFF   /* White */
    };

    uint32_t quad_width = arm_framebuffer.width / 2;
    uint32_t quad_height = arm_framebuffer.height / 2;

    /* Top-left: Red */
    for (uint32_t y = 0; y < quad_height; y++) {
        for (uint32_t x = 0; x < quad_width; x++) {
            fb[y * pitch_pixels + x] = colors[0];
        }
    }

    /* Top-right: Green */
    for (uint32_t y = 0; y < quad_height; y++) {
        for (uint32_t x = quad_width; x < quad_width * 2; x++) {
            fb[y * pitch_pixels + x] = colors[1];
        }
    }

    /* Bottom-left: Blue */
    for (uint32_t y = quad_height; y < quad_height * 2; y++) {
        for (uint32_t x = 0; x < quad_width; x++) {
            fb[y * pitch_pixels + x] = colors[2];
        }
    }

    /* Bottom-right: White */
    for (uint32_t y = quad_height; y < quad_height * 2; y++) {
        for (uint32_t x = quad_width; x < quad_width * 2; x++) {
            fb[y * pitch_pixels + x] = colors[3];
        }
    }

    Serial_WriteString("[FB] Test pattern displayed\n");
}

/*
 * Get current framebuffer resolution
 */
void arm_get_framebuffer_size(uint32_t *width, uint32_t *height) {
    if (width) {
        *width = arm_framebuffer.width;
    }
    if (height) {
        *height = arm_framebuffer.height;
    }
}

/*
 * Check if framebuffer is ready
 */
int arm_framebuffer_is_ready(void) {
    return framebuffer_ready;
}
