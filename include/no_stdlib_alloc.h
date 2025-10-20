/**
 * no_stdlib_alloc.h - Prevents use of standard C memory allocators
 *
 * This header MUST be included in all source files to prevent accidental
 * use of malloc/free/calloc/realloc instead of Memory Manager functions.
 *
 * Mac OS uses a separate Memory Manager (NewPtr/DisposePtr/NewHandle/etc.)
 * which maintains its own heap structures. Mixing standard C allocators
 * with Memory Manager calls corrupts the heap and causes crashes.
 *
 * EXCEPTION: MemoryManager.c itself needs these functions for internal
 * heap management. It should NOT include this header.
 */

#ifndef NO_STDLIB_ALLOC_H
#define NO_STDLIB_ALLOC_H

/* Only apply restrictions outside of Memory Manager implementation */
#ifndef MEMORY_MANAGER_INTERNAL

/*
 * Redefine standard allocators to produce compile-time errors
 *
 * If you see these errors, you must use Memory Manager functions instead:
 *   malloc(size)        → NewPtr(size)
 *   calloc(1, size)     → NewPtrClear(size)
 *   calloc(n, size)     → NewPtrClear(n * size)
 *   free(ptr)           → DisposePtr((Ptr)ptr)
 *   realloc(ptr, size)  → NewPtr/BlockMove/DisposePtr pattern (see docs)
 */

#define malloc(size) \
    DO_NOT_USE_MALLOC__USE_NewPtr_INSTEAD(size)

#define calloc(nmemb, size) \
    DO_NOT_USE_CALLOC__USE_NewPtrClear_INSTEAD(nmemb, size)

#define realloc(ptr, size) \
    DO_NOT_USE_REALLOC__USE_NewPtr_BlockMove_DisposePtr_PATTERN(ptr, size)

#define free(ptr) \
    DO_NOT_USE_FREE__USE_DisposePtr_INSTEAD(ptr)

/* Declare the error functions (they don't need to exist - compile will fail) */
extern void* DO_NOT_USE_MALLOC__USE_NewPtr_INSTEAD(unsigned long size);
extern void* DO_NOT_USE_CALLOC__USE_NewPtrClear_INSTEAD(unsigned long nmemb, unsigned long size);
extern void* DO_NOT_USE_REALLOC__USE_NewPtr_BlockMove_DisposePtr_PATTERN(void* ptr, unsigned long size);
extern void  DO_NOT_USE_FREE__USE_DisposePtr_INSTEAD(void* ptr);

#endif /* !MEMORY_MANAGER_INTERNAL */

#endif /* NO_STDLIB_ALLOC_H */
