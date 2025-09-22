/* VFS operations for Trash support
 * Bridges HFS catalog operations with trash requirements
 */

#include "FS/vfs_ops.h"
#include "FS/vfs.h"
#include <string.h>
#include <stdio.h>

/* Stub implementations - will connect to actual HFS later */

bool VFS_EnsureHiddenFolder(VRefNum vref, const char* name, DirID* outDir) {
    /* For now, return a fake directory ID */
    static DirID nextDir = 1000;

    /* In real implementation:
     * 1. Check if folder exists at root of volume
     * 2. If not, create it
     * 3. Set hidden/system flags
     */

    *outDir = nextDir++;
    return true;
}

bool VFS_Move(VRefNum vref, DirID fromDir, FileID id, DirID toDir, const char* newName) {
    /* In real implementation:
     * 1. Update catalog B-tree entry
     * 2. Change parent directory ID
     * 3. Optionally rename
     */
    return true;
}

bool VFS_DeleteTree(VRefNum vref, DirID parent, FileID id) {
    /* In real implementation:
     * 1. If folder, recursively delete contents
     * 2. Remove catalog entry
     * 3. Free allocation blocks
     */
    return true;
}

bool VFS_Enumerate(VRefNum vref, DirID dir, CatEntry* out, int max, int* outCount) {
    /* In real implementation:
     * Walk catalog B-tree for entries with parent=dir
     */
    *outCount = 0;
    return true;
}

bool VFS_GetDirItemCount(VRefNum vref, DirID dir, uint32_t* outCount, bool recursive) {
    /* Count items in directory */
    *outCount = 0;
    return true;
}

bool VFS_IsOpenByAnyProcess(FileID id) {
    /* Check if file is open */
    return false;
}

bool VFS_IsLocked(FileID id) {
    /* Check Finder info locked flag */
    return false;
}

bool VFS_SetFinderFlags(FileID id, uint16_t setMask, uint16_t clearMask) {
    /* Update Finder info flags */
    return true;
}

bool VFS_GenerateUniqueName(VRefNum vref, DirID dir, const char* base, char* out) {
    /* Generate "Name", "Name 2", "Name 3", etc. */
    if (!VFS_Exists(vref, dir, base)) {
        strncpy(out, base, 31);
        out[31] = 0;
        return true;
    }

    for (int i = 2; i < 1000; i++) {
        char tmp[40];
        snprintf(tmp, sizeof(tmp), "%s %d", base, i);
        if (!VFS_Exists(vref, dir, tmp)) {
            strncpy(out, tmp, 31);
            out[31] = 0;
            return true;
        }
    }

    return false;
}

bool VFS_Exists(VRefNum vref, DirID dir, const char* name) {
    /* Check if name exists in directory */
    return false;  /* For now, assume nothing exists */
}

const char* VFS_GetNameByID(VRefNum vref, DirID parent, FileID id) {
    /* Look up name from catalog */
    static char nameBuf[32] = "Item";
    return nameBuf;
}