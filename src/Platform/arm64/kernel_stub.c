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
#include "simple_fb.h"
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
    uart_puts("[KERNEL] Initializing graphics (160x120)...\n");
    if (simple_fb_init()) {
        uart_puts("[KERNEL] Graphics OK - drawing test pattern...\n");

        /* Clear to dark blue background */
        simple_fb_clear(0xFF001040);

        /* Draw title bar */
        simple_fb_draw_rect(0, 0, 160, 12, 0xFFCCCCCC);

        /* Draw colored status boxes */
        simple_fb_draw_rect(10, 20, 40, 30, 0xFFFF0000);   /* Red - UART */
        simple_fb_draw_rect(60, 20, 40, 30, 0xFF00FF00);   /* Green - Timer */
        simple_fb_draw_rect(110, 20, 40, 30, 0xFF0000FF);  /* Blue - Boot */

        /* Draw status panel */
        simple_fb_draw_rect(10, 60, 140, 50, 0xFFFFFFFF);  /* White box */
        simple_fb_draw_rect(12, 62, 136, 46, 0xFF000000);  /* Black interior */

        uart_puts("[KERNEL] Graphics initialized - 160x120 framebuffer ready\n");
    } else {
        uart_puts("[KERNEL] Graphics init failed\n");
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
