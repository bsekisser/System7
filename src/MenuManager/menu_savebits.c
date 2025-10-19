// #include "CompatibilityFix.h" // Removed
#include <stdlib.h>
#include <string.h>
/*
 * menu_savebits.c - Menu Manager Screen Bits Save/Restore
 *
 * Implementation of screen bits save/restore functions based on
 *
 * RE-AGENT-BANNER: Extracted from Mac OS System 7.1 SaveRestoreBits trap
 */

#include "SystemTypes.h"
#include "System71StdLib.h"

#include "MenuManager/menu_private.h"
#include "MenuManager/MenuDisplay.h"
#include "MemoryMgr/MemoryManager.h"


/* Screen bits handle structure */
typedef struct {
    Rect bounds;        /* Rectangle that was saved */
    SInt16 mode;       /* Save mode flags */
    void *bitsData;     /* Saved pixel data */
    SInt32 dataSize;   /* Size of saved data */
    Boolean valid;      /* Handle is valid */
} SavedBitsRec, *SavedBitsPtr, **SavedBitsHandle;

/* External framebuffer access */
extern void* framebuffer;
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;

/*
 * SaveBits - Save screen bits for menu display
 */
Handle SaveBits(const Rect *bounds, SInt16 mode) {
    extern void serial_logf(int module, int level, const char* fmt, ...);

    if (!bounds || !framebuffer) {
        serial_logf(3, 2, "[SAVEBITS] SaveBits: NULL bounds or framebuffer\n");
        return NULL;
    }

    /* Calculate rectangle dimensions */
    SInt16 width = bounds->right - bounds->left;
    SInt16 height = bounds->bottom - bounds->top;

    if (width <= 0 || height <= 0) {
        serial_logf(3, 2, "[SAVEBITS] SaveBits: Invalid dimensions %dx%d\n", width, height);
        return NULL;
    }

    serial_logf(3, 2, "[SAVEBITS] SaveBits: Allocating for %dx%d rect\n", width, height);

    /* Allocate handle for saved bits record */
    SavedBitsHandle bitsHandle = (SavedBitsHandle)NewHandle(sizeof(SavedBitsRec));
    if (!bitsHandle) {
        serial_logf(3, 2, "[SAVEBITS] SaveBits: NewHandle failed for SavedBitsRec\n");
        return NULL;
    }

    serial_logf(3, 2, "[SAVEBITS] SaveBits: bitsHandle=%p *bitsHandle=%p\n",
               bitsHandle, *bitsHandle);

    /* CRITICAL: Lock handle before dereferencing to prevent heap compaction issues */
    HLock((Handle)bitsHandle);
    SavedBitsPtr savedBits = *bitsHandle;

    /* Store bounds and mode */
    savedBits->bounds = *bounds;
    savedBits->mode = mode;

    /* Calculate data size (32 bits per pixel = 4 bytes) */
    savedBits->dataSize = width * height * 4;

    serial_logf(3, 2, "[SAVEBITS] SaveBits: Allocating %d bytes for pixel data\n",
               savedBits->dataSize);

    /* Allocate memory for pixel data */
    savedBits->bitsData = malloc(savedBits->dataSize);
    if (!savedBits->bitsData) {
        serial_logf(3, 2, "[SAVEBITS] SaveBits: malloc failed for pixel data\n");
        HUnlock((Handle)bitsHandle);
        DisposeHandle((Handle)bitsHandle);
        return NULL;
    }

    serial_logf(3, 2, "[SAVEBITS] SaveBits: bitsData=%p size=%d\n",
               savedBits->bitsData, savedBits->dataSize);

    /* Copy pixels from framebuffer to save buffer */
    /* CRITICAL: Use separate index for buffer to prevent overflow when bounds are clipped */
    {
        uint32_t* fb = (uint32_t*)framebuffer;
        uint32_t* savePtr = (uint32_t*)savedBits->bitsData;
        int pitch = fb_pitch / 4;
        int y, x;
        int bufferIndex = 0;

        for (y = 0; y < height; y++) {
            int screenY = bounds->top + y;
            /* If row is out of bounds, fill with black/transparent */
            if (screenY < 0 || screenY >= (int)fb_height) {
                for (x = 0; x < width; x++) {
                    savePtr[bufferIndex++] = 0xFF000000; /* Black */
                }
                continue;
            }

            for (x = 0; x < width; x++) {
                int screenX = bounds->left + x;
                /* If pixel is out of bounds, use black/transparent */
                if (screenX < 0 || screenX >= (int)fb_width) {
                    savePtr[bufferIndex++] = 0xFF000000; /* Black */
                } else {
                    savePtr[bufferIndex++] = fb[screenY * pitch + screenX];
                }
            }
        }
    }

    savedBits->valid = true;

    serial_logf(3, 2, "[SAVEBITS] SaveBits: Complete. Returning handle=%p bitsData=%p\n",
               bitsHandle, savedBits->bitsData);

    /* Unlock handle before returning */
    HUnlock((Handle)bitsHandle);

    return (Handle)bitsHandle;
}

