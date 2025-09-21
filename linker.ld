/* Linker script for System 7.1 kernel with Multiboot2 */

ENTRY(_start)

SECTIONS
{
    /* Kernel loads at 1MB */
    . = 0x100000;

    /* Multiboot2 header must be at the beginning */
    .multiboot2 ALIGN(8) :
    {
        KEEP(*(.multiboot2))
    }

    /* Text section */
    .text ALIGN(4K) :
    {
        KEEP(build/obj/multiboot2.o(.text))  /* Ensure _start comes first */
        *(.text)
        *(.text.*)
    }

    /* Read-only data */
    .rodata ALIGN(4K) :
    {
        *(.rodata)
        *(.rodata.*)
    }

    /* Data section */
    .data ALIGN(4K) :
    {
        *(.data)
        *(.data.*)
    }

    /* BSS section */
    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /* Kernel end marker */
    kernel_end = .;
}