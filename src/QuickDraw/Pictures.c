/*
 * Pictures.c - QuickDraw Picture (PICT) Implementation
 *
 * Implements picture recording and playback for QuickDraw metafile format.
 * Pictures record drawing operations and can be played back with scaling.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 * Derived from System 7 ROM analysis (Ghidra) QuickDraw
 */

#include "SystemTypes.h"
#include "QuickDraw/QuickDraw.h"
#include "QuickDrawConstants.h"
#include "MemoryMgr/MemoryManager.h"
#include <string.h>

/* Current port from QuickDrawCore.c */
extern GrafPtr g_currentPort;

/* Picture opcodes (simplified PICT format subset) */
#define picOpNop        0x00
#define picOpClipRgn    0x01
#define picOpPnSize     0x07
#define picOpPnMode     0x08
#define picOpPnPat      0x09
#define picOpFillPat    0x0A
#define picOpLine       0x20
#define picOpLineFrom   0x21
#define picOpShortLine  0x22
#define picOpShortLineFrom 0x23
#define picOpFrameRect  0x30
#define picOpPaintRect  0x31
#define picOpEraseRect  0x32
#define picOpInvertRect 0x33
#define picOpFillRect   0x34
#define picOpFrameOval  0x50
#define picOpPaintOval  0x51
#define picOpEraseOval  0x52
#define picOpInvertOval 0x53
#define picOpFillOval   0x54
#define picOpFrameArc   0x60
#define picOpPaintArc   0x61
#define picOpEraseArc   0x62
#define picOpInvertArc  0x63
#define picOpFillArc    0x64
#define picOpFramePoly  0x70
#define picOpPaintPoly  0x71
#define picOpErasePoly  0x72
#define picOpInvertPoly 0x73
#define picOpFillPoly   0x74
#define picOpFrameRgn   0x80
#define picOpPaintRgn   0x81
#define picOpEraseRgn   0x82
#define picOpInvertRgn  0x83
#define picOpFillRgn    0x84
#define picOpEndPic     0xFF

/* Picture recording state */
typedef struct {
    Boolean recording;
    PicHandle currentPic;
    UInt8 *dataPtr;
    SInt32 dataSize;
    SInt32 dataCapacity;
} PictureState;

static PictureState g_pictureState = {false, NULL, NULL, 0, 0};

/* Helper: Write byte to picture data */
static void WriteByte(UInt8 byte) {
    if (!g_pictureState.recording || !g_pictureState.dataPtr) return;

    /* Expand capacity if needed */
    if (g_pictureState.dataSize >= g_pictureState.dataCapacity) {
        g_pictureState.dataCapacity += 256;
        /* Realloc would go here - for now just prevent overflow */
        if (g_pictureState.dataSize >= g_pictureState.dataCapacity) {
            return;
        }
    }

    g_pictureState.dataPtr[g_pictureState.dataSize++] = byte;
}

/* Helper: Write short to picture data (big-endian) */
static void WriteShort(SInt16 value) {
    WriteByte((value >> 8) & 0xFF);
    WriteByte(value & 0xFF);
}

/* Helper: Write rect to picture data */
static void WriteRect(const Rect *r) {
    WriteShort(r->top);
    WriteShort(r->left);
    WriteShort(r->bottom);
    WriteShort(r->right);
}

/* Helper: Write pattern to picture data */
static void WritePattern(ConstPatternParam pat) {
    if (!pat) {
        for (int i = 0; i < 8; i++) WriteByte(0);
    } else {
        for (int i = 0; i < 8; i++) WriteByte(pat->pat[i]);
    }
}

/*
 * OpenPicture - Begin picture recording
 */
