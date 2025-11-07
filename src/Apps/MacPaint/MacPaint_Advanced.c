/*
 * MacPaint_Advanced.c - Advanced Features: Undo/Redo, Selections, Editors
 *
 * Implements advanced MacPaint features:
 * - Undo/Redo system with circular buffer
 * - Selection and clipboard operations
 * - Pattern editor
 * - Brush editor
 * - Additional drawing modes (XOR, OR, AND)
 *
 * Ported from original MacPaint undo system and advanced tools
 */

#include "SystemTypes.h"
#include "Apps/MacPaint.h"
#include "System71StdLib.h"
#include "MemoryMgr/MemoryManager.h"
#include "FileManagerTypes.h"
#include <string.h>

/*
 * UNDO/REDO SYSTEM - Circular Buffer Implementation
 */

#define MAX_UNDO_BUFFERS 8      /* Maximum undo levels */
#define UNDO_BUFFER_SIZE 65536  /* Size of each undo buffer (64KB) */

typedef struct {
    Ptr data;                   /* Bitmap snapshot */
    UInt32 dataSize;           /* Compressed size */
    UInt32 timestamp;          /* When this state was saved */
    char description[32];      /* Operation description */
} UndoFrame;

typedef struct {
    UndoFrame frames[MAX_UNDO_BUFFERS];
    int currentFrame;          /* Current position in circular buffer */
    int frameCount;            /* Number of valid frames */
    int undoPosition;          /* Position for undo navigation */
    Ptr compressionBuffer;     /* Temporary compression buffer */
} UndoBuffer;

static UndoBuffer gUndoBuffer = {0};

/* External paint buffer and state from MacPaint_Core */
extern BitMap gPaintBuffer;
extern int gDocDirty;

/**
 * MacPaint_InitializeUndo - Set up undo system
 */
OSErr MacPaint_InitializeUndo(void)
{
    /* Allocate compression buffer for undo operations */
    gUndoBuffer.compressionBuffer = NewPtr(UNDO_BUFFER_SIZE + 1024);
    if (!gUndoBuffer.compressionBuffer) {
        return memFullErr;
    }

    /* Initialize undo buffer array */
    for (int i = 0; i < MAX_UNDO_BUFFERS; i++) {
        gUndoBuffer.frames[i].data = NewPtr(UNDO_BUFFER_SIZE);
        if (!gUndoBuffer.frames[i].data) {
            return memFullErr;
        }
        gUndoBuffer.frames[i].dataSize = 0;
        gUndoBuffer.frames[i].timestamp = 0;
        strcpy(gUndoBuffer.frames[i].description, "");
    }

    gUndoBuffer.currentFrame = 0;
    gUndoBuffer.frameCount = 0;
    gUndoBuffer.undoPosition = 0;

    return noErr;
}

/**
 * MacPaint_ShutdownUndo - Clean up undo buffers
 */
void MacPaint_ShutdownUndo(void)
{
    for (int i = 0; i < MAX_UNDO_BUFFERS; i++) {
        if (gUndoBuffer.frames[i].data) {
            DisposePtr(gUndoBuffer.frames[i].data);
            gUndoBuffer.frames[i].data = NULL;
        }
    }

    if (gUndoBuffer.compressionBuffer) {
        DisposePtr(gUndoBuffer.compressionBuffer);
        gUndoBuffer.compressionBuffer = NULL;
    }
}

/**
 * MacPaint_SaveUndoState - Save current bitmap state for undo
 * Returns: noErr if saved, error code otherwise
 */
