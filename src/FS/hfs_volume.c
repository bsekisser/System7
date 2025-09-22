/* HFS Volume Management Implementation */
#include "../../include/FS/hfs_volume.h"
#include "../../include/FS/hfs_endian.h"
#include <string.h>
#include <stdlib.h>

/* Serial debug output */
extern void serial_printf(const char* fmt, ...);

/* Pascal string to C string conversion */
static void pstr_to_cstr(char* dst, const uint8_t* src, size_t maxlen) {
    uint8_t len = src[0];
    if (len > maxlen - 1) len = maxlen - 1;
    memcpy(dst, src + 1, len);
    dst[len] = '\0';
}

/* C string to Pascal string conversion */
static void cstr_to_pstr(uint8_t* dst, const char* src, size_t maxlen) {
    size_t len = strlen(src);
    if (len > maxlen) len = maxlen;
    dst[0] = len;
    memcpy(dst + 1, src, len);
}

bool HFS_VolumeMount(HFS_Volume* vol, const char* imagePath, VRefNum vRefNum) {
    if (!vol) return false;

    memset(vol, 0, sizeof(HFS_Volume));

    /* Initialize block device */
    if (!HFS_BD_InitFile(&vol->bd, imagePath, false)) {
        return false;
    }

    /* Read MDB from sector 2 */
    uint8_t mdbBuffer[512];
    if (!HFS_BD_ReadSector(&vol->bd, HFS_MDB_SECTOR, mdbBuffer)) {
        HFS_BD_Close(&vol->bd);
        return false;
    }

    /* Verify HFS signature */
    uint16_t sig = be16_read(&mdbBuffer[0]);
    if (sig != HFS_SIGNATURE) {
        serial_printf("HFS: Invalid signature 0x%04x (expected 0x4244)\n", sig);
        HFS_BD_Close(&vol->bd);
        return false;
    }

    /* Parse MDB fields */
    HFS_MDB* mdb = &vol->mdb;
    mdb->drSigWord    = sig;
    mdb->drCrDate     = be32_read(&mdbBuffer[4]);
    mdb->drLsMod      = be32_read(&mdbBuffer[8]);
    mdb->drAtrb       = be16_read(&mdbBuffer[12]);
    mdb->drNmFls      = be16_read(&mdbBuffer[14]);
    mdb->drVBMSt      = be16_read(&mdbBuffer[16]);
    mdb->drAllocPtr   = be16_read(&mdbBuffer[18]);
    mdb->drNmAlBlks   = be16_read(&mdbBuffer[20]);
    mdb->drAlBlkSiz   = be32_read(&mdbBuffer[22]);
    mdb->drClpSiz     = be32_read(&mdbBuffer[26]);
    mdb->drAlBlSt     = be16_read(&mdbBuffer[30]);
    mdb->drNxtCNID    = be32_read(&mdbBuffer[32]);
    mdb->drFreeBks    = be16_read(&mdbBuffer[36]);

    /* Volume name */
    memcpy(mdb->drVN, &mdbBuffer[38], 28);
    pstr_to_cstr(vol->volName, mdb->drVN, sizeof(vol->volName));

    /* More MDB fields */
    mdb->drVolBkUp    = be32_read(&mdbBuffer[66]);
    mdb->drVSeqNum    = be16_read(&mdbBuffer[70]);
    mdb->drWrCnt      = be32_read(&mdbBuffer[72]);
    mdb->drXTClpSiz   = be32_read(&mdbBuffer[76]);
    mdb->drCTClpSiz   = be32_read(&mdbBuffer[80]);
    mdb->drNmRtDirs   = be16_read(&mdbBuffer[84]);
    mdb->drFilCnt     = be32_read(&mdbBuffer[86]);
    mdb->drDirCnt     = be32_read(&mdbBuffer[90]);

    /* Finder info */
    for (int i = 0; i < 8; i++) {
        mdb->drFndrInfo[i] = be32_read(&mdbBuffer[94 + i * 4]);
    }

    /* Extents overflow file */
    mdb->drXTFlSize = be32_read(&mdbBuffer[126]);
    for (int i = 0; i < 3; i++) {
        mdb->drXTExtRec[i].startBlock = be16_read(&mdbBuffer[130 + i * 4]);
        mdb->drXTExtRec[i].blockCount = be16_read(&mdbBuffer[132 + i * 4]);
    }

    /* Catalog file */
    mdb->drCTFlSize = be32_read(&mdbBuffer[142]);
    for (int i = 0; i < 3; i++) {
        mdb->drCTExtRec[i].startBlock = be16_read(&mdbBuffer[146 + i * 4]);
        mdb->drCTExtRec[i].blockCount = be16_read(&mdbBuffer[148 + i * 4]);
    }

    /* Cache frequently used values */
    vol->alBlkSize   = mdb->drAlBlkSiz;
    vol->alBlSt      = mdb->drAlBlSt;
    vol->vbmStart    = mdb->drVBMSt;
    vol->numAlBlks   = mdb->drNmAlBlks;
    vol->catFileSize = mdb->drCTFlSize;
    memcpy(vol->catExtents, mdb->drCTExtRec, sizeof(vol->catExtents));
    vol->extFileSize = mdb->drXTFlSize;
    memcpy(vol->extExtents, mdb->drXTExtRec, sizeof(vol->extExtents));
    vol->rootDirID   = 2;  /* Standard HFS root CNID */
    vol->nextCNID    = mdb->drNxtCNID;
    vol->mounted     = true;
    vol->vRefNum     = vRefNum;

    serial_printf("HFS: Mounted volume '%s' (vRef=%d)\n", vol->volName, vRefNum);
    serial_printf("  Allocation blocks: %u x %u bytes\n", vol->numAlBlks, vol->alBlkSize);
    serial_printf("  Catalog size: %u bytes\n", vol->catFileSize);
    serial_printf("  Files: %u, Dirs: %u\n", mdb->drFilCnt, mdb->drDirCnt);

    return true;
}

