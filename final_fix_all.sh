#!/bin/bash

echo "=== FINAL AGGRESSIVE FIX FOR ALL COMPILATION ISSUES ==="

# Add all missing FileMgr types
cat >> include/SystemTypes.h << 'EOF'

// FileMgr BTree types
typedef struct BTHeader {
    UInt16 treeDepth;
    UInt32 rootNode;
    UInt32 leafRecords;
    UInt32 firstLeafNode;
    UInt32 lastLeafNode;
    UInt16 nodeSize;
    UInt16 maxKeyLength;
    UInt32 totalNodes;
    UInt32 freeNodes;
} BTHeader;

typedef struct BTNode {
    UInt16 kind;
    UInt16 numRecords;
    UInt32 fLink;
    UInt32 bLink;
    UInt8 data[1];
} BTNode;

typedef struct BTNodeDescriptor {
    UInt32 fLink;
    UInt32 bLink;
    UInt8 kind;
    UInt8 height;
    UInt16 numRecords;
    UInt16 reserved;
} BTNodeDescriptor;

// HFS+ types
typedef struct HFSPlusForkData {
    UInt64 logicalSize;
    UInt32 clumpSize;
    UInt32 totalBlocks;
} HFSPlusForkData;

typedef struct HFSPlusVolumeHeader {
    UInt16 signature;
    UInt16 version;
    UInt32 attributes;
    UInt32 lastMountedVersion;
    UInt32 journalInfoBlock;
    UInt32 createDate;
    UInt32 modifyDate;
    UInt32 backupDate;
    UInt32 checkedDate;
    UInt32 fileCount;
    UInt32 folderCount;
    UInt32 blockSize;
    UInt32 totalBlocks;
    UInt32 freeBlocks;
} HFSPlusVolumeHeader;

typedef struct HFSPlusExtentRecord {
    UInt32 startBlock;
    UInt32 blockCount;
} HFSPlusExtentRecord;

typedef struct HFSPlusExtentDescriptor {
    HFSPlusExtentRecord extents[8];
} HFSPlusExtentDescriptor;

typedef struct HFSPlusCatalogKey {
    UInt16 keyLength;
    UInt32 parentID;
    UInt16 nodeName[255];
} HFSPlusCatalogKey;

// Volume types
typedef struct VolumeHeader {
    UInt16 signature;
    UInt32 createDate;
    UInt32 modifyDate;
    UInt16 attributes;
    UInt16 nmFls;
    UInt16 vBMSt;
    UInt16 allocPtr;
    UInt16 nmAlBlks;
    UInt32 alBlkSiz;
    UInt32 clpSiz;
    UInt16 alBlSt;
    UInt32 nxtCNID;
    UInt16 freeBks;
    Str27 vN;
} VolumeHeader;

typedef struct ExtentRecord {
    UInt16 startBlock;
    UInt16 blockCount;
} ExtentRecord;

typedef struct ExtentDescriptor {
    ExtentRecord extents[3];
} ExtentDescriptor;

// QuickDraw missing types
typedef struct QDState {
    GrafPtr thePort;
    Pattern grayPattern;
    Pattern blackPattern;
    Pattern whitePattern;
    Cursor arrow;
    BitMap screenBits;
    SInt32 randSeed;
} QDState;

typedef struct QDGlobals {
    void* privates;
    SInt32 randSeed;
    BitMap screenBits;
    Cursor arrow;
    Pattern dkGray;
    Pattern ltGray;
    Pattern gray;
    Pattern black;
    Pattern white;
    GrafPtr thePort;
    SInt32 hiliteRGB;
    void* reserved;
} QDGlobals;

typedef struct GDevice {
    SInt16 gdRefNum;
    SInt16 gdID;
    SInt16 gdType;
    Handle gdITable;
    SInt16 gdResPref;
    Handle gdSearchProc;
    Handle gdCompProc;
    SInt16 gdFlags;
    PixMapHandle gdPMap;
    SInt32 gdRefCon;
    Handle gdNextGD;
    Rect gdRect;
    SInt32 gdMode;
    SInt16 gdCCBytes;
    SInt16 gdCCDepth;
    Handle gdCCXData;
    Handle gdCCXMask;
    SInt32 gdReserved;
} GDevice;

typedef GDevice* GDPtr;
typedef GDPtr* GDHandle;

