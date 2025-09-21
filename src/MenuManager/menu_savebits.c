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


/* Screen bits handle structure */
typedef struct {
    Rect bounds;        /* Rectangle that was saved */
    SInt16 mode;       /* Save mode flags */
    void *bitsData;     /* Saved pixel data */
    SInt32 dataSize;   /* Size of saved data */
    Boolean valid;      /* Handle is valid */
} SavedBitsRec, *SavedBitsPtr, **SavedBitsHandle;

/*
 * SaveBits - Save screen bits for menu display
 */
Handle SaveBits(const Rect *bounds, SInt16 mode) {
    if (!bounds) {
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

    /* Calculate data size (assume 8 bits per pixel for now) */
    savedBits->dataSize = width * height;

    /* Allocate memory for pixel data */
    savedBits->bitsData = malloc(savedBits->dataSize);
    if (!savedBits->bitsData) {
        DisposeHandle((Handle)bitsHandle);
        return NULL;
    }

    /* TODO: Platform-specific screen capture implementation */
    /* This would interface with the graphics system to copy pixels */
    /* For now, zero the data as a placeholder */
    memset(savedBits->bitsData, 0, savedBits->dataSize);

    savedBits->valid = true;

    return (Handle)bitsHandle;
}

/*
 * RestoreBits - Restore saved screen bits
 */
OSErr RestoreBits(Handle bitsHandle) {
    if (!bitsHandle || !*bitsHandle) {
        return paramErr;
    }

    SavedBitsPtr savedBits = (SavedBitsPtr)*bitsHandle;

    if (!savedBits->valid || !savedBits->bitsData) {
        return paramErr;
    }

    /* TODO: Platform-specific screen restore implementation */
    /* This would interface with the graphics system to restore pixels */
    /* For now, just validate the operation */

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

/*
