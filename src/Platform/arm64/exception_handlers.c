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
    uint64_t esr, elr, far;

    /* Read exception syndrome register */
    __asm__ volatile("mrs %0, esr_el1" : "=r"(esr));
    __asm__ volatile("mrs %0, elr_el1" : "=r"(elr));
    __asm__ volatile("mrs %0, far_el1" : "=r"(far));

    uart_puts("\n[EXCEPTION] Synchronous exception!\n");
    uart_puts("[EXCEPTION] ESR: ");
    uart_puts("[EXCEPTION] Halting...\n");

    /* Hang */
    while (1) {
        __asm__ volatile("wfe");
    }
}
