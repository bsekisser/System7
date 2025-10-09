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
    if (!bounds || !framebuffer) {
        return NULL;
    }

    /* Calculate rectangle dimensions */
    SInt16 width = bounds->right - bounds->left;
    SInt16 height = bounds->bottom - bounds->top;

    if (width <= 0 || height <= 0) {
        return NULL;
    }

    /* Allocate handle for saved bits record */
    SavedBitsHandle bitsHandle = (SavedBitsHandle)NewHandle(sizeof(SavedBitsRec));
    if (!bitsHandle) {
        return NULL;
    }

    SavedBitsPtr savedBits = *bitsHandle;

    /* Store bounds and mode */
    savedBits->bounds = *bounds;
    savedBits->mode = mode;

    /* Calculate data size (32 bits per pixel = 4 bytes) */
    savedBits->dataSize = width * height * 4;

    /* Allocate memory for pixel data */
    savedBits->bitsData = malloc(savedBits->dataSize);
    if (!savedBits->bitsData) {
        DisposeHandle((Handle)bitsHandle);
        return NULL;
    }

    /* Copy pixels from framebuffer to save buffer */
    {
        uint32_t* fb = (uint32_t*)framebuffer;
        uint32_t* savePtr = (uint32_t*)savedBits->bitsData;
        int pitch = fb_pitch / 4;
        int y, x;

        for (y = 0; y < height; y++) {
            int screenY = bounds->top + y;
            if (screenY < 0 || screenY >= (int)fb_height) continue;

            for (x = 0; x < width; x++) {
                int screenX = bounds->left + x;
                if (screenX < 0 || screenX >= (int)fb_width) continue;

                savePtr[y * width + x] = fb[screenY * pitch + screenX];
            }
        }
    }

    savedBits->valid = true;

    return (Handle)bitsHandle;
}

/*
 * RestoreBits - Restore saved screen bits
 */
OSErr RestoreBits(Handle bitsHandle) {
    if (!bitsHandle || !*bitsHandle || !framebuffer) {
        return paramErr;
    }

    SavedBitsPtr savedBits = (SavedBitsPtr)*bitsHandle;

    if (!savedBits->valid || !savedBits->bitsData) {
        return paramErr;
    }

    /* Restore pixels from save buffer to framebuffer */
    {
        uint32_t* fb = (uint32_t*)framebuffer;
        uint32_t* savePtr = (uint32_t*)savedBits->bitsData;
        int pitch = fb_pitch / 4;
        SInt16 width = savedBits->bounds.right - savedBits->bounds.left;
        SInt16 height = savedBits->bounds.bottom - savedBits->bounds.top;
        int y, x;

        for (y = 0; y < height; y++) {
            int screenY = savedBits->bounds.top + y;
            if (screenY < 0 || screenY >= (int)fb_height) continue;

            for (x = 0; x < width; x++) {
                int screenX = savedBits->bounds.left + x;
                if (screenX < 0 || screenX >= (int)fb_width) continue;

                fb[screenY * pitch + screenX] = savePtr[y * width + x];
            }
        }
    }

    return noErr;
}

/*
 * DiscardBits - Discard saved screen bits without restoring
 */
OSErr DiscardBits(Handle bitsHandle) {
    if (!bitsHandle || !*bitsHandle) {
        return paramErr;
    }

    SavedBitsPtr savedBits = (SavedBitsPtr)*bitsHandle;

    /* Free pixel data if allocated */
    if (savedBits->bitsData) {
        free(savedBits->bitsData);
        savedBits->bitsData = NULL;
    }

    /* Mark as invalid */
    savedBits->valid = false;

    /* Dispose the handle */
    DisposeHandle(bitsHandle);

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
