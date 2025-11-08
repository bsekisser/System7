/*
 * ARM64 Exception Handlers
 * Debug handlers for exception tracking
 */

#include <stdint.h>
#include "uart.h"

extern void exception_vectors(void);

/*
 * Install exception vector table
 */
void exceptions_init(void) {
    __asm__ volatile("msr vbar_el1, %0" :: "r"(exception_vectors));
    __asm__ volatile("isb");
}

/*
 * Synchronous exception handler
 */
void exception_sync_handler(void) {
    uart_puts("\n[EXCEPTION] Synchronous exception caught\n");

    /* Hang */
    while (1) {
        __asm__ volatile("wfe");
    }
}
