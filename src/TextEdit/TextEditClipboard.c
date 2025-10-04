/*
 * TextEditClipboard.c - TextEdit Clipboard Operations
 *
 * Handles cut, copy, paste operations with the Scrap Manager
 */

#include "TextEdit/TextEdit.h"
#include "MemoryMgr/MemoryManager.h"
#include "ScrapManager/ScrapManager.h"
#include <string.h>

/* Boolean constants */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* Extended TERec with additional fields - must match TextEdit.c */
typedef struct TEExtRec {
    TERec       base;           /* Standard TERec */
    Handle      hLines;         /* Line starts array */
    SInt16      nLines;         /* Number of lines */
    Handle      hStyles;        /* Style record handle */
    Boolean     dirty;          /* Needs recalc */
    Boolean     readOnly;       /* Read-only flag */
    Boolean     wordWrap;       /* Word wrap flag */
    SInt16      dragAnchor;     /* Drag selection anchor */
    Boolean     inDragSel;      /* In drag selection */
    UInt32      lastClickTime;  /* For double/triple click */
    SInt16      clickCount;     /* Click count */
    SInt16      viewDH;         /* Horizontal scroll */
    SInt16      viewDV;         /* Vertical scroll */
} TEExtRec;

typedef TEExtRec *TEExtPtr, **TEExtHandle;

/* External functions */
extern void BlockMove(const void *src, void *dest, Size size);

/* Debug logging */
#define TEC_DEBUG 1

#if TEC_DEBUG
extern void serial_printf(const char* fmt, ...);
#define TEC_LOG(...) serial_printf("TEC: " __VA_ARGS__)
#else
#define TEC_LOG(...)
#endif

/* Scrap types */
#define kScrapFlavorTypeText    'TEXT'
#define kScrapFlavorTypeStyle   'styl'

/* Global TextEdit scrap */
static Handle g_TEScrap = NULL;
static Handle g_TEStyleScrap = NULL;

/* Forward declarations */
static OSErr TE_CopyToScrap(TEHandle hTE);
static OSErr TE_GetFromScrap(TEHandle hTE);

/* ============================================================================
 * Cut/Copy/Paste Operations
 * ============================================================================ */

/*
 * TECut - Cut selection to clipboard
 */
void TECut(TEHandle hTE) {
    TEExtPtr pTE;

    if (!hTE) return;

    HLock((Handle)hTE);
    pTE = (TEExtPtr)*hTE;

    /* Check read-only */
    if (pTE->readOnly) {
        HUnlock((Handle)hTE);
        return;
    }

    /* Check if there's a selection */
    if (pTE->base.selStart == pTE->base.selEnd) {
        HUnlock((Handle)hTE);
        return;
    }

    TEC_LOG("TECut: cutting [%d,%d]\n", pTE->base.selStart, pTE->base.selEnd);

    /* Copy to scrap */
    if (TE_CopyToScrap(hTE) == noErr) {
        /* Delete selection */
        TEDelete(hTE);
    }

    HUnlock((Handle)hTE);
}

/*
 * TECopy - Copy selection to clipboard
 */
void TECopy(TEHandle hTE) {
    TEExtPtr pTE;

    if (!hTE) return;

    HLock((Handle)hTE);
    pTE = (TEExtPtr)*hTE;

    /* Check if there's a selection */
    if (pTE->base.selStart == pTE->base.selEnd) {
        HUnlock((Handle)hTE);
        return;
    }

    TEC_LOG("TECopy: copying [%d,%d]\n", pTE->base.selStart, pTE->base.selEnd);

    /* Copy to scrap */
    TE_CopyToScrap(hTE);

    HUnlock((Handle)hTE);
}

/*
 * TEPaste - Paste clipboard content
 */
void TEPaste(TEHandle hTE) {
    TEExtPtr pTE;
    Handle scrapHandle;
    SInt32 scrapSize;
    char *scrapData;

    if (!hTE) return;

    HLock((Handle)hTE);
    pTE = (TEExtPtr)*hTE;

    /* Check read-only */
    if (pTE->readOnly) {
        HUnlock((Handle)hTE);
        return;
    }

    TEC_LOG("TEPaste: pasting at %d\n", pTE->base.selStart);

    /* Get text from scrap */
    if (g_TEScrap) {
        scrapSize = GetHandleSize(g_TEScrap);
        if (scrapSize > 0) {
            HLock(g_TEScrap);
            scrapData = *g_TEScrap;

            /* Replace selection with scrap content */
            TEReplaceSel(scrapData, scrapSize, hTE);

            HUnlock(g_TEScrap);
        }
    }

    /* TODO: Handle style scrap for styled text */

    HUnlock((Handle)hTE);
}

