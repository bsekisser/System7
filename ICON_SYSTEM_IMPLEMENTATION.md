# System 7 Icon System Implementation Guide

This document provides a complete implementation of a faithful System 7-style icon system that replaces the current hardcoded HD icon approach with a proper, extensible architecture.

## Overview

The new icon system provides:
- Unified icon handling for all desktop and folder items
- Priority-based icon resolution (custom → BNDL/FREF → defaults)
- Shared label renderer with proper text metrics
- QuickDraw-faithful compositing
- Hit testing for icon + label areas

## File Structure

```
src/Finder/Icon/
├── icon_system.c       # System default icons
├── icon_resources.c    # Resource loading
├── icon_resolver.c     # Icon resolution logic
├── icon_draw.c         # Drawing implementation
├── icon_label.c        # Label rendering
└── icon_cache.c        # Icon caching (optional)

include/Finder/Icon/
├── icon_types.h        # Core data structures
├── icon_resources.h    # Resource loader API
└── icon_label.h        # Label renderer API
```

## Implementation Files

### 1. System Default Icons (icon_system.c)

```c
/* System Default Icons
 * Provides built-in icons until resources are loaded
 */

#include "Finder/Icon/icon_types.h"
#include <stdint.h>

/* Import existing HD icon data */
extern const uint8_t g_HDIcon[128];
extern const uint8_t g_HDIconMask[128];

/* Generic folder icon (System 7 style) */
static const uint8_t gFolderMask32[128] = {
    0x7F, 0xFF, 0xF0, 0x00,  /* Top tab */
    0x80, 0x00, 0x08, 0x00,
    0xFF, 0xFF, 0xFC, 0x00,
    0x80, 0x00, 0x02, 0x00,
    /* Folder body - simplified */
    0x80, 0x00, 0x02, 0x00,
    0x80, 0x00, 0x02, 0x00,
    0x80, 0x00, 0x02, 0x00,
    0x80, 0x00, 0x02, 0x00,
    0x80, 0x00, 0x02, 0x00,
    0x7F, 0xFF, 0xFE, 0x00,
    /* Rest zeros */
};

static const uint8_t gFolderImg32[128] = {
    0x3F, 0xFF, 0xE0, 0x00,
    0x40, 0x00, 0x10, 0x00,
    0x7F, 0xFF, 0xF8, 0x00,
    0x40, 0x00, 0x04, 0x00,
    0x40, 0x00, 0x04, 0x00,
    0x40, 0x00, 0x04, 0x00,
    0x40, 0x00, 0x04, 0x00,
    0x40, 0x00, 0x04, 0x00,
    0x40, 0x00, 0x04, 0x00,
    0x3F, 0xFF, 0xFC, 0x00,
    /* Rest zeros */
};

/* Generic document icon */
static const uint8_t gDocMask32[128] = {
    0x1F, 0xF8, 0x00, 0x00,
    0x20, 0x0C, 0x00, 0x00,
    0x40, 0x0A, 0x00, 0x00,
    0x40, 0x09, 0x00, 0x00,
    0x7F, 0xFF, 0x00, 0x00,
    0x40, 0x01, 0x00, 0x00,
    0x40, 0x01, 0x00, 0x00,
    0x40, 0x01, 0x00, 0x00,
    0x40, 0x01, 0x00, 0x00,
    0x3F, 0xFE, 0x00, 0x00,
    /* Rest zeros */
};

static const uint8_t gDocImg32[128] = {
    0x0F, 0xF0, 0x00, 0x00,
    0x10, 0x08, 0x00, 0x00,
    0x20, 0x04, 0x00, 0x00,
    0x20, 0x06, 0x00, 0x00,
    0x3F, 0xFE, 0x00, 0x00,
    0x20, 0x02, 0x00, 0x00,
    0x20, 0x02, 0x00, 0x00,
    0x20, 0x02, 0x00, 0x00,
    0x20, 0x02, 0x00, 0x00,
    0x1F, 0xFC, 0x00, 0x00,
    /* Rest zeros */
};

/* Icon families */
static IconFamily gVolumeIF = {
    .large = {32, 32, kIconDepth1, g_HDIconMask, g_HDIcon, NULL},
    .hasSmall = false
};

static IconFamily gFolderIF = {
    .large = {32, 32, kIconDepth1, gFolderMask32, gFolderImg32, NULL},
    .hasSmall = false
};

static IconFamily gDocIF = {
    .large = {32, 32, kIconDepth1, gDocMask32, gDocImg32, NULL},
    .hasSmall = false
};

/* Public API */
const IconFamily* IconSys_DefaultFolder(void) { return &gFolderIF; }
const IconFamily* IconSys_DefaultVolume(void) { return &gVolumeIF; }
const IconFamily* IconSys_DefaultDoc(void)    { return &gDocIF; }
```

