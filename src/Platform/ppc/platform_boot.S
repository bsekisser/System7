/* PowerPC boot strap for QEMU/firmware.
 *
 * Accepts an Open Firmware entry point (in r3) if available, falls back to
 * serial console if not. Sets up stack, zeros BSS, and hands control to boot_main.
 *
 * When running under QEMU:
 * - If QEMU provides Open Firmware: r3 contains OF entry point
 * - If QEMU doesn't provide OF: r3 will be 0, and serial console fallback is used
 */

    .section ".bss"
    .align 5
stack_bottom:
    .space 0x10000
stack_top:

    .section ".text"
    .globl _start
    .extern boot_main
    .extern ofw_early_init
    .extern ofw_boot_banner
    .extern __bss_start
    .extern __bss_end
    .type _start,@function
_start:
    /* CRITICAL: Direct output to verify _start is executing */
    /* This code has NO dependencies - pure assembly */

    /* Setup ESCC for direct I/O (no function calls!) */
    /* QEMU mac99 uses ESCC (Zilog Z8530) at 0x80013000 */
    lis     r10, 0x8001               /* Load ESCC base 0x80013000 */
    ori     r10, r10, 0x3000

    /* ESCC Initialization: Minimal setup for TX-only boot output */
    /* Channel A Control register is at offset +4, Data at offset +5 */

    /* Reset ESCC channel (write index 9, value 0xC0) */
    li      r11, 0x09                 /* Index: WR9 (reset) */
    stb     r11, 4(r10)               /* Write to control register */
    isync
    li      r11, 0xC0                 /* Reset both channels */
    stb     r11, 4(r10)               /* Write value to control register */
    isync

    /* Configure baud rate generator (115200 @ 14.7456MHz) */
    li      r11, 0x0E                 /* Index: WR14 (misc control) */
    stb     r11, 4(r10)
    isync
    li      r11, 0x03                 /* Enable BRG, source from crystal */
    stb     r11, 4(r10)
    isync

    /* Set TX/RX clock sources to BRG */
    li      r11, 0x0B                 /* Index: WR11 (clock mode) */
    stb     r11, 4(r10)
    isync
    li      r11, 0x50                 /* TX from BRG, RX from BRG */
    stb     r11, 4(r10)
    isync

    /* Set baud rate (0x00 for 115200 with 14.7456MHz) */
    li      r11, 0x0C                 /* Index: WR12 (BRG lo) */
    stb     r11, 4(r10)
    isync
    li      r11, 0x00                 /* Low byte */
    stb     r11, 4(r10)
    isync

    /* Configure mode: 8N1, x16 clock */
    li      r11, 0x04                 /* Index: WR4 (misc) */
    stb     r11, 4(r10)
    isync
    li      r11, 0x44                 /* x16 clock, 1 stop bit, 8-bit */
    stb     r11, 4(r10)
    isync

    /* Disable interrupts */
    li      r11, 0x01                 /* Index: WR1 (interrupt control) */
    stb     r11, 4(r10)
    isync
    li      r11, 0x00                 /* Disable all interrupts */
    stb     r11, 4(r10)
    isync

    /* Enable receiver and transmitter */
    li      r11, 0x03                 /* Index: WR3 (RX control) */
    stb     r11, 4(r10)
    isync
    li      r11, 0xC1                 /* Enable RX, 8-bit, no auto modes */
    stb     r11, 4(r10)
    isync

    li      r11, 0x05                 /* Index: WR5 (TX control) */
    stb     r11, 4(r10)
    isync
    li      r11, 0xEA                 /* Enable TX, assert DTR/RTS, 8-bit */
    stb     r11, 4(r10)
    isync

    /* Reset index to RR0 for status reads */
    li      r11, 0x00                 /* Index: RR0 (status) */
    stb     r11, 4(r10)
    isync

    /* Write boot banner to ESCC Channel A data register */
    /* Use r9 for data writes, r10 for channel base */
    li      r9, 0x3A                  /* ':' - entry marker */
    bl      boot_putchar_escc
    li      r9, 0x53                  /* 'S' */
    bl      boot_putchar_escc
    li      r9, 0x37                  /* '7' */
    bl      boot_putchar_escc
    li      r9, 0x20                  /* ' ' */
    bl      boot_putchar_escc
    li      r9, 0x42                  /* 'B' - Boot marker */
    bl      boot_putchar_escc
    li      r9, 0x0A                  /* '\n' */
    bl      boot_putchar_escc
    li      r9, 0x0D                  /* '\r' */
    bl      boot_putchar_escc

    b       boot_start_continue

