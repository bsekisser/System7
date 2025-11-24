#ifndef WINDOWMANAGER_WMLOGGING_H
#define WINDOWMANAGER_WMLOGGING_H

#include "System71StdLib.h"

/* ============================================================================
 * Window Manager Debug Logging Configuration
 * ============================================================================
 *
 * Set WM_DEBUG_LEVEL to control logging verbosity:
 *   0 = No logging (production)
 *   1 = Errors only (WM_LOG_ERROR)
 *   2 = Warnings + Errors (WM_LOG_WARN)
 *   3 = Info + Warnings + Errors (WM_LOG_INFO)
 *   4 = Debug + Info + Warnings + Errors (WM_LOG_DEBUG, WM_DEBUG)
 *   5 = Trace + all above (WM_LOG_TRACE)
 */

#ifndef WM_DEBUG_LEVEL
  #ifdef DEBUG
    #define WM_DEBUG_LEVEL 3  /* Info level for debug builds */
  #else
    #define WM_DEBUG_LEVEL 1  /* Errors only for release builds */
  #endif
#endif

/* Logging macros - conditionally compiled based on WM_DEBUG_LEVEL */

#if WM_DEBUG_LEVEL >= 5
  #define WM_LOG_TRACE(fmt, ...) serial_logf(kLogModuleWindow, kLogLevelTrace, "[WM] " fmt, ##__VA_ARGS__)
#else
  #define WM_LOG_TRACE(fmt, ...) do {} while(0)
#endif

#if WM_DEBUG_LEVEL >= 4
  #define WM_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleWindow, kLogLevelDebug, "[WM] " fmt, ##__VA_ARGS__)
  #define WM_DEBUG(fmt, ...) serial_logf(kLogModuleWindow, kLogLevelDebug, "[WM] " fmt, ##__VA_ARGS__)
#else
  #define WM_LOG_DEBUG(fmt, ...) do {} while(0)
  #define WM_DEBUG(fmt, ...) do {} while(0)
#endif

#if WM_DEBUG_LEVEL >= 3
  #define WM_LOG_INFO(fmt, ...)  serial_logf(kLogModuleWindow, kLogLevelInfo,  "[WM] " fmt, ##__VA_ARGS__)
#else
  #define WM_LOG_INFO(fmt, ...)  do {} while(0)
#endif

#if WM_DEBUG_LEVEL >= 2
  #define WM_LOG_WARN(fmt, ...)  serial_logf(kLogModuleWindow, kLogLevelWarn,  "[WM] " fmt, ##__VA_ARGS__)
#else
  #define WM_LOG_WARN(fmt, ...)  do {} while(0)
#endif

#if WM_DEBUG_LEVEL >= 1
  #define WM_LOG_ERROR(fmt, ...) serial_logf(kLogModuleWindow, kLogLevelError, "[WM] " fmt, ##__VA_ARGS__)
#else
  #define WM_LOG_ERROR(fmt, ...) do {} while(0)
#endif

#endif /* WINDOWMANAGER_WMLOGGING_H */
