/**
 * @file DesktopPatterns.h
 * @brief Public interface for the Desktop Patterns control panel.
 */

#ifndef DESKTOP_PATTERNS_H
#define DESKTOP_PATTERNS_H

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void OpenDesktopCdev(void);
void CloseDesktopCdev(void);
void HandleDesktopCdevEvent(EventRecord *event);

#ifdef __cplusplus
}
#endif

#endif /* DESKTOP_PATTERNS_H */
