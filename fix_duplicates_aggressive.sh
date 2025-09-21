#!/bin/bash

cd /home/k/iteration2

echo "=== Removing duplicate type definitions from SystemTypes.h ==="

# Create a clean version of SystemTypes.h without the duplicates we added
cp include/SystemTypes.h include/SystemTypes.h.backup

# Remove the duplicate sections we added at the end
sed -i '/^\/\/ Dialog extensions - ensure DialogPeek has all needed fields/,/^#endif \/\/ BASE_TYPES_ULTRA_DEFINED$/d' include/SystemTypes.h

# Now add back only what's missing without duplicates
cat >> include/SystemTypes.h << 'EOF'

// Dialog extensions - ensure DialogPeek has all needed fields
#ifndef DIALOG_PEEK_EXTENDED
#define DIALOG_PEEK_EXTENDED

// Forward declare if needed
#ifndef MODAL_FILTER_UPP_DEFINED
#define MODAL_FILTER_UPP_DEFINED
typedef void (*ModalFilterUPP)(DialogPtr dialog, void* event, short* item);
typedef void (*ControlActionUPP)(ControlHandle control, short part);
#endif

typedef struct DialogRecordEx {
    WindowRecord window;
    Handle items;
    TEHandle textH;
    short editField;
    short editOpen;
    short aDefItem;
    // Additional fields for compatibility
    Handle itemList;
    short itemCount;
    ModalFilterUPP filterProc;
    void* refCon;
} DialogRecordEx;

typedef DialogRecordEx* DialogPeekEx;

// Add missing EditionManager types
typedef short SectionType;
typedef short FormatType;
typedef short UpdateMode;

// Add missing keyboard types
typedef struct KeyboardLayoutRec {
    short version;
    short keyMapID;
    Handle keyMapData;
    Str255 layoutName;
} KeyboardLayoutRec;

// Ensure thePort is declared
#ifndef THE_PORT_DECLARED
#define THE_PORT_DECLARED
extern GrafPtr thePort;
#endif

// Additional missing constants only if not defined
#ifndef MENU_BAR_CONSTANTS_DEFINED
#define MENU_BAR_CONSTANTS_DEFINED
enum {
    kMenuBarHeight = 20,
    kTitleBarHeight = 20,
    kScrollBarWidth = 16,
    kGrowBoxSize = 16
};
#endif

// Missing dialog constants
#ifndef DIALOG_ITEM_CONSTANTS_DEFINED
#define DIALOG_ITEM_CONSTANTS_DEFINED
enum {
    ctrlItem = 4,
    btnCtrl = 0,
    chkCtrl = 1,
    radCtrl = 2,
    resCtrl = 3,
    statText = 8,
    editText = 16,
    iconItem = 32,
    picItem = 64,
    userItem = 0,
    itemDisable = 128
};
#endif

// Missing window constants
#ifndef WINDOW_PROC_CONSTANTS_DEFINED
#define WINDOW_PROC_CONSTANTS_DEFINED
enum {
    documentProc = 0,
    dBoxProc = 1,
    plainDBox = 2,
    altDBoxProc = 3,
    noGrowDocProc = 4,
    movableDBoxProc = 5,
    zoomDocProc = 8,
    zoomNoGrow = 12,
    rDocProc = 16
};
#endif

// Missing event constants
#ifndef EVENT_TYPE_CONSTANTS_DEFINED
#define EVENT_TYPE_CONSTANTS_DEFINED
enum {
    nullEvent = 0,
    mouseDown = 1,
    mouseUp = 2,
    keyDown = 3,
    keyUp = 4,
    autoKey = 5,
    updateEvt = 6,
    diskEvt = 7,
    activateEvt = 8,
    osEvt = 15,
    everyEvent = 0xFFFF
};
#endif

// Only define CharsHandle if not already defined
#ifndef CHARS_HANDLE_DEFINED
#define CHARS_HANDLE_DEFINED
typedef Handle CharsHandle;
#endif

// Only define Style if not already defined
#ifndef STYLE_TYPE_DEFINED
#define STYLE_TYPE_DEFINED
typedef unsigned char Style;
#endif

#endif // DIALOG_PEEK_EXTENDED
EOF

echo "=== Fixed SystemTypes.h, now compiling ==="
make clean
make -j4 2>&1 | tee fix_duplicates.log

# Count results
echo "=== Results ==="
echo -n "Object files created: "
find build/obj -name "*.o" 2>/dev/null | wc -l
echo -n "Total source files: "
find src -name "*.c" | wc -l