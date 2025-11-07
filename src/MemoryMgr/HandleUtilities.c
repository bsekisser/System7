/*
 * HandleUtilities.c - Memory Manager Handle Utility Functions
 *
 * Implements System 7 handle utility functions for common memory operations:
 * - HandToHand: Duplicate a handle
 * - PtrToHand: Create a handle from pointer data
 * - PtrAndHand: Append pointer data to a handle
 * - HandAndHand: Concatenate two handles
 *
 * Based on Inside Macintosh: Memory, Chapter 2
 */

#include "MemoryMgr/MemoryManager.h"
#include "System71StdLib.h"
#include <string.h>

/* Debug logging */
#define HANDLE_UTIL_DEBUG 0

#if HANDLE_UTIL_DEBUG
extern void serial_puts(const char* str);
#define HUTIL_LOG(...) do { \
    char buf[256]; \
    snprintf(buf, sizeof(buf), "[HandleUtil] " __VA_ARGS__); \
    serial_puts(buf); \
} while(0)
#else
#define HUTIL_LOG(...)
#endif

/*
 * HandToHand - Duplicate a handle
 *
 * Creates a copy of the handle pointed to by theHndl and returns the
 * new handle through theHndl. The original handle's data is copied to
 * the new handle.
 *
 * Parameters:
 *   theHndl - Pointer to handle to duplicate (on input: source, on output: copy)
 *
 * Returns:
 *   noErr (0) on success
 *   memFullErr (-108) if insufficient memory
 *   nilHandleErr (-109) if theHndl is NULL or *theHndl is NULL
 */
OSErr HandToHand(Handle* theHndl) {
    Handle sourceHandle;
    Handle newHandle;
    Size handleSize;
    Ptr sourceData;
    Ptr newData;

    if (!theHndl) {
        HUTIL_LOG("HandToHand: NULL handle pointer\n");
        return nilHandleErr;
    }

    sourceHandle = *theHndl;
    if (!sourceHandle) {
        HUTIL_LOG("HandToHand: NULL handle\n");
        return nilHandleErr;
    }

    /* Get size of source handle */
    handleSize = GetHandleSize(sourceHandle);
    if (handleSize == 0) {
        /* Handle is empty or purged */
        HUTIL_LOG("HandToHand: Empty or purged handle\n");
        return memPurgedErr;
    }

    /* Allocate new handle of same size */
    newHandle = NewHandle(handleSize);
    if (!newHandle) {
        HUTIL_LOG("HandToHand: Failed to allocate new handle (size=%lu)\n",
                  (unsigned long)handleSize);
        return memFullErr;
    }

    /* Lock source handle to prevent purging/compaction during copy */
    HLock(sourceHandle);

    /* Copy data from source to new handle */
    sourceData = *sourceHandle;
    newData = *newHandle;

    if (sourceData && newData) {
        memcpy(newData, sourceData, handleSize);
        HUnlock(sourceHandle);
    } else {
        /* Shouldn't happen, but handle gracefully */
        HUnlock(sourceHandle);
        DisposeHandle(newHandle);
        HUTIL_LOG("HandToHand: NULL data pointer\n");
        return nilHandleErr;
    }

    /* Return new handle through parameter */
    *theHndl = newHandle;

    HUTIL_LOG("HandToHand: Duplicated handle (size=%lu)\n",
              (unsigned long)handleSize);

    return noErr;
}

/*
 * PtrToHand - Create a handle from pointer data
 *
 * Creates a new handle and copies size bytes from srcPtr into it.
 * The new handle is returned through dstHndl.
 *
 * Parameters:
 *   srcPtr - Pointer to source data
 *   dstHndl - Pointer to receive new handle
 *   size - Number of bytes to copy
 *
 * Returns:
 *   noErr (0) on success
 *   memFullErr (-108) if insufficient memory
 *   paramErr (-50) if parameters are invalid
 */
OSErr PtrToHand(const void* srcPtr, Handle* dstHndl, Size size) {
    Handle newHandle;
    Ptr handleData;

    if (!srcPtr || !dstHndl) {
        HUTIL_LOG("PtrToHand: NULL parameter\n");
        return paramErr;
    }

    if (size == 0) {
        /* Create empty handle */
        newHandle = NewHandle(0);
        if (!newHandle) {
            return memFullErr;
        }
        *dstHndl = newHandle;
        HUTIL_LOG("PtrToHand: Created empty handle\n");
        return noErr;
    }

    /* Allocate new handle */
    newHandle = NewHandle(size);
    if (!newHandle) {
        HUTIL_LOG("PtrToHand: Failed to allocate handle (size=%lu)\n",
                  (unsigned long)size);
        return memFullErr;
    }

    /* Copy data from pointer to handle */
    handleData = *newHandle;
    if (handleData) {
        memcpy(handleData, srcPtr, size);
    } else {
        /* Shouldn't happen */
        DisposeHandle(newHandle);
        HUTIL_LOG("PtrToHand: NULL handle data\n");
        return nilHandleErr;
    }

    *dstHndl = newHandle;

    HUTIL_LOG("PtrToHand: Created handle from pointer (size=%lu)\n",
              (unsigned long)size);

    return noErr;
}

