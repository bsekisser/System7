#ifndef WINDOWMANAGER_WMLOGGING_H
#define WINDOWMANAGER_WMLOGGING_H

#include "System71StdLib.h"

/* Disable all WM logging for performance */
#define WM_LOG_TRACE(fmt, ...) do {} while(0)
#define WM_LOG_DEBUG(fmt, ...) do {} while(0)
#define WM_DEBUG(fmt, ...) do {} while(0)
#define WM_LOG_INFO(fmt, ...)  do {} while(0)
#define WM_LOG_WARN(fmt, ...)  do {} while(0)
#define WM_LOG_ERROR(fmt, ...) do {} while(0)

#endif /* WINDOWMANAGER_WMLOGGING_H */