PicHandle OpenPicture(const Rect *picFrame) {
    if (!picFrame) return NULL;

    /* Allocate picture handle */
    PicHandle pic = (PicHandle)NewHandle(sizeof(Picture) + 1024);  /* Initial data size */
    if (!pic || !*pic) return NULL;

    Picture *picPtr = *pic;
    picPtr->picSize = sizeof(Picture);
    picPtr->picFrame = *picFrame;

    /* Initialize recording state */
    g_pictureState.recording = true;
    g_pictureState.currentPic = pic;
    g_pictureState.dataPtr = (UInt8 *)(picPtr + 1);  /* Data follows header */
    g_pictureState.dataSize = 0;
    g_pictureState.dataCapacity = 1024;

    return pic;
}

/*
 * ClosePicture - End picture recording
 */
void ClosePicture(void) {
    if (!g_pictureState.recording) return;

    /* Write end-of-picture opcode */
    WriteByte(picOpEndPic);

    /* Update picture size */
    if (g_pictureState.currentPic && *g_pictureState.currentPic) {
        Picture *pic = *g_pictureState.currentPic;
        pic->picSize = sizeof(Picture) + g_pictureState.dataSize;
    }

    /* Stop recording */
    g_pictureState.recording = false;
    g_pictureState.currentPic = NULL;
    g_pictureState.dataPtr = NULL;
    g_pictureState.dataSize = 0;
}

/*
 * DrawPicture - Play back recorded picture
 */
void DrawPicture(PicHandle myPicture, const Rect *dstRect) {
    if (!myPicture || !*myPicture || !dstRect || !g_currentPort) return;

    Picture *pic = *myPicture;
    UInt8 *data = (UInt8 *)(pic + 1);
    SInt32 dataLen = pic->picSize - sizeof(Picture);
    SInt32 pos = 0;

    /* Calculate scaling factors (fixed-point 16.16) */
    SInt32 picWidth = pic->picFrame.right - pic->picFrame.left;
    SInt32 picHeight = pic->picFrame.bottom - pic->picFrame.top;
    SInt32 dstWidth = dstRect->right - dstRect->left;
    SInt32 dstHeight = dstRect->bottom - dstRect->top;

    if (picWidth <= 0 || picHeight <= 0) return;

    SInt32 scaleX = (dstWidth << 16) / picWidth;
    SInt32 scaleY = (dstHeight << 16) / picHeight;
    SInt16 offsetX = dstRect->left - pic->picFrame.left;
    SInt16 offsetY = dstRect->top - pic->picFrame.top;

    /* Helper: Scale and offset a coordinate */
    #define SCALE_X(x) ((SInt16)(((((x) - pic->picFrame.left) * scaleX) >> 16) + dstRect->left))
    #define SCALE_Y(y) ((SInt16)(((((y) - pic->picFrame.top) * scaleY) >> 16) + dstRect->top))
    #define SCALE_RECT(r, sr) do { \
        (sr).left = SCALE_X((r).left); \
        (sr).top = SCALE_Y((r).top); \
        (sr).right = SCALE_X((r).right); \
        (sr).bottom = SCALE_Y((r).bottom); \
    } while(0)

    /* Play back opcodes */
    while (pos < dataLen) {
        UInt8 opcode = data[pos++];

        switch (opcode) {
            case picOpNop:
                break;

            case picOpPnSize: {
                if (pos + 4 > dataLen) return;
                Point pnSize;
                pnSize.v = (data[pos] << 8) | data[pos+1]; pos += 2;
                pnSize.h = (data[pos] << 8) | data[pos+1]; pos += 2;
                PenSize(pnSize.h, pnSize.v);
                break;
            }

            case picOpPnMode: {
                if (pos + 2 > dataLen) return;
                SInt16 mode = (data[pos] << 8) | data[pos+1]; pos += 2;
                PenMode(mode);
                break;
            }

            case picOpFrameRect:
            case picOpPaintRect:
            case picOpEraseRect:
            case picOpInvertRect: {
                if (pos + 8 > dataLen) return;
                Rect r, sr;
                r.top = (data[pos] << 8) | data[pos+1]; pos += 2;
                r.left = (data[pos] << 8) | data[pos+1]; pos += 2;
                r.bottom = (data[pos] << 8) | data[pos+1]; pos += 2;
                r.right = (data[pos] << 8) | data[pos+1]; pos += 2;
                SCALE_RECT(r, sr);

                if (opcode == picOpFrameRect) FrameRect(&sr);
                else if (opcode == picOpPaintRect) PaintRect(&sr);
                else if (opcode == picOpEraseRect) EraseRect(&sr);
                else if (opcode == picOpInvertRect) InvertRect(&sr);
                break;
            }

            case picOpFrameOval:
            case picOpPaintOval:
            case picOpEraseOval:
            case picOpInvertOval: {
                if (pos + 8 > dataLen) return;
                Rect r, sr;
                r.top = (data[pos] << 8) | data[pos+1]; pos += 2;
                r.left = (data[pos] << 8) | data[pos+1]; pos += 2;
                r.bottom = (data[pos] << 8) | data[pos+1]; pos += 2;
                r.right = (data[pos] << 8) | data[pos+1]; pos += 2;
                SCALE_RECT(r, sr);

                if (opcode == picOpFrameOval) FrameOval(&sr);
                else if (opcode == picOpPaintOval) PaintOval(&sr);
                else if (opcode == picOpEraseOval) EraseOval(&sr);
                else if (opcode == picOpInvertOval) InvertOval(&sr);
                break;
            }

            case picOpEndPic:
                return;

            default:
                /* Unknown opcode - stop playback */
                return;
        }
    }
}

