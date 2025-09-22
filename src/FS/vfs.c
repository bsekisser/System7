/* Virtual File System Implementation */
#include "../../include/FS/vfs.h"
#include "../../include/FS/hfs_volume.h"
#include "../../include/FS/hfs_catalog.h"
#include "../../include/FS/hfs_file.h"
#include <stdlib.h>
#include <string.h>

/* Serial debug output */
extern void serial_printf(const char* fmt, ...);

/* Static volume buffer - avoid malloc in kernel */
static uint8_t g_volumeBuffer[4 * 1024 * 1024];  /* 4MB static buffer */

/* VFS state */
static struct {
    bool         initialized;
    HFS_Volume   bootVolume;
    HFS_Catalog  bootCatalog;
    VRefNum      bootVRef;
} g_vfs = { 0 };

/* VFS file wrapper */
struct VFSFile {
    HFSFile* hfsFile;
    VRefNum  vref;
};

bool VFS_Init(void) {
    if (g_vfs.initialized) {
        serial_printf("VFS: Already initialized\n");
        return true;
    }

    memset(&g_vfs, 0, sizeof(g_vfs));
    g_vfs.bootVRef = 1;  /* Boot volume is always vRef 1 */

    serial_printf("VFS: Initialized\n");
    g_vfs.initialized = true;
    return true;
}

void VFS_Shutdown(void) {
    if (!g_vfs.initialized) return;

    /* Close catalog if open */
    HFS_CatalogClose(&g_vfs.bootCatalog);

    /* Unmount volume if mounted */
    HFS_VolumeUnmount(&g_vfs.bootVolume);

    g_vfs.initialized = false;
    serial_printf("VFS: Shutdown complete\n");
}

bool VFS_MountBootVolume(const char* volName) {
    if (!g_vfs.initialized) {
        serial_printf("VFS: Not initialized\n");
        return false;
    }

    /* Use static buffer instead of malloc */
    uint64_t volumeSize = sizeof(g_volumeBuffer);
    void* volumeData = g_volumeBuffer;

    /* Create blank HFS volume */
    if (!HFS_CreateBlankVolume(volumeData, volumeSize, volName)) {
        serial_printf("VFS: Failed to create blank volume\n");
        return false;
    }

    /* Mount the volume */
    if (!HFS_VolumeMountMemory(&g_vfs.bootVolume, volumeData, volumeSize, g_vfs.bootVRef)) {
        serial_printf("VFS: Failed to mount volume\n");
        return false;
    }

    /* Skip catalog initialization for now - just show volume icon */
    /* TODO: Fix malloc issues in B-tree before enabling catalog */
    /*
    if (!HFS_CatalogInit(&g_vfs.bootCatalog, &g_vfs.bootVolume)) {
        HFS_VolumeUnmount(&g_vfs.bootVolume);
        serial_printf("VFS: Failed to initialize catalog\n");
        return false;
    }
    */

    serial_printf("VFS: Mounted boot volume successfully\n");

    /* Create some default folders for testing */
    /* These would normally be created by the System installer */
    /* For now, we just report success */

    return true;
}

bool VFS_GetVolumeInfo(VRefNum vref, VolumeControlBlock* vcb) {
    if (!g_vfs.initialized || !vcb) return false;

    if (vref != g_vfs.bootVRef || !g_vfs.bootVolume.mounted) {
        return false;
    }

    return HFS_GetVolumeInfo(&g_vfs.bootVolume, vcb);
}

VRefNum VFS_GetBootVRef(void) {
    return g_vfs.bootVRef;
}

bool VFS_Enumerate(VRefNum vref, DirID dir, CatEntry* entries, int maxEntries, int* count) {
    if (!g_vfs.initialized || !entries || !count) return false;

    if (vref != g_vfs.bootVRef || !g_vfs.bootVolume.mounted) {
        return false;
    }

    /* If enumerating an empty volume, return empty list for now */
    if (!g_vfs.bootCatalog.bt.nodeBuffer) {
        *count = 0;
        return true;
    }

    return HFS_CatalogEnumerate(&g_vfs.bootCatalog, dir, entries, maxEntries, count);
}

