/*
 * MemoryGuards.h - Memory Safety Utilities
 *
 * Provides compile-time and runtime checks to prevent memory corruption bugs.
 * These utilities help catch pointer errors, buffer overflows, and other
 * memory safety violations early.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 */

#ifndef PLATFORM_MEMORY_GUARDS_H
#define PLATFORM_MEMORY_GUARDS_H

#include "SystemTypes.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Buffer Overlap Detection
 * ============================================================================ */

/**
 * Check if a pointer falls within a memory range.
 * Used to detect when pointers incorrectly reference other structures.
 *
 * @param ptr The pointer to check
 * @param rangeStart Start of the memory range
 * @param rangeSize Size of the memory range in bytes
 * @return true if ptr falls within [rangeStart, rangeStart+rangeSize)
 */
static inline Boolean MemGuard_PointerInRange(const void* ptr,
                                               const void* rangeStart,
                                               size_t rangeSize) {
    uintptr_t ptrAddr = (uintptr_t)ptr;
    uintptr_t rangeBegin = (uintptr_t)rangeStart;
    uintptr_t rangeEnd = rangeBegin + rangeSize;

    return (ptrAddr >= rangeBegin && ptrAddr < rangeEnd);
}

/**
 * Validate that a buffer pointer does NOT point into a structure.
 * This catches common corruption bugs where pointers are incorrectly initialized.
 *
 * @param bufferPtr The buffer pointer to validate
 * @param structPtr Pointer to the structure that should NOT contain the buffer
 * @param structSize Size of the structure
 * @param errorMsg Error message to log if corruption detected
 * @return true if validation passed (buffer is safe), false if corruption detected
 */
Boolean MemGuard_ValidateBufferNotInStruct(const void* bufferPtr,
                                           const void* structPtr,
                                           size_t structSize,
                                           const char* errorMsg);

/* ============================================================================
 * Debug Assertions (only active in DEBUG builds)
 * ============================================================================ */

#ifdef DEBUG

/**
 * Assert that a buffer write will not exceed bounds.
 * Triggers debugger if bounds would be exceeded.
 */
#define MEMGUARD_ASSERT_BUFFER_BOUNDS(buf, offset, size) \
    do { \
        if ((offset) + (size) > (buf)->size) { \
            extern void serial_puts(const char* str); \
            serial_puts("[ASSERT] Buffer overflow detected!\n"); \
            __builtin_trap(); \
        } \
    } while (0)

/**
 * Assert that a pointer is valid and properly aligned.
 */
#define MEMGUARD_ASSERT_VALID_POINTER(ptr, type) \
    do { \
        if (!(ptr)) { \
            extern void serial_puts(const char* str); \
            serial_puts("[ASSERT] NULL pointer detected!\n"); \
            __builtin_trap(); \
        } \
        if (((uintptr_t)(ptr) % _Alignof(type)) != 0) { \
            extern void serial_puts(const char* str); \
            serial_puts("[ASSERT] Misaligned pointer detected!\n"); \
            __builtin_trap(); \
        } \
    } while (0)

/**
 * Assert that two memory ranges do not overlap.
 */
#define MEMGUARD_ASSERT_NO_OVERLAP(ptr1, size1, ptr2, size2) \
    do { \
        uintptr_t start1 = (uintptr_t)(ptr1); \
        uintptr_t end1 = start1 + (size1); \
        uintptr_t start2 = (uintptr_t)(ptr2); \
        uintptr_t end2 = start2 + (size2); \
        if ((start1 < end2) && (start2 < end1)) { \
            extern void serial_puts(const char* str); \
            serial_puts("[ASSERT] Memory overlap detected!\n"); \
            __builtin_trap(); \
        } \
    } while (0)

#else /* !DEBUG */

/* In release builds, assertions are no-ops */
#define MEMGUARD_ASSERT_BUFFER_BOUNDS(buf, offset, size)
#define MEMGUARD_ASSERT_VALID_POINTER(ptr, type)
#define MEMGUARD_ASSERT_NO_OVERLAP(ptr1, size1, ptr2, size2)

#endif /* DEBUG */

/* ============================================================================
 * Runtime Validation (always active)
 * ============================================================================ */

/**
 * Validate a memory range before performing operations.
 * Returns an error code instead of asserting, for production use.
 *
 * @param ptr Pointer to validate
 * @param size Expected accessible size
 * @return noErr if valid, appropriate error code otherwise
 */
OSErr MemGuard_ValidateRange(const void* ptr, size_t size);

/**
 * Check if a pointer appears to be valid (non-NULL, aligned, in reasonable range).
 * This is a heuristic check, not a guarantee.
 *
 * @param ptr Pointer to check
 * @param alignment Required alignment (e.g., 4 for 4-byte alignment)
 * @return true if pointer appears valid
 */
Boolean MemGuard_LooksValid(const void* ptr, size_t alignment);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_MEMORY_GUARDS_H */