### 2. Icon Resource Loader (icon_resources.c)

```c
/* Icon Resource Loading
 * Stub implementation - expand with real resource loading later
 */

#include "Finder/Icon/icon_resources.h"
#include <string.h>

/* Stub: Load icon family by resource ID */
bool IconRes_LoadFamilyByID(int16_t rsrcID, IconFamily* out) {
    /* TODO: Implement ICN# and cicn loading from resources */
    return false;
}

/* Stub: Map type/creator to icon resource ID */
bool IconRes_MapTypeCreatorToIcon(uint32_t type, uint32_t creator, int16_t* outRsrcID) {
    /* Simple hardcoded mappings for now */
    if (type == 'APPL') {
        *outRsrcID = 128;  /* Generic app icon */
        return true;
    }
    if (type == 'TEXT' && creator == 'ttxt') {
        *outRsrcID = 129;  /* TeachText document */
        return true;
    }
    return false;
}

/* Stub: Load custom icon from path */
bool IconRes_LoadCustomIconForPath(const char* path, IconFamily* out) {
    /* TODO: Check for Icon\r file in folder */
    return false;
}
```

### 3. Icon Resolver (icon_resolver.c)

```c
/* Icon Resolution with Priority System
 * Faithful to System 7 icon lookup order
 */

#include "Finder/Icon/icon_types.h"
#include "Finder/Icon/icon_resources.h"

/* Import system defaults */
extern const IconFamily* IconSys_DefaultFolder(void);
extern const IconFamily* IconSys_DefaultVolume(void);
extern const IconFamily* IconSys_DefaultDoc(void);

/* Initialize icon system */
bool Icon_Init(void) {
    /* Load system icon resources, initialize cache, etc. */
    return true;
}

/* Resolve icon for file/folder */
bool Icon_ResolveForNode(const FileKind* fk, IconHandle* out) {
    static IconFamily tempFam;

    /* 1) Custom icon (folder/file flag + "Icon\r") */
    if (fk->hasCustomIcon && fk->path) {
        if (IconRes_LoadCustomIconForPath(fk->path, &tempFam)) {
            out->fam = &tempFam;  /* Should cache this */
            out->selected = false;
            return true;
        }
    }

    /* 2) Bundles: BNDL/FREF by creator/type */
    int16_t iconID = 0;
    if (IconRes_MapTypeCreatorToIcon(fk->type, fk->creator, &iconID)) {
        if (IconRes_LoadFamilyByID(iconID, &tempFam)) {
            out->fam = &tempFam;  /* Should cache this */
            out->selected = false;
            return true;
        }
    }

    /* 3) System defaults */
    if (fk->isVolume) {
        out->fam = IconSys_DefaultVolume();
        out->selected = false;
        return true;
    }

    if (fk->isFolder) {
        out->fam = IconSys_DefaultFolder();
        out->selected = false;
        return true;
    }

    /* Default document */
    out->fam = IconSys_DefaultDoc();
    out->selected = false;
    return true;
}
```

### 4. Icon Drawing (icon_draw.c)

