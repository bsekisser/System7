/*
 * ARM Memory-Mapped I/O Implementation
 * Provides actual implementations for MMIO operations
 * Most are inlined in mmio.h, but this file provides any non-inlined versions
 */

#include <stdint.h>
#include "mmio.h"

/* Currently all MMIO operations are inlined in mmio.h
 * This file exists for future expansion of non-inlined operations
 */

/*
 * Busywait for a duration
 * Useful for synchronizing with slow peripherals
 */
void mmio_busywait(uint32_t cycles) {
    for (uint32_t i = 0; i < cycles; i++) {
        __asm__ __volatile__("nop");
    }
}

/*
 * Memory barrier - ensure all previous memory operations complete
 */
void mmio_memory_barrier(void) {
    __asm__ __volatile__("dsb sy" ::: "memory");
}

/*
 * Instruction barrier - ensure pipeline flush
 */
void mmio_instruction_barrier(void) {
    __asm__ __volatile__("isb" ::: "memory");
}
