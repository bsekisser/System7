/* #include "SystemTypes.h" */
#include <string.h>
/**
 * @file ControlResources.c
 * @brief CNTL resource loading and template processing
 *
 * This file implements functionality for loading and managing control
 * resources, including CNTL resources and control templates.
 *
 * Copyright (c) 2024 - System 7.1 Portable Toolbox Project
 * Licensed under MIT License
 */

// #include "CompatibilityFix.h" // Removed
#include "SystemTypes.h"
#include "System71StdLib.h"
/* ControlResources.h local */
#include "ControlManager/ControlManager.h"
#include "ResourceMgr/resource_types.h"
#include "SystemTypes.h"
#include "System71StdLib.h"


/* CNTL resource structure */
typedef struct CNTLResource {
    Rect    boundsRect;
    SInt16 value;
    SInt16 visible;
    SInt16 max;
    SInt16 min;
    SInt16 procID;
    SInt32 refCon;
    Str255  title;
} CNTLResource;

/**
 * Load a control from a CNTL resource
 */
ControlHandle LoadControlFromResource(Handle cntlResource, WindowPtr owner) {
    CNTLResource cntlData;
    ControlHandle control;
    OSErr err;

    if (!cntlResource || !owner) {
        return NULL;
    }

    /* Parse CNTL resource */
    err = ParseCNTLResource(cntlResource, &cntlData);
    if (err != noErr) {
        return NULL;
    }

    /* Create control from resource data */
    control = NewControl(owner, &cntlData.boundsRect, cntlData.title,
                         cntlData.visible != 0, cntlData.value,
                         cntlData.min, cntlData.max, cntlData.procID,
                         cntlData.refCon);

    return control;
}

/**
 * Parse a CNTL resource
 */
OSErr ParseCNTLResource(Handle resource, CNTLResource *cntlData) {
    UInt8 *data;
    SInt16 titleLen;

    if (!resource || !cntlData) {
        return paramErr;
    }

    HLock(resource);
    data = (UInt8 *)*resource;

    /* Read rectangle bounds */
    (cntlData)->top = *(SInt16 *)data; data += 2;
    (cntlData)->left = *(SInt16 *)data; data += 2;
    (cntlData)->bottom = *(SInt16 *)data; data += 2;
    (cntlData)->right = *(SInt16 *)data; data += 2;

    /* Read control properties */
    cntlData->value = *(SInt16 *)data; data += 2;
    cntlData->visible = *(SInt16 *)data; data += 2;
    cntlData->max = *(SInt16 *)data; data += 2;
    cntlData->min = *(SInt16 *)data; data += 2;
    cntlData->procID = *(SInt16 *)data; data += 2;
    cntlData->refCon = *(SInt32 *)data; data += 4;

    /* Read title (Pascal string) */
    titleLen = *data;
    if (titleLen > 255) titleLen = 255;
    memcpy(cntlData->title, data, titleLen + 1);

    HUnlock(resource);
    return noErr;
}

/**
 * Create a CNTL resource from control data
 */
Handle CreateCNTLResource(const CNTLResource *cntlData) {
    Handle resource;
    UInt8 *data;
    SInt32 dataSize;
    SInt16 titleLen;

    if (!cntlData) {
        return NULL;
    }

    /* Calculate resource size */
    titleLen = cntlData->title[0];
    dataSize = 8 + 10 + 4 + titleLen + 1; /* rect + 5 shorts + long + string */

    /* Allocate resource */
    resource = NewHandle(dataSize);
    if (!resource) {
        return NULL;
    }

    HLock(resource);
    data = (UInt8 *)*resource;

    /* Write bounds rectangle */
    *(SInt16 *)data = (cntlData)->top; data += 2;
    *(SInt16 *)data = (cntlData)->left; data += 2;
    *(SInt16 *)data = (cntlData)->bottom; data += 2;
    *(SInt16 *)data = (cntlData)->right; data += 2;

    /* Write control properties */
    *(SInt16 *)data = cntlData->value; data += 2;
    *(SInt16 *)data = cntlData->visible; data += 2;
    *(SInt16 *)data = cntlData->max; data += 2;
    *(SInt16 *)data = cntlData->min; data += 2;
    *(SInt16 *)data = cntlData->procID; data += 2;
    *(SInt32 *)data = cntlData->refCon; data += 4;

    /* Write title */
    memcpy(data, cntlData->title, titleLen + 1);

    HUnlock(resource);
    return resource;
}
