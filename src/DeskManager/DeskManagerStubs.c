/* DeskManager Stub Functions */

#include <stdlib.h>
#include <string.h>
#include "SystemTypes.h"
#include "System71StdLib.h"
#include "System/SystemLogging.h"
#include "DeskManager/DeskManager.h"
#include "DeskManager/DeskAccessory.h"

/* Global DA Registry - using a simplified linked list */
typedef struct DARegistryNode {
    DARegistryEntry entry;
    struct DARegistryNode *next;
} DARegistryNode;

static DARegistryNode *g_daRegistry = NULL;

/* Register built-in desk accessories */
int DeskManager_RegisterBuiltinDAs(void) {
    /* Stub - would register Calculator, KeyCaps, etc. */
    SYSTEM_LOG_DEBUG("DeskManager: Registering built-in DAs\n");
    return 0;
}

/* Find a DA in the registry */
DARegistryEntry *DA_FindRegistryEntry(const char *name) {
    if (!name) return NULL;

    DARegistryNode *node = g_daRegistry;
    while (node) {
        if (strcmp(node->entry.name, name) == 0) {
            return &node->entry;
        }
        node = node->next;
    }
    return NULL;
}

/* System Menu functions */
void SystemMenu_Update(void) {
    /* Stub - would update the Apple menu */
    SYSTEM_LOG_DEBUG("SystemMenu: Update\n");
}

int SystemMenu_AddDA(DeskAccessory *da) {
    if (!da) return -1;
    /* Stub - would add DA to Apple menu */
    SYSTEM_LOG_DEBUG("SystemMenu: Add DA %s\n", da->name);
    return 0;
}

void SystemMenu_RemoveDA(DeskAccessory *da) {
    if (!da) return;
    /* Stub - would remove DA from Apple menu */
    SYSTEM_LOG_DEBUG("SystemMenu: Remove DA %s\n", da->name);
}

/* This DA_Register conflicts with the one in DeskAccessory.h - removed */