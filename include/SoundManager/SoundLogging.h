#ifndef SOUNDMANAGER_SOUNDLOGGING_H
#define SOUNDMANAGER_SOUNDLOGGING_H

#include "System71StdLib.h"

#define SND_LOG_TRACE(fmt, ...) serial_logf(kLogModuleSound, kLogLevelTrace, "[SND] " fmt, ##__VA_ARGS__)
#define SND_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleSound, kLogLevelDebug, "[SND] " fmt, ##__VA_ARGS__)
#define SND_LOG_INFO(fmt, ...)  serial_logf(kLogModuleSound, kLogLevelInfo,  "[SND] " fmt, ##__VA_ARGS__)
#define SND_LOG_WARN(fmt, ...)  serial_logf(kLogModuleSound, kLogLevelWarn,  "[SND] " fmt, ##__VA_ARGS__)
#define SND_LOG_ERROR(fmt, ...) serial_logf(kLogModuleSound, kLogLevelError, "[SND] " fmt, ##__VA_ARGS__)

#endif /* SOUNDMANAGER_SOUNDLOGGING_H */
