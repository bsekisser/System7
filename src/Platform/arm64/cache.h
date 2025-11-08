/*
 * ARM64 Cache Management Interface
 * Data and instruction cache operations
 */

#ifndef ARM64_CACHE_H
#define ARM64_CACHE_H

#include <stdint.h>
#include <stddef.h>

/* Data cache operations */
void dcache_clean_range(void *start, size_t length);
void dcache_invalidate_range(void *start, size_t length);
void dcache_flush_range(void *start, size_t length);

/* Instruction cache operations */
void icache_invalidate_all(void);
void icache_invalidate_range(void *start, size_t length);

/* Synchronize code after modification */
void cache_sync_code(void *start, size_t length);

/* Get cache line sizes */
uint32_t dcache_get_line_size(void);
uint32_t icache_get_line_size(void);

/* Memory barriers */
void dmb(void);  /* Data Memory Barrier */
void dsb(void);  /* Data Synchronization Barrier */
void isb(void);  /* Instruction Synchronization Barrier */

#endif /* ARM64_CACHE_H */
