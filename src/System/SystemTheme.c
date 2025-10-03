/*
 * System Theme Implementation
 * Manages system-wide theme preferences including highlight color
 */

#include "SystemTheme.h"

/* Global system theme - default to teal highlight */
static SystemTheme g_systemTheme = {
    .highlightColor = kHighlightTeal
};

/*
 * GetSystemTheme - Returns pointer to global system theme
 */
SystemTheme* GetSystemTheme(void) {
    return &g_systemTheme;
}
