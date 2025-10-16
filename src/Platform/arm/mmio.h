/*
 * ARM Memory-Mapped I/O Operations
 * Platform-specific memory access for ARM-based Raspberry Pi systems
 */

#ifndef ARM_MMIO_H
#define ARM_MMIO_H

#include <stdint.h>

/* ARM memory-mapped I/O read/write operations */

/* 32-bit reads/writes */
static inline uint32_t mmio_read32(uint32_t addr) {
    return *(volatile uint32_t *)addr;
}

static inline void mmio_write32(uint32_t addr, uint32_t value) {
    *(volatile uint32_t *)addr = value;
}

/* 16-bit reads/writes */
static inline uint16_t mmio_read16(uint32_t addr) {
    return *(volatile uint16_t *)addr;
}

static inline void mmio_write16(uint32_t addr, uint16_t value) {
    *(volatile uint16_t *)addr = value;
}

/* 8-bit reads/writes */
static inline uint8_t mmio_read8(uint32_t addr) {
    return *(volatile uint8_t *)addr;
}

static inline void mmio_write8(uint32_t addr, uint8_t value) {
    *(volatile uint8_t *)addr = value;
}

/* Set bits in 32-bit register */
static inline void mmio_set_bits(uint32_t addr, uint32_t mask) {
    uint32_t value = mmio_read32(addr);
    value |= mask;
    mmio_write32(addr, value);
}

/* Clear bits in 32-bit register */
static inline void mmio_clear_bits(uint32_t addr, uint32_t mask) {
    uint32_t value = mmio_read32(addr);
    value &= ~mask;
    mmio_write32(addr, value);
}

/* Read-modify-write with mask */
static inline void mmio_modify(uint32_t addr, uint32_t mask, uint32_t value) {
    uint32_t current = mmio_read32(addr);
    current = (current & ~mask) | (value & mask);
    mmio_write32(addr, current);
}

#endif /* ARM_MMIO_H */
