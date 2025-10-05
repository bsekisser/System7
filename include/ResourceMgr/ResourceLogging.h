/*
 * ResourceLogging.h - Logging macros for Resource Manager subsystem
 *
 * Provides module-specific logging macros that route through the
 * hierarchical logging system with [RM] tag prefix.
 */

#ifndef RESOURCE_LOGGING_H
#define RESOURCE_LOGGING_H

#include "System71StdLib.h"

#define RM_LOG_TRACE(fmt, ...) serial_logf(kLogModuleResource, kLogLevelTrace, "[RM] " fmt, ##__VA_ARGS__)
#define RM_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleResource, kLogLevelDebug, "[RM] " fmt, ##__VA_ARGS__)
#define RM_LOG_INFO(fmt, ...)  serial_logf(kLogModuleResource, kLogLevelInfo,  "[RM] " fmt, ##__VA_ARGS__)
#define RM_LOG_WARN(fmt, ...)  serial_logf(kLogModuleResource, kLogLevelWarn,  "[RM] " fmt, ##__VA_ARGS__)
#define RM_LOG_ERROR(fmt, ...) serial_logf(kLogModuleResource, kLogLevelError, "[RM] " fmt, ##__VA_ARGS__)

#endif /* RESOURCE_LOGGING_H */
