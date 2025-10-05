/*
 * ResourceLogging.h - Logging macros for Resource Manager subsystem
 *
 * Provides module-specific logging macros that route through the
 * hierarchical logging system with [RES] tag prefix.
 */

#ifndef RESOURCE_LOGGING_H
#define RESOURCE_LOGGING_H

#include "System71StdLib.h"

#define RESOURCE_LOG_TRACE(fmt, ...) serial_logf(kLogModuleResource, kLogLevelTrace, "[RES] " fmt, ##__VA_ARGS__)
#define RESOURCE_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleResource, kLogLevelDebug, "[RES] " fmt, ##__VA_ARGS__)
#define RESOURCE_LOG_INFO(fmt, ...)  serial_logf(kLogModuleResource, kLogLevelInfo,  "[RES] " fmt, ##__VA_ARGS__)
#define RESOURCE_LOG_WARN(fmt, ...)  serial_logf(kLogModuleResource, kLogLevelWarn,  "[RES] " fmt, ##__VA_ARGS__)
#define RESOURCE_LOG_ERROR(fmt, ...) serial_logf(kLogModuleResource, kLogLevelError, "[RES] " fmt, ##__VA_ARGS__)

#endif /* RESOURCE_LOGGING_H */
