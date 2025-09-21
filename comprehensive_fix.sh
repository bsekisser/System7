#!/bin/bash

echo "Applying comprehensive fixes to all source files..."

# Add missing includes to source files that need them
for file in src/QuickDraw/*.c src/WindowManager/*.c src/EventManager/*.c src/MenuManager/*.c src/DialogManager/*.c src/ControlManager/*.c; do
    if [ -f "$file" ]; then
        # Add QuickDrawConstants.h if not already included and file uses QuickDraw
        if ! grep -q "QuickDrawConstants.h" "$file" && grep -qE "(srcCopy|patCopy|frame|paint|erase|invert|fill)" "$file"; then
            sed -i '/#include "SystemTypes.h"/a #include "QuickDrawConstants.h"' "$file" 2>/dev/null
        fi

        # Add thePort global if needed
        if grep -q "thePort" "$file" && ! grep -q "g_currentPort" "$file"; then
            sed -i '/#include.*SystemTypes/a\
\
/* Global current port - from QuickDrawCore.c */\
extern GrafPtr g_currentPort;\
#define thePort g_currentPort' "$file" 2>/dev/null
        fi
    fi
done

# Fix common member access patterns
find src -name "*.c" -type f -exec sed -i '
s/(g_currentPort)->h\b/g_currentPort->pnLoc.h/g
s/(g_currentPort)->v\b/g_currentPort->pnLoc.v/g
s/port)->h\b/port->pnLoc.h/g
s/port)->v\b/port->pnLoc.v/g
s/(bitmap)->left\b/bitmap->bounds.left/g
s/(bitmap)->right\b/bitmap->bounds.right/g
s/(bitmap)->top\b/bitmap->bounds.top/g
s/(bitmap)->bottom\b/bitmap->bounds.bottom/g
' {} \; 2>/dev/null

# Create missing header files
if [ ! -f "include/WindowManager/WindowManagerInternal.h" ]; then
    cat > include/WindowManager/WindowManagerInternal.h << 'EOF'
#ifndef WINDOW_MANAGER_INTERNAL_H
#define WINDOW_MANAGER_INTERNAL_H

#include "SystemTypes.h"
#include "WindowManager/WindowTypes.h"

/* Internal window manager structures and functions */

#endif /* WINDOW_MANAGER_INTERNAL_H */
EOF
fi

if [ ! -f "include/QuickDraw/quickdraw_portable.h" ]; then
    cat > include/QuickDraw/quickdraw_portable.h << 'EOF'
#ifndef QUICKDRAW_PORTABLE_H
#define QUICKDRAW_PORTABLE_H

#include "SystemTypes.h"

/* Portable QuickDraw functions */

#endif /* QUICKDRAW_PORTABLE_H */
EOF
fi

echo "Fixes applied. Recompiling..."