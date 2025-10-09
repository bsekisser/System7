/*
 * DialogDrawing.c - Dialog Item Drawing Implementation
 *
 * Implements faithful System 7.1-style drawing for all dialog item types including
 * buttons, checkboxes, radio buttons, static text, edit text, icons, and user items.
 * Uses classic Mac look with proper beveling, focus rings, and state rendering.
 */

#include <stdlib.h>
#include <string.h>
#include "SystemTypes.h"
#include "System71StdLib.h"
#include "DialogManager/DialogManager.h"
#include "DialogManager/DialogTypes.h"
#include "DialogManager/DialogDrawing.h"
#include "DialogManager/DialogInternal.h"
#include "DialogManager/DialogManagerInternal.h"  /* For DialogItemEx */
#include "DialogManager/DialogManagerStateExt.h"   /* For extended state with focus tracking */
#include "DialogManager/DialogLogging.h"

/* External QuickDraw dependencies */
extern void SetPort(GrafPtr port);
extern void GetPort(GrafPtr* port);
extern void FrameRect(const Rect* r);
extern void PaintRect(const Rect* r);
extern void EraseRect(const Rect* r);
extern void InvertRect(const Rect* r);
extern void FillRect(const Rect* r, const Pattern* pat);
extern void FrameRoundRect(const Rect* r, SInt16 ovalWidth, SInt16 ovalHeight);
extern void PaintRoundRect(const Rect* r, SInt16 ovalWidth, SInt16 ovalHeight);
extern void MoveTo(SInt16 h, SInt16 v);
extern void LineTo(SInt16 h, SInt16 v);
extern void PenSize(SInt16 width, SInt16 height);
extern void PenNormal(void);
extern void TextFont(SInt16 font);
extern void TextSize(SInt16 size);
extern void TextFace(Style face);
extern void DrawString(const unsigned char* s);
extern SInt16 StringWidth(const unsigned char* s);

/* External Window Manager dependencies */
extern void InvalRect(const Rect* rect);

/* QuickDraw globals */
extern QDGlobals qd;

/* Dialog Manager state access */
extern DialogManagerState* GetDialogManagerState(void);

/* Helper: Inset rectangle */
static void InsetRect(Rect* r, SInt16 dh, SInt16 dv) {
    r->left += dh;
    r->top += dv;
    r->right -= dh;
    r->bottom -= dv;
}

/* Helper: Offset rectangle */
static void OffsetRect(Rect* r, SInt16 dh, SInt16 dv) {
    r->left += dh;
    r->top += dv;
    r->right += dh;
    r->bottom += dv;
}

/* Helper: Check if point is in rect */
static Boolean PtInRect(Point pt, const Rect* r) {
    return (pt.h >= r->left && pt.h < r->right &&
            pt.v >= r->top && pt.v < r->bottom);
}

/* Draw push button or default button */
void DrawDialogButton(DialogPtr theDialog, const Rect* bounds, const unsigned char* title,
                     Boolean isDefault, Boolean isEnabled, Boolean isPressed) {
    Rect btnRect = *bounds;
    Rect frameRect;
    SInt16 textWidth, textH, textV;
    GrafPtr savePort;

    GetPort(&savePort);
    if (theDialog) {
        SetPort((GrafPtr)theDialog);
    }

    DIALOG_LOG_DEBUG("Dialog: DrawButton '%.*s' default=%d enabled=%d\n",
                 title ? title[0] : 0, title ? (const char*)&title[1] : "",
                 isDefault, isEnabled);

    /* Draw default button ring if needed (3-pixel inset frame) */
    if (isDefault) {
        frameRect = btnRect;
        InsetRect(&frameRect, -4, -4);
        PenSize(3, 3);
        FrameRoundRect(&frameRect, 16, 16);
        PenNormal();
    }

    /* Draw button background */
    if (isPressed) {
        PaintRoundRect(&btnRect, 12, 12);
    } else {
        EraseRect(&btnRect);
        FrameRoundRect(&btnRect, 12, 12);
    }

    /* Draw button text */
    if (title && title[0] > 0) {
        TextFont(0);  /* System font */
        TextSize(12);
        TextFace(isEnabled ? 0 : 0x80);  /* Dim if disabled */

        textWidth = StringWidth(title);
        textH = btnRect.left + ((btnRect.right - btnRect.left - textWidth) / 2);
        textV = btnRect.bottom - 5;

        MoveTo(textH, textV);
        if (isPressed) {
            /* Invert for pressed look */
            DrawString(title);
        } else {
            DrawString(title);
        }
    }

    /* Draw disabled stipple if needed */
    if (!isEnabled) {
        FillRect(&btnRect, &qd.ltGray);
    }

    SetPort(savePort);
}

