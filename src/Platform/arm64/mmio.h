/*
 * ARM64 Memory-Mapped I/O Operations
 * Platform-specific memory access for ARM64/AArch64 systems
 */

#ifndef ARM64_MMIO_H
#define ARM64_MMIO_H

#include <stdint.h>

/* Memory barriers for ARM64
 * Essential for correct ordering on multi-core systems */

/* Data Memory Barrier - ensures all memory accesses before complete */
static inline void dmb(void) {
    __asm__ volatile("dmb sy" ::: "memory");
}

/* Data Synchronization Barrier - ensures all instructions before complete */
static inline void dsb(void) {
    __asm__ volatile("dsb sy" ::: "memory");
}

/* Instruction Synchronization Barrier - flushes pipeline */
static inline void isb(void) {
    __asm__ volatile("isb" ::: "memory");
}

/* ARM64 memory-mapped I/O read/write operations with barriers */

/* 64-bit reads/writes */
static inline uint64_t mmio_read64(uint64_t addr) {
    uint64_t value = *(volatile uint64_t *)addr;
    dmb();  /* Ensure read completes before subsequent operations */
    return value;
}

static inline void mmio_write64(uint64_t addr, uint64_t value) {
    dmb();  /* Ensure previous operations complete before write */
    *(volatile uint64_t *)addr = value;
    dmb();  /* Ensure write completes */
}

/* 32-bit reads/writes */
static inline uint32_t mmio_read32(uint64_t addr) {
    uint32_t value = *(volatile uint32_t *)addr;
    dmb();
    return value;
}

static inline void mmio_write32(uint64_t addr, uint32_t value) {
    dmb();
    *(volatile uint32_t *)addr = value;
    dmb();
}

/* 16-bit reads/writes */
static inline uint16_t mmio_read16(uint64_t addr) {
    uint16_t value = *(volatile uint16_t *)addr;
    dmb();
    return value;
}

static inline void mmio_write16(uint64_t addr, uint16_t value) {
    dmb();
    *(volatile uint16_t *)addr = value;
    dmb();
}

/* 8-bit reads/writes */
static inline uint8_t mmio_read8(uint64_t addr) {
    uint8_t value = *(volatile uint8_t *)addr;
    dmb();
    return value;
}

static inline void mmio_write8(uint64_t addr, uint8_t value) {
    dmb();
    *(volatile uint8_t *)addr = value;
    dmb();
}

/* Set bits in 32-bit register */
static inline void mmio_set_bits32(uint64_t addr, uint32_t mask) {
    uint32_t value = mmio_read32(addr);
    value |= mask;
    mmio_write32(addr, value);
}

/* Clear bits in 32-bit register */
static inline void mmio_clear_bits32(uint64_t addr, uint32_t mask) {
    uint32_t value = mmio_read32(addr);
    value &= ~mask;
    mmio_write32(addr, value);
}

/* Read-modify-write with mask */
static inline void mmio_modify32(uint64_t addr, uint32_t mask, uint32_t value) {
    uint32_t current = mmio_read32(addr);
    current = (current & ~mask) | (value & mask);
    mmio_write32(addr, current);
}

/* Set bits in 64-bit register */
static inline void mmio_set_bits64(uint64_t addr, uint64_t mask) {
    uint64_t value = mmio_read64(addr);
    value |= mask;
    mmio_write64(addr, value);
}

/* Clear bits in 64-bit register */
static inline void mmio_clear_bits64(uint64_t addr, uint64_t mask) {
    uint64_t value = mmio_read64(addr);
    value &= ~mask;
    mmio_write64(addr, value);
}

#endif /* ARM64_MMIO_H */