/*
 * RestoreBits - Restore saved screen bits
 */
OSErr RestoreBits(Handle bitsHandle) {
    extern void serial_logf(int module, int level, const char* fmt, ...);

    serial_logf(3, 2, "[SAVEBITS] RestoreBits: ENTRY bitsHandle=%p\n", bitsHandle);

    if (!bitsHandle || !*bitsHandle || !framebuffer) {
        serial_logf(3, 2, "[SAVEBITS] RestoreBits: Invalid params\n");
        return paramErr;
    }

    serial_logf(3, 2, "[SAVEBITS] RestoreBits: *bitsHandle=%p\n", *bitsHandle);

    /* CRITICAL: Lock handle before dereferencing to prevent heap compaction issues */
    HLock(bitsHandle);
    SavedBitsPtr savedBits = (SavedBitsPtr)*bitsHandle;

    serial_logf(3, 2, "[SAVEBITS] RestoreBits: savedBits=%p valid=%d bitsData=%p\n",
               savedBits, savedBits->valid, savedBits->bitsData);

    if (!savedBits->valid || !savedBits->bitsData) {
        serial_logf(3, 2, "[SAVEBITS] RestoreBits: Invalid savedBits or bitsData\n");
        HUnlock(bitsHandle);
        return paramErr;
    }

    /* Restore pixels from save buffer to framebuffer */
    /* CRITICAL: Use separate index for buffer to match SaveBits sequential write */
    {
        uint32_t* fb = (uint32_t*)framebuffer;
        uint32_t* savePtr = (uint32_t*)savedBits->bitsData;
        int pitch = fb_pitch / 4;
        SInt16 width = savedBits->bounds.right - savedBits->bounds.left;
        SInt16 height = savedBits->bounds.bottom - savedBits->bounds.top;
        int y, x;
        int bufferIndex = 0;

        for (y = 0; y < height; y++) {
            int screenY = savedBits->bounds.top + y;
            /* If row is out of bounds, skip the entire row in buffer */
            if (screenY < 0 || screenY >= (int)fb_height) {
                bufferIndex += width; /* Skip this row in buffer */
                continue;
            }

            for (x = 0; x < width; x++) {
                int screenX = savedBits->bounds.left + x;
                /* Read from buffer sequentially, only write to screen if in bounds */
                uint32_t pixel = savePtr[bufferIndex++];
                if (screenX >= 0 && screenX < (int)fb_width) {
                    fb[screenY * pitch + screenX] = pixel;
                }
            }
        }
    }

    /* Unlock handle after use */
    HUnlock(bitsHandle);

    serial_logf(3, 2, "[SAVEBITS] RestoreBits: EXIT\n");

    return noErr;
}

/*
 * DiscardBits - Discard saved screen bits without restoring
 */
