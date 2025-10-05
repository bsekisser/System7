/*
 * ProcessLogging.h - Logging macros for Process Manager subsystem
 *
 * Provides module-specific logging macros that route through the
 * hierarchical logging system with [PROC] tag prefix.
 */

#ifndef PROCESS_LOGGING_H
#define PROCESS_LOGGING_H

#include "System71StdLib.h"

#define PROCESS_LOG_TRACE(fmt, ...) serial_logf(kLogModuleProcess, kLogLevelTrace, "[PROC] " fmt, ##__VA_ARGS__)
#define PROCESS_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleProcess, kLogLevelDebug, "[PROC] " fmt, ##__VA_ARGS__)
#define PROCESS_LOG_INFO(fmt, ...)  serial_logf(kLogModuleProcess, kLogLevelInfo,  "[PROC] " fmt, ##__VA_ARGS__)
#define PROCESS_LOG_WARN(fmt, ...)  serial_logf(kLogModuleProcess, kLogLevelWarn,  "[PROC] " fmt, ##__VA_ARGS__)
#define PROCESS_LOG_ERROR(fmt, ...) serial_logf(kLogModuleProcess, kLogLevelError, "[PROC] " fmt, ##__VA_ARGS__)

#endif /* PROCESS_LOGGING_H */