/* Draw checkbox */
void DrawDialogCheckBox(const Rect* bounds, const unsigned char* title,
                       Boolean isChecked, Boolean isEnabled) {
    Rect boxRect;
    Rect textRect;
    SInt16 textV;
    GrafPtr savePort;

    GetPort(&savePort);

    DIALOG_LOG_DEBUG("Dialog: DrawCheckBox '%.*s' checked=%d enabled=%d\n",
                 title ? title[0] : 0, title ? (const char*)&title[1] : "",
                 isChecked, isEnabled);

    /* Checkbox is 13x13 square on left */
    boxRect.top = bounds->top + 1;
    boxRect.left = bounds->left;
    boxRect.bottom = boxRect.top + 13;
    boxRect.right = boxRect.left + 13;

    /* Draw box */
    EraseRect(&boxRect);
    FrameRect(&boxRect);

    /* Draw check mark if checked */
    if (isChecked) {
        Rect checkRect = boxRect;
        InsetRect(&checkRect, 2, 2);
        /* Draw X pattern for check */
        MoveTo(checkRect.left, checkRect.top);
        LineTo(checkRect.right-1, checkRect.bottom-1);
        MoveTo(checkRect.right-1, checkRect.top);
        LineTo(checkRect.left, checkRect.bottom-1);
    }

    /* Draw title text */
    if (title && title[0] > 0) {
        TextFont(0);
        TextSize(12);
        TextFace(isEnabled ? 0 : 0x80);

        textRect.left = boxRect.right + 6;
        textRect.top = bounds->top;
        textRect.right = bounds->right;
        textRect.bottom = bounds->bottom;

        textV = textRect.bottom - 4;
        MoveTo(textRect.left, textV);
        DrawString(title);
    }

    /* Draw disabled stipple if needed */
    if (!isEnabled) {
        FillRect(&boxRect, &qd.ltGray);
    }

    SetPort(savePort);
}

/* Draw radio button */
void DrawDialogRadioButton(const Rect* bounds, const unsigned char* title,
                          Boolean isSelected, Boolean isEnabled) {
    Rect circleRect;
    Rect fillRect;
    SInt16 textV;
    GrafPtr savePort;

    GetPort(&savePort);

    DIALOG_LOG_DEBUG("Dialog: DrawRadioButton '%.*s' selected=%d enabled=%d\n",
                 title ? title[0] : 0, title ? (const char*)&title[1] : "",
                 isSelected, isEnabled);

    /* Radio button is 13x13 circle on left */
    circleRect.top = bounds->top + 1;
    circleRect.left = bounds->left;
    circleRect.bottom = circleRect.top + 13;
    circleRect.right = circleRect.left + 13;

    /* Draw circle using RoundRect with equal width/height */
    EraseRect(&circleRect);
    FrameRoundRect(&circleRect, 13, 13);

    /* Draw filled center if selected */
    if (isSelected) {
        fillRect = circleRect;
        InsetRect(&fillRect, 3, 3);
        PaintRoundRect(&fillRect, 7, 7);
    }

    /* Draw title text */
    if (title && title[0] > 0) {
        TextFont(0);
        TextSize(12);
        TextFace(isEnabled ? 0 : 0x80);

        textV = circleRect.bottom - 3;
        MoveTo(circleRect.right + 6, textV);
        DrawString(title);
    }

    /* Draw disabled stipple if needed */
    if (!isEnabled) {
        FillRect(&circleRect, &qd.ltGray);
    }

    SetPort(savePort);
}