bool HFS_VolumeMountMemory(HFS_Volume* vol, void* buffer, uint64_t size, VRefNum vRefNum) {
    if (!vol || !buffer || size < 1024 * 1024) return false;  /* Minimum 1MB */

    memset(vol, 0, sizeof(HFS_Volume));

    /* Initialize block device from memory */
    if (!HFS_BD_InitMemory(&vol->bd, buffer, size)) {
        return false;
    }

    /* Try to read existing MDB */
    uint8_t mdbBuffer[512];
    if (HFS_BD_ReadSector(&vol->bd, HFS_MDB_SECTOR, mdbBuffer)) {
        uint16_t sig = be16_read(&mdbBuffer[0]);
        if (sig == HFS_SIGNATURE) {
            /* Valid HFS volume - mount it */
            HFS_BD_Close(&vol->bd);
            HFS_BD_InitMemory(&vol->bd, buffer, size);  /* Reinit */
            return HFS_VolumeMount(vol, NULL, vRefNum);
        }
    }

    /* No valid HFS - create blank volume */
    return HFS_CreateBlankVolume(buffer, size, "Macintosh HD");
}

void HFS_VolumeUnmount(HFS_Volume* vol) {
    if (!vol) return;

    if (vol->mounted) {
        serial_printf("HFS: Unmounting volume '%s'\n", vol->volName);
    }

    HFS_BD_Close(&vol->bd);
    memset(vol, 0, sizeof(HFS_Volume));
}

uint64_t HFS_AllocBlockToByteOffset(const HFS_Volume* vol, uint32_t allocBlock) {
    if (!vol || !vol->mounted) return 0;

    /* Calculate offset: allocation blocks start after system area */
    uint64_t systemAreaBytes = (uint64_t)vol->alBlSt * vol->alBlkSize;
    return systemAreaBytes + ((uint64_t)allocBlock * vol->alBlkSize);
}

bool HFS_ReadAllocBlocks(const HFS_Volume* vol, uint32_t startBlock, uint32_t blockCount,
                         void* buffer) {
    if (!vol || !vol->mounted || !buffer) return false;

    uint64_t offset = HFS_AllocBlockToByteOffset(vol, startBlock);
    uint32_t length = blockCount * vol->alBlkSize;

    return HFS_BD_Read(&vol->bd, offset, buffer, length);
}

