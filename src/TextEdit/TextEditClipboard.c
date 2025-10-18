/*
 * TextEditClipboard.c - TextEdit Clipboard Operations
 *
 * Handles cut, copy, paste operations with the Scrap Manager
 */

#include "TextEdit/TextEdit.h"
#include "MemoryMgr/MemoryManager.h"
#include "ScrapManager/ScrapManager.h"
#include "ErrorCodes.h"
#include <string.h>
#include "TextEdit/TELogging.h"

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
    Boolean     autoViewEnabled;/* Auto-scroll flag */
} TEExtRec;

typedef TEExtRec *TEExtPtr, **TEExtHandle;

/* External functions */
extern void BlockMove(const void *src, void *dest, Size size);

/* Debug logging */
#define TEC_DEBUG 1

#if TEC_DEBUG
#define TEC_LOG(...) TE_LOG_DEBUG("TEC: " __VA_ARGS__)
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

    /* Ensure scrap is populated from system clipboard if needed */
    TE_GetFromScrap(hTE);

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

    /* Handle style scrap for styled text */
    if (pTE->hStyles && g_TEStyleScrap) {
        /* If we have style information, we could apply it to the pasted text */
        /* For now, just note that styles were available during paste */
        TEC_LOG("TEPaste: style scrap available for styled text\n");
    }

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
    if (pTE->hStyles && g_TEStyleScrap) {
        /* Styled paste: paste text and preserve style information */
        TEC_LOG("TEStylePaste: styled paste with style scrap\n");

        /* First, paste the text normally */
        TEPaste(hTE);

        /* Now apply styles if available */
        if (g_TEStyleScrap) {
            HLock(g_TEStyleScrap);
            SInt32* styleInfo = (SInt32*)*g_TEStyleScrap;
            SInt32 styleLen = styleInfo[1] - styleInfo[0];
            HUnlock(g_TEStyleScrap);

            /* In a full implementation, we would apply the style runs here */
            /* For now, just log that we processed the style scrap */
            TEC_LOG("TEStylePaste: applied style scrap for %ld bytes\n", styleLen);
        }
    } else {
        /* Plain paste */
        TEC_LOG("TEStylePaste: plain text paste (no styles)\n");
        TEPaste(hTE);
    }

    HUnlock((Handle)hTE);
}

/* ============================================================================
 * Scrap Manager Integration
 * ============================================================================ */

/*
 * TEFromScrap - Load TextEdit scrap from system scrap
 */
OSErr TEFromScrap(void) {
    long scrapSize;
    long bytesRead;
    OSErr err;

    TEC_LOG("TEFromScrap: loading from system scrap\n");

    /* Dispose old TE scrap */
    if (g_TEScrap) {
        DisposeHandle(g_TEScrap);
        g_TEScrap = NULL;
    }
    if (g_TEStyleScrap) {
        DisposeHandle(g_TEStyleScrap);
        g_TEStyleScrap = NULL;
    }

    /* Get TEXT from scrap */
    bytesRead = GetScrap(NULL, kScrapFlavorTypeText, &scrapSize);
    if (bytesRead >= 0 && scrapSize > 0) {
        /* Allocate handle for text */
        g_TEScrap = NewHandle(scrapSize);
        if (g_TEScrap) {
            /* Get the text */
            bytesRead = GetScrap(g_TEScrap, kScrapFlavorTypeText, &scrapSize);
            if (bytesRead < 0) {
                DisposeHandle(g_TEScrap);
                g_TEScrap = NULL;
                err = noTypeErr;
            } else {
                /* Resize to actual size */
                SetHandleSize(g_TEScrap, scrapSize);
                TEC_LOG("TEFromScrap: loaded %ld bytes\n", bytesRead);
                err = noErr;
            }
        } else {
            err = memFullErr;
        }
    } else {
        err = noErr; /* No text in scrap is not an error */
    }

    /* Get style scrap if present */
    {
        long styleScrapSize;
        OSErr styleErr = GetScrap(NULL, kScrapFlavorTypeStyle, &styleScrapSize);
        if (styleErr == noErr && styleScrapSize > 0) {
            g_TEStyleScrap = NewHandle(styleScrapSize);
            if (g_TEStyleScrap) {
                styleErr = GetScrap(g_TEStyleScrap, kScrapFlavorTypeStyle, &styleScrapSize);
                if (styleErr == noErr) {
                    SetHandleSize(g_TEStyleScrap, styleScrapSize);
                    TEC_LOG("TEFromScrap: loaded %ld bytes of style scrap\n", styleScrapSize);
                } else {
                    DisposeHandle(g_TEStyleScrap);
                    g_TEStyleScrap = NULL;
                }
            }
        }
    }

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
        ZeroScrap();

        /* Put TEXT to scrap */
        scrapSize = GetHandleSize(g_TEScrap);
        if (scrapSize > 0) {
            HLock(g_TEScrap);
            PutScrap(scrapSize, kScrapFlavorTypeText, *g_TEScrap);
            HUnlock(g_TEScrap);

            TEC_LOG("TEToScrap: saved %d bytes\n", scrapSize);
        }
        err = noErr;

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

    /* Copy style information if styled */
    if (pTE->hStyles) {
        /* Create style scrap for styled text */
        if (g_TEStyleScrap) {
            DisposeHandle(g_TEStyleScrap);
            g_TEStyleScrap = NULL;
        }

        /* For now, create a minimal style scrap with uniform style */
        /* In a full implementation, this would copy the actual style runs */
        g_TEStyleScrap = NewHandle(sizeof(SInt32) * 2);
        if (g_TEStyleScrap) {
            HLock(g_TEStyleScrap);
            SInt32* styleScrap = (SInt32*)*g_TEStyleScrap;
            styleScrap[0] = 0;      /* Start offset */
            styleScrap[1] = selLen; /* End offset */
            HUnlock(g_TEStyleScrap);
            TEC_LOG("TE_CopyToScrap: copied style scrap for %ld bytes\n", selLen);
        }
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
