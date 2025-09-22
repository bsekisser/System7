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
} DialogManagerState;

/* DialogItemInternal for DialogItems.h */
typedef struct DialogItemInternal {
    Handle itemHandle;
    Rect itemRect;
    UInt8 itemType;
    UInt8 itemLength;
    SInt16 controlItem;
    void* itemData;
} DialogItemInternal;

#endif /* DIALOGMANAGERSTATEEXT_H */