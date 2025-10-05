#ifndef TEXTEDIT_TELOGGING_H
#define TEXTEDIT_TELOGGING_H

#include "System71StdLib.h"

#define TE_LOG_TRACE(fmt, ...) serial_logf(kLogModuleTextEdit, kLogLevelTrace, "[TE] " fmt, ##__VA_ARGS__)
#define TE_LOG_DEBUG(fmt, ...) serial_logf(kLogModuleTextEdit, kLogLevelDebug, "[TE] " fmt, ##__VA_ARGS__)
#define TE_LOG_INFO(fmt, ...)  serial_logf(kLogModuleTextEdit, kLogLevelInfo,  "[TE] " fmt, ##__VA_ARGS__)
#define TE_LOG_WARN(fmt, ...)  serial_logf(kLogModuleTextEdit, kLogLevelWarn,  "[TE] " fmt, ##__VA_ARGS__)
#define TE_LOG_ERROR(fmt, ...) serial_logf(kLogModuleTextEdit, kLogLevelError, "[TE] " fmt, ##__VA_ARGS__)

#endif /* TEXTEDIT_TELOGGING_H */