```c
/* Icon Drawing with QuickDraw-faithful Compositing
 * Handles ICN# (1-bit) and cicn (color) icons
 */

#include "Finder/Icon/icon_types.h"
#include <stdint.h>

/* Import framebuffer access */
extern uint32_t* g_vram;
extern int g_screenWidth;

/* Helper: Get bit from bitmap */
static inline uint8_t GetBit(const uint8_t* row, int x) {
    return (row[x >> 3] >> (7 - (x & 7))) & 1;
}

/* Draw pixel to framebuffer */
static void SetPixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < 800 && y >= 0 && y < 600) {
        g_vram[y * g_screenWidth + x] = color;
    }
}

/* Draw 1-bit ICN# icon */
static void DrawICN32(const IconBitmap* ib, int dx, int dy) {
    uint32_t black = 0xFF000000;
    uint32_t white = 0xFFFFFFFF;

    /* First pass: Draw white fill where mask is set */
    for (int y = 0; y < 32; ++y) {
        if (dy + y >= 600) break;
        const uint8_t* mrow = ib->mask1b + y * 4;  /* 32px = 4 bytes */

        for (int x = 0; x < 32; ++x) {
            if (dx + x >= 800) break;
            if (GetBit(mrow, x)) {
                SetPixel(dx + x, dy + y, white);
            }
        }
    }

    /* Second pass: Draw black pixels where image is set */
    for (int y = 0; y < 32; ++y) {
        if (dy + y >= 600) break;
        const uint8_t* mrow = ib->mask1b + y * 4;
        const uint8_t* irow = ib->img1b + y * 4;

        for (int x = 0; x < 32; ++x) {
            if (dx + x >= 800) break;
            if (GetBit(mrow, x) && GetBit(irow, x)) {
                SetPixel(dx + x, dy + y, black);
            }
        }
    }
}

/* Draw color cicn icon */
static void DrawCICN32(const IconBitmap* ib, int dx, int dy) {
    for (int y = 0; y < 32; ++y) {
        if (dy + y >= 600) break;
        const uint32_t* src = ib->argb32 + y * 32;

        for (int x = 0; x < 32; ++x) {
            if (dx + x >= 800) break;
            uint32_t pixel = src[x];
            uint8_t alpha = (pixel >> 24) & 0xFF;

            if (alpha > 0) {
                /* Simple alpha blend if needed */
                SetPixel(dx + x, dy + y, pixel);
            }
        }
    }
}

/* Public API: Draw 32x32 icon */
void Icon_Draw32(const IconHandle* h, int x, int y) {
    if (!h || !h->fam) return;

    const IconBitmap* b = &h->fam->large;

    if (b->depth == kIconColor32 && b->argb32) {
        DrawCICN32(b, x, y);
    } else if (b->img1b && b->mask1b) {
        DrawICN32(b, x, y);
    }
}

/* Draw 16x16 icon (for list views) */
void Icon_Draw16(const IconHandle* h, int x, int y) {
    /* TODO: Implement SICN drawing */
    if (!h || !h->fam || !h->fam->hasSmall) return;
    /* Draw h->fam->small */
}
```

### 5. Label Renderer (icon_label.c)

```c
/* Shared Label Renderer
 * Reuses perfected text rendering from HD icon
 */

#include "Finder/Icon/icon_label.h"
#include "Finder/Icon/icon_types.h"
#include <string.h>

/* Import Chicago font */
extern const uint8_t chicago_bitmaps[][13];
extern const uint8_t chicago_widths[];
extern const uint8_t chicago_bit_widths[];

/* Import framebuffer */
extern uint32_t* g_vram;
extern int g_screenWidth;

/* Draw rectangle */
static void FillRect(int left, int top, int right, int bottom, uint32_t color) {
    for (int y = top; y < bottom && y < 600; y++) {
        for (int x = left; x < right && x < 800; x++) {
            if (x >= 0 && y >= 0) {
                g_vram[y * g_screenWidth + x] = color;
            }
        }
    }
}

/* Draw character */
static void DrawChar(char ch, int x, int y, uint32_t color) {
    if (ch < 32 || ch > 126) return;

    const uint8_t* bitmap = chicago_bitmaps[ch - 32];
    int bit_width = chicago_bit_widths[ch - 32];

    for (int row = 0; row < 13; row++) {
        uint16_t bits = (bitmap[row] << 8);
        for (int col = 0; col < bit_width; col++) {
            if (bits & (0x8000 >> col)) {
                if (x + col < 800 && y + row < 600) {
                    g_vram[(y + row) * g_screenWidth + (x + col)] = color;
                }
            }
        }
    }
}

/* Measure text */
void IconLabel_Measure(const char* name, int* outWidth, int* outHeight) {
    int width = 0;
    int len = strlen(name);

    for (int i = 0; i < len; i++) {
        char ch = name[i];
        if (ch >= 32 && ch <= 126) {
            width += chicago_bit_widths[ch - 32] + 1;
            if (ch == ' ') width += 3;  /* Extra space width */
        }
    }

    *outWidth = width;
    *outHeight = 13;  /* Chicago font height */
}

/* Draw label */
void IconLabel_Draw(const char* name, int cx, int topY, bool selected) {
    int textWidth, textHeight;
    IconLabel_Measure(name, &textWidth, &textHeight);

    int textX = cx - (textWidth / 2);

    /* Draw white background */
    uint32_t bgColor = selected ? 0xFF000080 : 0xFFFFFFFF;
    uint32_t fgColor = selected ? 0xFFFFFFFF : 0xFF000000;

    FillRect(textX - 2, topY - 1, textX + textWidth, topY + textHeight + 1, bgColor);

    /* Draw text */
    int currentX = textX;
    int len = strlen(name);

    for (int i = 0; i < len; i++) {
        char ch = name[i];
        if (ch >= 32 && ch <= 126) {
            DrawChar(ch, currentX, topY, fgColor);
            currentX += chicago_bit_widths[ch - 32] + 1;
            if (ch == ' ') currentX += 3;
        }
    }
}

/* Draw icon with label */
IconRect Icon_DrawWithLabel(const IconHandle* h, const char* name,
                            int centerX, int iconTopY, bool selected) {
    /* Draw icon centered at centerX */
    int iconLeft = centerX - 16;  /* 32x32 icon */
    Icon_Draw32(h, iconLeft, iconTopY);

    /* Draw label below icon */
    int labelTop = iconTopY + 32 + 6;
    IconLabel_Draw(name, centerX, labelTop, selected);

    /* Return combined bounds for hit testing */
    int textWidth, textHeight;
    IconLabel_Measure(name, &textWidth, &textHeight);

    IconRect bounds;
    bounds.left = iconLeft;
    bounds.top = iconTopY;
    bounds.right = iconLeft + 32;
    bounds.bottom = labelTop + textHeight + 2;

    /* Expand to include label width */
    int labelLeft = centerX - (textWidth / 2) - 2;
    int labelRight = centerX + (textWidth / 2) + 2;
    if (labelLeft < bounds.left) bounds.left = labelLeft;
    if (labelRight > bounds.right) bounds.right = labelRight;

    return bounds;
}
```

