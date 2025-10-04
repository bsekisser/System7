/*
 * STView.c - SimpleText View/TextEdit Handling
 *
 * Manages TextEdit records, scrolling, selection, and styling
 */

#include <string.h>
#include "Apps/SimpleText.h"
#include "MemoryMgr/MemoryManager.h"
#include "FontManager/FontManager.h"

/* Helper functions */
static void ApplyStyleToSelection(STDocument* doc, SInt16 font, SInt16 size, Style style);
static void GetSelectionStyle(STDocument* doc, SInt16* font, SInt16* size, Style* style);

/*
 * STView_Create - Create TextEdit view for document
 */
void STView_Create(STDocument* doc) {
    Rect destRect, viewRect;

    if (!doc || !doc->window) return;

    ST_Log("Creating TextEdit view\n");

    /* Set up port */
    SetPort((GrafPtr)doc->window);

    /* Calculate text rectangles */
    destRect = ((GrafPtr)doc->window)->portRect;
    destRect.top += 4;
    destRect.left += 4;
    destRect.bottom -= 4;
    destRect.right -= kScrollBarWidth + 4;  /* Leave room for scrollbar */

    viewRect = destRect;

    /* Create TextEdit record */
    doc->hTE = TENew(&destRect, &viewRect);
    if (!doc->hTE) {
        ST_Log("Failed to create TextEdit record\n");
        return;
    }

    /* Set default attributes */
    (*doc->hTE)->txFont = g_ST.currentFont;
    (*doc->hTE)->txSize = g_ST.currentSize;
    (*doc->hTE)->txFace = g_ST.currentStyle;

    /* Enable word wrap */
    (*doc->hTE)->crOnly = -1;  /* Word wrap mode */

    /* Create vertical scrollbar (optional for v1) */
    /* TODO: Implement scrollbar if ControlManager available */
    doc->vScroll = NULL;

    /* Initialize style runs */
    doc->styles.numRuns = 0;
    doc->styles.hRuns = NULL;

    ST_Log("TextEdit view created\n");
}

/*
 * STView_Dispose - Dispose TextEdit view
 */
void STView_Dispose(STDocument* doc) {
    if (!doc) return;

    ST_Log("Disposing TextEdit view\n");

    /* Dispose TextEdit record */
    if (doc->hTE) {
        TEDispose(doc->hTE);
        doc->hTE = NULL;
    }

    /* Dispose scrollbar */
    if (doc->vScroll) {
        /* TODO: DisposeControl(doc->vScroll); */
        doc->vScroll = NULL;
    }
}

/*
 * STView_Draw - Draw TextEdit content
 */
void STView_Draw(STDocument* doc) {
    Rect updateRect;

    if (!doc || !doc->hTE || !doc->window) return;

    SetPort((GrafPtr)doc->window);

    /* Get update rectangle */
    updateRect = ((GrafPtr)doc->window)->portRect;

    /* Draw TextEdit content */
    TEUpdate(&updateRect, doc->hTE);

    /* Draw scrollbar if present */
    if (doc->vScroll) {
        /* TODO: Draw scrollbar */
    }
}

/*
 * STView_Click - Handle mouse click in view
 */
void STView_Click(STDocument* doc, EventRecord* event) {
    Point localPt;
    Boolean shiftDown;
    static UInt32 lastClickTime = 0;
    static Point lastClickPt = {0, 0};
    static int clickCount = 1;

    if (!doc || !doc->hTE) return;

    SetPort((GrafPtr)doc->window);

    /* Convert to local coordinates */
    localPt = event->where;
    GlobalToLocal(&localPt);

    /* Check for shift key (extend selection) */
    shiftDown = (event->modifiers & shiftKey) != 0;

    /* Check for double/triple click */
    if ((event->when - lastClickTime) < GetDblTime()) {
        /* Quick enough for multi-click */
        if (abs(localPt.h - lastClickPt.h) < 3 && abs(localPt.v - lastClickPt.v) < 3) {
            clickCount++;
            if (clickCount == 2) {
                /* Double-click - select word */
                /* TODO: Implement word selection */
                ST_Log("Double-click at (%d,%d)\n", localPt.h, localPt.v);
            } else if (clickCount >= 3) {
                /* Triple-click - select line */
                /* TODO: Implement line selection */
                ST_Log("Triple-click at (%d,%d)\n", localPt.h, localPt.v);
                clickCount = 3;  /* Cap at triple */
            }
        } else {
            clickCount = 1;
        }
    } else {
        clickCount = 1;
    }

    lastClickTime = event->when;
    lastClickPt = localPt;

    /* Handle regular click or shift-click */
    TEClick(localPt, shiftDown, doc->hTE);

    /* Save undo state after selection change */
    STClip_SaveUndo(doc);
}

/*
 * STView_Key - Handle keyboard input
 */
