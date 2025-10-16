/* ARM Bootstrap for System 7.1 Kernel
 * Raspberry Pi 3/4/5 ARMv7-A
 *
 * Entry point: typically at 0x8000
 * Bootloader provides:
 *   r0 = 0 (reserved)
 *   r1 = Machine type ID (for older firmware)
 *   r2 = Device Tree Blob (DTB) physical address
 *   r3 = 0 (reserved)
 *
 * References:
 *   ARM EABI (Embedded ABI)
 *   ARM Generic Interrupt Controller (GIC) - optional
 *   ARM generic timer
 */

/* .section .text.boot must be first, before regular .text */
.section .text.boot
.global _start
.align 4

_start:
    /* Save boot parameters before any stack operations
     * r0 = 0 (reserved)
     * r1 = machine ID
     * r2 = DTB address <- Important!
     * r3 = 0 (reserved)
     */
    mov r7, r2              /* Save DTB address in r7 (callee-saved) */

    /* Disable interrupts */
    cpsid if                /* Disable IRQ and FIQ */

    /* Set up stack
     * Stack grows downward on ARM
     * Place stack at end of available memory
     */
    ldr sp, =stack_top

    /* Clear BSS section
     * Bootloader doesn't clear BSS, we must do it
     */
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0

.clear_bss:
    cmp r0, r1
    beq bss_done
    str r2, [r0], #4
    b .clear_bss

bss_done:
    /* Initialize minimal state for C environment
     * ARM EABI requires:
     * - Stack 8-byte aligned
     * - No assumptions about registers except sp
     */
    mov r0, #0
    mov r1, #0
    mov r2, #0
    mov r3, #0
    mov r4, #0
    mov r5, #0
    mov r6, #0
    /* r7 = DTB address (saved above) */
    mov r8, #0
    mov r9, #0
    mov r10, #0
    mov r11, #0
    mov r12, #0
    /* r13 = sp (stack pointer) - already set */
    mov r14, #0             /* lr = link register */
    /* r15 = pc (program counter) - cannot write directly */

    /* Align stack to 8-byte boundary
     * ARM calling convention requires sp % 8 == 0
     */
    bic sp, sp, #7

    /* Push return address for later */
    sub sp, sp, #4
    ldr r0, =halt
    str r0, [sp]

    /* Call boot initialization with DTB address as argument
     * ARM EABI calling convention: first arg in r0, second in r1, etc.
     */
    mov r0, r7              /* r0 = DTB address */
    bl boot_main            /* Call boot_main(dtb_addr) */

    /* Should not return from boot_main
     * If it does, halt
     */
halt:
    wfe                     /* Wait for event (low power) */
    b halt

/* Stack definition
 * Place in BSS section (uninitialized data)
 */
.section .bss
.align 4
stack_bottom:
    .space 65536            /* 64KB stack */
stack_top:

/* Mark stack as non-executable for security
 * This is a note section that some tools recognize
 */
.section .note.GNU-stack,"",@progbits

/* Export labels for linker script and debugging */
.global _kernel_entry
_kernel_entry = _start
