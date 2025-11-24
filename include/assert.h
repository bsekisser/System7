#ifndef ASSERT_H
#define ASSERT_H

/* Minimal assert implementation for freestanding builds */

#ifdef NDEBUG
#define assert(expr) ((void)0)
#else
/* Use platform-appropriate output function */
#ifdef __aarch64__
extern void uart_puts(const char *str);
#define assert(expr) ((void)((expr) || (uart_puts("[ASSERT] " #expr "\n"), 0)))
#else
extern void serial_puts(const char *str);
#define assert(expr) ((void)((expr) || (serial_puts("[ASSERT] " #expr "\n"), 0)))
#endif
#endif

#endif
