/*
 * MemoryUtilities.c - Memory Manipulation Utilities
 *
 * Implements memory manipulation functions for Mac OS. These utilities
 * provide safe and efficient memory copying, moving, and manipulation
 * operations used throughout the Toolbox.
 *
 * Based on Inside Macintosh: Operating System Utilities
 */

#include "SystemTypes.h"
#include "System71StdLib.h"
#include <string.h>

/* Forward declarations */
void BlockMove(const void* srcPtr, void* destPtr, Size byteCount);
void BlockMoveData(const void* srcPtr, void* destPtr, Size byteCount);
SInt16 HiWord(SInt32 x);
SInt16 LoWord(SInt32 x);
void LongMul(SInt32 a, SInt32 b, wide* result);

/* Debug logging */
#define MEM_UTIL_DEBUG 0

#if MEM_UTIL_DEBUG
extern void serial_puts(const char* str);
#define MEMUTIL_LOG(...) do { \
    char buf[256]; \
    snprintf(buf, sizeof(buf), "[MemUtil] " __VA_ARGS__); \
    serial_puts(buf); \
} while(0)
#else
#define MEMUTIL_LOG(...)
#endif

/*
 * BlockMove - Move a block of memory
 *
 * Copies a block of memory from source to destination. This function
 * properly handles overlapping memory regions (uses memmove internally).
 * This is one of the most fundamental memory operations in Mac OS.
 *
 * Parameters:
 *   srcPtr - Pointer to source memory
 *   destPtr - Pointer to destination memory
 *   byteCount - Number of bytes to copy
 *
 * Note: This function handles overlapping memory regions correctly.
 * If source and destination overlap, the data is copied in the correct
 * order to preserve data integrity.
 *
 * Example:
 *   char src[10] = "Hello";
 *   char dest[10];
 *   BlockMove(src, dest, 6);  // Copies "Hello\0" to dest
 *
 * Based on Inside Macintosh: Operating System Utilities, Chapter 1
 */
void BlockMove(const void* srcPtr, void* destPtr, Size byteCount) {
    if (!srcPtr || !destPtr) {
        MEMUTIL_LOG("BlockMove: NULL pointer\n");
        return;
    }

    if (byteCount == 0) {
        return;
    }

    /* Use memmove to handle overlapping memory regions correctly */
    memmove(destPtr, srcPtr, byteCount);

    MEMUTIL_LOG("BlockMove: Moved %ld bytes from %p to %p\n",
                (long)byteCount, srcPtr, destPtr);
}

/*
 * BlockMoveData - Move a block of memory (data variant)
 *
 * This is identical to BlockMove but is provided for compatibility.
 * Some code uses BlockMoveData to explicitly indicate that it's moving
 * data (not code or handles).
 *
 * Parameters:
 *   srcPtr - Pointer to source memory
 *   destPtr - Pointer to destination memory
 *   byteCount - Number of bytes to copy
 *
 * Based on Inside Macintosh: Memory
 */
void BlockMoveData(const void* srcPtr, void* destPtr, Size byteCount) {
    BlockMove(srcPtr, destPtr, byteCount);
}

/*
 * HiWord - Extract high-order 16 bits from 32-bit value
 *
 * Extracts the high-order (most significant) 16 bits from a 32-bit value.
 * This is commonly used for unpacking coordinates, dimensions, and other
 * paired 16-bit values stored in a single 32-bit long.
 *
 * Parameters:
 *   x - 32-bit value to extract from
 *
 * Returns:
 *   High-order 16 bits as signed 16-bit value
 *
 * Example:
 *   HiWord(0x12345678) -> 0x1234
 *   HiWord(0xABCD0000) -> 0xABCD
 *
 * Common uses:
 *   - Extracting vertical coordinate from Point (stored as SInt32)
 *   - Extracting height from dimension pair
 *   - Unpacking two 16-bit values from a long
 *
 * Based on Inside Macintosh: Operating System Utilities
 */
SInt16 HiWord(SInt32 x) {
    return (SInt16)((x >> 16) & 0xFFFF);
}

/*
 * LoWord - Extract low-order 16 bits from 32-bit value
 *
 * Extracts the low-order (least significant) 16 bits from a 32-bit value.
 * This is commonly used for unpacking coordinates, dimensions, and other
 * paired 16-bit values stored in a single 32-bit long.
 *
 * Parameters:
 *   x - 32-bit value to extract from
 *
 * Returns:
 *   Low-order 16 bits as signed 16-bit value
 *
 * Example:
 *   LoWord(0x12345678) -> 0x5678
 *   LoWord(0x0000ABCD) -> 0xABCD
 *
 * Common uses:
 *   - Extracting horizontal coordinate from Point (stored as SInt32)
 *   - Extracting width from dimension pair
 *   - Unpacking two 16-bit values from a long
 *
 * Based on Inside Macintosh: Operating System Utilities
 */
SInt16 LoWord(SInt32 x) {
    return (SInt16)(x & 0xFFFF);
}

/*
 * LongMul - Multiply two 32-bit values producing 64-bit result
 *
 * Multiplies two signed 32-bit integers and produces a 64-bit result.
 * This is useful for calculations that might overflow a 32-bit result,
 * such as fixed-point arithmetic or large numeric computations.
 *
 * Parameters:
 *   a - First 32-bit multiplicand
 *   b - Second 32-bit multiplicand
 *   result - Pointer to 64-bit wide result structure
 *
 * The result is stored in a wide structure:
 *   result->hi - High 32 bits of product
 *   result->lo - Low 32 bits of product
 *
 * Example:
 *   wide result;
 *   LongMul(1000000, 1000000, &result);
 *   // result = 1,000,000,000,000 (0xE8D4A51000)
 *   // result.hi = 0x000000E8 (232)
 *   // result.lo = 0xD4A51000 (3,567,587,328)
 *
 * Common uses:
 *   - Large integer arithmetic
 *   - Fixed-point calculations requiring extended precision
 *   - Financial calculations
 *   - Time calculations (ticks to milliseconds, etc.)
 *
 * Based on Inside Macintosh: Operating System Utilities
 */
void LongMul(SInt32 a, SInt32 b, wide* result) {
    SInt64 product;

    if (!result) {
        MEMUTIL_LOG("LongMul: NULL result pointer\n");
        return;
    }

    /* Perform 64-bit multiplication */
    product = (SInt64)a * (SInt64)b;

    /* Split result into high and low 32-bit parts */
    result->hi = (SInt32)((product >> 32) & 0xFFFFFFFF);
    result->lo = (UInt32)(product & 0xFFFFFFFF);

    MEMUTIL_LOG("LongMul: %ld * %ld = 0x%08X%08X\n",
                (long)a, (long)b,
                (unsigned int)result->hi,
                (unsigned int)result->lo);
}
