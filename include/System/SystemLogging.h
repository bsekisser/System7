/*
 * SystemLogging.h - Logging macros for System/General subsystem
 *
 * Provides module-specific logging macros that route through the
 * hierarchical logging system with [SYS] tag prefix.
 */

#ifndef SYSTEM_LOGGING_H
#define SYSTEM_LOGGING_H

#include "System71StdLib.h"

#define SYSTEM_LOG_TRACE(fmt, ...) serial_logf(kLogModuleSystem, kLogLevelTrace, "[SYS] " fmt, ##__VA_ARGS__)
#define SYSTEM_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleSystem, kLogLevelDebug, "[SYS] " fmt, ##__VA_ARGS__)
#define SYSTEM_LOG_INFO(fmt, ...)  serial_logf(kLogModuleSystem, kLogLevelInfo,  "[SYS] " fmt, ##__VA_ARGS__)
#define SYSTEM_LOG_WARN(fmt, ...)  serial_logf(kLogModuleSystem, kLogLevelWarn,  "[SYS] " fmt, ##__VA_ARGS__)
#define SYSTEM_LOG_ERROR(fmt, ...) serial_logf(kLogModuleSystem, kLogLevelError, "[SYS] " fmt, ##__VA_ARGS__)

#endif /* SYSTEM_LOGGING_H */
