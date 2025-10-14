/* CPU/CPULogging.h - Logging macros for CPU/M68K subsystem */
#ifndef CPU_LOGGING_H
#define CPU_LOGGING_H

#include "System71StdLib.h"

#define M68K_LOG_TRACE(fmt, ...) serial_logf(kLogModuleCPU, kLogLevelTrace, "[M68K] " fmt, ##__VA_ARGS__)
#define M68K_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleCPU, kLogLevelDebug, "[M68K] " fmt, ##__VA_ARGS__)
#define M68K_LOG_INFO(fmt, ...)  serial_logf(kLogModuleCPU, kLogLevelInfo,  "[M68K] " fmt, ##__VA_ARGS__)
#define M68K_LOG_WARN(fmt, ...)  serial_logf(kLogModuleCPU, kLogLevelWarn,  "[M68K] " fmt, ##__VA_ARGS__)
#define M68K_LOG_ERROR(fmt, ...) serial_logf(kLogModuleCPU, kLogLevelError, "[M68K] " fmt, ##__VA_ARGS__)

#endif /* CPU_LOGGING_H */

