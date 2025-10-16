/*
 * MacPaint_Rendering.c - UI Rendering and Display
 *
 * Implements all visual rendering for MacPaint:
 * - Paint window rendering (bitmap display)
 * - Dialog UI rendering (pattern/brush editors)
 * - Grid overlay
 * - Selection rectangle display
 * - Tool cursor management
 * - Toolbox and interface elements
 *
 * Uses System 7.1 QuickDraw for all rendering operations
 */

#include "SystemTypes.h"
#include "Apps/MacPaint.h"
#include "QuickDraw/QuickDraw.h"
#include "QuickDrawConstants.h"
#include "WindowManager/WindowManager.h"
#include "System71StdLib.h"
#include <string.h>

/*
 * RENDERING STATE
 */

typedef struct {
    int showGrid;               /* Grid overlay enabled */
    int gridSpacing;           /* Pixels between grid lines */
    int gridColor;             /* Grid line color */
    int fatBitsMode;           /* Fat Bits zoom display */
    int fatBitsZoom;           /* Zoom factor (2, 4, 8, etc) */
    int showSelectionRect;     /* Selection rectangle visible */
    Rect selectionRect;        /* Current selection bounds */
    int selectionMarching;     /* Marching ants animation */
} RenderState;

static RenderState gRenderState = {0, 16, 0xCCCCCCCC, 0, 2, 0};

/* External state from MacPaint modules */
extern BitMap gPaintBuffer;
extern WindowPtr gPaintWindow;
extern int gCurrentTool;

/*
 * PAINT WINDOW RENDERING
 */

/**
 * MacPaint_RenderPaintBuffer - Draw the paint buffer to window
 * This is the main rendering routine called on every update
 */
void MacPaint_RenderPaintBuffer(void)
{
    if (!gPaintWindow) {
        return;
    }

    GrafPtr port = GetWindowPort(gPaintWindow);
    if (!port) {
        return;
    }

    SetPort(port);
    PenNormal();

    /* Render paint buffer using appropriate mode */
    if (gRenderState.fatBitsMode) {
        MacPaint_RenderFatBits();
    } else {
        /* Normal 1:1 rendering using CopyBits */
        Rect dstRect;
        MacPaint_GetPaintRect(&dstRect);
        CopyBits(&gPaintBuffer, &port->portBits,
                 &gPaintBuffer.bounds, &dstRect,
                 srcCopy, NULL);
    }

    /* Render overlay elements */
    if (gRenderState.showGrid) {
        MacPaint_DrawGridOverlay();
    }

    if (gRenderState.showSelectionRect) {
        MacPaint_DrawSelectionRectangle();
    }

    MacPaint_DrawToolbox();
    MacPaint_DrawStatusBar();
}

/**
 * MacPaint_RenderFatBits - Render in Fat Bits zoom mode
 * Shows each pixel as a larger block for pixel-level editing
 */
void MacPaint_RenderFatBits(void)
{
    GrafPtr port = GetWindowPort(gPaintWindow);
    if (!port) {
        return;
    }

    SetPort(port);
    int zoom = gRenderState.fatBitsZoom;
    int x, y;
    Rect pixelRect;
    Rect dstRect;
    MacPaint_GetPaintRect(&dstRect);

    PenNormal();
    PenSize(1, 1);

    /* Draw each pixel as a zoom block */
    for (y = 0; y < gPaintBuffer.bounds.bottom; y++) {
        for (x = 0; x < gPaintBuffer.bounds.right; x++) {
            pixelRect.top = dstRect.top + (y * zoom);
            pixelRect.left = dstRect.left + (x * zoom);
            pixelRect.bottom = pixelRect.top + zoom;
            pixelRect.right = pixelRect.left + zoom;

            if (MacPaint_PixelTrue(x, y, &gPaintBuffer)) {
                PaintRect(&pixelRect);
            } else {
                FrameRect(&pixelRect);
            }
        }
    }

    /* Draw pixel grid lines */
    PenMode(patXor);
    for (x = 0; x <= gPaintBuffer.bounds.right; x++) {
        int screenX = dstRect.left + (x * zoom);
        MoveTo(screenX, dstRect.top);
        LineTo(screenX, dstRect.bottom);
    }
    for (y = 0; y <= gPaintBuffer.bounds.bottom; y++) {
        int screenY = dstRect.top + (y * zoom);
        MoveTo(dstRect.left, screenY);
        LineTo(dstRect.right, screenY);
    }
    PenNormal();
}

