/*
 * OSUtils.h - Operating System Utilities
 *
 * Provides various utility functions for memory manipulation, data searching,
 * and other common operations.
 *
 * Based on Inside Macintosh: Operating System Utilities
 */

#ifndef __OSUTILS_H__
#define __OSUTILS_H__

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Munger - Memory munging (search and replace) in a handle
 *
 * Searches for data in a handle and optionally replaces it or inserts data.
 * This is a powerful utility used throughout the Toolbox.
 *
 * Parameters:
 *   h - Handle to search/modify
 *   offset - Starting offset for search
 *   ptr1 - Pointer to data to search for (NULL = don't search)
 *   len1 - Length of search data (0 = don't search)
 *   ptr2 - Pointer to replacement data (NULL = delete)
 *   len2 - Length of replacement data
 *
 * Returns:
 *   Offset where match was found, or -1 if not found
 *   If ptr1 is NULL, inserts ptr2 at offset and returns offset
 *
 * Based on Inside Macintosh: Memory
 */
SInt32 Munger(Handle h, SInt32 offset, const void* ptr1, SInt32 len1,
              const void* ptr2, SInt32 len2);

/* Date/Time utilities */
void GetDateTime(UInt32* secs);
void SetDateTime(UInt32 secs);
void ReadDateTime(UInt32* secs);

/* Delay utilities */
void Delay(UInt32 numTicks, UInt32* finalTicks);

/* Queue utilities */
void Enqueue(QElemPtr qElement, QHdr* qHeader);
OSErr Dequeue(QElemPtr qElement, QHdr* qHeader);

#ifdef __cplusplus
}
#endif

#endif /* __OSUTILS_H__ */
