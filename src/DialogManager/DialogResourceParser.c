/*
#include "DialogManager/DialogInternal.h"
 * DialogResourceParser.c - DLOG and DITL Resource Parsing
 *
 * Implements faithful System 7.1-compatible resource parsing for dialog templates
 * and dialog item lists. This module parses the binary resource format and creates
 * internal structures used by the Dialog Manager.
 */

#include <stdlib.h>
#include <string.h>
#include "SystemTypes.h"
#include "System71StdLib.h"
#include "DialogManager/DialogManager.h"
#include "DialogManager/DialogTypes.h"
#include "DialogManager/DialogManagerInternal.h"
#include "DialogManager/DialogLogging.h"

/* DITL resource format (System 7.1):
 *
 * Word: item count - 1  (so count = value + 1)
 *
 * For each item:
 *   Long: placeholder (always 0)
 *   Rect: display rectangle (top, left, bottom, right as 4 words)
 *   Byte: item type (with flags in high bit)
 *   Byte or Word: item data length
 *   Data: item-specific data (varies by type)
 */

/* Parse DITL resource into DialogItemEx array */
OSErr ParseDITL(Handle ditlHandle, DialogItemEx** items, SInt16* itemCount) {
    unsigned char* p;
    SInt16 count;
    SInt16 i;
    DialogItemEx* itemArray;

    if (!ditlHandle || !items || !itemCount) {
        // DIALOG_LOG_DEBUG("Dialog: ParseDITL - invalid parameters\n");
        return -1;
    }

    if (!*ditlHandle) {
        // DIALOG_LOG_DEBUG("Dialog: ParseDITL - null handle data\n");
        return -1;
    }

    /* CRITICAL: Lock handle before dereferencing to prevent heap compaction issues */
    HLock(ditlHandle);
    p = (unsigned char*)*ditlHandle;

    /* Read item count (stored as count-1 in resource) */
    count = ((SInt16)p[0] << 8) | p[1];
    count += 1;  /* Actual count is stored value + 1 */
    p += 2;

    if (count < 0 || count > 256) {
        // DIALOG_LOG_DEBUG("Dialog: ParseDITL - invalid item count %d\n", count);
        HUnlock(ditlHandle);
        return -1;
    }

    *itemCount = count;
    // DIALOG_LOG_DEBUG("Dialog: Parsing DITL with %d items\n", count);

    /* Allocate item array */
    itemArray = (DialogItemEx*)malloc(count * sizeof(DialogItemEx));
    if (!itemArray) {
        // DIALOG_LOG_DEBUG("Dialog: ParseDITL - malloc failed\n");
        HUnlock(ditlHandle);
        return -108;  /* memFullErr */
    }

    memset(itemArray, 0, count * sizeof(DialogItemEx));

    /* Parse each item */
    for (i = 0; i < count; i++) {
        SInt16 itemType;
        SInt16 dataLen;
        Rect bounds;

        /* Skip placeholder (4 bytes) */
        p += 4;

        /* Read bounds (8 bytes: top, left, bottom, right) */
        bounds.top = ((SInt16)p[0] << 8) | p[1];
        bounds.left = ((SInt16)p[2] << 8) | p[3];
        bounds.bottom = ((SInt16)p[4] << 8) | p[5];
        bounds.right = ((SInt16)p[6] << 8) | p[7];
        p += 8;

        /* Read item type */
        itemType = *p++;

        /* Read data length (byte or word depending on odd length) */
        dataLen = *p++;
        if (dataLen == 0xFF) {
            /* Long data length (next word) */
            dataLen = ((SInt16)p[0] << 8) | p[1];
            p += 2;
        }

        /* Initialize item */
        itemArray[i].bounds = bounds;
        itemArray[i].type = itemType;
        itemArray[i].index = i;
        itemArray[i].visible = true;
        itemArray[i].enabled = (itemType & itemDisable) == 0;
        itemArray[i].refCon = 0;
        itemArray[i].userData = NULL;
        itemArray[i].platformData = NULL;

        /* Parse item data based on type */
        {
            SInt16 baseType = itemType & itemTypeMask;

            if (baseType == btnCtrl || baseType == chkCtrl || baseType == radCtrl ||
                baseType == statText || baseType == editText) {
                /* Text-based item - copy string data */
                if (dataLen > 0) {
                    unsigned char* textData = (unsigned char*)malloc(dataLen + 1);
                    if (textData) {
                        memcpy(textData, p, dataLen);
                        textData[dataLen] = 0;
                        itemArray[i].data = textData;
                    }
                }
            } else if (baseType == iconItem || baseType == picItem) {
                /* Resource ID stored as 2-byte integer */
                if (dataLen >= 2) {
                    SInt16 resID = ((SInt16)p[0] << 8) | p[1];
                    itemArray[i].refCon = resID;
                }
            } else if (baseType == userItem) {
                /* User item has no data, just note it */
                itemArray[i].handle = NULL;
            }
        }

        p += dataLen;

        /* Align to word boundary */
        if ((unsigned long)p & 1) {
            p++;
        }

        // DIALOG_LOG_DEBUG("Dialog: Item %d: type=%d bounds=(%d,%d,%d,%d)\n", i+1, itemType, bounds.top, bounds.left, bounds.bottom, bounds.right);
    }

    *items = itemArray;
    HUnlock(ditlHandle);
    return 0;  /* noErr */
}

/* Free parsed DITL items */
void FreeParsedDITL(DialogItemEx* items, SInt16 itemCount) {
    SInt16 i;

    if (!items) return;

    for (i = 0; i < itemCount; i++) {
        if (items[i].data) {
            free(items[i].data);
            items[i].data = NULL;
        }
    }

    free(items);
}

/* Create a simple default DITL for testing (OK and Cancel buttons) */
Handle CreateDefaultDITL(void) {
    /* Simplified DITL with just OK button for now */
    static unsigned char defaultDITL[] = {
        0x00, 0x00,  /* 1 item (stored as 0) */

        /* Item 1: OK Button */
        0x00, 0x00, 0x00, 0x00,  /* Placeholder */
        0x00, 0x78,  /* top = 120 */
        0x00, 0xFA,  /* left = 250 */
        0x00, 0x8C,  /* bottom = 140 */
        0x01, 0x3E,  /* right = 318 */
        0x04,        /* type = btnCtrl (4) */
        0x02,        /* data length = 2 */
        0x4F, 0x4B   /* "OK" in Pascal string format (length byte omitted for simplicity) */
    };

    Handle h = (Handle)malloc(sizeof(Ptr));
    if (!h) return NULL;

    *h = (Ptr)malloc(sizeof(defaultDITL));
    if (!*h) {
        free(h);
        return NULL;
    }

    memcpy(*h, defaultDITL, sizeof(defaultDITL));

    return h;
}