OSErr DiscardBits(Handle bitsHandle) {
    extern void serial_logf(int module, int level, const char* fmt, ...);

    serial_logf(3, 2, "[SAVEBITS] DiscardBits: ENTRY bitsHandle=%p\n", bitsHandle);

    if (!bitsHandle || !*bitsHandle) {
        serial_logf(3, 2, "[SAVEBITS] DiscardBits: NULL handle, returning paramErr\n");
        return paramErr;
    }

    serial_logf(3, 2, "[SAVEBITS] DiscardBits: *bitsHandle=%p\n", *bitsHandle);

    /* CRITICAL: Lock handle before dereferencing to prevent heap compaction issues */
    HLock(bitsHandle);
    SavedBitsPtr savedBits = (SavedBitsPtr)*bitsHandle;

    serial_logf(3, 2, "[SAVEBITS] DiscardBits: savedBits=%p valid=%d bitsData=%p dataSize=%d\n",
               savedBits, savedBits->valid, savedBits->bitsData, savedBits->dataSize);

    /* Validate bitsData pointer before freeing */
    if (savedBits->bitsData) {
        serial_logf(3, 2, "[SAVEBITS] DiscardBits: About to free bitsData=%p\n",
                   savedBits->bitsData);

        /* Read first few bytes for debugging */
        unsigned char* bytes = (unsigned char*)savedBits->bitsData;
        serial_logf(3, 2, "[SAVEBITS] DiscardBits: bitsData first 16 bytes: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
                   bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7],
                   bytes[8], bytes[9], bytes[10], bytes[11], bytes[12], bytes[13], bytes[14], bytes[15]);

        free(savedBits->bitsData);
        serial_logf(3, 2, "[SAVEBITS] DiscardBits: free() completed\n");
        savedBits->bitsData = NULL;
    }

    /* Mark as invalid */
    savedBits->valid = false;

    /* Unlock handle before disposing */
    HUnlock(bitsHandle);

    serial_logf(3, 2, "[SAVEBITS] DiscardBits: About to DisposeHandle\n");

    /* Dispose the handle */
    DisposeHandle(bitsHandle);

    serial_logf(3, 2, "[SAVEBITS] DiscardBits: EXIT\n");

    return noErr;
}

/*
 * SaveRestoreBitsDispatch - Dispatcher for SaveRestoreBits trap
 */
OSErr SaveRestoreBitsDispatch(SInt16 selector, void *params) {
    switch (selector) {
        case selectSaveBits: {
            /* Parameters: bounds (Rect*), mode (SInt16) */
            struct {
                const Rect *bounds;
                SInt16 mode;
                Handle *result;
            } *saveParams = params;

            if (!saveParams || !saveParams->bounds || !saveParams->result) {
                return paramErr;
            }

            *saveParams->result = SaveBits(saveParams->bounds, saveParams->mode);
            return (*saveParams->result) ? noErr : memFullErr;
        }

        case selectRestoreBits: {
            /* Parameters: bitsHandle (Handle) */
            struct {
                Handle bitsHandle;
            } *restoreParams = params;

            if (!restoreParams) {
                return paramErr;
            }

            return RestoreBits(restoreParams->bitsHandle);
        }

        case selectDiscardBits: {
            /* Parameters: bitsHandle (Handle) */
            struct {
                Handle bitsHandle;
            } *discardParams = params;

            if (!discardParams) {
                return paramErr;
            }

            return DiscardBits(discardParams->bitsHandle);
        }

        default:
            return -1; /* unimplemented */
    }
}

/*
 * Convenience wrapper functions for menu system use
 */

/*
 * SaveMenuBits - Save bits under a menu rectangle
 */
Handle SaveMenuBits(const Rect *menuRect) {
    if (!menuRect) {
        return NULL;
    }

    /* Save with default mode */
    return SaveBits(menuRect, 0);
}

/*
 * RestoreMenuBits - Restore bits under a menu
 */
OSErr RestoreMenuBits(Handle bitsHandle) {
    return RestoreBits(bitsHandle);
}

/*
 * DiscardMenuBits - Discard saved menu bits
 */
OSErr DiscardMenuBits(Handle bitsHandle) {
    return DiscardBits(bitsHandle);
}