/* Helper function: boot_putchar_escc
 * Input: r9 = character to write, r10 = ESCC base (0x80013000)
 * Output: none
 * Clobbers: r11, r12
 */
boot_putchar_escc:
    /* Wait for TX buffer empty (bit 2 of RR0 status register) */
    li      r12, 100000                /* Spin limit */
.wait_tx_ready:
    cmpwi   r12, 0
    ble     .tx_ready_done             /* Bail if timeout */
    addi    r12, r12, -1

    li      r11, 0x00                  /* Set index to RR0 (status) */
    stb     r11, 4(r10)                /* Write to control register */
    isync
    lbz     r11, 4(r10)                /* Read status from control register */
    isync

    andi.   r11, r11, 0x04             /* Check TX buffer empty bit (bit 2) */
    beq     .wait_tx_ready             /* Not ready yet, loop */

.tx_ready_done:
    /* Write character to data register */
    stb     r9, 5(r10)                 /* Write to data register */
    isync
    blr

boot_start_continue:
    /* Align and prime the stack pointer (r1) */
    lis     r1, stack_top@ha
    addi    r1, r1, stack_top@l
    addi    r1, r1, -16
    rlwinm  r1, r1, 0, 0, 27          /* Align to 16 bytes */

    /* Save Open Firmware callback pointer (r5 per IEEE 1275 calling convention)
     * When booted by OF via 'boot' command, r5 holds the OF entry point
     * r3/r4 hold initrd address/length (or 0 if not present)
     */
    mr      r31, r5                   /* Save OF callback pointer in r31 */
    mr      r6, r3                    /* r6 = initrd address (if any) */
    mr      r7, r4                    /* r7 = initrd length (if any) */

    /* Zero out .bss so C globals start from a known state */
    li      r0, 0
    lis     r5, __bss_start@ha
    addi    r5, r5, __bss_start@l
    lis     r6, __bss_end@ha
    addi    r6, r6, __bss_end@l
1:
    cmpw    r5, r6
    bge     2f
    stw     r0, 0(r5)
    addi    r5, r5, 4
    b       1b
2:

    /* DEBUG: Write marker 'A' before OF init */
    lis     r10, 0x8001               /* Load ESCC base */
    ori     r10, r10, 0x3000
    li      r9, 0x41                  /* 'A' - about to call ofw_early_init */
    bl      boot_putchar_escc

    /* Call Open Firmware early initialization */
    mr      r3, r31
    bl      ofw_early_init

    /* DEBUG: Write marker 'B' after OF init */
    lis     r10, 0x8001               /* Load ESCC base */
    ori     r10, r10, 0x3000
    li      r9, 0x42                  /* 'B' - ofw_early_init returned */
    bl      boot_putchar_escc

    /* Emit boot banner if OF is available */
    mr      r3, r31
    bl      ofw_boot_banner

    /* DEBUG: Write marker 'C' after boot banner */
    lis     r10, 0x8001               /* Load ESCC base */
    ori     r10, r10, 0x3000
    li      r9, 0x43                  /* 'C' - ofw_boot_banner returned */
    bl      boot_putchar_escc

    /* Prepare arguments for boot_main(magic, boot_arg) */
    li      r3, 0                     /* No Multiboot magic on PPC */
    mr      r4, r31                   /* Pass firmware argument pointer */

    /* DEBUG: Write marker 'D' before boot_main */
    lis     r10, 0x8001               /* Load ESCC base */
    ori     r10, r10, 0x3000
    li      r9, 0x44                  /* 'D' - about to call boot_main */
    bl      boot_putchar_escc

    bl      boot_main

    /* DEBUG: Write marker 'E' after boot_main (if it returns) */
    lis     r10, 0x8001               /* Load ESCC base */
    ori     r10, r10, 0x3000
    li      r9, 0x45                  /* 'E' - boot_main returned (shouldn't happen) */
    bl      boot_putchar_escc

/* If boot_main returns, just spin so we can spot it in a debugger. */
1:
    b       1b

    .size _start, .-_start