### 6. Hit Testing

```c
/* Hit testing implementation */
int Icon_HitTest(const IconSlot* slots, int count, int x, int y) {
    for (int i = count - 1; i >= 0; --i) {
        const IconSlot* s = &slots[i];

        /* Check icon bounds */
        if (x >= s->iconR.left && x < s->iconR.right &&
            y >= s->iconR.top && y < s->iconR.bottom) {
            return s->objectId;
        }

        /* Check label bounds */
        if (x >= s->labelR.left && x < s->labelR.right &&
            y >= s->labelR.top && y < s->labelR.bottom) {
            return s->objectId;
        }
    }
    return -1;
}
```

## Integration Steps

### 1. Update Makefile

Add the new icon system files to your Makefile:

```makefile
C_SOURCES += src/Finder/Icon/icon_system.c \
             src/Finder/Icon/icon_resources.c \
             src/Finder/Icon/icon_resolver.c \
             src/Finder/Icon/icon_draw.c \
             src/Finder/Icon/icon_label.c

# Add pattern rule for Icon subdirectory
$(OBJ_DIR)/%.o: src/Finder/Icon/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@
```

### 2. Update Desktop Manager

Replace the hardcoded HD icon drawing in `desktop_manager.c`:

```c
#include "Finder/Icon/icon_types.h"
#include "Finder/Icon/icon_label.h"

void DrawDesktop(void) {
    /* Initialize icon for Macintosh HD */
    FileKind hdKind = {
        .type = '    ',
        .creator = '    ',
        .isVolume = true,
        .isFolder = false,
        .hasCustomIcon = false,
        .path = NULL
    };

    IconHandle hdIcon;
    Icon_ResolveForNode(&hdKind, &hdIcon);

    /* Draw with shared label renderer */
    Icon_DrawWithLabel(&hdIcon, "Macintosh HD", 72, 50, false);
}
```

### 3. Remove Old Code

Delete from `desktop_manager.c`:
- The hardcoded `DrawVolumeIcon` function
- Direct Chicago font rendering for HD label
- Special case HD icon handling

## Benefits

1. **Unified System**: All icons use the same drawing path
2. **Extensible**: Easy to add new icon types
3. **Faithful**: Matches System 7 icon resolution priority
4. **Maintainable**: Clean separation of concerns
5. **Reusable**: Label renderer works for all icons

## Future Enhancements

- Implement real ICN#/cicn resource loading
- Add BNDL/FREF parser for app icons
- Support custom icons via Icon\r files
- Add icon caching with LRU eviction
- Implement SICN for list views
- Add drag & drop with icon ghosting
- Support alias badge overlays

This architecture provides a solid foundation that can grow with your System 7 implementation while maintaining fidelity to the original design.