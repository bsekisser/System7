/* VFS operations for Trash support
 * Bridges HFS catalog operations with trash requirements
 */

#include "FS/vfs_ops.h"
#include <string.h>

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
    extern void serial_printf(const char* fmt, ...);
    serial_printf("VFS_Move: id=%u from dir=%u to dir=%u, newName=%s\n",
                 id, fromDir, toDir, newName ? newName : "(null)");
    return true;
}

bool VFS_Copy(VRefNum vref, DirID fromDir, FileID id, DirID toDir, const char* newName, FileID* newID) {
    /* In real implementation:
     * 1. Read source file data and metadata
     * 2. Create new catalog entry in destination
     * 3. Copy file data blocks
     * 4. Copy resource fork if present
     */
    extern void serial_printf(const char* fmt, ...);
    static FileID nextCopyID = 10000;

    serial_printf("VFS_Copy: id=%u from dir=%u to dir=%u, newName=%s\n",
                 id, fromDir, toDir, newName ? newName : "(null)");

    if (newID) {
        *newID = nextCopyID++;
    }
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

/* VFS_Enumerate is already defined in vfs.c */

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
        /* Manual string formatting to avoid stdio dependency */
        char tmp[40];
        int base_len = strlen(base);
        if (base_len > 25) base_len = 25;  /* Leave room for " NNN" */
        memcpy(tmp, base, base_len);
        tmp[base_len] = ' ';

        /* Convert number to string manually */
        int num = i;
        int digits = 0;
        char numbuf[10];
        do {
            numbuf[digits++] = '0' + (num % 10);
            num /= 10;
        } while (num > 0);

        /* Copy digits in reverse order */
        for (int d = 0; d < digits; d++) {
            tmp[base_len + 1 + d] = numbuf[digits - 1 - d];
        }
        tmp[base_len + 1 + digits] = 0;

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

VRefNum VFS_GetVRefByID(FileID id) {
    /* In real implementation: look up volume ref from catalog */
    extern VRefNum VFS_GetBootVRef(void);
    return VFS_GetBootVRef();  /* For now, assume all files on boot volume */
}

bool VFS_GetParentDir(VRefNum vref, FileID id, DirID* parentDir) {
    /* In real implementation: read catalog entry to get parent dirID */
    extern void serial_printf(const char* fmt, ...);
    if (parentDir) {
        *parentDir = 2;  /* HFS_ROOT_CNID - For now, assume root */
        serial_printf("VFS_GetParentDir: id=%u -> parent=%u\n", id, *parentDir);
        return true;
    }
    return false;
}