void STView_Key(STDocument* doc, EventRecord* event) {
    char key;
    SInt16 selStart, selEnd;

    if (!doc || !doc->hTE) return;

    key = event->message & charCodeMask;

    /* Get current selection */
    selStart = (*doc->hTE)->selStart;
    selEnd = (*doc->hTE)->selEnd;

    /* Check for special keys */
    switch (key) {
        case 0x08:  /* Backspace */
            if (selStart == selEnd && selStart > 0) {
                /* Delete character before cursor */
                TESetSelect(selStart - 1, selStart, doc->hTE);
            }
            TEDelete(doc->hTE);
            break;

        case 0x7F:  /* Delete */
            if (selStart == selEnd && selStart < (*doc->hTE)->teLength) {
                /* Delete character after cursor */
                TESetSelect(selStart, selStart + 1, doc->hTE);
            }
            TEDelete(doc->hTE);
            break;

        case 0x0D:  /* Return */
            /* Convert to CR for classic Mac */
            TEKey('\r', doc->hTE);
            break;

        case 0x1C:  /* Left arrow */
            if (selStart > 0) {
                if (event->modifiers & cmdKey) {
                    /* Cmd-Left: beginning of line */
                    /* TODO: Find line start */
                    TESetSelect(0, 0, doc->hTE);
                } else {
                    TESetSelect(selStart - 1, selStart - 1, doc->hTE);
                }
            }
            break;

        case 0x1D:  /* Right arrow */
            if (selEnd < (*doc->hTE)->teLength) {
                if (event->modifiers & cmdKey) {
                    /* Cmd-Right: end of line */
                    /* TODO: Find line end */
                    TESetSelect((*doc->hTE)->teLength, (*doc->hTE)->teLength, doc->hTE);
                } else {
                    TESetSelect(selEnd + 1, selEnd + 1, doc->hTE);
                }
            }
            break;

        case 0x1E:  /* Up arrow */
            /* TODO: Implement up arrow navigation */
            break;

        case 0x1F:  /* Down arrow */
            /* TODO: Implement down arrow navigation */
            break;

        default:
            /* Regular character */
            if (key >= 0x20 || key == 0x09) {  /* Printable or tab */
                /* Check for overflow */
                if ((*doc->hTE)->teLength >= kMaxFileSize) {
                    ST_Beep();
                    ST_Log("Text buffer overflow\n");
                    return;
                }
                TEKey(key, doc->hTE);
            }
            break;
    }

    /* Mark as dirty */
    STDoc_SetDirty(doc, true);
}

/*
 * STView_Resize - Handle window resize
 */
void STView_Resize(STDocument* doc) {
    Rect destRect, viewRect;

    if (!doc || !doc->hTE || !doc->window) return;

    ST_Log("Resizing TextEdit view\n");

    SetPort((GrafPtr)doc->window);

    /* Calculate new text rectangles */
    destRect = ((GrafPtr)doc->window)->portRect;
    destRect.top += 4;
    destRect.left += 4;
    destRect.bottom -= 4;
    destRect.right -= kScrollBarWidth + 4;

    viewRect = destRect;

    /* Update TextEdit rectangles */
    (*doc->hTE)->destRect = destRect;
    (*doc->hTE)->viewRect = viewRect;

    /* Recalculate line breaks */
    TECalText(doc->hTE);

    /* Invalidate window */
    InvalRect(&((GrafPtr)doc->window)->portRect);
}

/*
 * STView_Scroll - Scroll view
 */
void STView_Scroll(STDocument* doc, SInt16 dv, SInt16 dh) {
    if (!doc || !doc->hTE) return;

    /* Use TEScroll to scroll content */
    TEScroll(dh, dv, doc->hTE);
}

/*
 * STView_SetStyle - Apply style to selection
 */
void STView_SetStyle(STDocument* doc, SInt16 font, SInt16 size, Style style) {
    if (!doc || !doc->hTE) return;

    ST_Log("Setting style: font=%d size=%d style=%d\n", font, size, style);

    /* Apply to selection or insertion point */
    ApplyStyleToSelection(doc, font, size, style);

    /* Mark as dirty */
    STDoc_SetDirty(doc, true);
}

/*
 * STView_GetStyle - Get style at selection
 */
void STView_GetStyle(STDocument* doc, SInt16* font, SInt16* size, Style* style) {
    if (!doc || !doc->hTE) return;

    GetSelectionStyle(doc, font, size, style);
}

/*
 * STView_UpdateCaret - Update caret blink
 */
void STView_UpdateCaret(STDocument* doc) {
    if (!doc || !doc->hTE) return;

    /* Let TextEdit handle caret */
    TEIdle(doc->hTE);
}

/*
 * ApplyStyleToSelection - Apply text style to current selection
 */
static void ApplyStyleToSelection(STDocument* doc, SInt16 font, SInt16 size, Style style) {
    TextStyle theStyle;
    SInt16 selStart, selEnd;

    if (!doc || !doc->hTE) return;

    selStart = (*doc->hTE)->selStart;
    selEnd = (*doc->hTE)->selEnd;

    /* If no TESetStyle available, use basic TE style setting */
    if (selStart == selEnd) {
        /* Set style for next typed characters */
        (*doc->hTE)->txFont = font;
        (*doc->hTE)->txSize = size;
        (*doc->hTE)->txFace = style;
    } else {
        /* For selection, we'd need TESetStyle or custom implementation */
        /* For now, just set the default style */
        (*doc->hTE)->txFont = font;
        (*doc->hTE)->txSize = size;
        (*doc->hTE)->txFace = style;

        /* TODO: Implement proper styled text support */
        /* Would need to maintain style runs and apply during drawing */

        /* Recalculate and redraw */
        TECalText(doc->hTE);
        InvalRect(&(*doc->hTE)->viewRect);
    }

    /* Update global current style */
    g_ST.currentFont = font;
    g_ST.currentSize = size;
    g_ST.currentStyle = style;
}

/*
 * GetSelectionStyle - Get style at current selection
 */
static void GetSelectionStyle(STDocument* doc, SInt16* font, SInt16* size, Style* style) {
    /* For basic TE, return the default style */
    if (font) *font = (*doc->hTE)->txFont;
    if (size) *size = (*doc->hTE)->txSize;
    if (style) *style = (*doc->hTE)->txFace;

    /* TODO: For styled text, would need to examine style runs */
}