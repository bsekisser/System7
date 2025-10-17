/*
 * PowerPC Bootloader Shim for System 7 Portable
 *
 * This code runs at 0x01000000 when loaded via OpenBIOS -kernel
 * It's a minimal bootloader that:
 * 1. Initializes UART for debugging
 * 2. Attempts to load the real kernel
 * 3. Sets up proper CPU context
 * 4. Transfers control to kernel
 *
 * The real kernel is expected to be at 0x02000000 or loaded separately
 */

.section ".bss"
.align 5
stack_bottom:
    .space 0x10000     /* 64KB stack */
stack_top:

.section ".text"
.globl _start

_start:
    /*
     * Bootstrap Phase 1: Initialize CPU and UART
     * This MUST work to prove bootloader is executing
     */

    /* Set up stack for bootloader code */
    lis     r1, stack_top@ha
    addi    r1, r1, stack_top@l
    addi    r1, r1, -16
    rlwinm  r1, r1, 0, 0, 27      /* Align to 16 bytes */

    /* Save incoming arguments */
    mr      r31, r3                /* OF entry point - CRITICAL */
    mr      r30, r4                /* Boot args */

    /* Initialize UART at 0xF0200000 for debugging */
    lis     r10, 0xF020
    ori     r10, r10, 0x0000

    /* UART Init: IER (disable interrupts) */
    li      r11, 0x00
    stb     r11, 1(r10)

    /* UART: Enable DLAB for baud rate setting */
    li      r11, 0x80
    stb     r11, 3(r10)

    /* UART: Set divisor for 115200 baud */
    li      r11, 0x01              /* DLL = 1 */
    stb     r11, 0(r10)
    li      r11, 0x00              /* DLH = 0 */
    stb     r11, 1(r10)

    /* UART: Disable DLAB, set 8N1 */
    li      r11, 0x03
    stb     r11, 3(r10)

    /* UART: Enable FIFO */
    li      r11, 0x07
    stb     r11, 2(r10)

    /* UART: Set RTS/DTR */
    li      r11, 0x03
    stb     r11, 4(r10)

    /*
     * Phase 2: Bootloader banner
     * Print "BOOT" to UART to prove bootloader is executing
     */

    li      r11, 0x42              /* 'B' */
    stb     r11, 0(r10)
    li      r11, 0x4F              /* 'O' */
    stb     r11, 0(r10)
    li      r11, 0x4F              /* 'O' */
    stb     r11, 0(r10)
    li      r11, 0x54              /* 'T' */
    stb     r11, 0(r10)
    li      r11, 0x0A              /* '\n' */
    stb     r11, 0(r10)
    li      r11, 0x0D              /* '\r' */
    stb     r11, 0(r10)

    /*
     * Phase 3: Prepare for kernel transfer
     *
     * The challenge: The real kernel is in the same binary at a different section
     * We need to copy/relocate it or it should be loaded at a different address
     *
     * For now, we'll implement a simple approach:
     * - The real kernel code should be embedded after bootloader in same binary
     * - Or loaded at 0x02000000 by a separate mechanism
     * - We just need to transfer control with proper setup
     */

    /* Print "LOAD" to indicate we're loading kernel */
    li      r11, 0x4C              /* 'L' */
    stb     r11, 0(r10)
    li      r11, 0x4F              /* 'O' */
    stb     r11, 0(r10)
    li      r11, 0x41              /* 'A' */
    stb     r11, 0(r10)
    li      r11, 0x44              /* 'D' */
    stb     r11, 0(r10)
    li      r11, 0x0A              /* '\n' */
    stb     r11, 0(r10)

    /*
     * Phase 4: Kernel Transfer
     *
     * Expected kernel location: 0x02000000
     * This address can be configured in the build system
     * For now, attempt jump - kernel should be loaded there
     */

    /* Disable exceptions temporarily to avoid interference */
    mfmsr   r12
    lis     r11, 0xFFFF
    ori     r11, r11, 0xFFFE       /* Clear MSR bits except ME */
    and     r12, r12, r11
    mtmsr   r12

    /* Flush cache to ensure consistency */
    isync

    /*
     * Method 1: Direct jump to kernel at 0x02000000
     * Kernel entry expects:
     * - r3 = OF entry point (we have in r31)
     * - r4 = boot args (we have in r30)
     * - Stack = initialized (we did above)
     */

    lis     r12, 0x0200            /* Load kernel address 0x02000000 */
    ori     r12, r12, 0x0000
    mtctr   r12                    /* Move to CTR register */

    /* Prepare kernel arguments */
    mr      r3, r31                /* r3 = OF entry point */
    mr      r4, r30                /* r4 = boot args */

    /* Print "GO" before jump */
    lis     r10, 0xF020
    ori     r10, r10, 0x0000
    li      r11, 0x47              /* 'G' */
    stb     r11, 0(r10)
    li      r11, 0x4F              /* 'O' */
    stb     r11, 0(r10)
    li      r11, 0x0A              /* '\n' */
    stb     r11, 0(r10)

    /* Jump to kernel via CTR */
    bctr

    /*
     * If we get here, kernel jumped back (shouldn't happen)
     * Print error and loop
     */

.bootloader_error:
    lis     r10, 0xF020
    ori     r10, r10, 0x0000
    li      r11, 0x45              /* 'E' */
    stb     r11, 0(r10)
    li      r11, 0x52              /* 'R' */
    stb     r11, 0(r10)
    li      r11, 0x52              /* 'R' */
    stb     r11, 0(r10)
    li      r11, 0x0A              /* '\n' */
    stb     r11, 0(r10)

.bootloader_loop:
    nop
    b       .bootloader_loop

.size _start, .-_start
