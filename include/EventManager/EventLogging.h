#ifndef EVENTMANAGER_EVENTLOGGING_H
#define EVENTMANAGER_EVENTLOGGING_H

#include "System71StdLib.h"

#define EVT_LOG_TRACE(fmt, ...) serial_logf(kLogModuleEvent, kLogLevelTrace, "[EVT] " fmt, ##__VA_ARGS__)
#define EVT_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleEvent, kLogLevelDebug, "[EVT] " fmt, ##__VA_ARGS__)
#define EVT_LOG_INFO(fmt, ...)  serial_logf(kLogModuleEvent, kLogLevelInfo,  "[EVT] " fmt, ##__VA_ARGS__)
#define EVT_LOG_WARN(fmt, ...)  serial_logf(kLogModuleEvent, kLogLevelWarn,  "[EVT] " fmt, ##__VA_ARGS__)
#define EVT_LOG_ERROR(fmt, ...) serial_logf(kLogModuleEvent, kLogLevelError, "[EVT] " fmt, ##__VA_ARGS__)

#endif /* EVENTMANAGER_EVENTLOGGING_H */
