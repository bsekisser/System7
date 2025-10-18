/* #include "SystemTypes.h" */
#include "MenuManager/menu_private.h"
#include <string.h>
/*
 * MenuResources.c - Menu Resource Loading Implementation
 *
 * This file implements MENU and MBAR resource loading and menu creation from resources.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 * Derived from System 7 ROM analysis (Ghidra) Menu Manager
 */

// #include "CompatibilityFix.h" // Removed
#include "SystemTypes.h"
#include "System71StdLib.h"
#include "ResourceMgr/ResourceMgr.h"

#include "MenuManager/MenuManager.h"
#include "MenuManager/MenuTypes.h"
#include "MenuManager/MenuLogging.h"

/* ===== MENU Resource Parser ===== */

/*
 * ParseMENUResource
 * Parses a MENU resource and returns a MenuHandle
 *
 * MENU resource format (big-endian):
 * 0x00-0x01: menuID (int16)
 * 0x02-0x03: menuWidth (int16, 0 = auto)
 * 0x04-0x05: menuHeight (int16, 0 = auto)
 * 0x06-0x09: reserved (should be 0xFFFFFFFF)
 * 0x0A: title length (byte)
 * 0x0B+: title text
 * Then: item count and items...
 */
MenuHandle ParseMENUResource(Handle resourceHandle)
{
    if (!resourceHandle) {
        return NULL;
    }

    uint8_t* data = (uint8_t*)*resourceHandle;
    if (!data) {
        return NULL;
    }

    /* Read menu ID (big-endian) */
    short menuID = (data[0] << 8) | data[1];

    /* Read menu dimensions (we'll auto-calculate in most cases) */
    /* short menuWidth = (data[2] << 8) | data[3]; */
    /* short menuHeight = (data[4] << 8) | data[5]; */

    /* Skip reserved field (0x06-0x09) */

    /* Read title length */
    uint8_t titleLen = data[0x0A];

    if (titleLen == 0) {
        MENU_LOG_WARN("ParseMENUResource: Menu %d has no title\n", menuID);
        titleLen = 1;  /* Use minimal title */
    }

    /* Extract title as Pascal string */
    Str255 titlePascal;
    titlePascal[0] = titleLen;
    memcpy(&titlePascal[1], &data[0x0B], titleLen);

    /* Create menu with parsed title */
    MenuHandle theMenu = NewMenu(menuID, titlePascal);
    if (!theMenu) {
        MENU_LOG_ERROR("ParseMENUResource: NewMenu failed for ID %d\n", menuID);
        return NULL;
    }

    /* Parse menu items - simplified: append raw item data */
    /* In a real implementation, we'd parse each item and call AppendMenu */
    /* For now, just parse items from the resource data starting after title */
    uint8_t itemCount = data[0x0B + titleLen];

    MENU_LOG_DEBUG("ParseMENUResource: Parsed menu ID=%d, title len=%d, items=%d\n",
                   menuID, titleLen, itemCount);

    return theMenu;
}

/*
 * ParseMBARResource
 * Parses an MBAR resource and returns array of menu IDs
 *
 * MBAR resource format (big-endian):
 * 0x00-0x01: count of menus (int16)
 * 0x02+: array of menu IDs (int16 each)
 */
short* ParseMBARResource(Handle resourceHandle, short* outMenuCount)
{
    if (!resourceHandle || !outMenuCount) {
        return NULL;
    }

    uint8_t* data = (uint8_t*)*resourceHandle;
    if (!data) {
        return NULL;
    }

    /* Read menu count (big-endian) */
    short count = (data[0] << 8) | data[1];

    if (count <= 0 || count > 32) {
        MENU_LOG_ERROR("ParseMBARResource: Invalid menu count %d\n", count);
        return NULL;
    }

    /* Allocate array for menu IDs */
    short* menuIDs = (short*)malloc(count * sizeof(short));
    if (!menuIDs) {
        return NULL;
    }

    /* Extract menu IDs (big-endian int16) */
    for (int i = 0; i < count; i++) {
        uint8_t* menuIDPtr = &data[2 + (i * 2)];
        /* Big-endian conversion */
        menuIDs[i] = (int16_t)(((int16_t)menuIDPtr[0] << 8) | menuIDPtr[1]);
    }

    *outMenuCount = count;
    return menuIDs;
}

/* Resource menu functions (implemented in sys71_stubs.c to avoid duplication) */
/* See AddResMenu, InsertResMenu, InsertFontResMenu, InsertIntlResMenu in sys71_stubs.c */
