/*
 * MemoryLogging.h - Logging macros for Memory Manager subsystem
 *
 * Provides module-specific logging macros that route through the
 * hierarchical logging system with [MEM] tag prefix.
 */

#ifndef MEMORY_LOGGING_H
#define MEMORY_LOGGING_H

#include "System71StdLib.h"

#define MEMORY_LOG_TRACE(fmt, ...) serial_logf(kLogModuleMemory, kLogLevelTrace, "[MEM] " fmt, ##__VA_ARGS__)
#define MEMORY_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleMemory, kLogLevelDebug, "[MEM] " fmt, ##__VA_ARGS__)
#define MEMORY_LOG_INFO(fmt, ...)  serial_logf(kLogModuleMemory, kLogLevelInfo,  "[MEM] " fmt, ##__VA_ARGS__)
#define MEMORY_LOG_WARN(fmt, ...)  serial_logf(kLogModuleMemory, kLogLevelWarn,  "[MEM] " fmt, ##__VA_ARGS__)
#define MEMORY_LOG_ERROR(fmt, ...) serial_logf(kLogModuleMemory, kLogLevelError, "[MEM] " fmt, ##__VA_ARGS__)

#endif /* MEMORY_LOGGING_H */