OSErr MacPaint_SaveUndoState(const char *description)
{
    int uncompressedSize = gPaintBuffer.rowBytes *
                          (gPaintBuffer.bounds.bottom - gPaintBuffer.bounds.top);

    /* Compress bitmap data */
    UInt32 compressedSize = MacPaint_PackBits(
        (unsigned char *)gPaintBuffer.baseAddr,
        uncompressedSize,
        (unsigned char *)gUndoBuffer.compressionBuffer,
        UNDO_BUFFER_SIZE + 1024
    );

    if (compressedSize == 0 || compressedSize > UNDO_BUFFER_SIZE) {
        return ioErr;  /* Compression failed or too large */
    }

    /* Move to next frame in circular buffer */
    gUndoBuffer.currentFrame = (gUndoBuffer.currentFrame + 1) % MAX_UNDO_BUFFERS;
    if (gUndoBuffer.frameCount < MAX_UNDO_BUFFERS) {
        gUndoBuffer.frameCount++;
    }

    /* Save compressed data */
    UndoFrame *frame = &gUndoBuffer.frames[gUndoBuffer.currentFrame];
    memcpy(frame->data, gUndoBuffer.compressionBuffer, compressedSize);
    frame->dataSize = compressedSize;

    /* Get current tick count for undo frame timestamp */
    extern UInt32 TickCount(void);
    frame->timestamp = TickCount();

    if (description) {
        strncpy(frame->description, description, sizeof(frame->description) - 1);
        frame->description[sizeof(frame->description) - 1] = '\0';
    } else {
        strcpy(frame->description, "");
    }

    gUndoBuffer.undoPosition = gUndoBuffer.currentFrame;
    return noErr;
}

/**
 * MacPaint_CanUndo - Check if undo is available
 */
int MacPaint_CanUndo(void)
{
    return (gUndoBuffer.frameCount > 0);
}

/**
 * MacPaint_CanRedo - Check if redo is available
 */
int MacPaint_CanRedo(void)
{
    /* Redo available if we're not at the most recent state */
    if (gUndoBuffer.frameCount == 0) {
        return 0;
    }

    int prevPosition = (gUndoBuffer.undoPosition + 1) % MAX_UNDO_BUFFERS;
    return (prevPosition != gUndoBuffer.currentFrame &&
            gUndoBuffer.frames[prevPosition].dataSize > 0);
}

/**
 * MacPaint_Undo - Restore previous state
 */
OSErr MacPaint_Undo(void)
{
    if (!MacPaint_CanUndo()) {
        return noErr;  /* Nothing to undo */
    }

    /* Move to previous frame */
    gUndoBuffer.undoPosition = (gUndoBuffer.undoPosition - 1 + MAX_UNDO_BUFFERS) % MAX_UNDO_BUFFERS;
    UndoFrame *frame = &gUndoBuffer.frames[gUndoBuffer.undoPosition];

    if (frame->dataSize == 0) {
        return noErr;  /* Frame is empty */
    }

    /* Decompress bitmap data */
    int uncompressedSize = gPaintBuffer.rowBytes *
                          (gPaintBuffer.bounds.bottom - gPaintBuffer.bounds.top);

    UInt32 decompressedSize = MacPaint_UnpackBits(
        (unsigned char *)frame->data,
        frame->dataSize,
        (unsigned char *)gPaintBuffer.baseAddr,
        uncompressedSize
    );

    if (decompressedSize != uncompressedSize) {
        return ioErr;  /* Decompression failed */
    }

    gDocDirty = 1;
    return noErr;
}

/**
 * MacPaint_Redo - Restore next state
 */
OSErr MacPaint_Redo(void)
{
    if (!MacPaint_CanRedo()) {
        return noErr;  /* Nothing to redo */
    }

    /* Move to next frame */
    gUndoBuffer.undoPosition = (gUndoBuffer.undoPosition + 1) % MAX_UNDO_BUFFERS;
    UndoFrame *frame = &gUndoBuffer.frames[gUndoBuffer.undoPosition];

    if (frame->dataSize == 0) {
        return noErr;  /* Frame is empty */
    }

    /* Decompress bitmap data */
    int uncompressedSize = gPaintBuffer.rowBytes *
                          (gPaintBuffer.bounds.bottom - gPaintBuffer.bounds.top);

    UInt32 decompressedSize = MacPaint_UnpackBits(
        (unsigned char *)frame->data,
        frame->dataSize,
        (unsigned char *)gPaintBuffer.baseAddr,
        uncompressedSize
    );

    if (decompressedSize != uncompressedSize) {
        return ioErr;  /* Decompression failed */
    }

    gDocDirty = 1;
    return noErr;
}

