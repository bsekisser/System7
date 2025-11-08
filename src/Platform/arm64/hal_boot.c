/*
 * ARM64 Bootloader HAL Implementation
 * Raspberry Pi 3/4/5 AArch64 boot initialization
 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "uart.h"
#include "timer.h"

/* ARM64-specific boot information */
typedef struct {
    uint64_t dtb_address;
    uint64_t memory_base;
    uint64_t memory_size;
    uint32_t board_revision;
    char board_model[256];
} arm64_boot_info_t;

static arm64_boot_info_t boot_info = {0};

/*
 * Early ARM64 boot entry from assembly
 * Called with DTB address in x0
 */
void arm64_boot_main(void *dtb_ptr) {
    char buf[256];

    /* Initialize UART for early serial output */
    uart_init();

    /* Initialize timer */
    timer_init();

    /* Save DTB address */
    boot_info.dtb_address = (uint64_t)dtb_ptr;

    /* Early serial output */
    uart_puts("\n");
    uart_puts("[ARM64] ══════════════════════════════════════════════════════════\n");
    uart_puts("[ARM64] System 7.1 Portable - ARM64/AArch64 Boot\n");
    uart_puts("[ARM64] ══════════════════════════════════════════════════════════\n");

    /* Report exception level */
    uint64_t current_el;
    __asm__ volatile("mrs %0, CurrentEL" : "=r"(current_el));
    current_el = (current_el >> 2) & 3;
    snprintf(buf, sizeof(buf), "[ARM64] Running at Exception Level: EL%llu\n", current_el);
    uart_puts(buf);

    /* Report DTB location */
    if (dtb_ptr) {
        snprintf(buf, sizeof(buf), "[ARM64] Device Tree Blob at: 0x%016llx\n", (uint64_t)dtb_ptr);
        uart_puts(buf);
    } else {
        uart_puts("[ARM64] Warning: No Device Tree provided\n");
    }

    /* Report timer frequency */
    uint64_t timer_freq = timer_get_freq();
    snprintf(buf, sizeof(buf), "[ARM64] Timer frequency: %llu Hz\n", timer_freq);
    uart_puts(buf);

    /* Set default memory parameters
     * Will be updated from DTB parsing */
    boot_info.memory_base = 0x00000000;
    boot_info.memory_size = 1024 * 1024 * 1024;  /* 1GB default */

    /* Report processor features */
    uint64_t midr_el1;
    __asm__ volatile("mrs %0, midr_el1" : "=r"(midr_el1));
    uint32_t implementer = (midr_el1 >> 24) & 0xFF;
    uint32_t variant = (midr_el1 >> 20) & 0xF;
    uint32_t architecture = (midr_el1 >> 16) & 0xF;
    uint32_t partnum = (midr_el1 >> 4) & 0xFFF;
    uint32_t revision = midr_el1 & 0xF;

    snprintf(buf, sizeof(buf), "[ARM64] CPU Implementer: 0x%02x Variant: 0x%x Arch: 0x%x\n",
             implementer, variant, architecture);
    uart_puts(buf);
    snprintf(buf, sizeof(buf), "[ARM64] CPU Part: 0x%03x Revision: 0x%x\n", partnum, revision);
    uart_puts(buf);

    /* Detect Cortex-A53/A72/A76 */
    if (implementer == 0x41) {  /* ARM */
        const char *cpu_name = "Unknown ARM CPU";
        if (partnum == 0xD03) cpu_name = "Cortex-A53";
        else if (partnum == 0xD08) cpu_name = "Cortex-A72";
        else if (partnum == 0xD0B) cpu_name = "Cortex-A76";
        snprintf(buf, sizeof(buf), "[ARM64] Detected: %s\n", cpu_name);
        uart_puts(buf);
    }

    uart_puts("[ARM64] Early boot complete, entering kernel...\n");
    uart_puts("[ARM64] ══════════════════════════════════════════════════════════\n");

    /* Jump to main kernel entry point */
    extern int main(int argc, char **argv);
    main(0, NULL);

    /* Should not return */
    while (1) {
        __asm__ volatile("wfe");
    }
}

/*
 * Get detected memory size
 */
uint64_t hal_get_memory_size(void) {
    return boot_info.memory_size;
}

/*
 * Get DTB address
 */
void *hal_get_dtb_address(void) {
    return (void *)boot_info.dtb_address;
}
