#include "Platform/include/boot.h"
#include "SystemInternal.h"
#include "System71StdLib.h"
#include <stdint.h>

/* Forward declaration of kernel main */
extern void kernel_main(uint32_t magic, uint32_t* mb2_info);

/* Direct UART output for early boot (no dependencies) */
static inline void uart_putchar(char c) {
    /* UART at 0xF0200000 on PowerPC QEMU */
    volatile unsigned char *uart = (volatile unsigned char *)0xF0200000;
    /* Wait for TX empty */
    while ((uart[5] & 0x20) == 0) {}
    /* Send character */
    uart[0] = c;
}

static void uart_puts(const char *s) {
    while (*s) {
        uart_putchar(*s++);
    }
}

void boot_main(uint32_t magic, uint32_t* mb2_info) {
    /* Use serial_puts directly to bypass logging system during early boot */
    extern void serial_puts(const char* str);

    serial_puts("BOOT:M\n");  /* Entry marker */

    hal_boot_init(mb2_info);
    serial_puts("BOOT:H\n");  /* After hal_boot_init */

    Serial_WriteString("BOOT\n");
    serial_puts("BOOT:S\n");  /* After Serial_WriteString ("BOOT") */

    serial_puts("BOOT:K\n");  /* Before kernel_main */
    /* Call the kernel main function */
    kernel_main(magic, mb2_info);
    serial_puts("BOOT:EXIT\n");  /* If kernel_main returns */
}
