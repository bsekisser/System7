/*
 * Simple Resource Manager
 * Minimal implementation for kernel environment
 */

#include "SystemTypes.h"
#include "ResourceMgr/resource_manager.h"
#include <string.h>
#include <stdlib.h>

/* Stub implementations */

OSErr ResourceManagerInit(void) {
    return noErr;
}

/* These are already defined in sys71_stubs.c */
/* Just declare them as external */