/*
 * GRID RENDERING
 */

/**
 * MacPaint_DrawGridOverlay - Draw grid overlay on canvas
 */
void MacPaint_DrawGridOverlay(void)
{
    if (!gRenderState.showGrid || !gPaintWindow) {
        return;
    }

    GrafPtr port = GetWindowPort(gPaintWindow);
    if (!port) {
        return;
    }

    SetPort(port);

    Rect paintRect;
    MacPaint_GetPaintRect(&paintRect);

    /* Use XOR pen mode so grid is visible on any background */
    PenMode(patXor);
    PenSize(1, 1);

    int spacing = gRenderState.gridSpacing;

    /* Draw vertical grid lines */
    for (int x = 0; x < paintRect.right; x += spacing) {
        MoveTo(paintRect.left + x, paintRect.top);
        LineTo(paintRect.left + x, paintRect.bottom);
    }

    /* Draw horizontal grid lines */
    for (int y = 0; y < paintRect.bottom; y += spacing) {
        MoveTo(paintRect.left, paintRect.top + y);
        LineTo(paintRect.right, paintRect.top + y);
    }

    PenNormal();
}

/**
 * MacPaint_ToggleGridDisplay - Turn grid on/off
 */
void MacPaint_ToggleGridDisplay(void)
{
    gRenderState.showGrid = !gRenderState.showGrid;
    MacPaint_InvalidateWindowArea();
}

/**
 * MacPaint_SetGridSpacing - Set grid line spacing in pixels
 */
void MacPaint_SetGridSpacing(int spacing)
{
    if (spacing >= 4 && spacing <= 64) {
        gRenderState.gridSpacing = spacing;
        MacPaint_InvalidateWindowArea();
    }
}

/*
 * SELECTION RECTANGLE RENDERING
 */

/**
 * MacPaint_DrawSelectionRectangle - Draw marching ants selection outline
 */
void MacPaint_DrawSelectionRectangle(void)
{
    if (!gRenderState.showSelectionRect || !gPaintWindow) {
        return;
    }

    GrafPtr port = GetWindowPort(gPaintWindow);
    if (!port) {
        return;
    }

    SetPort(port);

    /* Use XOR pen mode so visible on any background */
    PenMode(patXor);
    PenSize(1, 1);

    /* Create marching ants pattern with animation offset */
    UInt8 marchPattern[8];
    int offset = gRenderState.selectionMarching % 8;
    for (int i = 0; i < 8; i++) {
        marchPattern[i] = (i >= offset) ? 0xFF : 0x00;
    }

    PenPat((Pattern *)marchPattern);

    /* Draw outline with marching ants effect */
    FrameRect(&gRenderState.selectionRect);

    PenNormal();
}

/**
 * MacPaint_UpdateSelectionDisplay - Update selection rectangle display
 */
void MacPaint_UpdateSelectionDisplay(const Rect *rect)
{
    if (!rect) {
        gRenderState.showSelectionRect = 0;
    } else {
        gRenderState.selectionRect = *rect;
        gRenderState.showSelectionRect = 1;
    }
    MacPaint_InvalidateWindowArea();
}

/**
 * MacPaint_AnimateSelection - Animate marching ants
 * Called periodically to update marching ants animation
 */
void MacPaint_AnimateSelection(void)
{
    if (!gRenderState.showSelectionRect) {
        return;
    }

    /* Update animation frame for marching ants */
    gRenderState.selectionMarching++;
    if (gRenderState.selectionMarching > 7) {
        gRenderState.selectionMarching = 0;
    }
    MacPaint_InvalidateWindowArea();
}

/*
 * PATTERN EDITOR DIALOG RENDERING
 */

/**
 * MacPaint_RenderPatternEditorDialog - Draw pattern editor window
 */
void MacPaint_RenderPatternEditorDialog(void)
{
    /* TODO: Use DialogManager to draw pattern editor
     *
     * Layout:
     * +-------------------+
     * | Pattern Editor    |
     * +---+---+---+---+---+---+---+---+
     * | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
     * +---+---+---+---+---+---+---+---+
     * | 8 | 9 | 10| 11| 12| 13| 14| 15|
     * +---+---+---+---+---+---+---+---+
     * ...
     * | 56| 57| 58| 59| 60| 61| 62| 63|
     * +---+---+---+---+---+---+---+---+
     *
     * [Preview]  [OK] [Cancel] [Reset]
     *
     * Each pixel is a clickable checkbox (8x8 grid)
     * Preview shows current pattern
     * Buttons for OK/Cancel/Reset
     */
}

