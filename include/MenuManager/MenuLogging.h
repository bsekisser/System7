#ifndef MENUMANAGER_MENULOGGING_H
#define MENUMANAGER_MENULOGGING_H

#include "System71StdLib.h"

#define MENU_LOG_TRACE(fmt, ...) serial_logf(kLogModuleMenu, kLogLevelTrace, "[MENU] " fmt, ##__VA_ARGS__)
#define MENU_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleMenu, kLogLevelDebug, "[MENU] " fmt, ##__VA_ARGS__)
#define MENU_LOG_INFO(fmt, ...)  serial_logf(kLogModuleMenu, kLogLevelInfo,  "[MENU] " fmt, ##__VA_ARGS__)
#define MENU_LOG_WARN(fmt, ...)  serial_logf(kLogModuleMenu, kLogLevelWarn,  "[MENU] " fmt, ##__VA_ARGS__)
#define MENU_LOG_ERROR(fmt, ...) serial_logf(kLogModuleMenu, kLogLevelError, "[MENU] " fmt, ##__VA_ARGS__)

#endif /* MENUMANAGER_MENULOGGING_H */
