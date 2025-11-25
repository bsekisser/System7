/*
 * Phase2Tests.h - Phase 2 Integration Test Suite Header
 *
 * Public interface for Phase 2 integration testing covering advanced
 * system functionality:
 * - Event dispatch critical path
 * - File I/O operations
 * - Dialog manager workflows
 * - TextEdit functionality
 *
 * Phase 2 tests validate interactions between multiple subsystems and
 * exercise end-to-end workflows that depend on proper subsystem integration.
 *
 * Copyright (c) 2025 System 7.1 Portable Project
 */

#ifndef _PHASE2_TESTS_H_
#define _PHASE2_TESTS_H_

#include "SystemTypes.h"
#include "Errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * EVENT DISPATCH TEST SUITE
 * ============================================================================
 */

OSErr Phase2_EventDispatch_Initialize(void);
void Phase2_EventDispatch_Run(void);
void Phase2_EventDispatch_Cleanup(void);

/* ============================================================================
 * FILE I/O TEST SUITE
 * ============================================================================
 */

OSErr Phase2_FileIO_Initialize(void);
void Phase2_FileIO_Run(void);
void Phase2_FileIO_Cleanup(void);

/* ============================================================================
 * FUTURE PHASE 2 TEST SUITES
 * ============================================================================
 */

/* Dialog Manager Integration Tests */
OSErr Phase2_DialogManager_Initialize(void);
void Phase2_DialogManager_Run(void);
void Phase2_DialogManager_Cleanup(void);

/* TextEdit Integration Tests */
OSErr Phase2_TextEdit_Initialize(void);
void Phase2_TextEdit_Run(void);
void Phase2_TextEdit_Cleanup(void);

/* QuickDraw Integration Tests */
/* OSErr Phase2_QuickDraw_Initialize(void); */
/* void Phase2_QuickDraw_Run(void); */
/* void Phase2_QuickDraw_Cleanup(void); */

/* Window Manager Integration Tests */
/* OSErr Phase2_WindowManager_Initialize(void); */
/* void Phase2_WindowManager_Run(void); */
/* void Phase2_WindowManager_Cleanup(void); */

/* Application Startup Workflow Tests */
/* OSErr Phase2_AppStartup_Initialize(void); */
/* void Phase2_AppStartup_Run(void); */
/* void Phase2_AppStartup_Cleanup(void); */

/* Sound Manager Integration Tests */
/* OSErr Phase2_SoundManager_Initialize(void); */
/* void Phase2_SoundManager_Run(void); */
/* void Phase2_SoundManager_Cleanup(void); */

/* Rendering Path Integration Tests */
/* OSErr Phase2_Rendering_Initialize(void); */
/* void Phase2_Rendering_Run(void); */
/* void Phase2_Rendering_Cleanup(void); */

#ifdef __cplusplus
}
#endif

#endif /* _PHASE2_TESTS_H_ */
