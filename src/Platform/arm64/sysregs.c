/*
 * ARM64 System Register Inspection
 * Display useful system configuration
 */

#include <stdint.h>
#include "uart.h"

/*
 * Display CPU features from ID registers
 */
void sysregs_show_cpu_features(void) {
    uint64_t aa64pfr0, aa64mmfr0, aa64isar0;

    /* Read ID registers */
    __asm__ volatile("mrs %0, id_aa64pfr0_el1" : "=r"(aa64pfr0));
    __asm__ volatile("mrs %0, id_aa64mmfr0_el1" : "=r"(aa64mmfr0));
    __asm__ volatile("mrs %0, id_aa64isar0_el1" : "=r"(aa64isar0));

    uart_puts("\n[SYSREGS] ARM64 CPU Features:\n");

    /* Check EL0/EL1 support */
    uint8_t el0 = aa64pfr0 & 0xF;
    uint8_t el1 = (aa64pfr0 >> 4) & 0xF;
    uart_puts("[SYSREGS] EL0: ");
    if (el0 == 1) uart_puts("AArch64 only\n");
    else if (el0 == 2) uart_puts("AArch64 and AArch32\n");
    else uart_puts("Unknown\n");

    uart_puts("[SYSREGS] EL1: ");
    if (el1 == 1) uart_puts("AArch64 only\n");
    else if (el1 == 2) uart_puts("AArch64 and AArch32\n");
    else uart_puts("Unknown\n");

    /* Check FP/SIMD support */
    uint8_t fp = (aa64pfr0 >> 16) & 0xF;
    uint8_t asimd = (aa64pfr0 >> 20) & 0xF;

    uart_puts("[SYSREGS] FP: ");
    if (fp == 0) uart_puts("Supported\n");
    else if (fp == 1) uart_puts("FP and Half-precision\n");
    else uart_puts("Not supported\n");

    uart_puts("[SYSREGS] SIMD: ");
    if (asimd == 0) uart_puts("Supported\n");
    else if (asimd == 1) uart_puts("SIMD and Half-precision\n");
    else uart_puts("Not supported\n");

    /* Check crypto extensions */
    uint8_t aes = (aa64isar0 >> 4) & 0xF;
    uint8_t sha1 = (aa64isar0 >> 8) & 0xF;
    uint8_t sha2 = (aa64isar0 >> 12) & 0xF;

    uart_puts("[SYSREGS] AES: ");
    if (aes) uart_puts("Yes\n");
    else uart_puts("No\n");

    uart_puts("[SYSREGS] SHA1: ");
    if (sha1) uart_puts("Yes\n");
    else uart_puts("No\n");

    uart_puts("[SYSREGS] SHA2: ");
    if (sha2) uart_puts("Yes\n");
    else uart_puts("No\n");

    /* Check PA size */
    uint8_t parange = aa64mmfr0 & 0xF;
    uart_puts("[SYSREGS] Physical Address Size: ");
    if (parange == 0) uart_puts("32 bits (4GB)\n");
    else if (parange == 1) uart_puts("36 bits (64GB)\n");
    else if (parange == 2) uart_puts("40 bits (1TB)\n");
    else if (parange == 3) uart_puts("42 bits (4TB)\n");
    else if (parange == 4) uart_puts("44 bits (16TB)\n");
    else if (parange == 5) uart_puts("48 bits (256TB)\n");
    else if (parange == 6) uart_puts("52 bits (4PB)\n");
    else uart_puts("Unknown\n");
}

/*
 * Display cache information
 */
void sysregs_show_cache_info(void) {
    uint64_t ctr, clidr;

    __asm__ volatile("mrs %0, ctr_el0" : "=r"(ctr));
    __asm__ volatile("mrs %0, clidr_el1" : "=r"(clidr));

    uart_puts("\n[SYSREGS] Cache Information:\n");

    /* Get cache line sizes */
    uint32_t dminline = (ctr >> 16) & 0xF;
    uint32_t iminline = ctr & 0xF;

    uint32_t dcache_line = 4 << dminline;
    uint32_t icache_line = 4 << iminline;

    uart_puts("[SYSREGS] DCache Line Size: ");
    if (dcache_line == 64) uart_puts("64 bytes\n");
    else if (dcache_line == 32) uart_puts("32 bytes\n");
    else if (dcache_line == 128) uart_puts("128 bytes\n");
    else uart_puts("Unknown\n");

    uart_puts("[SYSREGS] ICache Line Size: ");
    if (icache_line == 64) uart_puts("64 bytes\n");
    else if (icache_line == 32) uart_puts("32 bytes\n");
    else if (icache_line == 128) uart_puts("128 bytes\n");
    else uart_puts("Unknown\n");

    /* Check cache levels */
    uint8_t loc = (clidr >> 24) & 0x7;
    uart_puts("[SYSREGS] Levels of Cache: ");
    if (loc == 1) uart_puts("L1 only\n");
    else if (loc == 2) uart_puts("L1 and L2\n");
    else if (loc == 3) uart_puts("L1, L2, and L3\n");
    else uart_puts("Unknown\n");
}

/*
 * Display current system state
 */
void sysregs_show_current_state(void) {
    uint64_t currentel, sp, sctlr, tcr, ttbr0;

    __asm__ volatile("mrs %0, currentel" : "=r"(currentel));
    __asm__ volatile("mov %0, sp" : "=r"(sp));
    __asm__ volatile("mrs %0, sctlr_el1" : "=r"(sctlr));
    __asm__ volatile("mrs %0, tcr_el1" : "=r"(tcr));
    __asm__ volatile("mrs %0, ttbr0_el1" : "=r"(ttbr0));

    uart_puts("\n[SYSREGS] Current System State:\n");

    uint8_t el = (currentel >> 2) & 3;
    uart_puts("[SYSREGS] Exception Level: EL");
    if (el == 0) uart_puts("0\n");
    else if (el == 1) uart_puts("1\n");
    else if (el == 2) uart_puts("2\n");
    else uart_puts("3\n");

    uart_puts("[SYSREGS] MMU: ");
    if (sctlr & 1) uart_puts("Enabled\n");
    else uart_puts("Disabled\n");

    uart_puts("[SYSREGS] DCache: ");
    if (sctlr & (1 << 2)) uart_puts("Enabled\n");
    else uart_puts("Disabled\n");

    uart_puts("[SYSREGS] ICache: ");
    if (sctlr & (1 << 12)) uart_puts("Enabled\n");
    else uart_puts("Disabled\n");

    uart_puts("[SYSREGS] Alignment Check: ");
    if (sctlr & (1 << 1)) uart_puts("Enabled\n");
    else uart_puts("Disabled\n");
}
