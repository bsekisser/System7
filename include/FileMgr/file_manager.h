/*
 * RE-AGENT-BANNER
 * HFS File Manager Core Interface
 * Reverse-engineered from Mac OS System 7.1 source code

 * Preserving original HFS File Manager semantics and API conventions
 */

#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "hfs_structs.h"

/* File permission constants */
#define fsRdPerm    1    /* Read permission */
#define fsWrPerm    2    /* Write permission */
#define fsRdWrPerm  3    /* Read/Write permission */

/* Directory ID constants */
#define fsRtDirID   2    /* Root directory ID */

/* Script constants */
#define smSystemScript  0  /* System script */

/* File attributes */
#define ioDirMask   0x10  /* Directory attribute mask */

/* Additional types for File Manager */
typedef struct BTNode BTNode;
typedef const unsigned char* ConstStringPtr;

/* File Manager Function Prototypes - Evidence from ROM File Manager code */

/*
 * Core File Manager Operations

 */
OSErr TFSDispatch(UInt16 trap_index, UInt16 trap_word, void* param_block);
OSErr VolumeCall(void* param_block);
OSErr RefNumCall(void* param_block);
OSErr OpenCall(void* param_block);

/*
 * Volume Management Operations

 */
OSErr MountVol(void* param_block);
OSErr CheckRemount(VCB* vcb);
OSErr MtCheck(VCB* vcb);
void* FindDrive(SInt16 drive_num);
OSErr OffLine(VCB* vcb);
OSErr FlushVol(VCB* vcb);

/*
 * B-Tree Services for Catalog and Extent Management

 */
OSErr BTOpen(FCB* fcb, void* btcb);
OSErr BTClose(void* btcb);
OSErr BTSearch(void* btcb, void* key, BTNode** node, UInt16* record_index);
OSErr BTInsert(void* btcb, void* key, void* data, UInt16 data_len);
OSErr BTDelete(void* btcb, void* key);
OSErr BTGetRecord(void* btcb, SInt16 selection_mode, void** key, void** data, UInt16* data_len);
OSErr BTFlush(void* btcb);

/*
 * Catalog Manager Services

 */
OSErr CMGetCatalogHint(VCB* vcb, UInt32 dir_id, ConstStringPtr name, void* hint);
OSErr CMLocateCatalogHint(VCB* vcb, void* hint, void* catalog_record);
OSErr CMAddCatalogRecord(VCB* vcb, void* key, void* catalog_record);
OSErr CMDeleteCatalogRecord(VCB* vcb, void* key);
OSErr CMUpdateCatalogRecord(VCB* vcb, void* key, void* catalog_record);

/*
 * File Extent Management (FXM)

 */
OSErr FXMFindExtent(FCB* fcb, UInt32 file_block, ExtentRecord* extent_rec);
OSErr FXMAllocateExtent(VCB* vcb, FCB* fcb, UInt32 bytes_needed, ExtentRecord* extent_rec);
OSErr FXMDeallocateExtent(VCB* vcb, ExtentRecord* extent_rec);
OSErr FXMExtendFile(FCB* fcb, UInt32 bytes_to_add);
OSErr FXMTruncateFile(FCB* fcb, UInt32 new_length);

/*
 * File System Queue Operations

 */
OSErr FSQueue(void* param_block);
OSErr FSQueueSync(void);

/*
 * Internal Support Functions

 */
void CmdDone(void);
OSErr DSHook(void);  /* Disk switch hook */

/*
 * Utility Functions for HFS Implementation
 */
Boolean IsValidHFSSignature(UInt16 signature);
UInt32 CalculateAllocationBlocks(UInt32 file_size, UInt32 block_size);
OSErr ValidateVCB(const VCB* vcb);
OSErr ValidateFCB(const FCB* fcb);
OSErr ConvertPascalStringToC(const UInt8* pascal_str, char* c_str, size_t max_len);
OSErr ConvertCStringToPascal(const char* c_str, UInt8* pascal_str, size_t max_len);

/*
 * File Manager Constants and Macros
 */
#define HFS_DEFAULT_CLUMP_SIZE  4096
#define HFS_ROOT_DIR_ID         2
#define HFS_ROOT_PARENT_ID      1
#define MAX_HFS_FILENAME        31

#include "SystemTypes.h"
#define MAX_HFS_VOLUME_NAME     27

#include "SystemTypes.h"

/* File Manager Trap Numbers (Evidence: ROM File Manager code trap dispatch table) */
#define kHFSDispatch            0xA060
#define kMountVol               0xA00F
#define kUnmountVol             0xA00E
#define kFlushVol               0xA013
#define kGetVol                 0xA014
#define kSetVol                 0xA015

/* Volume Attributes */
#define kHFSVolumeHardwareLockBit   7
#define kHFSVolumeSoftwareLockBit   15

/* File Attributes */
#define kHFSFileLockedBit           0
#define kHFSFileInvisibleBit        1
#define kHFSFileHasBundleBit        5

/*
 * Error codes specific to HFS operations

 */
#define badMDBErr               -60     /* Bad master directory block */
#define wrPermErr               -61     /* Write permissions error */
#define memFullErr              -108    /* Not enough memory */
#define nsDrvErr                -56     /* No such drive */
#define offLinErr               -53     /* Volume is offline */
#define volOnLinErr             -55     /* Volume already online */
#define ioErr                   -36     /* I/O error */

/*
 * High-level File Manager API Functions
 */
OSErr FSMakeFSSpec(short vRefNum, long dirID, const unsigned char *fileName, FSSpec *spec);
OSErr FSpCreate(const FSSpec *spec, OSType creator, OSType fileType, short scriptTag);
OSErr FSpOpenDF(const FSSpec *spec, short permission, short *refNum);
OSErr FSClose(short refNum);
OSErr FSRead(short refNum, long *count, void *buffPtr);
OSErr FSWrite(short refNum, long *count, const void *buffPtr);
OSErr SetEOF(short refNum, long logEOF);
OSErr PBHGetVInfoSync(void *paramBlock);
OSErr PBGetCatInfoSync(void *paramBlock);
OSErr PBSetCatInfoSync(void *paramBlock);

#endif /* FILE_MANAGER_H */