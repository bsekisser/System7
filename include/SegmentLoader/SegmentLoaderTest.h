/*
 * SegmentLoaderTest.h - Segment Loader Test Harness
 *
 * Provides synthetic CODE resources and test infrastructure to validate
 * segment loading before attempting real Classic Mac applications.
 */

#ifndef SEGMENT_LOADER_TEST_H
#define SEGMENT_LOADER_TEST_H

#include "SystemTypes.h"
#include "SegmentLoader/SegmentLoader.h"
#include "CPU/CPUBackend.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Big-Endian Write Helpers (complement to BE_Read16/32)
 */
static inline void BE_Write16_Ptr(UInt8* ptr, UInt16 value) {
    ptr[0] = (value >> 8) & 0xFF;
    ptr[1] = value & 0xFF;
}

static inline void BE_Write32_Ptr(UInt8* ptr, UInt32 value) {
    ptr[0] = (value >> 24) & 0xFF;
    ptr[1] = (value >> 16) & 0xFF;
    ptr[2] = (value >> 8) & 0xFF;
    ptr[3] = value & 0xFF;
}

/*
 * Test Harness Entry Points
 */

/*
 * SegmentLoader_TestBoot - Install synthetic CODE resources and run smoke tests
 *
 * This creates a minimal 2-segment synthetic app:
 * - CODE 0: A5 world metadata with 1 jump table entry
 * - CODE 1: Entry segment that triggers lazy load of CODE 2
 * - CODE 2: Trace trap to prove execution
 */
void SegmentLoader_TestBoot(void);

/*
 * SegmentLoader_RunSmokeChecks - Validate A5 invariants
 *
 * Call after EnsureEntrySegmentsLoaded to verify:
 * - a5BelowBase + a5BelowSize == a5Base
 * - jtBase == a5Base + jtOffsetFromA5
 * - All JT slots are materialized as stubs
 */
OSErr SegmentLoader_RunSmokeChecks(SegmentLoaderContext* ctx);

/*
 * Test Trap Handlers
 */

/*
 * LoadSeg_TrapHandler - _LoadSeg (0xA9F0) trap handler
 *
 * Pops segment ID from stack, loads segment, patches JT entry
 */
OSErr LoadSeg_TrapHandler(void* context, CPUAddr* pc, CPUAddr* registers);

/*
 * Trace_TrapHandler - Test trace trap (0xA800) handler
 *
 * Logs "CODE 2 executed!" to prove lazy loading worked
 */
OSErr Trace_TrapHandler(void* context, CPUAddr* pc, CPUAddr* registers);

#ifdef __cplusplus
}
#endif

#endif /* SEGMENT_LOADER_TEST_H */
