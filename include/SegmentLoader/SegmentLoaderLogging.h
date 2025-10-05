/*
 * SegmentLoaderLogging.h - Segment Loader Module Logging
 *
 * Hierarchical logging for Segment Loader subsystem using the
 * System 7.1 serial logging framework.
 */

#ifndef SEGMENT_LOADER_LOGGING_H
#define SEGMENT_LOADER_LOGGING_H

#include "System71StdLib.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Segment Loader Logging Macros
 *
 * Usage:
 *   SEG_LOG_ERROR("Failed to load CODE %d: %d", segID, err);
 *   SEG_LOG_DEBUG("A5 world: base=0x%08X", a5Base);
 */
#define SEG_LOG_ERROR(fmt, ...) \
    serial_logf(kLogModuleSegmentLoader, kLogLevelError, "[SEG] " fmt, ##__VA_ARGS__)

#define SEG_LOG_WARN(fmt, ...) \
    serial_logf(kLogModuleSegmentLoader, kLogLevelWarn, "[SEG] " fmt, ##__VA_ARGS__)

#define SEG_LOG_INFO(fmt, ...) \
    serial_logf(kLogModuleSegmentLoader, kLogLevelInfo, "[SEG] " fmt, ##__VA_ARGS__)

#define SEG_LOG_DEBUG(fmt, ...) \
    serial_logf(kLogModuleSegmentLoader, kLogLevelDebug, "[SEG] " fmt, ##__VA_ARGS__)

#define SEG_LOG_TRACE(fmt, ...) \
    serial_logf(kLogModuleSegmentLoader, kLogLevelTrace, "[SEG] " fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* SEGMENT_LOADER_LOGGING_H */
