/*
 * hal_boot.c - PowerPC Boot HAL Implementation
 *
 * Initial scaffolding for bringing up the PowerPC port. The routines here
 * currently provide stub behaviour so that the core kernel can build while
 * the real hardware bring-up is implemented.
 */

#include <stdint.h>
#include <stddef.h>

#include "Platform/include/boot.h"
#include "Platform/PowerPC/OpenFirmware.h"

static void *g_boot_arg = NULL;
static uint32_t g_memory_size = 256u * 1024u * 1024u; /* 256 MB default */
static ofw_memory_range_t g_cached_ranges[OFW_MAX_MEMORY_RANGES];
static size_t g_cached_range_count = 0;
static hal_framebuffer_info_t g_fb_info = {0};
static int g_fb_present = 0;

void hal_boot_init(void *boot_arg) {
    /* Stash the Open Firmware / loader argument for later inspection. */
    g_boot_arg = boot_arg;
    ofw_client_init(boot_arg);

    ofw_memory_range_t ranges[OFW_MAX_MEMORY_RANGES];
    size_t count = ofw_get_memory_ranges(ranges, OFW_MAX_MEMORY_RANGES);
    size_t copy_count = count;
    if (copy_count > OFW_MAX_MEMORY_RANGES) {
        copy_count = OFW_MAX_MEMORY_RANGES;
    }
    g_cached_range_count = copy_count;
    for (size_t i = 0; i < copy_count; ++i) {
        g_cached_ranges[i] = ranges[i];
    }
    if (count > 0) {
        uint64_t total = 0;
        for (size_t i = 0; i < count; ++i) {
            total += ranges[i].size;
        }
        if (total == 0) {
            total = ranges[0].size;
        }
        if (total > UINT32_MAX) {
            g_memory_size = UINT32_MAX;
        } else if (total != 0) {
            g_memory_size = (uint32_t)total;
        }
    }
    ofw_framebuffer_info_t fb_info;
    if (ofw_get_framebuffer_info(&fb_info) == 0 && fb_info.base != 0 && fb_info.width != 0 && fb_info.height != 0) {
        g_fb_present = 1;
        g_fb_info.framebuffer = (void *)(uintptr_t)fb_info.base;
        g_fb_info.width = fb_info.width;
        g_fb_info.height = fb_info.height;
        g_fb_info.pitch = fb_info.stride ? fb_info.stride : fb_info.width * 4;
        g_fb_info.depth = fb_info.depth ? fb_info.depth : 32;
        g_fb_info.red_offset = 16;
        g_fb_info.red_size = 8;
        g_fb_info.green_offset = 8;
        g_fb_info.green_size = 8;
        g_fb_info.blue_offset = 0;
        g_fb_info.blue_size = 8;
    } else {
        g_fb_present = 0;
    }
}

int hal_get_framebuffer_info(hal_framebuffer_info_t *info) {
    if (!info || !g_fb_present) {
        return -1;
    }
    *info = g_fb_info;
    return 0;
}

uint32_t hal_get_memory_size(void) {
    return g_memory_size;
}

int hal_platform_init(void) {
    (void)g_boot_arg;
    /* TODO: Detect PowerPC peripherals and expose them through the HAL. */
    return 0;
}

void hal_platform_shutdown(void) {
}

int hal_framebuffer_present(void) {
    return g_fb_present;
}

size_t hal_ppc_get_memory_ranges(ofw_memory_range_t *out_ranges, size_t max_ranges) {
    if (!out_ranges || max_ranges == 0) {
        return 0;
    }
    size_t copy_count = g_cached_range_count;
    if (copy_count > max_ranges) {
        copy_count = max_ranges;
    }
    for (size_t i = 0; i < copy_count; ++i) {
        out_ranges[i] = g_cached_ranges[i];
    }
    return copy_count;
}

size_t hal_ppc_memory_range_count(void) {
    return g_cached_range_count;
}
