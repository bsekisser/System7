/*
 * System 7.1 Portable - System Initialization
 *
 * This module handles critical system initialization including:
 * - ExpandMem (extended memory globals) setup
 * - Input system initialization (ADB abstraction)
 * - Resource decompression hook installation
 * - System boot sequence coordination
 *
 * Based on System 7.1 BeforePatches.a assembly code
 * Copyright (c) 2024 - Portable Mac OS Project
 */

#ifndef SYSTEMINIT_H

#include "SystemTypes.h"
#define SYSTEMINIT_H

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */

/* System version information */
#define SYSTEM_VERSION      0x0710  /* System 7.1 */
#define SYSTEM_VERSION_BCD  0x0701  /* BCD format */

/* Error codes */

/* System initialization stages */

/* Boot configuration flags */

/* System capabilities (detected hardware features) */

/* System initialization callbacks */

/* Main system initialization API */

/**
 * Initialize the Mac OS 7.1 portable system
 * This is the main entry point for system initialization
 *
 * @param config Boot configuration options (can be NULL for defaults)
 * @param callbacks Optional callbacks for initialization progress
 * @return SYS_OK on success, error code on failure
 */
SystemError SystemInit(const BootConfiguration* config,
                       const SystemInitCallbacks* callbacks);

/**
 * Initialize system to a specific stage
 * Allows partial initialization for testing or special boot modes
 *
 * @param target_stage Stage to initialize up to
 * @return SYS_OK on success, error code on failure
 */
SystemError SystemInitToStage(SystemInitStage target_stage);

/**
 * Get current system initialization stage
 *
 * @return Current initialization stage
 */
SystemInitStage SystemGetInitStage(void);

/**
 * Get system capabilities (detected hardware features)
 *
 * @param caps Pointer to capabilities structure to fill
 * @return SYS_OK on success
 */
SystemError SystemGetCapabilities(SystemCapabilities* caps);

/**
 * Get system globals structure
 * Returns the main system globals used throughout Mac OS
 *
 * @return Pointer to system globals (never NULL after init)
 */
SystemGlobals* SystemGetGlobals(void);

/**
 * Get ExpandMem structure
 * Returns the extended memory globals structure
 *
 * @return Pointer to ExpandMem (NULL if not initialized)
 */
ExpandMemRec* SystemGetExpandMem(void);

/* System shutdown and cleanup */

/**
 * Shutdown the system cleanly
 * Performs orderly shutdown of all subsystems
 *
 * @param restart If true, prepare for restart instead of poweroff
 * @return SYS_OK on success
 */
SystemError SystemShutdown(Boolean restart);

/**
 * Emergency system halt
 * Used for fatal errors - minimal cleanup
 *
 * @param error_code System error code that caused the halt
 * @param message Optional error message
 */
void SystemPanic(SystemError error_code, const char* message);

/* Debugging and diagnostics */

/**
 * Dump system state for debugging
 *
 * @param output_func Function to output debug text
 */
void SystemDumpState(void (*output_func)(const char* text));

/**
 * Validate system integrity
 * Performs self-tests on critical system structures
 *
 * @return SYS_OK if system is healthy
 */
SystemError SystemValidate(void);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEMINIT_H */