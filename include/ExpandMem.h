/*
 * System 7.1 Portable - ExpandMem (Extended Memory Globals)
 *
 * ExpandMem provides extended global variable space beyond the original
 * Mac OS low memory globals. It's essential for System 7.1's extended
 * functionality including international support, extended file system
 * features, and process management.
 *
 * Based on System 7.1 ExpandMemPriv.a definitions
 * Copyright (c) 2024 - Portable Mac OS Project
 */

#ifndef EXPANDMEM_H

#include "SystemTypes.h"
#define EXPANDMEM_H

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ExpandMem version and sizing */
#define EM_CURRENT_VERSION      0x0200     /* Version 2.0 for System 7.1 */
#define EM_MIN_SIZE            0x1000      /* Minimum ExpandMem size (4KB) */
#define EM_STANDARD_SIZE       0x2000      /* Standard size (8KB) */
#define EM_EXTENDED_SIZE       0x4000      /* Extended size for future (16KB) */

/* Key cache constants for keyboard support */
#define KEY_CACHE_MIN          0x400       /* Minimum key cache size */
#define KEY_CACHE_SLOP         0x100       /* Extra space for safety */
#define KEY_DEAD_STATE_SIZE    16          /* Dead key state buffer */

/* ExpandMem Record Structure */

/* ExpandMem API Functions */

/**
 * Initialize ExpandMem structure
 * Called early in system initialization
 *
 * @param size Size of ExpandMem to allocate (0 for default)
 * @return Pointer to initialized ExpandMem, NULL on failure
 */
ExpandMemRec* ExpandMemInit(size_t size);

/**
 * Get current ExpandMem pointer
 *
 * @return Current ExpandMem pointer (NULL if not initialized)
 */
ExpandMemRec* ExpandMemGet(void);

/**
 * Extend ExpandMem size
 * Used when system components need more global space
 *
 * @param new_size New size required
 * @return true on success, false on failure
 */
Boolean ExpandMemExtend(size_t new_size);

/**
 * Initialize keyboard cache in ExpandMem
 * Sets up KCHR resource and dead key state
 *
 * @param em ExpandMem pointer
 * @param kchr_id KCHR resource ID to load (0 for default)
 * @return true on success
 */
Boolean ExpandMemInitKeyboard(ExpandMemRec* em, SInt16 kchr_id);

/**
 * Set AppleTalk inactive flag
 * Called during boot if AppleTalk is not configured
 *
 * @param em ExpandMem pointer
 * @param inactive true if AppleTalk is inactive
 */
void ExpandMemSetAppleTalkInactive(ExpandMemRec* em, Boolean inactive);

/**
 * Install decompressor in ExpandMem
 * Sets up resource decompression hook
 *
 * @param em ExpandMem pointer
 * @param decompressor Decompressor procedure pointer
 */
void ExpandMemInstallDecompressor(ExpandMemRec* em, void* decompressor);

/**
 * Validate ExpandMem integrity
 * Checks version, size, and critical pointers
 *
 * @param em ExpandMem pointer to validate
 * @return true if valid, false if corrupted
 */
Boolean ExpandMemValidate(const ExpandMemRec* em);

/**
 * Dump ExpandMem contents for debugging
 *
 * @param em ExpandMem pointer
 * @param output_func Function to output debug text
 */
void ExpandMemDump(const ExpandMemRec* em,
                   void (*output_func)(const char* text));

/**
 * Clean up ExpandMem on shutdown
 * Frees resources and cleans up allocations
 *
 * @param em ExpandMem pointer
 */
void ExpandMemCleanup(ExpandMemRec* em);

#ifdef __cplusplus
}
#endif

#endif /* EXPANDMEM_H */