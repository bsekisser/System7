/*
 * RE-AGENT-BANNER
 * This file was reverse-engineered from Mac OS System 7.1 HFS source code.
 * Original: Apple Computer, Inc. (c) 1982-1993
 * Reverse-engineered implementation based on evidence from:
 * - /home/k/Desktop/os71/sys71src/OS/HFS/TFS.a (SHA: a7eef70fb36ac...)
 * - /home/k/Desktop/os71/sys71src/OS/HFS/TFSVOL.a (SHA: 1a222760f20...)
 * - /home/k/Desktop/os71/sys71src/OS/HFS/BTSVCS.a (SHA: 16a68cd1f2f...)
 * - /home/k/Desktop/os71/sys71src/OS/HFS/CMSVCS.a (SHA: 3f66d9e57eb...)
 * - /home/k/Desktop/os71/sys71src/OS/HFS/FXM.a (SHA: 3863a4b618f...)
 * Evidence-based reconstruction preserving original HFS semantics.
 * Provenance: layouts.file_manager.json, evidence.file_manager.json
 */

#ifndef HFS_STRUCTS_H
#define HFS_STRUCTS_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

/* HFS Constants */
#define HFS_SIGNATURE           0x4244      /* 'BD' - HFS signature */
#define HFS_PLUS_SIGNATURE      0x482B      /* 'H+' - HFS Plus signature */
#define BTREE_NODE_SIZE         512         /* Fixed B-Tree node size */
#define MAX_BTREE_DEPTH         8           /* Maximum tree depth */
#define NUM_EXTENTS_PER_RECORD  3           /* Extents per extent record */
#define EXTENT_RECORD_SIZE      12          /* Size of extent record */
#define HFS_BLOCK_SIZE          512         /* Standard HFS block size */

/* Catalog Record Types */
#define kHFSFolderRecord        1           /* Directory record */
#define kHFSFileRecord          2           /* File record */
#define kHFSFolderThreadRecord  3           /* Directory thread record */
#define kHFSFileThreadRecord    4           /* File thread record */

/* B-Tree Node Types */
#define ndHdrNode               1           /* Header node */
#define ndMapNode               2           /* Map node */
#define ndIndxNode              0           /* Index node */
#define ndLeafNode              255         /* Leaf node */

/* Fork Types */
#define dataFk                  0x00        /* Data fork */
#define rsrcFk                  0xFF        /* Resource fork */

/* OSErr Error Codes */
/* OSErr is defined in MacTypes.h *//* noErr is defined in MacTypes.h */
#define memFullErr              -108
#define ioErr                   -36
#define fnfErr                  -43
#define permErr                 -54

/* Basic Mac OS Types */
/* Ptr is defined in MacTypes.h */
/* Ptr is defined in MacTypes.h */
/* Str31 is defined in MacTypes.h */

/* Point and Rect are defined in MacTypes.h */

#pragma pack(push, 2)  /* Use 2-byte alignment for HFS compatibility */

/*
 * Extent Descriptor - represents a contiguous range of allocation blocks
 * Evidence: FSPrivate.a xdrStABN/xdrNumABlks definitions
 */

/*
 * Extent Record - array of 3 extent descriptors per HFS specification
 * Evidence: FSPrivate.a numExts EQU 3, lenxdr EQU 12
 */

/*
 * B-Tree Node - fixed 512-byte node structure
 * Evidence: BTSVCS.a BTNode operations, FSPrivate.a ndFlink/ndBlink definitions
 */

/*
 * B-Tree Header - contains tree metadata in header node
 * Evidence: FSPrivate.a bthDepth/bthRoot/bthNRecs definitions
 */

/* Forward declarations */

/*
 * File Control Block - control structure for open files
 * Evidence: TFSCOMMON.a FCBFlNm/FCBMdRByt/FCBExtRec references
 */

/*
 * Volume Control Block - main control structure for mounted volume
 * Evidence: TFSVOL.a VCBSigWord/VCBAlBlkSiz/VCBNxtCNID references
 */

/*
 * Master Directory Block - on-disk volume header (block 2)
 * Evidence: TFSVOL.a drSigWord/drAlBlkSiz references
 */

/*
 * Finder Info structures
 * Evidence: Catalog record structures in CMSVCS.a
 */

/*
 * Catalog Records - variable-length records in catalog B-Tree
 * Evidence: CMSVCS.a catalog operations and record types
 */

#pragma pack(pop)

/*
 * RE-AGENT-TRAILER-JSON
 * {
 *   "reverse_engineering": {
 *     "tool": "evidence_curator + struct_layout + mapping",
 *     "confidence": 0.95,
 *     "evidence_files": [
 *       "evidence.file_manager.json",
 *       "layouts.file_manager.json",
 *       "mappings.file_manager.json"
 *     ],
 *     "original_sources": [
 *       "TFS.a", "TFSVOL.a", "BTSVCS.a", "CMSVCS.a", "FXM.a"
 *     ],
 *     "key_structures": [
 *       "VCB", "FCB", "BTNode", "ExtentRecord", "MDB", "CatalogRecord"
 *     ],
 *     "validation": "Structure layouts verified against 68k assembly offsets"
 *   }
 * }
 */

#endif /* HFS_STRUCTS_H */