/**
 * MacPaint_DrawPatternPreview - Draw preview of edited pattern
 */
void MacPaint_DrawPatternPreview(const Rect *previewRect)
{
    if (!previewRect) {
        return;
    }

    /* TODO: Render pattern in preview box
     *
     * Pseudocode:
     * Pattern editPat = MacPaint_GetPatternEditorPattern();
     *
     * // Tile pattern to fill preview rect
     * for (y = previewRect->top; y < previewRect->bottom; y += 8) {
     *     for (x = previewRect->left; x < previewRect->right; x += 8) {
     *         // Draw 8x8 pattern tile at (x, y)
     *         for (py = 0; py < 8; py++) {
     *             for (px = 0; px < 8; px++) {
     *                 if ((editPat.pat[py] >> (7 - px)) & 1) {
     *                     // Draw filled pixel
     *                     Point pt = {x + px, y + py};
     *                     SetCPixel(pt.h, pt.v, blackColor);
     *                 }
     *             }
     *         }
     *     }
     * }
     */
}

/*
 * BRUSH EDITOR DIALOG RENDERING
 */

/**
 * MacPaint_RenderBrushEditorDialog - Draw brush editor window
 */
void MacPaint_RenderBrushEditorDialog(void)
{
    /* TODO: Use DialogManager to draw brush editor
     *
     * Layout:
     * +-------------------+
     * | Brush Editor      |
     * +-------------------+
     * Brush Shapes:
     *   ○ Circle (filled)
     *   ○ Square (filled)
     *   ○ Diamond
     *   ○ Spray
     *   ○ Custom Pattern
     *
     * Size: [====|====] (slider from 1 to 64)
     *
     * [Preview Area]
     *
     * [OK] [Cancel]
     */
}

/**
 * MacPaint_DrawBrushPreview - Draw preview of edited brush
 */
void MacPaint_DrawBrushPreview(const Rect *previewRect)
{
    if (!previewRect) {
        return;
    }

    /* TODO: Render brush shape in preview
     *
     * Pseudocode:
     * size = MacPaint_GetBrushSize();
     *
     * // Draw centered preview
     * centerX = (previewRect->left + previewRect->right) / 2;
     * centerY = (previewRect->top + previewRect->bottom) / 2;
     *
     * switch (gCurrentBrushShape) {
     *     case BRUSH_CIRCLE:
     *         PaintOval(centerX - size/2, centerY - size/2,
     *                   centerX + size/2, centerY + size/2);
     *         break;
     *     case BRUSH_SQUARE:
     *         PaintRect(...);
     *         break;
     *     // etc
     * }
     */
}

/*
 * TOOLBOX RENDERING
 */

/**
 * MacPaint_DrawToolbox - Draw tool palette on screen
 */
void MacPaint_DrawToolbox(void)
{
    /* TODO: Render tool palette
     *
     * Layout (typical MacPaint toolbox):
     * +-------+
     * | 1 | 2 |  (lasso, select)
     * +---+---+
     * | 3 | 4 |  (grabber, text)
     * +---+---+
     * | 5 | 6 |  (fill, spray)
     * +---+---+
     * | 7 | 8 |  (brush, pencil)
     * +---+---+
     * | 9 | A |  (line, erase)
     * +---+---+
     * | B | C |  (rect, oval)
     * +---+---+
     *
     * Highlight currently selected tool
     * Show tool icons with QuickDraw or ICON resources
     */
}

/**
 * MacPaint_HighlightActiveTool - Draw highlight on currently selected tool
 */
void MacPaint_HighlightActiveTool(void)
{
    /* TODO: Draw selection highlight around current tool icon
     * Use inverse colors or frame to show active tool
     */
}

/*
 * STATUS BAR RENDERING
 */

/**
 * MacPaint_DrawStatusBar - Draw status information at bottom
 */
