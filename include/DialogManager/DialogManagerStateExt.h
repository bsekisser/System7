#ifndef DIALOGMANAGERSTATEEXT_H
#define DIALOGMANAGERSTATEEXT_H

#include "../SystemTypes.h"
#include "../WindowManager/WindowManager.h"
#include "DialogManager.h"

/* DialogRecord is already defined in SystemTypes.h */

/* Global dialog manager parameters */
typedef struct DialogGlobals {
    ResumeProcPtr resumeProc;
    void* soundProc;
    SInt16 alertStage;
    SInt16 dialogFont;
    SInt16 spareFlags;
    DialogPtr frontModal;
    SInt16 defaultItem;
    SInt16 cancelItem;
    Boolean tracksCursor;
    unsigned char paramText[4][256];
    SInt16 alertStageCount;
    SInt16 alertButtonSpacing;
    SInt16 defaultButtonOutlineWidth;
    SInt16 buttonFrameInset;
    void* dialogTextProc;
    void* alertSoundProc;
    ModalFilterProcPtr modalFilterProc;
} DialogGlobals;

/* Only define extended DialogManagerState if not already defined */
#ifndef DIALOGMANAGERINTERNAL_H
/* Extended DialogManagerState with all expected fields */
typedef struct DialogManagerState {
    /* Basic fields from DialogManagerInternal.h */
    DialogPtr currentDialog;
    short modalDepth;
    Boolean inProgress;
    Handle itemList;
    short itemCount;

    /* Extended fields expected by DialogManagerCore.c */
    DialogGlobals globals;
    Boolean initialized;
    SInt16 modalLevel;
    Boolean systemModal;
    Boolean useNativeDialogs;
    Boolean useAccessibility;
    float scaleFactor;
    void* platformContext;
    DialogPtr modalStack[16];

    /* Edit-text focus tracking */
    SInt16 focusedEditTextItem;     /* Item number of focused edit text, or 0 if none */
    UInt32 caretBlinkTime;          /* Tick count of last caret blink */
    Boolean caretVisible;           /* Current caret visibility state */
} DialogManagerState;
#endif

/* DialogItemInternal for DialogItems.h */
typedef struct DialogItemInternal {
    Handle itemHandle;
    Rect itemRect;
    UInt8 itemType;
    UInt8 itemLength;
    SInt16 controlItem;
    void* itemData;
} DialogItemInternal;

/* Helper to access extended DialogManagerState fields
   Cast basic DialogManagerState* to extended version */
#define GET_EXTENDED_DLG_STATE(state) ((DialogManagerState_Extended*)(state))

/* Extended state type with focus tracking fields */
typedef struct DialogManagerState_Extended {
    /* Basic fields from DialogManagerInternal.h */
    DialogPtr currentDialog;
    short modalDepth;
    Boolean inProgress;
    Handle itemList;
    short itemCount;

    /* Extended fields */
    DialogGlobals globals;
    Boolean initialized;
    SInt16 modalLevel;
    Boolean systemModal;
    Boolean useNativeDialogs;
    Boolean useAccessibility;
    float scaleFactor;
    void* platformContext;
    DialogPtr modalStack[16];

    /* Edit-text focus tracking */
    SInt16 focusedEditTextItem;
    UInt32 caretBlinkTime;
    Boolean caretVisible;
} DialogManagerState_Extended;

#endif /* DIALOGMANAGERSTATEEXT_H */