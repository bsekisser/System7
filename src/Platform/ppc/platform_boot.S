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

    /* Setup UART for direct I/O (no function calls!) */
    lis     r10, 0xF020               /* Load UART base 0xF0200000 */
    ori     r10, r10, 0x0000

    /* Initialize 16550 UART - DIRECT WRITES */
    li      r11, 0x00
    stb     r11, 1(r10)               /* Disable interrupts (offset 1) */
    li      r11, 0x80
    stb     r11, 3(r10)               /* Enable DLAB (offset 3) */
    li      r11, 0x01
    stb     r11, 0(r10)               /* Divisor lo = 1 (115200) */
    li      r11, 0x00
    stb     r11, 1(r10)               /* Divisor hi = 0 */
    li      r11, 0x03
    stb     r11, 3(r10)               /* LCR: 8N1 */
    li      r11, 0x07
    stb     r11, 2(r10)               /* FCR: Enable FIFO */
    li      r11, 0x03
    stb     r11, 4(r10)               /* MCR: RTS/DTR */

    /* Write boot banner directly */
    li      r11, 0x53                 /* 'S' */
    stb     r11, 0(r10)
    li      r11, 0x37                 /* '7' */
    stb     r11, 0(r10)
    li      r11, 0x20                 /* ' ' */
    stb     r11, 0(r10)
    li      r11, 0x42                 /* 'B' - Boot marker */
    stb     r11, 0(r10)
    li      r11, 0x0A                 /* '\n' */
    stb     r11, 0(r10)
    li      r11, 0x0D                 /* '\r' */
    stb     r11, 0(r10)

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

    /* Call Open Firmware early initialization */
    mr      r3, r31
    bl      ofw_early_init

    /* Emit boot banner if OF is available */
    mr      r3, r31
    bl      ofw_boot_banner

    /* Prepare arguments for boot_main(magic, boot_arg) */
    li      r3, 0                     /* No Multiboot magic on PPC */
    mr      r4, r31                   /* Pass firmware argument pointer */

    bl      boot_main

/* If boot_main returns, just spin so we can spot it in a debugger. */
1:
    b       1b

    .size _start, .-_start

