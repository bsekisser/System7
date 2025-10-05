#ifndef WINDOWMANAGER_WMLOGGING_H
#define WINDOWMANAGER_WMLOGGING_H

#include "System71StdLib.h"

#define WM_LOG_TRACE(fmt, ...) serial_logf(kLogModuleWindow, kLogLevelTrace, "[WM] " fmt, ##__VA_ARGS__)
#define WM_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleWindow, kLogLevelDebug, "[WM] " fmt, ##__VA_ARGS__)
#define WM_LOG_INFO(fmt, ...)  serial_logf(kLogModuleWindow, kLogLevelInfo,  "[WM] " fmt, ##__VA_ARGS__)
#define WM_LOG_WARN(fmt, ...)  serial_logf(kLogModuleWindow, kLogLevelWarn,  "[WM] " fmt, ##__VA_ARGS__)
#define WM_LOG_ERROR(fmt, ...) serial_logf(kLogModuleWindow, kLogLevelError, "[WM] " fmt, ##__VA_ARGS__)

#endif /* WINDOWMANAGER_WMLOGGING_H */
