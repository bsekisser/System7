#ifndef SCRAPMANAGER_SCRAPLOGGING_H
#define SCRAPMANAGER_SCRAPLOGGING_H

#include "System71StdLib.h"

#define SCRAP_LOG_TRACE(fmt, ...) serial_logf(kLogModuleScrap, kLogLevelTrace, "[SCRAP] " fmt, ##__VA_ARGS__)
#define SCRAP_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleScrap, kLogLevelDebug, "[SCRAP] " fmt, ##__VA_ARGS__)
#define SCRAP_LOG_INFO(fmt, ...)  serial_logf(kLogModuleScrap, kLogLevelInfo,  "[SCRAP] " fmt, ##__VA_ARGS__)
#define SCRAP_LOG_WARN(fmt, ...)  serial_logf(kLogModuleScrap, kLogLevelWarn,  "[SCRAP] " fmt, ##__VA_ARGS__)
#define SCRAP_LOG_ERROR(fmt, ...) serial_logf(kLogModuleScrap, kLogLevelError, "[SCRAP] " fmt, ##__VA_ARGS__)

#endif /* SCRAPMANAGER_SCRAPLOGGING_H */
