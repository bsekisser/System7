/* Placeholder PowerPC boot strap.
 *
 * The long-term goal is to accept an Open Firmware entry point, stitch in
 * device tree discovery, and pass a boot argument block down to boot_main.
 * For now we simply zero the argument registers and jump into boot_main so
 * the kernel can continue bringing up higher level systems.
 */

    .section ".bss"
    .align 5
stack_bottom:
    .space 0x10000
stack_top:

    .section ".text"
    .globl _start
    .extern boot_main
    .extern __bss_start
    .extern __bss_end
    .type _start,@function
_start:
    /* Align and prime the stack pointer (r1) */
    lis     r1, stack_top@ha
    addi    r1, r1, stack_top@l
    addi    r1, r1, -16
    rlwinm  r1, r1, 0, 0, 27          /* Align to 16 bytes */

    /* Preserve incoming Open Firmware argument (r3) for C side */
    mr      r31, r3                   /* Save boot arg pointer */

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

    /* Prepare arguments for boot_main(magic, boot_arg) */
    li      r3, 0                     /* No Multiboot magic on PPC */
    mr      r4, r31                   /* Pass firmware argument pointer */

    bl      boot_main

/* If boot_main returns just spin so we can spot it in a debugger. */
1:
    b       1b

    .size _start, .-_start
