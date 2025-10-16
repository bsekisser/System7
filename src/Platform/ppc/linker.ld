/* Linker script skeleton for the PowerPC port.
 *
 * The binary currently boots at 0x01000000 to leave room for firmware, but this
 * will be revisited once we target a concrete machine model.
 */

ENTRY(_start)

SECTIONS
{
    /* Tentative load address; Open Firmware will relocate as needed. */
    . = 0x01000000;

    .boot ALIGN(4K) :
    {
        KEEP(build/obj/platform_boot.o(.text))
    }

    .text ALIGN(4K) :
    {
        *(.text)
        *(.text.*)
    }

    .rodata ALIGN(4K) :
    {
        *(.rodata)
        *(.rodata.*)
    }

    .data ALIGN(4K) :
    {
        *(.data)
        *(.data.*)
    }

    .bss ALIGN(4K) :
    {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        *(.bss.*)
        __bss_end = .;
    }

    kernel_end = .;
}
