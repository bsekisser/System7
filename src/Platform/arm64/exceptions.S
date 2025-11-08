/* ARM64 Exception Handlers
 * Proper exception handling with full context save/restore
 */

.section .text

/* Exception context structure (matches exception_context_t in C)
 * Must be kept in sync with exception_handlers.h
 */
.set CONTEXT_SIZE, 272  /* 34 registers * 8 bytes */

/* Save all registers macro */
.macro save_context
    /* Make space on stack for context */
    sub sp, sp, #CONTEXT_SIZE

    /* Save general purpose registers x0-x30 */
    stp x0, x1, [sp, #0]
    stp x2, x3, [sp, #16]
    stp x4, x5, [sp, #32]
    stp x6, x7, [sp, #48]
    stp x8, x9, [sp, #64]
    stp x10, x11, [sp, #80]
    stp x12, x13, [sp, #96]
    stp x14, x15, [sp, #112]
    stp x16, x17, [sp, #128]
    stp x18, x19, [sp, #144]
    stp x20, x21, [sp, #160]
    stp x22, x23, [sp, #176]
    stp x24, x25, [sp, #192]
    stp x26, x27, [sp, #208]
    stp x28, x29, [sp, #224]
    str x30, [sp, #240]

    /* Save exception return address */
    mrs x0, elr_el1
    str x0, [sp, #248]

    /* Save saved program status */
    mrs x0, spsr_el1
    str x0, [sp, #256]

    /* Save stack pointer (before context save) */
    add x0, sp, #CONTEXT_SIZE
    str x0, [sp, #264]
.endm

/* Restore all registers macro */
.macro restore_context
    /* Restore exception return address */
    ldr x0, [sp, #248]
    msr elr_el1, x0

    /* Restore saved program status */
    ldr x0, [sp, #256]
    msr spsr_el1, x0

    /* Restore general purpose registers x0-x30 */
    ldp x0, x1, [sp, #0]
    ldp x2, x3, [sp, #16]
    ldp x4, x5, [sp, #32]
    ldp x6, x7, [sp, #48]
    ldp x8, x9, [sp, #64]
    ldp x10, x11, [sp, #80]
    ldp x12, x13, [sp, #96]
    ldp x14, x15, [sp, #112]
    ldp x16, x17, [sp, #128]
    ldp x18, x19, [sp, #144]
    ldp x20, x21, [sp, #160]
    ldp x22, x23, [sp, #176]
    ldp x24, x25, [sp, #192]
    ldp x26, x27, [sp, #208]
    ldp x28, x29, [sp, #224]
    ldr x30, [sp, #240]

    /* Remove context from stack */
    add sp, sp, #CONTEXT_SIZE
.endm

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
    save_context
    mov x0, sp  /* Pass context pointer as argument */
    bl handle_sync_exception
    /* Should not return */
    b exception_hang

exception_curr_el_spx_irq:
    save_context
    mov x0, sp
    bl handle_irq_exception
    restore_context
    eret

exception_curr_el_spx_fiq:
    save_context
    mov x0, sp
    bl handle_fiq_exception
    restore_context
    eret

exception_curr_el_spx_serror:
    save_context
    mov x0, sp
    bl handle_serror_exception
    /* Should not return */
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
