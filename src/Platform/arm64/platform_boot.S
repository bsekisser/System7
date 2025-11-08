/* ARM64 Bootstrap for System 7.1 Kernel
 * Raspberry Pi 3/4/5 AArch64 (ARMv8-A)
 *
 * Entry point: typically at 0x80000
 * Bootloader provides:
 *   x0 = DTB (Device Tree Blob) physical address
 *   x1 = 0 (reserved)
 *   x2 = 0 (reserved)
 *   x3 = 0 (reserved)
 *
 * Boot at EL2 or EL1 (depending on firmware)
 *
 * References:
 *   ARM Architecture Reference Manual ARMv8
 *   Procedure Call Standard for ARM 64-bit (AAPCS64)
 */

.section .text.boot
.global _start
.type _start, %function

_start:
    /* Save DTB address - x0 contains DTB pointer */
    mov x19, x0                 /* x19 is callee-saved */

    /* Check current exception level */
    mrs x0, CurrentEL
    and x0, x0, #0xC            /* Extract EL[3:2] */
    cmp x0, #8                  /* Check if EL2 */
    beq setup_el2
    cmp x0, #4                  /* Check if EL1 */
    beq setup_el1
    /* If EL3 or EL0, halt */
    b halt

setup_el2:
    /* Running at EL2 (Hypervisor mode)
     * Drop to EL1 for kernel execution */

    /* Enable AArch64 in EL1 */
    mov x0, #(1 << 31)          /* RW = 1 (AArch64) */
    msr hcr_el2, x0

    /* Set EL1 execution state to AArch64 */
    mov x0, #0x3c5              /* D=1,A=1,I=1,F=1,EL1h */
    msr spsr_el2, x0

    /* Set EL1 entry point */
    adr x0, setup_el1
    msr elr_el2, x0

    /* Drop to EL1 */
    eret

setup_el1:
    /* Now at EL1 - kernel mode */

    /* Disable interrupts */
    msr daifset, #0xF           /* Mask D,A,I,F */

    /* Set up stack pointer for EL1 */
    adr x0, stack_top
    mov sp, x0

    /* Clear BSS section */
    adrp x0, __bss_start
    add x0, x0, :lo12:__bss_start
    adrp x1, __bss_end
    add x1, x1, :lo12:__bss_end

clear_bss:
    cmp x0, x1
    beq bss_done
    str xzr, [x0], #8           /* Store zero, post-increment by 8 */
    b clear_bss

bss_done:
    /* Initialize vector table for EL1 */
    adr x0, vector_table_el1
    msr vbar_el1, x0

    /* Enable floating point */
    mrs x0, cpacr_el1
    orr x0, x0, #(3 << 20)      /* FPEN = 11 (no trap) */
    msr cpacr_el1, x0
    isb

    /* Set up system control
     * Enable caches and MMU later, for now run uncached */
    mrs x0, sctlr_el1
    orr x0, x0, #(1 << 2)       /* C bit - enable data cache */
    orr x0, x0, #(1 << 12)      /* I bit - enable instruction cache */
    msr sctlr_el1, x0
    isb

    /* Call C entry point
     * AAPCS64 calling convention: first arg in x0 */
    mov x0, x19                 /* DTB address */
    bl arm64_boot_main

    /* Should not return */
    b halt

halt:
    wfe                         /* Wait for event */
    b halt

/* Exception vectors for EL1
 * Each entry is 128 bytes, aligned to 2KB boundary */
.align 11
vector_table_el1:
    /* Current EL with SP0 */
    .align 7
    b halt                      /* Synchronous */
    .align 7
    b halt                      /* IRQ */
    .align 7
    b halt                      /* FIQ */
    .align 7
    b halt                      /* SError */

    /* Current EL with SPx */
    .align 7
    b halt                      /* Synchronous */
    .align 7
    b halt                      /* IRQ */
    .align 7
    b halt                      /* FIQ */
    .align 7
    b halt                      /* SError */

    /* Lower EL using AArch64 */
    .align 7
    b halt                      /* Synchronous */
    .align 7
    b halt                      /* IRQ */
    .align 7
    b halt                      /* FIQ */
    .align 7
    b halt                      /* SError */

    /* Lower EL using AArch32 */
    .align 7
    b halt                      /* Synchronous */
    .align 7
    b halt                      /* IRQ */
    .align 7
    b halt                      /* FIQ */
    .align 7
    b halt                      /* SError */

/* Stack allocation */
.section .bss
.align 16
stack_bottom:
    .space 65536                /* 64KB stack */
stack_top:

/* Mark stack as non-executable */
.section .note.GNU-stack,"",@progbits