bool HFS_GetVolumeInfo(const HFS_Volume* vol, VolumeControlBlock* vcb) {
    if (!vol || !vol->mounted || !vcb) return false;

    memset(vcb, 0, sizeof(VolumeControlBlock));
    strncpy(vcb->name, vol->volName, sizeof(vcb->name) - 1);
    vcb->vRefNum = vol->vRefNum;
    vcb->totalBytes = (uint64_t)vol->numAlBlks * vol->alBlkSize;
    vcb->freeBytes = (uint64_t)vol->mdb.drFreeBks * vol->alBlkSize;
    vcb->rootID = vol->rootDirID;
    vcb->mounted = vol->mounted;

    return true;
}

bool HFS_CreateBlankVolume(void* buffer, uint64_t size, const char* volName) {
    if (!buffer || size < 1024 * 1024) return false;  /* Minimum 1MB */

    /* Clear the buffer */
    memset(buffer, 0, size);

    /* Calculate volume parameters - use 32-bit to avoid libgcc dependency */
    uint32_t alBlkSize = 512;  /* Start with 512 byte allocation blocks */
    uint32_t size32 = (size > 0xFFFFFFFF) ? 0xFFFFFFFF : (uint32_t)size;
    uint32_t totalBlocks = size32 / alBlkSize;

    /* Adjust allocation block size for larger volumes */
    while (totalBlocks > 65535 && alBlkSize < 4096) {
        alBlkSize *= 2;
        totalBlocks = size32 / alBlkSize;
    }

    /* System area: boot blocks (2) + MDB (1) + VBM + reserved */
    uint16_t alBlSt = 16;  /* First allocation block for data */
    uint16_t numAlBlks = totalBlocks - alBlSt;
    uint16_t vbmStart = 3;  /* VBM starts at block 3 */
    uint16_t vbmBlocks = (numAlBlks + 4095) / 4096;  /* 1 bit per block */

    /* Create MDB at sector 2 */
    uint8_t mdb[512];
    memset(mdb, 0, sizeof(mdb));

    /* Fill MDB fields */
    be16_write(&mdb[0], HFS_SIGNATURE);               /* drSigWord */
    be32_write(&mdb[4], 0);                          /* drCrDate - will set later */
    be32_write(&mdb[8], 0);                          /* drLsMod */
    be16_write(&mdb[12], 0);                         /* drAtrb */
    be16_write(&mdb[14], 0);                         /* drNmFls */
    be16_write(&mdb[16], vbmStart);                  /* drVBMSt */
    be16_write(&mdb[18], 0);                         /* drAllocPtr */
    be16_write(&mdb[20], numAlBlks);                 /* drNmAlBlks */
    be32_write(&mdb[22], alBlkSize);                 /* drAlBlkSiz */
    be32_write(&mdb[26], 4096);                      /* drClpSiz */
    be16_write(&mdb[30], alBlSt);                    /* drAlBlSt */
    be32_write(&mdb[32], 16);                        /* drNxtCNID */
    be16_write(&mdb[36], numAlBlks - 10);            /* drFreeBks (reserve some) */

    /* Volume name */
    uint8_t pname[28];
    memset(pname, 0, sizeof(pname));
    cstr_to_pstr(pname, volName, 27);
    memcpy(&mdb[38], pname, 28);                     /* drVN */

    /* Catalog file - allocate 10 blocks */
    be32_write(&mdb[142], 10 * alBlkSize);           /* drCTFlSize */
    be16_write(&mdb[146], 0);                        /* drCTExtRec[0].startBlock */
    be16_write(&mdb[148], 10);                       /* drCTExtRec[0].blockCount */

    /* Extents file - allocate 3 blocks */
    be32_write(&mdb[126], 3 * alBlkSize);            /* drXTFlSize */
    be16_write(&mdb[130], 10);                       /* drXTExtRec[0].startBlock */
    be16_write(&mdb[132], 3);                        /* drXTExtRec[0].blockCount */

    /* Write MDB */
    memcpy((uint8_t*)buffer + (HFS_MDB_SECTOR * 512), mdb, sizeof(mdb));

    /* Initialize Volume Bitmap */
    uint8_t* vbm = (uint8_t*)buffer + (vbmStart * alBlkSize);
    memset(vbm, 0, vbmBlocks * alBlkSize);

    /* Mark system blocks as used (first 13 blocks) */
    for (int i = 0; i < 13; i++) {
        vbm[i / 8] |= (1 << (7 - (i % 8)));
    }

    serial_printf("HFS: Created blank volume '%s' (%llu bytes)\n", volName, size);
    return true;
}