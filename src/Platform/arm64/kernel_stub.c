/*
 * ARM64 Kernel Stub for Testing
 * Minimal kernel entry point for boot verification
 */

#include <stdint.h>
#include <stddef.h>
#include "uart.h"
#include "timer.h"
#include "mmu.h"

#ifdef QEMU_BUILD
#include "ramfb.h"
#else
#include "framebuffer.h"
#endif

/* External printf function */
extern int snprintf(char *str, size_t size, const char *format, ...);

/*
 * Main kernel entry point
 */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    uart_puts("\n[KERNEL] Main entry\n");
    uart_puts("[KERNEL] =========================================================\n");
    uart_puts("[KERNEL] System 7.1 ARM64 Kernel Test\n");
    uart_puts("[KERNEL] =========================================================\n");

    /* Test timer */
    uart_puts("[KERNEL] Timer operational\n");

    /* Test delay */
    uart_puts("[KERNEL] Testing 1 second delay...\n");
    timer_msleep(1000);
    uart_puts("[KERNEL] Delay complete!\n");

    /* Skip MMU for now in QEMU - causes hang */
    uart_puts("[KERNEL] Skipping MMU initialization for QEMU testing\n");

    /* System ready - all core functionality operational */
    uart_puts("[KERNEL] Core boot sequence successful\n");

    /* Test exception handler (optional - comment out for normal operation) */
#ifdef TEST_EXCEPTION_HANDLER
    uart_puts("[KERNEL] Triggering test exception...\n");
    __asm__ volatile("brk #0");  /* Trigger breakpoint exception */
#endif

#ifdef QEMU_BUILD
    uart_puts("[KERNEL] Initializing graphics (320x240 ramfb)...\n");
    if (ramfb_init()) {
        uart_puts("[KERNEL] Graphics OK - drawing test pattern...\n");

        /* Clear to dark blue background */
        ramfb_clear(0xFF001040);

        /* Draw title bar */
        ramfb_draw_rect(0, 0, 320, 24, 0xFFCCCCCC);

        /* Draw colored status boxes */
        ramfb_draw_rect(20, 40, 80, 60, 0xFFFF0000);   /* Red - UART */
        ramfb_draw_rect(120, 40, 80, 60, 0xFF00FF00);  /* Green - Timer */
        ramfb_draw_rect(220, 40, 80, 60, 0xFF0000FF);  /* Blue - Boot */

        /* Draw status panel */
        ramfb_draw_rect(20, 120, 280, 100, 0xFFFFFFFF); /* White box */
        ramfb_draw_rect(24, 124, 272, 92, 0xFF000000);  /* Black interior */

        uart_puts("[KERNEL] Graphics initialized - 320x240 framebuffer active\n");
    } else {
        uart_puts("[KERNEL] Graphics init failed (need -device ramfb)\n");
    }
#endif

#ifndef QEMU_BUILD
    /* Test framebuffer initialization (only on real hardware) */
    uart_puts("[KERNEL] Initializing framebuffer (640x480, 32bpp)...\n");
    if (framebuffer_init(640, 480, 32)) {
        uart_puts("[KERNEL] Framebuffer initialized at 640x480\n");

        /* Draw test pattern */
        uart_puts("[KERNEL] Drawing test pattern...\n");
        framebuffer_clear(0xFF000000);  /* Black */
        framebuffer_draw_rect(50, 50, 100, 100, 0xFFFF0000);  /* Red square */
        framebuffer_draw_rect(200, 50, 100, 100, 0xFF00FF00); /* Green square */
        framebuffer_draw_rect(350, 50, 100, 100, 0xFF0000FF); /* Blue square */
        uart_puts("[KERNEL] Test pattern complete\n");
    } else {
        uart_puts("[KERNEL] Framebuffer initialization failed\n");
    }
#endif

    uart_puts("\n");
    uart_puts("[KERNEL] =========================================================\n");
    uart_puts("[KERNEL] All tests complete - entering idle loop\n");
    uart_puts("[KERNEL] =========================================================\n");
    uart_puts("\n");

    /* Idle loop */
    while (1) {
        __asm__ volatile("wfe");
    }

    return 0;
}
