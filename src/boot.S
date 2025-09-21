/* Minimal Multiboot2 + kernel boot */
.section .multiboot
.align 8
multiboot_header:
    .long 0xe85250d6                /* magic */
    .long 0                          /* architecture: i386 */
    .long multiboot_header_end - multiboot_header
    .long -(0xe85250d6 + 0 + (multiboot_header_end - multiboot_header))

    /* Framebuffer tag */
    .align 8
    .short 5
    .short 0
    .long 20
    .long 1024
    .long 768
    .long 32

    /* End tag */
    .align 8
    .short 0
    .short 0
    .long 8
multiboot_header_end:

.section .text
.global _start
_start:
    /* Setup stack */
    movl $stack_top, %esp

    /* Call kernel_main */
    push %ebx
    push %eax
    call kernel_main

    /* Hang */
hang:
    cli
    hlt
    jmp hang

.section .bss
.align 16
stack_bottom:
    .space 16384
stack_top: