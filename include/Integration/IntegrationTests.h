/*
 * IntegrationTests.h - Phase 1 Integration Test Suite Header
 *
 * Public interface for integration testing framework.
 * Tests verify core System 7 functionality end-to-end.
 *
 * Copyright (c) 2025 System 7.1 Portable Project
 */

#ifndef _INTEGRATION_TESTS_H_
#define _INTEGRATION_TESTS_H_

#include "SystemTypes.h"
#include "Errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize integration test framework */
OSErr IntegrationTests_Initialize(void);

/* Run all integration tests */
void IntegrationTests_Run(void);

/* Cleanup integration tests */
void IntegrationTests_Cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* _INTEGRATION_TESTS_H_ */