void MacPaint_DrawStatusBar(void)
{
    /* TODO: Render status bar showing:
     * - Current tool name
     * - Brush size
     * - Current pattern
     * - Mouse coordinates
     * - Document name and dirty indicator
     *
     * Layout:
     * [Tool: Pencil] [Size: 2] [Pattern: Solid] | X: 0 Y: 0 | Untitled*
     */
}

/*
 * CURSOR MANAGEMENT
 */

/**
 * MacPaint_SetToolCursor - Update cursor based on current tool
 */
void MacPaint_SetToolCursor(void)
{
    /* TODO: Set cursor shape based on gCurrentTool
     *
     * Tool cursors:
     * - Pencil: crosshair
     * - Brush: brush shape
     * - Eraser: eraser shape
     * - Line: crosshair
     * - Rectangle: crosshair
     * - Oval: crosshair
     * - Lasso: lasso shape
     * - Select: rectangle select cursor
     * - Fill: bucket/paint can
     * - Spray: spray/airbrush
     * - Grabber: hand
     * - Text: i-beam
     *
     * Load cursors from CURS resources or create programmatically
     */
}

/**
 * MacPaint_UpdateCursorPosition - Update cursor on mouse move
 */
void MacPaint_UpdateCursorPosition(int x, int y)
{
    /* TODO: Update cursor appearance based on:
     * - Current tool
     * - Mouse position (over canvas vs toolbox vs window)
     * - Active selection (might change to move cursor)
     */
}

/*
 * INVALIDATION AND REDRAW COORDINATION
 */

/**
 * MacPaint_InvalidateToolArea - Mark toolbox for redraw
 */
void MacPaint_InvalidateToolArea(void)
{
    /* TODO: Invalidate toolbox rectangle
     * Typically on left or right side of window
     */
}

/**
 * MacPaint_InvalidateStatusArea - Mark status bar for redraw
 */
void MacPaint_InvalidateStatusArea(void)
{
    /* TODO: Invalidate status bar rectangle
     * Typically at bottom of window
     */
}

/*
 * ANIMATED ELEMENTS
 */

/**
 * MacPaint_UpdateAnimations - Update all animated elements
 * Called periodically during idle time
 */
void MacPaint_UpdateAnimations(void)
{
    if (gRenderState.showSelectionRect) {
        MacPaint_AnimateSelection();
    }

    /* TODO: Update other animations
     * - Tool cursor animation
     * - Blinking text cursor in text tool
     * - Spray particle animation
     */
}

/*
 * RENDERING OPTIONS
 */

/**
 * MacPaint_SetFatBitsMode - Enable/disable Fat Bits zoom display
 */
void MacPaint_SetFatBitsMode(int enabled, int zoomFactor)
{
    if (enabled) {
        gRenderState.fatBitsMode = 1;
        gRenderState.fatBitsZoom = (zoomFactor >= 1 && zoomFactor <= 16) ? zoomFactor : 2;
    } else {
        gRenderState.fatBitsMode = 0;
    }
    MacPaint_InvalidateWindowArea();
}

/**
 * MacPaint_IsFatBitsMode - Check if in Fat Bits mode
 */
int MacPaint_IsFatBitsMode(void)
{
    return gRenderState.fatBitsMode;
}

/*
 * FULL WINDOW UPDATE
 */

/**
 * MacPaint_FullWindowUpdate - Complete redraw of entire window
 */
void MacPaint_FullWindowUpdate(void)
{
    if (!gPaintWindow) {
        return;
    }

    BeginUpdate(gPaintWindow);

    GrafPtr port = GetWindowPort(gPaintWindow);
    if (port) {
        SetPort(port);

        /* Erase background with white */
        BackColor(whiteColor);
        EraseRect(&port->portRect);

        /* Render all elements in order */
        MacPaint_RenderPaintBuffer();
        MacPaint_DrawToolbox();
        MacPaint_DrawStatusBar();
    }

    EndUpdate(gPaintWindow);
}

/*
 * RENDERING STATISTICS (DEBUG)
 */

/**
 * MacPaint_GetRenderStats - Get rendering performance stats
 */
void MacPaint_GetRenderStats(int *pixelsRendered, int *updateTime)
{
    /* TODO: Track rendering performance
     * - Total pixels rendered per frame
     * - Time spent in rendering code
     * - Invalidated regions
     */
    if (pixelsRendered) *pixelsRendered = 0;
    if (updateTime) *updateTime = 0;
}
