#ifndef PLATFORM_PLATFORMLOGGING_H
#define PLATFORM_PLATFORMLOGGING_H

#include "System71StdLib.h"

#define PLATFORM_LOG_TRACE(fmt, ...) serial_logf(kLogModulePlatform, kLogLevelTrace, "[PLATFORM] " fmt, ##__VA_ARGS__)
#define PLATFORM_LOG_DEBUG(fmt, ...) serial_logf(kLogModulePlatform, kLogLevelDebug, "[PLATFORM] " fmt, ##__VA_ARGS__)
#define PLATFORM_LOG_INFO(fmt, ...)  serial_logf(kLogModulePlatform, kLogLevelInfo,  "[PLATFORM] " fmt, ##__VA_ARGS__)
#define PLATFORM_LOG_WARN(fmt, ...)  serial_logf(kLogModulePlatform, kLogLevelWarn,  "[PLATFORM] " fmt, ##__VA_ARGS__)
#define PLATFORM_LOG_ERROR(fmt, ...) serial_logf(kLogModulePlatform, kLogLevelError, "[PLATFORM] " fmt, ##__VA_ARGS__)

#endif /* PLATFORM_PLATFORMLOGGING_H */
