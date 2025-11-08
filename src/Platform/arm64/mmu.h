/*
 * ARM64 MMU Interface
 * Memory Management Unit for virtual memory
 */

#ifndef ARM64_MMU_H
#define ARM64_MMU_H

#include <stdbool.h>

/* Initialize MMU with identity mapping */
bool mmu_init(void);

/* Enable/disable MMU */
void mmu_enable(void);
void mmu_disable(void);

/* Check MMU status */
bool mmu_is_enabled(void);
bool mmu_is_initialized(void);

#endif /* ARM64_MMU_H */
