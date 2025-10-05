#ifndef QUICKDRAW_QDLOGGING_H
#define QUICKDRAW_QDLOGGING_H

#include "System71StdLib.h"

#define QD_LOG_TRACE(fmt, ...) serial_logf(kLogModuleSystem, kLogLevelTrace, "[QD] " fmt, ##__VA_ARGS__)
#define QD_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleSystem, kLogLevelDebug, "[QD] " fmt, ##__VA_ARGS__)
#define QD_LOG_INFO(fmt, ...)  serial_logf(kLogModuleSystem, kLogLevelInfo,  "[QD] " fmt, ##__VA_ARGS__)
#define QD_LOG_WARN(fmt, ...)  serial_logf(kLogModuleSystem, kLogLevelWarn,  "[QD] " fmt, ##__VA_ARGS__)
#define QD_LOG_ERROR(fmt, ...) serial_logf(kLogModuleSystem, kLogLevelError, "[QD] " fmt, ##__VA_ARGS__)

#endif /* QUICKDRAW_QDLOGGING_H */
