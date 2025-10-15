/* Icon Resolution with Priority System
 * Faithful to System 7 icon lookup order
 */

#include "Finder/Icon/icon_types.h"
#include "Finder/Icon/icon_resources.h"
#include "Finder/Icon/icon_system.h"
#include <stddef.h>

/* Initialize icon system */
bool Icon_Init(void) {
    /* TODO: Load system icon resources, initialize cache, etc. */
    return true;
}

/* Resolve icon for file/folder */
bool Icon_ResolveForNode(const FileKind* fk, IconHandle* out) {
    static IconFamily tempFam;

    if (!fk || !out) return false;

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
    if (fk->isTrash) {
        out->fam = fk->isTrashFull ? IconSys_TrashFull() : IconSys_TrashEmpty();
        out->selected = false;
        return true;
    }

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
