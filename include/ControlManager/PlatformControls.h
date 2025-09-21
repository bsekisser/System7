/**
 * @file PlatformControls.h
 * @brief Platform abstraction for native controls
 *
 * Copyright (c) 2024 - System 7.1 Portable Toolbox Project
 * Licensed under MIT License
 */

#ifndef PLATFORMCONTROLS_H
#define PLATFORMCONTROLS_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "ControlManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Platform types */

/* Platform control types */

/* Platform features */

/* Platform callback procedures */

/* Platform control management */
OSErr InitializePlatformControls(void);
OSErr CreatePlatformControl(ControlHandle control, PlatformControlType type);
void DestroyPlatformControl(ControlHandle control);

/* Platform settings */
void SetNativeControlsEnabled(Boolean enabled);
Boolean GetNativeControlsEnabled(void);
void SetHighDPIEnabled(Boolean enabled);
Boolean GetHighDPIEnabled(void);
void SetAccessibilityEnabled(Boolean enabled);
Boolean GetAccessibilityEnabled(void);

/* Platform information */
PlatformType GetCurrentPlatform(void);
Boolean ControlSupportsFeature(ControlHandle control, PlatformFeature feature);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORMCONTROLS_H */