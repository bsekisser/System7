/*
 * System Theme - Highlight Color Configuration
 * Provides Mac OS System 7 style configurable highlight colors
 */

#ifndef SYSTEM_THEME_H
#define SYSTEM_THEME_H

#include "SystemTypes.h"

/* System Theme structure */
typedef struct {
    RGBColor highlightColor;  /* Active window title bar highlight */
} SystemTheme;

/* Get global system theme */
SystemTheme* GetSystemTheme(void);

/* Predefined System 7 highlight colors (16-bit per component, Mac OS standard) */
#define kHighlightBlue    {0x0000, 0x0000, 0xFFFF}  /* Default System 7 blue */
#define kHighlightPurple  {0x8000, 0x0000, 0x8000}  /* Purple */
#define kHighlightRed     {0xFFFF, 0x0000, 0x0000}  /* Red */
#define kHighlightGreen   {0x0000, 0x8000, 0x0000}  /* Green */
#define kHighlightBrown   {0x8000, 0x4000, 0x0000}  /* Brown */
#define kHighlightTeal    {0x0000, 0xB000, 0xB000}  /* Teal (brighter) */

#endif /* SYSTEM_THEME_H */
