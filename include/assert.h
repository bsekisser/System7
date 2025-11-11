#ifndef ASSERT_H
#define ASSERT_H

/* Minimal assert implementation for freestanding ARM64 builds */

#ifdef NDEBUG
#define assert(expr) ((void)0)
#else
extern void uart_puts(const char *str);
#define assert(expr) ((void)((expr) || (uart_puts("[ASSERT] " #expr "\n"), 0)))
#endif

#endif
