#ifndef DIALOGMANAGERINTERNAL_H
#define DIALOGMANAGERINTERNAL_H

#include "../SystemTypes.h"
#include "DialogManager.h"

// Internal dialog structures
typedef struct DialogItemEx {
    Handle handle;
    Rect bounds;
    short type;
    short index;
    void* data;
} DialogItemEx;

typedef struct DialogManagerState {
    DialogPtr currentDialog;
    short modalDepth;
    Boolean inProgress;
    Handle itemList;
    short itemCount;
} DialogManagerState;

// Internal functions
void InitDialogInternals(void);
DialogItemEx* GetDialogItemEx(DialogPtr dialog, short item);
void UpdateDialogState(DialogManagerState* state);

extern DialogManagerState gDialogState;

#endif
