#!/bin/bash

# Fix all TextEdit source files

# List of files to fix
FILES="src/TextEdit/TextEditInput.c src/TextEdit/TextEditScroll.c src/TextEdit/TextEditClipboard.c src/TextEdit/TextBreak.c src/TextEdit/TextEditTest.c"

for file in $FILES; do
    echo "Fixing $file..."
    
    # Add TRUE/FALSE and TEExtRec at the beginning
    sed -i '/^#include.*QuickDraw/a\
\
/* Boolean constants */\
#ifndef TRUE\
#define TRUE 1\
#endif\
#ifndef FALSE\
#define FALSE 0\
#endif\
\
/* Extended TERec with additional fields - must match TextEdit.c */\
typedef struct TEExtRec {\
    TERec       base;           /* Standard TERec */\
    Handle      hLines;         /* Line starts array */\
    SInt16      nLines;         /* Number of lines */\
    Handle      hStyles;        /* Style record handle */\
    Boolean     dirty;          /* Needs recalc */\
    Boolean     readOnly;       /* Read-only flag */\
    Boolean     wordWrap;       /* Word wrap flag */\
    SInt16      dragAnchor;     /* Drag selection anchor */\
    Boolean     inDragSel;      /* In drag selection */\
    UInt32      lastClickTime;  /* For double/triple click */\
    SInt16      clickCount;     /* Click count */\
    SInt16      viewDH;         /* Horizontal scroll */\
    SInt16      viewDV;         /* Vertical scroll */\
} TEExtRec;\
\
typedef TEExtRec *TEExtPtr, **TEExtHandle;' "$file"

    # Replace TEPtr with TEExtPtr
    sed -i 's/TEPtr pTE;/TEExtPtr pTE;/g' "$file"
    sed -i 's/pTE = \*hTE;/pTE = (TEExtPtr)*hTE;/g' "$file"
    
    # Fix field accesses - add base. prefix for standard TERec fields
    sed -i 's/pTE->inPort/pTE->base.inPort/g' "$file"
    sed -i 's/pTE->viewRect/pTE->base.viewRect/g' "$file"
    sed -i 's/pTE->destRect/pTE->base.destRect/g' "$file"
    sed -i 's/pTE->teLength/pTE->base.teLength/g' "$file"
    sed -i 's/pTE->selStart/pTE->base.selStart/g' "$file"
    sed -i 's/pTE->selEnd/pTE->base.selEnd/g' "$file"
    sed -i 's/pTE->fontAscent/pTE->base.fontAscent/g' "$file"
    sed -i 's/pTE->lineHeight/pTE->base.lineHeight/g' "$file"
    sed -i 's/pTE->active/pTE->base.active/g' "$file"
    sed -i 's/pTE->caretState/pTE->base.caretState/g' "$file"
    sed -i 's/pTE->caretTime/pTE->base.caretTime/g' "$file"
    sed -i 's/pTE->hText/pTE->base.hText/g' "$file"
    sed -i 's/pTE->txFont/pTE->base.txFont/g' "$file"
    sed -i 's/pTE->txSize/pTE->base.txSize/g' "$file"
    sed -i 's/pTE->txFace/pTE->base.txFace/g' "$file"
    sed -i 's/pTE->just/pTE->base.just/g' "$file"
    sed -i 's/pTE->clickLoc/pTE->base.clickLoc/g' "$file"
    sed -i 's/pTE->clickTime/pTE->base.clickTime/g' "$file"
    sed -i 's/pTE->clikLoc/pTE->base.clickLoc/g' "$file"  # Fix typo
    
    # Fix GetKeys conflict in TextEditInput.c
    if [[ "$file" == *"TextEditInput.c"* ]]; then
        sed -i 's/Boolean GetKeys/static Boolean TE_GetKeys/g' "$file"
        sed -i 's/GetKeys(/TE_GetKeys(/g' "$file"
    fi
done

echo "Done!"