/* Draw static text */
void DrawDialogStaticText(DialogPtr theDialog, const Rect* bounds, const unsigned char* text,
                         Boolean isEnabled) {
    SInt16 textV;
    GrafPtr savePort;

    GetPort(&savePort);
    if (theDialog) {
        SetPort((GrafPtr)theDialog);
    }

    if (!text || text[0] == 0) {
        SetPort(savePort);
        return;
    }

    DIALOG_LOG_DEBUG("Dialog: DrawStaticText '%.*s'\n",
                 text[0], (const char*)&text[1]);

    /* Erase background */
    EraseRect(bounds);

    /* Draw text */
    TextFont(0);
    TextSize(12);
    TextFace(isEnabled ? 0 : 0x80);

    textV = bounds->top + 12;  /* Baseline */

    /* Debug: Check current port's coordinate mapping */
    {
        GrafPtr currentPort;
        GetPort(&currentPort);
        if (currentPort) {
            DIALOG_LOG_DEBUG("  portBits.bounds=(%d,%d,%d,%d) portRect=(%d,%d,%d,%d)\n",
                         currentPort->portBits.bounds.left, currentPort->portBits.bounds.top,
                         currentPort->portBits.bounds.right, currentPort->portBits.bounds.bottom,
                         currentPort->portRect.left, currentPort->portRect.top,
                         currentPort->portRect.right, currentPort->portRect.bottom);
        }
    }

    MoveTo(bounds->left + 2, textV);
    DrawString(text);

    SetPort(savePort);
}

/* Draw edit text field */
void DrawDialogEditText(const Rect* bounds, const unsigned char* text,
                       Boolean isEnabled, Boolean hasFocus) {
    Rect frameRect = *bounds;
    Rect textRect = *bounds;
    Rect caretRect;
    SInt16 textV;
    SInt16 textWidth;
    GrafPtr savePort;
    DialogManagerState* state;
    DialogManagerState_Extended* extState;

    GetPort(&savePort);
    state = GetDialogManagerState();
    extState = GET_EXTENDED_DLG_STATE(state);

    DIALOG_LOG_DEBUG("Dialog: DrawEditText '%.*s' enabled=%d focus=%d\n",
                 text ? text[0] : 0, text ? (const char*)&text[1] : "",
                 isEnabled, hasFocus);

    /* Draw recessed frame */
    EraseRect(&frameRect);
    InsetRect(&frameRect, -1, -1);
    FrameRect(&frameRect);

    /* Draw inner white background */
    EraseRect(bounds);

    /* Draw text if present */
    textWidth = 0;
    if (text && text[0] > 0) {
        TextFont(0);
        TextSize(12);
        TextFace(isEnabled ? 0 : 0x80);

        InsetRect(&textRect, 3, 2);
        textV = textRect.top + 11;
        MoveTo(textRect.left, textV);
        DrawString(text);
        textWidth = StringWidth(text);
    } else {
        InsetRect(&textRect, 3, 2);
        textV = textRect.top + 11;
    }

    /* Draw focus ring if active */
    if (hasFocus && isEnabled) {
        Rect focusRect = *bounds;
        InsetRect(&focusRect, -2, -2);
        PenSize(2, 2);
        FrameRect(&focusRect);
        PenNormal();

        /* Draw caret if focus and caret is visible */
        if (extState && extState->caretVisible) {
            caretRect.left = textRect.left + textWidth;
            caretRect.right = caretRect.left + 1;
            caretRect.top = textRect.top;
            caretRect.bottom = textRect.bottom;
            InvertRect(&caretRect);
        }
    }

    /* Draw disabled pattern if needed */
    if (!isEnabled) {
        FillRect(bounds, &qd.ltGray);
    }

    SetPort(savePort);
}