/*
 * PtrAndHand - Append pointer data to a handle
 *
 * Appends size bytes from srcPtr to the end of the handle dstHndl.
 * The handle is resized to accommodate the new data.
 *
 * Parameters:
 *   srcPtr - Pointer to data to append
 *   dstHndl - Handle to append to
 *   size - Number of bytes to append
 *
 * Returns:
 *   noErr (0) on success
 *   memFullErr (-108) if insufficient memory
 *   paramErr (-50) if parameters are invalid
 *   nilHandleErr (-109) if dstHndl is NULL
 */
OSErr PtrAndHand(const void* srcPtr, Handle dstHndl, Size size) {
    Size oldSize;
    Size newSize;
    Ptr handleData;

    if (!srcPtr) {
        HUTIL_LOG("PtrAndHand: NULL source pointer\n");
        return paramErr;
    }

    if (!dstHndl) {
        HUTIL_LOG("PtrAndHand: NULL handle\n");
        return nilHandleErr;
    }

    if (size == 0) {
        /* Nothing to append */
        return noErr;
    }

    /* Get current handle size */
    oldSize = GetHandleSize(dstHndl);
    newSize = oldSize + size;

    /* Resize handle to accommodate new data */
    if (!SetHandleSize(dstHndl, newSize)) {
        HUTIL_LOG("PtrAndHand: Failed to resize handle (old=%lu, new=%lu)\n",
                  (unsigned long)oldSize, (unsigned long)newSize);
        return memFullErr;
    }

    /* Append data to end of handle */
    handleData = *dstHndl;
    if (handleData) {
        memcpy(handleData + oldSize, srcPtr, size);
    } else {
        HUTIL_LOG("PtrAndHand: NULL handle data after resize\n");
        return nilHandleErr;
    }

    HUTIL_LOG("PtrAndHand: Appended %lu bytes (old=%lu, new=%lu)\n",
              (unsigned long)size, (unsigned long)oldSize, (unsigned long)newSize);

    return noErr;
}

/*
 * HandAndHand - Concatenate two handles
 *
 * Appends the contents of handle aHndl to the end of handle bHndl.
 * Handle bHndl is resized to accommodate the concatenated data.
 * Handle aHndl is unchanged.
 *
 * Parameters:
 *   aHndl - Handle whose contents to append
 *   bHndl - Handle to append to
 *
 * Returns:
 *   noErr (0) on success
 *   memFullErr (-108) if insufficient memory
 *   nilHandleErr (-109) if either handle is NULL
 */
OSErr HandAndHand(Handle aHndl, Handle bHndl) {
    Size aSize;
    Size bSize;
    Size newSize;
    Ptr aData;
    Ptr bData;

    if (!aHndl) {
        HUTIL_LOG("HandAndHand: NULL source handle\n");
        return nilHandleErr;
    }

    if (!bHndl) {
        HUTIL_LOG("HandAndHand: NULL destination handle\n");
        return nilHandleErr;
    }

    /* Get sizes of both handles */
    aSize = GetHandleSize(aHndl);
    bSize = GetHandleSize(bHndl);

    if (aSize == 0) {
        /* Nothing to append */
        HUTIL_LOG("HandAndHand: Source handle is empty\n");
        return noErr;
    }

    newSize = bSize + aSize;

    /* Resize destination handle */
    if (!SetHandleSize(bHndl, newSize)) {
        HUTIL_LOG("HandAndHand: Failed to resize destination (old=%lu, new=%lu)\n",
                  (unsigned long)bSize, (unsigned long)newSize);
        return memFullErr;
    }

    /* Append data from aHndl to bHndl */
    aData = *aHndl;
    bData = *bHndl;

    if (aData && bData) {
        memcpy(bData + bSize, aData, aSize);
    } else {
        HUTIL_LOG("HandAndHand: NULL data pointer\n");
        return nilHandleErr;
    }

    HUTIL_LOG("HandAndHand: Concatenated handles (a=%lu, b=%lu, new=%lu)\n",
              (unsigned long)aSize, (unsigned long)bSize, (unsigned long)newSize);

    return noErr;
}