/*
 * SELECTION AND CLIPBOARD SYSTEM
 */

typedef struct {
    int active;                 /* Selection is active */
    Rect bounds;               /* Selection rectangle */
    Ptr clipboardData;         /* Clipboard bitmap */
    UInt32 clipboardSize;      /* Clipboard size */
    int clipboardWidth;        /* Clipboard dimensions */
    int clipboardHeight;
} SelectionState;

static SelectionState gSelection = {0};

/**
 * MacPaint_CreateSelection - Create rectangular selection
 */
OSErr MacPaint_CreateSelection(int left, int top, int right, int bottom)
{
    if (left >= right || top >= bottom) {
        return paramErr;
    }

    gSelection.bounds.left = left;
    gSelection.bounds.top = top;
    gSelection.bounds.right = right;
    gSelection.bounds.bottom = bottom;
    gSelection.active = 1;

    return noErr;
}

/**
 * MacPaint_GetSelection - Get current selection rectangle
 */
int MacPaint_GetSelection(Rect *bounds)
{
    if (!bounds) {
        return 0;
    }

    if (gSelection.active) {
        *bounds = gSelection.bounds;
        return 1;
    }

    return 0;
}

/**
 * MacPaint_ClearSelection - Remove current selection
 */
void MacPaint_ClearSelection(void)
{
    gSelection.active = 0;
}

/**
 * MacPaint_CopySelectionToClipboard - Copy selected region to clipboard
 */
OSErr MacPaint_CopySelectionToClipboard(void)
{
    if (!gSelection.active) {
        return paramErr;
    }

    int width = gSelection.bounds.right - gSelection.bounds.left;
    int height = gSelection.bounds.bottom - gSelection.bounds.top;

    /* Calculate bitmap size for selection */
    int bytesPerRow = (width + 7) / 8;
    UInt32 bitmapSize = bytesPerRow * height;

    /* Allocate clipboard buffer if needed */
    if (!gSelection.clipboardData || gSelection.clipboardSize < bitmapSize) {
        if (gSelection.clipboardData) {
            DisposePtr(gSelection.clipboardData);
        }

        gSelection.clipboardData = NewPtr(bitmapSize + 1024);
        if (!gSelection.clipboardData) {
            return memFullErr;
        }

        gSelection.clipboardSize = bitmapSize + 1024;
    }

    /* Copy selected pixels from paint buffer to clipboard
     * Iterate through each pixel in the selection rectangle
     * Convert from source coordinates to clipboard coordinates
     */
    unsigned char *srcBits = (unsigned char *)gPaintBuffer.baseAddr;
    unsigned char *dstBits = (unsigned char *)gSelection.clipboardData;
    int srcRowBytes = gPaintBuffer.rowBytes;

    /* Zero the clipboard buffer first */
    memset(dstBits, 0, bitmapSize);

    /* Copy pixel data from selection */
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            /* Source pixel in paint buffer */
            int srcX = gSelection.bounds.left + x;
            int srcY = gSelection.bounds.top + y;

            /* Get source pixel */
            int srcByteOffset = srcY * srcRowBytes + (srcX / 8);
            int srcBitOffset = 7 - (srcX % 8);
            int srcPixel = (srcBits[srcByteOffset] >> srcBitOffset) & 1;

            /* Destination pixel in clipboard */
            int dstByteOffset = y * bytesPerRow + (x / 8);
            int dstBitOffset = 7 - (x % 8);

            if (srcPixel) {
                dstBits[dstByteOffset] |= (1 << dstBitOffset);
            }
        }
    }

    gSelection.clipboardWidth = width;
    gSelection.clipboardHeight = height;

    return noErr;
}