/* Draw icon item */
void DrawDialogIcon(const Rect* bounds, SInt16 iconID, Boolean isEnabled) {
    GrafPtr savePort;

    GetPort(&savePort);

    DIALOG_LOG_DEBUG("Dialog: DrawIcon id=%d at (%d,%d,%d,%d)\n",
                 iconID, bounds->top, bounds->left, bounds->bottom, bounds->right);

    /* For now, just draw a placeholder frame */
    /* In full implementation, would load and draw actual icon resource */
    EraseRect(bounds);
    FrameRect(bounds);

    /* Draw X through it as placeholder */
    MoveTo(bounds->left, bounds->top);
    LineTo(bounds->right-1, bounds->bottom-1);
    MoveTo(bounds->right-1, bounds->top);
    LineTo(bounds->left, bounds->bottom-1);

    if (!isEnabled) {
        FillRect(bounds, &qd.ltGray);
    }

    SetPort(savePort);
}

/* Draw user item (calls user proc) */
void DrawDialogUserItem(DialogPtr theDialog, SInt16 itemNo, const Rect* bounds,
                       UserItemProcPtr userProc) {
    GrafPtr savePort;

    GetPort(&savePort);

    DIALOG_LOG_DEBUG("Dialog: DrawUserItem %d\n", itemNo);

    if (userProc) {
        /* Call user's drawing procedure */
        userProc(theDialog, itemNo);
    } else {
        /* No procedure - draw placeholder */
        FrameRect(bounds);
    }

    SetPort(savePort);
}

/* Main dialog item drawing dispatcher */
void DrawDialogItemByType(DialogPtr theDialog, SInt16 itemNo,
                         const DialogItemEx* item) {
    SInt16 baseType;
    const unsigned char* textData;

    if (!item || !item->visible) return;

    baseType = item->type & itemTypeMask;
    textData = (const unsigned char*)item->data;

    /* Handle control items (buttons, checkboxes, radios) */
    /* Control items are type 4/5/6 = ctrlItem + control_type */
    if (baseType >= ctrlItem && baseType < statText) {
        SInt16 controlType = baseType - ctrlItem;

        if (controlType == btnCtrl) {
            /* Push button */
            Boolean isDefault = (itemNo == GetDialogDefaultItem(theDialog));
            DrawDialogButton(theDialog, &item->bounds, textData, isDefault,
                           item->enabled, false);
        } else if (controlType == chkCtrl) {
            /* Checkbox */
            Boolean isChecked = (item->refCon != 0);
            DrawDialogCheckBox(&item->bounds, textData, isChecked,
                             item->enabled);
        } else if (controlType == radCtrl) {
            /* Radio button */
            Boolean isSelected = (item->refCon != 0);
            DrawDialogRadioButton(&item->bounds, textData, isSelected,
                                item->enabled);
        } else {
            /* Unknown control type */
            DIALOG_LOG_DEBUG("Dialog: Unknown control type %d\n", controlType);
            FrameRect(&item->bounds);
        }
        return;
    }

    /* Handle other item types */
    switch (baseType) {
        case statText:  /* Static text */
            DrawDialogStaticText(theDialog, &item->bounds, textData, item->enabled);
            break;

        case editText:  /* Edit text */
        {
            DialogManagerState* state = GetDialogManagerState();
            DialogManagerState_Extended* extState = GET_EXTENDED_DLG_STATE(state);
            Boolean hasFocus = (extState && extState->focusedEditTextItem == itemNo);
            DrawDialogEditText(&item->bounds, textData, item->enabled, hasFocus);
            break;
        }

        case iconItem:  /* Icon */
            DrawDialogIcon(&item->bounds, (SInt16)item->refCon, item->enabled);
            break;

        case userItem:  /* User item (type 0) */
        {
            UserItemProcPtr proc = (UserItemProcPtr)item->handle;
            DrawDialogUserItem(theDialog, itemNo, &item->bounds, proc);
            break;
        }

        default:
            DIALOG_LOG_DEBUG("Dialog: Unknown item type %d\n", baseType);
            FrameRect(&item->bounds);
            break;
    }
}
