/*
 * ARM Bootloader HAL Implementation
 * Raspberry Pi boot initialization
 */

#include <stdint.h>
#include <stddef.h>
#include "System71StdLib.h"
#include "boot.h"
#include "mmio.h"

/* Multiboot-compatible structure for ARM (simplified) */
typedef struct {
    uint32_t size;
    uint32_t reserved;
} arm_boot_info_t;

/* Global boot information */
static arm_boot_info_t boot_info = {0};

/*
 * Initialize boot parameters
 * Called early in boot sequence from platform_boot.S
 */
void hal_boot_init(void *boot_ptr) {
    /* Parse boot parameters if provided */
    if (boot_ptr) {
        /* TODO: Parse Device Tree or ATAGS */
        /* For now, use reasonable defaults for Raspberry Pi */
    }

    /* Initialize basic boot state */
    boot_info.size = sizeof(arm_boot_info_t);
    boot_info.reserved = 0;

    /* Serial output for debugging */
    Serial_WriteString("[ARM] Boot initialization complete\n");
}

/*
 * Get memory size detected at boot
 * On ARM, this would typically come from Device Tree
 */
uint32_t hal_get_memory_size(void) {
    /* Default for Raspberry Pi 3/4/5: vary by model
     * Pi 3: 1GB
     * Pi 4: 1-8GB (we'll default to 4GB)
     * Pi 5: 4-8GB
     * For now return conservative default
     */
    return 512 * 1024 * 1024;  /* 512MB default */
}

/*
 * Get system framebuffer information
 * Required by QuickDraw for display output
 */
int hal_get_framebuffer_info(void *fb_info_ptr) {
    /* TODO: Query VideoCore GPU via mailbox interface for framebuffer
     * For now, return error indicating framebuffer not yet initialized
     */
    return -1;
}

/*
 * Platform-specific initialization
 * Called after basic kernel setup
 */
int hal_platform_init(void) {
    Serial_WriteString("[ARM] Platform-specific initialization\n");

    /* TODO: Initialize platform-specific hardware:
     * - GPIO controller
     * - Timer hardware
     * - SD card controller
     * - USB host controller
     */

    return 0;
}

/*
 * Platform shutdown
 * Called on kernel halt/reboot
 */
void hal_platform_shutdown(void) {
    Serial_WriteString("[ARM] Platform shutdown\n");

    /* TODO: Graceful shutdown sequence:
     * - Stop peripherals
     * - Flush caches
     * - Reset to bootloader
     */
}