// Picture opcodes
typedef enum {
    OP_NOP = 0x00,
    OP_CLIP = 0x01,
    OP_BKPAT = 0x02,
    OP_TXFONT = 0x03,
    OP_TXFACE = 0x04,
    OP_TXMODE = 0x05,
    OP_SPEXTRA = 0x06,
    OP_PNSIZE = 0x07,
    OP_PNMODE = 0x08,
    OP_PNPAT = 0x09,
    OP_FILLPAT = 0x0A,
    OP_OVSIZE = 0x0B,
    OP_ORIGIN = 0x0C,
    OP_TXSIZE = 0x0D,
    OP_FGCOLOR = 0x0E,
    OP_BKCOLOR = 0x0F,
    OP_LINE = 0x20,
    OP_LINE_FROM = 0x21,
    OP_SHORT_LINE = 0x22,
    OP_SHORT_LINE_FROM = 0x23,
    OP_LONG_TEXT = 0x28,
    OP_DH_TEXT = 0x29,
    OP_DV_TEXT = 0x2A,
    OP_DHDV_TEXT = 0x2B,
    OP_RECT = 0x30,
    OP_FRAMERECT = 0x30,
    OP_PAINTRECT = 0x31,
    OP_ERASERRECT = 0x32,
    OP_INVERTRECT = 0x33,
    OP_FILLRECT = 0x34,
    OP_FRAMESAMRECT = 0x38,
    OP_PAINTSAMRECT = 0x39,
    OP_ERASESAMERECT = 0x3A,
    OP_INVERTSAMERECT = 0x3B,
    OP_FILLSAMERECT = 0x3C,
    OP_RRECT = 0x40,
    OP_FRAMRRECT = 0x40,
    OP_PAINTRRECT = 0x41,
    OP_ERASERECT = 0x42,
    OP_INVERTRRECT = 0x43,
    OP_FILLRRECT = 0x44,
    OP_FRAMESAMRRECT = 0x48,
    OP_PAINTSAMRRECT = 0x49,
    OP_ERASESAMRRECT = 0x4A,
    OP_INVERTSAMRRECT = 0x4B,
    OP_FILLSAMRRECT = 0x4C,
    OP_OVAL = 0x50,
    OP_FRAMEOVAL = 0x50,
    OP_PAINTOVAL = 0x51,
    OP_ERASEOVAL = 0x52,
    OP_INVERTOVAL = 0x53,
    OP_FILLOVAL = 0x54,
    OP_FRAMESAMEOVAL = 0x58,
    OP_PAINTSAMEOVAL = 0x59,
    OP_ERASESAMEOVAL = 0x5A,
    OP_INVERTSAMEOVAL = 0x5B,
    OP_FILLSAMEOVAL = 0x5C,
    OP_FRAMESAMEARC = 0x68,
    OP_PAINTSAMEAVARC = 0x69,
    OP_ERASESAMEAVARC = 0x6A,
    OP_INVERTSAMEAVARC = 0x6B,
    OP_FILLSAMEAVARC = 0x6C,
    OP_FRAMEPOLY = 0x70,
    OP_PAINTPOLY = 0x71,
    OP_ERASEPOLY = 0x72,
    OP_INVERTPOLY = 0x73,
    OP_FILLPOLY = 0x74,
    OP_FRAMESAMEPOLY = 0x78,
    OP_PAINTSAMEPOLY = 0x79,
    OP_ERASESAMEPOLY = 0x7A,
    OP_INVERTSAMEPOLY = 0x7B,
    OP_FILLSAMEPOLY = 0x7C,
    OP_FRAMERGN = 0x80,
    OP_PAINTRGN = 0x81,
    OP_ERASERGN = 0x82,
    OP_INVERTRGN = 0x83,
    OP_FILLRGN = 0x84,
    OP_FRAMESAMERGN = 0x88,
    OP_PAINTSAMERGN = 0x89,
    OP_ERASESAMERGN = 0x8A,
    OP_INVERTSAMERGN = 0x8B,
    OP_FILLSAMERGN = 0x8C,
    OP_BITSRECT = 0x90,
    OP_BITSRGN = 0x91,
    OP_PACKBITSRECT = 0x98,
    OP_PACKBITSRGN = 0x99,
    OP_OPCOLOR = 0x9A,
    OP_SHORTCOMMENT = 0xA0,
    OP_LONGCOMMENT = 0xA1,
    OP_PICVERSION = 0x11,
    OP_ENDPIC = 0xFF
} PictureOpcode;

// Pattern types
typedef struct PixPat {
    SInt16 patType;
    PixMapHandle patMap;
    Handle patData;
    Handle patXData;
    SInt16 patXValid;
    Handle patXMap;
    Pattern pat1Data;
} PixPat;

typedef PixPat* PixPatPtr;
typedef PixPatPtr* PixPatHandle;

// Polygon types
typedef struct Polygon {
    SInt16 polySize;
    Rect polyBBox;
    Point polyPoints[1];
} Polygon;

// Picture types
typedef struct Picture {
    SInt16 picSize;
    Rect picFrame;
} Picture;

// Additional missing stubs
typedef void* BTNodePtr;
typedef void* BTHeaderPtr;
typedef void* HFSPlusVolumeHeaderPtr;
typedef void* ExtentKeyPtr;
typedef void* CatalogKeyPtr;
typedef void* AttributeKeyPtr;

EOF

echo "All missing types added!"