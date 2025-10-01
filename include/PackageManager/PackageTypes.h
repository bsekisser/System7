/*
 * PackageTypes.h
 * System 7.1 Portable Package Manager Type Definitions
 *
 * Common data structures and types used across all packages.
 * Maintains compatibility with original Mac OS Package Manager APIs.
 */

#ifndef __PACKAGE_TYPES_H__
#define __PACKAGE_TYPES_H__

#include "SystemTypes.h"

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Basic Mac OS types for compatibility */
/* OSErr is defined in MacTypes.h *//* OSType is defined in MacTypes.h */

/* Str255 is defined in MacTypes.h */
/* Str31 is defined in MacTypes.h */
/* ConstStr255Param is defined in MacTypes.h */
/* Str31 is defined in MacTypes.h */

/* Boolean type for Mac compatibility */
#ifndef Boolean
/* Boolean is defined in MacTypes.h */#endif

#ifndef true
#define true    1
#define false   0
#endif

/* Point and Rectangle structures */

/* Mac OS Handle type simulation */
/* Handle is defined in MacTypes.h */
/* Ptr is defined in MacTypes.h */
/* File system types */

/* Date/Time types */

/* String comparison types */
      /* relational operator result */

/* Package function pointer types */

/* Package descriptor structure */

/* Parameter block for package calls */

/* Memory management types */

/* Resource types */

/* Event types for dialog handling */

/* Dialog and window types */
/* Ptr is defined in MacTypes.h */
/* Ptr is defined in MacTypes.h */
/* Ptr is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */

/* Procedure pointer types */

/* List Manager types */

/* Ptr is defined in MacTypes.h */
/* Handle is defined in MacTypes.h */

/* Sound types */

/* Thread safety types */

/* Platform integration types */

/* Configuration structure */

/* Error handling types */

/* Package statistics */

/* Floating point environment types (for SANE) */

/* Internationalization types */

/* Constants for error codes */
#define PACKAGE_NO_ERROR            0
#define PACKAGE_INVALID_ID          -1
#define PACKAGE_NOT_LOADED          -2
#define PACKAGE_INIT_FAILED         -3
#define PACKAGE_INVALID_SELECTOR    -4
#define PACKAGE_INVALID_PARAMS      -5
#define PACKAGE_MEMORY_ERROR        -6
#define PACKAGE_RESOURCE_ERROR      -7
#define PACKAGE_THREAD_ERROR        -8
#define PACKAGE_PLATFORM_ERROR      -9
#define PACKAGE_COMPATIBILITY_ERROR -10

/* Maximum values */
#define MAX_PACKAGES                16
#define MAX_PACKAGE_NAME            64
#define MAX_ERROR_MESSAGE           256
#define MAX_RESOURCE_NAME           256

#ifdef __cplusplus
}
#endif

#endif /* __PACKAGE_TYPES_H__ */
#endif /* PACKAGETYPES_H */
