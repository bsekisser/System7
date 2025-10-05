#ifndef FS_FSLOGGING_H
#define FS_FSLOGGING_H

#include "System71StdLib.h"

#define FS_LOG_TRACE(fmt, ...) serial_logf(kLogModuleFileSystem, kLogLevelTrace, "[FS] " fmt, ##__VA_ARGS__)
#define FS_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleFileSystem, kLogLevelDebug, "[FS] " fmt, ##__VA_ARGS__)
#define FS_LOG_INFO(fmt, ...)  serial_logf(kLogModuleFileSystem, kLogLevelInfo,  "[FS] " fmt, ##__VA_ARGS__)
#define FS_LOG_WARN(fmt, ...)  serial_logf(kLogModuleFileSystem, kLogLevelWarn,  "[FS] " fmt, ##__VA_ARGS__)
#define FS_LOG_ERROR(fmt, ...) serial_logf(kLogModuleFileSystem, kLogLevelError, "[FS] " fmt, ##__VA_ARGS__)

#endif /* FS_FSLOGGING_H */
