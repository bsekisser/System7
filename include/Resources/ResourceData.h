/*
 * ResourceData.h - System 7 Resource Data Management
 *
 * Provides access to embedded System 7 resources including icons, cursors,
 * patterns, and sounds extracted from original System 7 resource files.
 *
 * Copyright (c) 2024 System7.1-Portable Project
 * MIT License
 */

#ifndef RESOURCE_DATA_H
#define RESOURCE_DATA_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"
#include "QuickDraw/QuickDraw.h"

/* Include the extracted System 7 resources */
#include "system7_resources.h"

/* Resource types */

/* Resource IDs for standard System 7 resources */

/* Resource data structure */

/* Public API */

/* Initialize resource data system */
OSErr InitResourceData(void);

/* Check if resource data system has been initialized */
Boolean GetResourceDataInitialized(void);

/* Get resource data by ID */
const ResourceData* GetResourceData(ResourceDataType type, UInt16 id);

/* Load cursor from resource data */
CursHandle LoadResourceCursor(UInt16 cursorID);

/* Load icon from resource data */
Handle LoadResourceIcon(UInt16 iconID, Boolean is16x16);

/* Load pattern from resource data */
Pattern* LoadResourcePattern(UInt16 patternID);

/* Load sound from resource data */
Handle LoadResourceSound(UInt16 soundID);

/* Get cursor hotspot */
Point GetCursorHotspot(UInt16 cursorID);

/* Create bitmap from resource data */
BitMap* CreateBitmapFromResource(const ResourceData* resData);

/* Draw icon at location */
void DrawResourceIcon(UInt16 iconID, short x, short y);

/* Play sound resource */
OSErr PlayResourceSound(UInt16 soundID);

/* Pattern utilities */
void SetResourcePattern(UInt16 patternID);
void FillWithResourcePattern(const Rect* rect, UInt16 patternID);

/* Menu icon utilities */
void DrawMenuIcon(UInt16 iconID, const Rect* destRect);

/* UI element drawing */
void DrawCheckbox(short x, short y, Boolean checked);
void DrawRadioButton(short x, short y, Boolean selected);
void DrawWindowControl(short x, short y, UInt16 controlID);

/* Resource table access */
UInt16 CountResourcesOfType(ResourceDataType type);
const ResourceData* GetIndexedResource(ResourceDataType type, UInt16 index);

/* Debug utilities */
void DumpResourceInfo(ResourceDataType type, UInt16 id);
void ListAllResources(void);

#endif /* RESOURCE_DATA_H */