/*
 * TEStylePaste - Paste with style information
 */
void TEStylePaste(TEHandle hTE) {
    TEExtPtr pTE;

    if (!hTE) return;

    HLock((Handle)hTE);
    pTE = (TEExtPtr)*hTE;

    /* Check if this is a styled TE record */
    if (pTE->hStyles) {
        /* TODO: Implement styled paste */
        TEC_LOG("TEStylePaste: styled paste not yet implemented\n");
    }

    /* Fall back to regular paste */
    TEPaste(hTE);

    HUnlock((Handle)hTE);
}

/* ============================================================================
 * Scrap Manager Integration
 * ============================================================================ */

/*
 * TEFromScrap - Load TextEdit scrap from system scrap
 */
OSErr TEFromScrap(void) {
    Handle textHandle;
    SInt32 scrapSize;
    OSErr err;

    TEC_LOG("TEFromScrap: loading from system scrap\n");

    /* Dispose old TE scrap */
    if (g_TEScrap) {
        DisposeHandle(g_TEScrap);
        g_TEScrap = NULL;
    }

    /* Get TEXT from scrap */
    err = GetScrap(NULL, kScrapFlavorTypeText, &scrapSize);
    if (err >= 0 && scrapSize > 0) {
        /* Allocate handle for text */
        g_TEScrap = NewHandle(scrapSize);
        if (g_TEScrap) {
            /* Get the text */
            err = GetScrap(g_TEScrap, kScrapFlavorTypeText, &scrapSize);
            if (err < 0) {
                DisposeHandle(g_TEScrap);
                g_TEScrap = NULL;
            } else {
                /* Resize to actual size */
                SetHandleSize(g_TEScrap, scrapSize);
                TEC_LOG("TEFromScrap: loaded %d bytes\n", scrapSize);
            }
        } else {
            err = memFullErr;
        }
    }

    /* TODO: Get style scrap if present */

    return err;
}

/*
 * TEToScrap - Save TextEdit scrap to system scrap
 */
OSErr TEToScrap(void) {
    OSErr err = noErr;
    SInt32 scrapSize;

    TEC_LOG("TEToScrap: saving to system scrap\n");

    if (g_TEScrap) {
        /* Clear system scrap */
        err = ZeroScrap();
        if (err != noErr) return err;

        /* Put TEXT to scrap */
        scrapSize = GetHandleSize(g_TEScrap);
        if (scrapSize > 0) {
            HLock(g_TEScrap);
            err = PutScrap(scrapSize, kScrapFlavorTypeText, *g_TEScrap);
            HUnlock(g_TEScrap);

            TEC_LOG("TEToScrap: saved %d bytes\n", scrapSize);
        }

        /* TODO: Put style scrap if present */
    }

    return err;
}

/*
 * TEScrapHandle - Get handle to TextEdit scrap
 */
Handle TEScrapHandle(void) {
    return g_TEScrap;
}

/* ============================================================================
 * Internal Functions
 * ============================================================================ */

/*
 * TE_CopyToScrap - Copy selection to internal scrap
 */
static OSErr TE_CopyToScrap(TEHandle hTE) {
    TEExtPtr pTE;
    SInt32 selLen;
    char *pText;

    if (!hTE) return paramErr;

    HLock((Handle)hTE);
    pTE = (TEExtPtr)*hTE;

    /* Calculate selection length */
    selLen = pTE->base.selEnd - pTE->base.selStart;
    if (selLen <= 0) {
        HUnlock((Handle)hTE);
        return noErr;
    }

    /* Dispose old scrap */
    if (g_TEScrap) {
        DisposeHandle(g_TEScrap);
    }

    /* Allocate new scrap */
    g_TEScrap = NewHandle(selLen);
    if (!g_TEScrap) {
        HUnlock((Handle)hTE);
        return memFullErr;
    }

    /* Copy selection to scrap */
    HLock(pTE->base.hText);
    HLock(g_TEScrap);

    pText = *pTE->base.hText;
    BlockMove(pText + pTE->base.selStart, *g_TEScrap, selLen);

    HUnlock(g_TEScrap);
    HUnlock(pTE->base.hText);

    TEC_LOG("TE_CopyToScrap: copied %d bytes\n", selLen);

    /* TODO: Copy style information if styled */
    if (pTE->hStyles) {
        /* Create style scrap */
    }

    HUnlock((Handle)hTE);

    /* Also put to system scrap */
    return TEToScrap();
}

/*
 * TE_GetFromScrap - Get text from scrap
 */
static OSErr TE_GetFromScrap(TEHandle hTE) {
    /* Load from system scrap if needed */
    if (!g_TEScrap) {
        return TEFromScrap();
    }

    return noErr;
}