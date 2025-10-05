/*
 * FontLogging.h - Logging macros for Font Manager subsystem
 *
 * Provides module-specific logging macros that route through the
 * hierarchical logging system with [FONT] tag prefix.
 */

#ifndef FONT_LOGGING_H
#define FONT_LOGGING_H

#include "System71StdLib.h"

#define FONT_LOG_TRACE(fmt, ...) serial_logf(kLogModuleFont, kLogLevelTrace, "[FONT] " fmt, ##__VA_ARGS__)
#define FONT_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleFont, kLogLevelDebug, "[FONT] " fmt, ##__VA_ARGS__)
#define FONT_LOG_INFO(fmt, ...)  serial_logf(kLogModuleFont, kLogLevelInfo,  "[FONT] " fmt, ##__VA_ARGS__)
#define FONT_LOG_WARN(fmt, ...)  serial_logf(kLogModuleFont, kLogLevelWarn,  "[FONT] " fmt, ##__VA_ARGS__)
#define FONT_LOG_ERROR(fmt, ...) serial_logf(kLogModuleFont, kLogLevelError, "[FONT] " fmt, ##__VA_ARGS__)

#endif /* FONT_LOGGING_H */