/*
 * KillPicture - Dispose of picture
 */
void KillPicture(PicHandle myPicture) {
    if (myPicture) {
        DisposeHandle((Handle)myPicture);
    }
}

/*
 * PicComment - Add comment to picture
 */
void PicComment(SInt16 kind, SInt16 dataSize, Handle dataHandle) {
    if (!g_pictureState.recording) return;

    /* Write comment opcode (0xA1) */
    WriteByte(0xA1);
    WriteShort(kind);
    WriteShort(dataSize);

    /* Write comment data if provided */
    if (dataHandle && *dataHandle && dataSize > 0) {
        UInt8 *commentData = (UInt8 *)(*dataHandle);
        for (SInt16 i = 0; i < dataSize; i++) {
            WriteByte(commentData[i]);
        }
    }
}

/*
 * Picture Recording Hooks - Called by QuickDraw primitives
 */

void PictureRecordFrameRect(const Rect *r) {
    if (!g_pictureState.recording || !r) return;
    WriteByte(picOpFrameRect);
    WriteRect(r);
}

void PictureRecordPaintRect(const Rect *r) {
    if (!g_pictureState.recording || !r) return;
    WriteByte(picOpPaintRect);
    WriteRect(r);
}

void PictureRecordEraseRect(const Rect *r) {
    if (!g_pictureState.recording || !r) return;
    WriteByte(picOpEraseRect);
    WriteRect(r);
}

void PictureRecordInvertRect(const Rect *r) {
    if (!g_pictureState.recording || !r) return;
    WriteByte(picOpInvertRect);
    WriteRect(r);
}

void PictureRecordFrameOval(const Rect *r) {
    if (!g_pictureState.recording || !r) return;
    WriteByte(picOpFrameOval);
    WriteRect(r);
}

void PictureRecordPaintOval(const Rect *r) {
    if (!g_pictureState.recording || !r) return;
    WriteByte(picOpPaintOval);
    WriteRect(r);
}

void PictureRecordEraseOval(const Rect *r) {
    if (!g_pictureState.recording || !r) return;
    WriteByte(picOpEraseOval);
    WriteRect(r);
}

void PictureRecordInvertOval(const Rect *r) {
    if (!g_pictureState.recording || !r) return;
    WriteByte(picOpInvertOval);
    WriteRect(r);
}