/**
 * MacPaint_PasteFromClipboard - Paste clipboard at position
 * x, y: position to paste at
 */
OSErr MacPaint_PasteFromClipboard(int x, int y)
{
    if (!gSelection.clipboardData) {
        return paramErr;
    }

    /* Paste clipboard bitmap at (x, y)
     * Copy from clipboard buffer back to paint buffer
     */
    unsigned char *dstBits = (unsigned char *)gPaintBuffer.baseAddr;
    unsigned char *srcBits = (unsigned char *)gSelection.clipboardData;
    int dstRowBytes = gPaintBuffer.rowBytes;

    int width = gSelection.clipboardWidth;
    int height = gSelection.clipboardHeight;
    int srcBytesPerRow = (width + 7) / 8;

    /* Paste pixels from clipboard to paint buffer at (x, y)
     * Bounds check to prevent writing outside canvas
     */
    for (int py = 0; py < height; py++) {
        int dstY = y + py;

        /* Skip rows that are outside the canvas */
        if (dstY < 0 || dstY >= gPaintBuffer.bounds.bottom - gPaintBuffer.bounds.top) {
            continue;
        }

        for (int px = 0; px < width; px++) {
            int dstX = x + px;

            /* Skip pixels that are outside the canvas */
            if (dstX < 0 || dstX >= gPaintBuffer.bounds.right - gPaintBuffer.bounds.left) {
                continue;
            }

            /* Source pixel in clipboard */
            int srcByteOffset = py * srcBytesPerRow + (px / 8);
            int srcBitOffset = 7 - (px % 8);
            int srcPixel = (srcBits[srcByteOffset] >> srcBitOffset) & 1;

            /* Destination pixel in paint buffer */
            int dstByteOffset = dstY * dstRowBytes + (dstX / 8);
            int dstBitOffset = 7 - (dstX % 8);

            if (srcPixel) {
                /* Paint black pixel (set bit) */
                dstBits[dstByteOffset] |= (1 << dstBitOffset);
            } else {
                /* Erase white pixel (clear bit) */
                dstBits[dstByteOffset] &= ~(1 << dstBitOffset);
            }
        }
    }

    /* Mark document as dirty and create new selection at paste location */
    gDocDirty = 1;
    MacPaint_CreateSelection(x, y, x + width, y + height);

    return noErr;
}

/**
 * MacPaint_CutSelection - Cut (copy and clear) selection
 */
OSErr MacPaint_CutSelection(void)
{
    OSErr err = MacPaint_CopySelectionToClipboard();
    if (err != noErr) {
        return err;
    }

    /* Clear selection area by erasing all pixels (setting to white/0) */
    unsigned char *bits = (unsigned char *)gPaintBuffer.baseAddr;
    int rowBytes = gPaintBuffer.rowBytes;
    int width = gSelection.bounds.right - gSelection.bounds.left;
    int height = gSelection.bounds.bottom - gSelection.bounds.top;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int srcX = gSelection.bounds.left + x;
            int srcY = gSelection.bounds.top + y;

            /* Clear this pixel (set to white) */
            int byteOffset = srcY * rowBytes + (srcX / 8);
            int bitOffset = 7 - (srcX % 8);
            bits[byteOffset] &= ~(1 << bitOffset);
        }
    }

    gDocDirty = 1;
    return noErr;
}

/*
 * PATTERN EDITOR
 */

typedef struct {
    int open;                   /* Editor window open */
    Pattern editPattern;        /* Pattern being edited */
    int selectedPattern;        /* Which pattern slot */
    Rect editorBounds;         /* Window bounds */
} PatternEditor;

static PatternEditor gPatternEditor = {0};

/**
 * MacPaint_OpenPatternEditor - Open pattern editor window
 */
