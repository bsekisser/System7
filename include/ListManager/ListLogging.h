#ifndef LISTMANAGER_LISTLOGGING_H
#define LISTMANAGER_LISTLOGGING_H

#include "System71StdLib.h"

#define LIST_LOG_TRACE(fmt, ...) serial_logf(kLogModuleListManager, kLogLevelTrace, "[LIST] " fmt, ##__VA_ARGS__)
#define LIST_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleListManager, kLogLevelDebug, "[LIST] " fmt, ##__VA_ARGS__)
#define LIST_LOG_INFO(fmt, ...)  serial_logf(kLogModuleListManager, kLogLevelInfo,  "[LIST] " fmt, ##__VA_ARGS__)
#define LIST_LOG_WARN(fmt, ...)  serial_logf(kLogModuleListManager, kLogLevelWarn,  "[LIST] " fmt, ##__VA_ARGS__)
#define LIST_LOG_ERROR(fmt, ...) serial_logf(kLogModuleListManager, kLogLevelError, "[LIST] " fmt, ##__VA_ARGS__)

#endif /* LISTMANAGER_LISTLOGGING_H */
