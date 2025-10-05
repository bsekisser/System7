/*
 * DialogLogging.h - Logging macros for Dialog Manager subsystem
 *
 * Provides module-specific logging macros that route through the
 * hierarchical logging system with [DM] tag prefix.
 */

#ifndef DIALOG_LOGGING_H
#define DIALOG_LOGGING_H

#include "System71StdLib.h"

#define DIALOG_LOG_TRACE(fmt, ...) serial_logf(kLogModuleDialog, kLogLevelTrace, "[DM] " fmt, ##__VA_ARGS__)
#define DIALOG_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleDialog, kLogLevelDebug, "[DM] " fmt, ##__VA_ARGS__)
#define DIALOG_LOG_INFO(fmt, ...)  serial_logf(kLogModuleDialog, kLogLevelInfo,  "[DM] " fmt, ##__VA_ARGS__)
#define DIALOG_LOG_WARN(fmt, ...)  serial_logf(kLogModuleDialog, kLogLevelWarn,  "[DM] " fmt, ##__VA_ARGS__)
#define DIALOG_LOG_ERROR(fmt, ...) serial_logf(kLogModuleDialog, kLogLevelError, "[DM] " fmt, ##__VA_ARGS__)

#endif /* DIALOG_LOGGING_H */
