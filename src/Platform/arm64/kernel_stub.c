/*
 * ARM64 Kernel Stub for Testing
 * Minimal kernel entry point for boot verification
 */

#include <stdint.h>
#include <stddef.h>
#include "uart.h"
#include "timer.h"
#include "framebuffer.h"
#include "mmu.h"

/*
 * Simple implementations for testing
 */
int snprintf(char *str, size_t size, const char *format, ...) {
    /* Very basic snprintf for integer formatting */
    const char *src = format;
    char *dst = str;
    size_t remaining = size - 1;

    __builtin_va_list args;
    __builtin_va_start(args, format);

    while (*src && remaining > 0) {
        if (*src == '%') {
            src++;
            if (*src == 'l') {
                src++;
                if (*src == 'l') src++;  /* Handle %llu */
            }

            if (*src == 'u' || *src == 'd') {
                uint64_t val = __builtin_va_arg(args, uint64_t);
                char temp[32];
                int i = 0;

                if (val == 0) {
                    temp[i++] = '0';
                } else {
                    while (val > 0 && i < 31) {
                        temp[i++] = '0' + (val % 10);
                        val /= 10;
                    }
                }

                while (i > 0 && remaining > 0) {
                    *dst++ = temp[--i];
                    remaining--;
                }
            } else if (*src == 'x') {
                uint64_t val = __builtin_va_arg(args, uint64_t);
                char temp[32];
                int i = 0;

                if (val == 0) {
                    temp[i++] = '0';
                } else {
                    while (val > 0 && i < 31) {
                        int digit = val & 0xF;
                        temp[i++] = digit < 10 ? '0' + digit : 'a' + digit - 10;
                        val >>= 4;
                    }
                }

                while (i > 0 && remaining > 0) {
                    *dst++ = temp[--i];
                    remaining--;
                }
            } else if (*src == 's') {
                const char *s = __builtin_va_arg(args, const char *);
                while (*s && remaining > 0) {
                    *dst++ = *s++;
                    remaining--;
                }
            } else {
                if (remaining > 0) {
                    *dst++ = *src;
                    remaining--;
                }
            }
            src++;
        } else {
            *dst++ = *src++;
            remaining--;
        }
    }

    *dst = '\0';
    __builtin_va_end(args);
    return dst - str;
}

/*
 * Main kernel entry point
 */
int main(int argc, char **argv) {
    char buf[256];

    (void)argc;
    (void)argv;

    uart_puts("\n");
    uart_puts("[KERNEL] ═══════════════════════════════════════════════════════\n");
    uart_puts("[KERNEL] System 7.1 ARM64 Kernel Test\n");
    uart_puts("[KERNEL] ═══════════════════════════════════════════════════════\n");

    /* Test timer */
    uint64_t time_us = timer_get_usec();
    snprintf(buf, sizeof(buf), "[KERNEL] Timer test: %llu microseconds since boot\n", time_us);
    uart_puts(buf);

    /* Test delay */
    uart_puts("[KERNEL] Testing 1 second delay...\n");
    timer_msleep(1000);
    uart_puts("[KERNEL] Delay complete!\n");

    /* Initialize and enable MMU */
    uart_puts("[KERNEL] Initializing MMU...\n");
    if (mmu_init()) {
        uart_puts("[KERNEL] MMU initialized\n");
        uart_puts("[KERNEL] Enabling MMU...\n");
        mmu_enable();
        if (mmu_is_enabled()) {
            uart_puts("[KERNEL] MMU enabled successfully!\n");
        } else {
            uart_puts("[KERNEL] Warning: MMU enable failed\n");
        }
    }

    /* Test framebuffer initialization */
    uart_puts("[KERNEL] Initializing framebuffer (640x480, 32bpp)...\n");
    if (framebuffer_init(640, 480, 32)) {
        uart_puts("[KERNEL] Framebuffer initialized!\n");

        uint32_t width = framebuffer_get_width();
        uint32_t height = framebuffer_get_height();
        snprintf(buf, sizeof(buf), "[KERNEL] Resolution: %llu x %llu\n",
                 (uint64_t)width, (uint64_t)height);
        uart_puts(buf);

        /* Draw test pattern */
        uart_puts("[KERNEL] Drawing test pattern...\n");
        framebuffer_clear(0xFF000000);  /* Black */
        framebuffer_draw_rect(50, 50, 100, 100, 0xFFFF0000);  /* Red square */
        framebuffer_draw_rect(200, 50, 100, 100, 0xFF00FF00); /* Green square */
        framebuffer_draw_rect(350, 50, 100, 100, 0xFF0000FF); /* Blue square */
        uart_puts("[KERNEL] Test pattern complete\n");
    } else {
        uart_puts("[KERNEL] Framebuffer initialization failed (normal in QEMU)\n");
    }

    uart_puts("\n");
    uart_puts("[KERNEL] ═══════════════════════════════════════════════════════\n");
    uart_puts("[KERNEL] All tests complete - entering idle loop\n");
    uart_puts("[KERNEL] ═══════════════════════════════════════════════════════\n");
    uart_puts("\n");

    /* Idle loop */
    while (1) {
        __asm__ volatile("wfe");
    }

    return 0;
}