OSErr MacPaint_OpenPatternEditor(void)
{
    if (gPatternEditor.open) {
        return noErr;  /* Already open */
    }

    /* TODO: Create modeless dialog window
     * Show 8x8 pixel grid with current pattern
     * Add OK/Cancel buttons
     */

    gPatternEditor.open = 1;
    return noErr;
}

/**
 * MacPaint_ClosePatternEditor - Close pattern editor
 */
void MacPaint_ClosePatternEditor(void)
{
    /* TODO: Destroy dialog window */
    gPatternEditor.open = 0;
}

/**
 * MacPaint_SetPatternEditorPattern - Load pattern into editor
 */
void MacPaint_SetPatternEditorPattern(int patternIndex)
{
    if (patternIndex >= 0 && patternIndex < 38) {
        gPatternEditor.selectedPattern = patternIndex;
        /* TODO: Render pattern in editor grid */
    }
}

/**
 * MacPaint_GetPatternEditorPattern - Get edited pattern
 */
Pattern MacPaint_GetPatternEditorPattern(void)
{
    return gPatternEditor.editPattern;
}

/**
 * MacPaint_PatternEditorPixelClick - Handle pixel click in editor
 */
void MacPaint_PatternEditorPixelClick(int x, int y)
{
    if (!gPatternEditor.open) {
        return;
    }

    /* TODO: Toggle pixel at (x, y) in pattern grid
     * Update both edit pattern and preview
     */
}

/*
 * BRUSH EDITOR
 */

typedef struct {
    int open;                   /* Editor window open */
    int selectedBrush;         /* Current brush style */
    Rect editorBounds;         /* Window bounds */
    int brushSize;             /* Brush diameter */
} BrushEditor;

static BrushEditor gBrushEditor = {0};

/**
 * MacPaint_OpenBrushEditor - Open brush editor window
 */
OSErr MacPaint_OpenBrushEditor(void)
{
    if (gBrushEditor.open) {
        return noErr;  /* Already open */
    }

    /* TODO: Create modeless dialog window
     * Show available brush shapes:
     * - Circle (filled)
     * - Square (filled)
     * - Diamond
     * - Spray pattern
     * - Custom pattern selection
     */

    gBrushEditor.open = 1;
    return noErr;
}

/**
 * MacPaint_CloseBrushEditor - Close brush editor
 */
void MacPaint_CloseBrushEditor(void)
{
    /* TODO: Destroy dialog window */
    gBrushEditor.open = 0;
}

/**
 * MacPaint_SetBrushSize - Set brush diameter
 */
void MacPaint_SetBrushSize(int diameter)
{
    if (diameter >= 1 && diameter <= 64) {
        gBrushEditor.brushSize = diameter;
        /* TODO: Update brush preview */
    }
}

/**
 * MacPaint_GetBrushSize - Get current brush size
 */
int MacPaint_GetBrushSize(void)
{
    return gBrushEditor.brushSize;
}

/*
 * ADVANCED DRAWING MODES
 */

/* Global drawing mode state */
static int gDrawingMode = 0;  /* 0=replace, 1=OR, 2=XOR, 3=AND */

/**
 * MacPaint_SetDrawingMode - Set pixel blending mode
 * mode: 0=replace, 1=OR, 2=XOR, 3=AND (clear)
 */
void MacPaint_SetDrawingMode(int mode)
{
    if (mode >= 0 && mode <= 3) {
        gDrawingMode = mode;
    }
}

/**
 * MacPaint_GetDrawingMode - Get current drawing mode
 */
int MacPaint_GetDrawingMode(void)
{
    return gDrawingMode;
}

/*
 * SELECTION TRANSFORMATIONS
 */

/**
 * MacPaint_FlipSelectionHorizontal - Mirror selection horizontally
 */
OSErr MacPaint_FlipSelectionHorizontal(void)
{
    if (!gSelection.active) {
        return paramErr;
    }

    /* TODO: Reverse pixel order horizontally within selection */
    return noErr;
}