bool VFS_Lookup(VRefNum vref, DirID dir, const char* name, CatEntry* entry) {
    if (!g_vfs.initialized || !name || !entry) return false;

    if (vref != g_vfs.bootVRef || !g_vfs.bootVolume.mounted) {
        return false;
    }

    return HFS_CatalogLookup(&g_vfs.bootCatalog, dir, name, entry);
}

bool VFS_GetByID(VRefNum vref, FileID id, CatEntry* entry) {
    if (!g_vfs.initialized || !entry) return false;

    if (vref != g_vfs.bootVRef || !g_vfs.bootVolume.mounted) {
        return false;
    }

    return HFS_CatalogGetByID(&g_vfs.bootCatalog, id, entry);
}

VFSFile* VFS_OpenFile(VRefNum vref, FileID id, bool resourceFork) {
    if (!g_vfs.initialized) return NULL;

    if (vref != g_vfs.bootVRef || !g_vfs.bootVolume.mounted) {
        return NULL;
    }

    HFSFile* hfsFile = HFS_FileOpen(&g_vfs.bootCatalog, id, resourceFork);
    if (!hfsFile) return NULL;

    VFSFile* vfsFile = (VFSFile*)malloc(sizeof(VFSFile));
    if (!vfsFile) {
        HFS_FileClose(hfsFile);
        return NULL;
    }

    vfsFile->hfsFile = hfsFile;
    vfsFile->vref = vref;

    return vfsFile;
}

VFSFile* VFS_OpenByPath(VRefNum vref, const char* path, bool resourceFork) {
    if (!g_vfs.initialized || !path) return NULL;

    if (vref != g_vfs.bootVRef || !g_vfs.bootVolume.mounted) {
        return NULL;
    }

    HFSFile* hfsFile = HFS_FileOpenByPath(&g_vfs.bootCatalog, path, resourceFork);
    if (!hfsFile) return NULL;

    VFSFile* vfsFile = (VFSFile*)malloc(sizeof(VFSFile));
    if (!vfsFile) {
        HFS_FileClose(hfsFile);
        return NULL;
    }

    vfsFile->hfsFile = hfsFile;
    vfsFile->vref = vref;

    return vfsFile;
}

void VFS_CloseFile(VFSFile* file) {
    if (!file) return;

    if (file->hfsFile) {
        HFS_FileClose(file->hfsFile);
    }

    free(file);
}

bool VFS_ReadFile(VFSFile* file, void* buffer, uint32_t length, uint32_t* bytesRead) {
    if (!file || !file->hfsFile) return false;
    return HFS_FileRead(file->hfsFile, buffer, length, bytesRead);
}

bool VFS_SeekFile(VFSFile* file, uint32_t position) {
    if (!file || !file->hfsFile) return false;
    return HFS_FileSeek(file->hfsFile, position);
}

uint32_t VFS_GetFileSize(VFSFile* file) {
    if (!file || !file->hfsFile) return 0;
    return HFS_FileGetSize(file->hfsFile);
}

uint32_t VFS_GetFilePosition(VFSFile* file) {
    if (!file || !file->hfsFile) return 0;
    return HFS_FileTell(file->hfsFile);
}

/* Write operations - stubs for now */
bool VFS_CreateFolder(VRefNum vref, DirID parent, const char* name, DirID* newID) {
    /* TODO: Implement folder creation */
    serial_printf("VFS: CreateFolder not implemented\n");
    return false;
}

bool VFS_CreateFile(VRefNum vref, DirID parent, const char* name,
                   uint32_t type, uint32_t creator, FileID* newID) {
    /* TODO: Implement file creation */
    serial_printf("VFS: CreateFile not implemented\n");
    return false;
}

bool VFS_Rename(VRefNum vref, FileID id, const char* newName) {
    /* TODO: Implement rename */
    serial_printf("VFS: Rename not implemented\n");
    return false;
}

bool VFS_Delete(VRefNum vref, FileID id) {
    /* TODO: Implement delete */
    serial_printf("VFS: Delete not implemented\n");
    return false;
}