/*
 * CPUBackend.c - CPU Backend Registry
 *
 * Manages registration and lookup of CPU backends.
 */

#include "CPU/CPUBackend.h"
#include "System71StdLib.h"
#include <string.h>

#define MAX_BACKENDS 8

typedef struct BackendEntry {
    char name[32];
    const ICPUBackend* backend;
} BackendEntry;

static BackendEntry gBackendRegistry[MAX_BACKENDS];
static int gBackendCount = 0;
static const ICPUBackend* gDefaultBackend = NULL;

/*
 * CPUBackend_Register - Register a CPU backend
 */
OSErr CPUBackend_Register(const char* name, const ICPUBackend* backend)
{
    if (!name || !backend) {
        return paramErr;
    }

    if (gBackendCount >= MAX_BACKENDS) {
        return memFullErr;
    }

    /* Check for duplicate */
    for (int i = 0; i < gBackendCount; i++) {
        if (strcmp(gBackendRegistry[i].name, name) == 0) {
            return dupFNErr; /* Already registered */
        }
    }

    /* Add to registry */
    strncpy(gBackendRegistry[gBackendCount].name, name, 31);
    gBackendRegistry[gBackendCount].name[31] = '\0';
    gBackendRegistry[gBackendCount].backend = backend;
    gBackendCount++;

    /* First backend registered becomes default */
    if (gDefaultBackend == NULL) {
        gDefaultBackend = backend;
    }

    return noErr;
}

/*
 * CPUBackend_Get - Get backend by name
 */
const ICPUBackend* CPUBackend_Get(const char* name)
{
    if (!name) {
        return NULL;
    }

    for (int i = 0; i < gBackendCount; i++) {
        if (strcmp(gBackendRegistry[i].name, name) == 0) {
            return gBackendRegistry[i].backend;
        }
    }

    return NULL;
}

/*
 * CPUBackend_GetDefault - Get default backend
 */
const ICPUBackend* CPUBackend_GetDefault(void)
{
    return gDefaultBackend;
}
