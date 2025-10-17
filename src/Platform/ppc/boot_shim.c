/*
 * boot_shim.c - PowerPC Boot Shim for OpenBIOS Compatibility
 *
 * This shim runs at the kernel entry point (0x01000000) and handles the
 * transition from OpenBIOS to the actual System 7 kernel.
 *
 * The issue: OpenBIOS loads the kernel but doesn't properly transfer control.
 * The solution: This shim acts as the real entry point and properly initializes
 * the environment before calling boot_main.
 */

#include <stdint.h>
#include <stddef.h>

/* Forward declarations */
extern void boot_main(uint32_t magic, uint32_t* boot_arg);
extern void kernel_main(uint32_t magic, uint32_t* mb2_info);

/* Early output for debugging */
static volatile uint8_t* uart_base = (volatile uint8_t*)0xF0200000;

static void uart_putchar(char c) {
    if (!uart_base) return;

    /* Wait for THRE (Transmitter Holding Register Empty) */
    int timeout = 100000;
    while ((uart_base[5] & 0x20) == 0 && timeout-- > 0);

    uart_base[0] = (uint8_t)c;
}

static void uart_puts(const char *str) {
    while (str && *str) {
        uart_putchar(*str);
        if (*str == '\n') {
            uart_putchar('\r');
        }
        str++;
    }
}

/*
 * Boot entry point - called by OpenBIOS
 *
 * Register state on entry:
 * r3 = OpenFirmware entry point (device tree pointer or OF entry)
 * r1 = stack pointer (should be set up by OpenBIOS)
 */
void boot_shim(void) {
    uint32_t magic = 0;      /* No Multiboot magic on PPC */
    uint32_t *boot_arg = 0;  /* Will be set from r3 via asm */

    /* Initialize UART for early output */
    uart_base[1] = 0x00;   /* Disable interrupts */
    uart_base[3] = 0x80;   /* Enable DLAB */
    uart_base[0] = 0x01;   /* Divisor lo for 115200 */
    uart_base[1] = 0x00;   /* Divisor hi */
    uart_base[3] = 0x03;   /* 8N1 */
    uart_base[2] = 0x07;   /* Enable FIFO */
    uart_base[4] = 0x03;   /* RTS/DTR */

    uart_puts("\n");
    uart_puts("================================================\n");
    uart_puts("BOOT: System 7 PowerPC Boot Shim Active\n");
    uart_puts("================================================\n");
    uart_puts("BOOT: Calling boot_main...\n");

    /* Call the actual boot main function */
    boot_main(magic, boot_arg);

    /* Should not return */
    uart_puts("ERROR: boot_main returned!\n");

    /* Hang */
    while(1);
}

/*
 * This assembly wrapper ensures boot_shim is called with the correct
 * register setup, and properly preserves r3 (OF entry point).
 */
void boot_shim_asm(void) __asm__("_start");
void boot_shim_asm(void) {
    __asm__ volatile(
        "mr r31, r3\n"           /* Save OF entry point */
        "bl boot_shim\n"         /* Call C boot shim */
        "1: b 1b\n"              /* Hang if we return */
    );
}
