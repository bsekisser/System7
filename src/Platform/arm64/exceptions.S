/* ARM64 Exception Handlers
 * Basic exception vector table for debugging
 */

.section .text

/* External C handler */
.extern exception_sync_handler

/* Exception handler macro */
.macro exception_entry label
    .align 7
    b exception_\label
.endm

/* Global exception vector table (2KB aligned) */
.align 11
.global exception_vectors
.type exception_vectors, %function
exception_vectors:
    /* Current EL with SP0 */
    exception_entry curr_el_sp0_sync
    exception_entry curr_el_sp0_irq
    exception_entry curr_el_sp0_fiq
    exception_entry curr_el_sp0_serror

    /* Current EL with SPx */
    exception_entry curr_el_spx_sync
    exception_entry curr_el_spx_irq
    exception_entry curr_el_spx_fiq
    exception_entry curr_el_spx_serror

    /* Lower EL using AArch64 */
    exception_entry lower_el_aarch64_sync
    exception_entry lower_el_aarch64_irq
    exception_entry lower_el_aarch64_fiq
    exception_entry lower_el_aarch64_serror

    /* Lower EL using AArch32 */
    exception_entry lower_el_aarch32_sync
    exception_entry lower_el_aarch32_irq
    exception_entry lower_el_aarch32_fiq
    exception_entry lower_el_aarch32_serror

/* Exception handlers */
exception_curr_el_sp0_sync:
    b exception_hang

exception_curr_el_sp0_irq:
    b exception_hang

exception_curr_el_sp0_fiq:
    b exception_hang

exception_curr_el_sp0_serror:
    b exception_hang

exception_curr_el_spx_sync:
    /* For now, just hang on any sync exception */
    /* TODO: Properly save all registers and call C handler */
    b exception_hang

exception_curr_el_spx_irq:
    eret

exception_curr_el_spx_fiq:
    eret

exception_curr_el_spx_serror:
    b exception_hang

exception_lower_el_aarch64_sync:
    b exception_hang

exception_lower_el_aarch64_irq:
    b exception_hang

exception_lower_el_aarch64_fiq:
    b exception_hang

exception_lower_el_aarch64_serror:
    b exception_hang

exception_lower_el_aarch32_sync:
    b exception_hang

exception_lower_el_aarch32_irq:
    b exception_hang

exception_lower_el_aarch32_fiq:
    b exception_hang

exception_lower_el_aarch32_serror:
    b exception_hang

/* Hang on unhandled exception */
exception_hang:
    wfe
    b exception_hang