/**
 * MacPaint_FlipSelectionVertical - Mirror selection vertically
 */
OSErr MacPaint_FlipSelectionVertical(void)
{
    if (!gSelection.active) {
        return paramErr;
    }

    /* TODO: Reverse pixel order vertically within selection */
    return noErr;
}

/**
 * MacPaint_RotateSelectionCW - Rotate selection 90° clockwise
 */
OSErr MacPaint_RotateSelectionCW(void)
{
    if (!gSelection.active) {
        return paramErr;
    }

    /* TODO: Rotate pixels 90° clockwise
     * Note: May change selection bounds
     */

    return noErr;
}

/**
 * MacPaint_RotateSelectionCCW - Rotate selection 90° counter-clockwise
 */
OSErr MacPaint_RotateSelectionCCW(void)
{
    if (!gSelection.active) {
        return paramErr;
    }

    /* TODO: Rotate pixels 90° counter-clockwise */
    return noErr;
}

/**
 * MacPaint_ScaleSelection - Scale selection to new size
 */
OSErr MacPaint_ScaleSelection(int newWidth, int newHeight)
{
    if (!gSelection.active || newWidth <= 0 || newHeight <= 0) {
        return paramErr;
    }

    /* TODO: Nearest-neighbor or bilinear scaling of selection
     * Update bounds to reflect new size
     */

    return noErr;
}

/*
 * ADVANCED FILL MODES
 */

/**
 * MacPaint_FillSelectionWithPattern - Fill selection with current pattern
 */
OSErr MacPaint_FillSelectionWithPattern(void)
{
    if (!gSelection.active) {
        return paramErr;
    }

    /* TODO: Fill selection rectangle with current pattern
     * Tile pattern across entire selection
     */

    return noErr;
}

/**
 * MacPaint_GradientFill - Fill with gradient (light to dark)
 */
OSErr MacPaint_GradientFill(void)
{
    if (!gSelection.active) {
        return paramErr;
    }

    /* TODO: Create dithered gradient from white to black
     * Useful for shading effects
     */

    return noErr;
}

/**
 * MacPaint_SmoothSelection - Smooth selection edges with anti-aliasing
 */
OSErr MacPaint_SmoothSelection(void)
{
    if (!gSelection.active) {
        return paramErr;
    }

    /* TODO: Apply simple box filter to selection edges
     * Reduces jagged pixel appearance
     */

    return noErr;
}

/*
 * STATE QUERIES
 */

/**
 * MacPaint_IsPatternEditorOpen - Check if pattern editor is open
 */
int MacPaint_IsPatternEditorOpen(void)
{
    return gPatternEditor.open;
}

/**
 * MacPaint_IsBrushEditorOpen - Check if brush editor is open
 */
int MacPaint_IsBrushEditorOpen(void)
{
    return gBrushEditor.open;
}

/**
 * MacPaint_IsSelectionActive - Check if selection exists
 */
int MacPaint_IsSelectionActive(void)
{
    return gSelection.active;
}

/**
 * MacPaint_HasClipboard - Check if clipboard has content
 */
int MacPaint_HasClipboard(void)
{
    return (gSelection.clipboardData != NULL);
}

/*
 * OPERATION DESCRIPTIONS FOR UNDO
 */

/**
 * MacPaint_GetUndoDescription - Get description of undo item
 */
const char* MacPaint_GetUndoDescription(void)
{
    if (!MacPaint_CanUndo()) {
        return "(no undo available)";
    }

    return gUndoBuffer.frames[gUndoBuffer.undoPosition].description;
}

/**
 * MacPaint_GetRedoDescription - Get description of redo item
 */
const char* MacPaint_GetRedoDescription(void)
{
    if (!MacPaint_CanRedo()) {
        return "(no redo available)";
    }

    int redoPos = (gUndoBuffer.undoPosition + 1) % MAX_UNDO_BUFFERS;
    return gUndoBuffer.frames[redoPos].description;
}
