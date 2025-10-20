/**
 * @file MenuBitsPool.h
 * @brief Menu Bits Memory Pool - Prevents heap fragmentation from menu operations
 *
 * Implements a pool-based memory management system for menu background buffers.
 * Instead of allocating/deallocating 50KB+ buffers on every menu open/close,
 * the pool preallocates fixed buffers that are reused, eliminating fragmentation.
 *
 * Copyright (c) 2025 System 7.1 Portable Project
 */

#pragma once
#ifndef __MENU_BITS_POOL_H__
#define __MENU_BITS_POOL_H__

#include "SystemTypes.h"
/* Don't include QuickDraw.h - Rect is in SystemTypes.h */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the menu bits pool
 * Should be called once during system startup
 *
 * @param numBuffers - Number of buffers to preallocate (recommend 3-4)
 * @param bufferSize - Size of each buffer in bytes (recommend 160KB for largest menus)
 * @return OSErr - noErr if successful, memFullErr if allocation fails
 */
OSErr MenuBitsPool_Init(SInt16 numBuffers, SInt32 bufferSize);

/**
 * Allocate a buffer from the pool for saving menu background bits
 * If no buffer available, returns NULL (caller should handle gracefully)
 *
 * @param bounds - Rectangle describing the menu area
 * @return Handle to pooled buffer, or NULL if no buffer available
 */
Handle MenuBitsPool_Allocate(const Rect* bounds);

/**
 * Return a buffer to the pool after menu operation completes
 * CRITICAL: Must be paired with MenuBitsPool_Allocate
 *
 * @param poolHandle - Handle returned from MenuBitsPool_Allocate
 * @return OSErr - noErr if successful
 */
OSErr MenuBitsPool_Free(Handle poolHandle);

/**
 * Shutdown the menu bits pool and free all resources
 * Should be called during system shutdown
 *
 * @return OSErr - noErr if successful
 */
OSErr MenuBitsPool_Shutdown(void);

/**
 * Get current pool statistics for debugging/monitoring
 *
 * @param outTotal - Returns total number of buffers
 * @param outInUse - Returns number currently in use
 * @return Boolean - true if pool is initialized
 */
Boolean MenuBitsPool_GetStats(SInt16* outTotal, SInt16* outInUse);

#ifdef __cplusplus
}
#endif

#endif /* __MENU_BITS_POOL_H__ */
