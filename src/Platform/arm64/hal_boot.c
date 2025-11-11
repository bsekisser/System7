/*
 * ARM64 Bootloader HAL Implementation
 * Raspberry Pi 3/4/5 AArch64 boot initialization
 */

#include <stdint.h>
#include <stddef.h>
#include "uart.h"
#include "timer.h"
#include "dtb.h"

#ifndef QEMU_BUILD
#include "mailbox.h"
#include "gic.h"
#endif

/* Minimal snprintf declaration */
extern int snprintf(char *str, size_t size, const char *format, ...);
extern char *strncpy(char *dest, const char *src, size_t n);

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

    /* Initialize exception handlers */
    extern void exceptions_init(void);
    exceptions_init();

    /* Save DTB address */
    boot_info.dtb_address = (uint64_t)dtb_ptr;

    /* Early serial output */
    uart_puts("\n");
    uart_puts("[ARM64] ==========================================================\n");
    uart_puts("[ARM64] System 7.1 Portable - ARM64/AArch64 Boot\n");
    uart_puts("[ARM64] ==========================================================\n");

    /* Report exception level */
    uint64_t current_el;
    __asm__ volatile("mrs %0, CurrentEL" : "=r"(current_el));
    current_el = (current_el >> 2) & 3;

    uart_puts("[ARM64] Running at Exception Level: EL");
    if (current_el == 0) uart_puts("0\n");
    else if (current_el == 1) uart_puts("1\n");
    else if (current_el == 2) uart_puts("2\n");
    else uart_puts("3\n");

    /* Report DTB location */
    if (dtb_ptr) {
        uart_puts("[ARM64] Device Tree Blob provided\n");
    } else {
        uart_puts("[ARM64] Warning: No Device Tree provided\n");
    }

    /* Report timer frequency */
    uart_puts("[ARM64] Timer initialized\n");

    /* Initialize DTB parser */
    uart_puts("[ARM64] Checking Device Tree...\n");
    if (dtb_ptr) {
        uart_puts("[ARM64] DTB pointer provided, attempting init...\n");
        if (dtb_init(dtb_ptr)) {
            uart_puts("[ARM64] Device Tree initialized\n");

            /* Skip model and memory parsing for now - just use defaults */
            boot_info.memory_base = 0x00000000;
            boot_info.memory_size = 1024 * 1024 * 1024;  /* 1GB default */
        } else {
            uart_puts("[ARM64] DTB init failed\n");
            boot_info.memory_base = 0x00000000;
            boot_info.memory_size = 1024 * 1024 * 1024;  /* 1GB default */
        }
    } else {
        uart_puts("[ARM64] No DTB pointer\n");
        uart_puts("[ARM64] Setting default memory base...\n");
        boot_info.memory_base = 0x00000000;
        uart_puts("[ARM64] Setting default memory size...\n");
        boot_info.memory_size = 1024 * 1024 * 1024;  /* 1GB default */
        uart_puts("[ARM64] Default memory set\n");
    }

    uart_puts("[ARM64] Memory setup complete\n");

#ifndef QEMU_BUILD
    /* Initialize mailbox (not available in QEMU virt) */
    if (mailbox_init()) {
        uart_puts("[ARM64] Mailbox initialized\n");

        /* Get board revision */
        uint32_t revision;
        if (mailbox_get_board_revision(&revision)) {
            boot_info.board_revision = revision;
            uart_puts("[ARM64] Board Revision detected\n");
        }

        /* Get ARM memory info from mailbox as well */
        uint32_t arm_base, arm_size;
        if (mailbox_get_arm_memory(&arm_base, &arm_size)) {
            uart_puts("[ARM64] ARM Memory info from mailbox\n");
        }
    }

    /* Initialize GIC (not available in QEMU virt) */
    if (gic_init()) {
        uart_puts("[ARM64] GIC interrupt controller initialized\n");
    }
#else
    uart_puts("[ARM64] Running in QEMU - skipping mailbox and GIC\n");
#endif

    /* Report processor features */
    uint64_t midr_el1;
    __asm__ volatile("mrs %0, midr_el1" : "=r"(midr_el1));
    uint32_t implementer = (midr_el1 >> 24) & 0xFF;
    uint32_t partnum = (midr_el1 >> 4) & 0xFFF;

    /* Detect Cortex-A53/A72/A76 */
    if (implementer == 0x41) {  /* ARM */
        uart_puts("[ARM64] CPU: ");
        if (partnum == 0xD03) uart_puts("Cortex-A53\n");
        else if (partnum == 0xD08) uart_puts("Cortex-A72\n");
        else if (partnum == 0xD0B) uart_puts("Cortex-A76\n");
        else uart_puts("ARM CPU\n");
    } else {
        uart_puts("[ARM64] CPU detected\n");
    }

    /* Initialize MMU with fixed TCR configuration */
    extern bool mmu_init(void);
    extern void mmu_enable(void);

    uart_puts("[ARM64] Initializing MMU...\n");
    if (mmu_init()) {
        uart_puts("[ARM64] MMU page tables configured\n");
        mmu_enable();
        uart_puts("[ARM64] MMU enabled - virtual memory active\n");
    } else {
        uart_puts("[ARM64] MMU init failed\n");
    }

    uart_puts("[ARM64] Early boot complete, entering kernel...\n");
    uart_puts("[ARM64] ==========================================================\n");

    /* Jump to main kernel entry point */
    uart_puts("[ARM64] About to call main()...\n");
    extern int main(int argc, char **argv);
    main(0, NULL);
    uart_puts("[ARM64] main() returned\n");

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
