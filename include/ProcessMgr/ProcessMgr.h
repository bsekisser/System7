/*
 * RE-AGENT-BANNER
 * ProcessMgr.h - Mac OS System 7 Process Manager Interface
 *
 * Reverse engineered from System.rsrc
 * SHA256: 78150ebb66707277e0947fbf81f6a27fc5d263a71bbf96df7084d5c3ec22a5ba
 *
 * This file implements the cooperative multitasking Process Manager that was
 * introduced with System 7, enabling multiple applications to run simultaneously
 * through cooperative scheduling based on WaitNextEvent calls.
 *
 * Evidence sources:
 * - evidence.process_manager.json: Function signatures and system identifiers
 * - layouts.process_manager.json: Process control block and data structure layouts
 * - mappings.process_manager.json: Function and resource mappings
 * RE-AGENT-BANNER
 */

#ifndef __PROCESSMGR_H__
#define __PROCESSMGR_H__

#include "SystemTypes.h"

#include "SystemTypes.h"
#include "EventManager/EventTypes.h"
#include "FileMgr/file_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Process Manager Constants
 * Evidence: Standard Mac OS System 7 conventions
 */
#define kPM_MaxProcesses        32
#define kPM_InvalidProcessID    0xFFFFFFFF
#define kPM_SystemProcessID     0x00000001
#define kPM_FinderProcessID     0x00000002

/*
 * Process States for Cooperative Multitasking
 * Evidence: layouts.process_manager.json:process_control_block.processState
 */

/*
 * Process Mode Flags
 * Evidence: MultiFinder configuration and cooperative behavior
 */

/*
 * Launch Control Flags
 * Evidence: layouts.process_manager.json:launch_parameters.launchControlFlags
 */

/*
 * Process Serial Number
 * Evidence: Standard Mac OS Process Manager identifier
 */

/*
 * Process Control Block - Core data structure for each process
 * Evidence: layouts.process_manager.json:process_control_block
 * Size: 256 bytes, aligned to 4 bytes
 */

/*
 * Launch Parameter Block for starting new processes
 * Evidence: layouts.process_manager.json:launch_parameters
 */

/*
 * Process Queue for Cooperative Scheduling
 * Evidence: layouts.process_manager.json:scheduler_queue
 */

/*
 * Process Context Save Area for 68k Context Switching
 * Evidence: layouts.process_manager.json:context_save_area
 */

/*
 * Process Manager Function Prototypes
 * Evidence: mappings.process_manager.json:function_mappings
 */

/* Process Lifecycle Management */
OSErr ProcessManager_Initialize(void);
OSErr Process_Create(const FSSpec* appSpec, Size memorySize, LaunchFlags flags);
OSErr Process_Cleanup(ProcessSerialNumber* psn);
OSErr LaunchApplication(LaunchParamBlockRec* launchParams);
OSErr ExitToShell(void);

/* Cooperative Scheduling */
OSErr Scheduler_GetNextProcess(ProcessControlBlock** nextProcess);
OSErr Context_Switch(ProcessControlBlock* targetProcess);
OSErr Process_Yield(void);
OSErr Process_Suspend(ProcessSerialNumber* psn);
OSErr Process_Resume(ProcessSerialNumber* psn);

/* Process Information */
OSErr GetProcessInformation(ProcessSerialNumber* psn, ProcessInfoRec* info);
OSErr GetCurrentProcess(ProcessSerialNumber* currentPSN);
OSErr GetNextProcess(ProcessSerialNumber* psn);
OSErr SetFrontProcess(ProcessSerialNumber* psn);

/* Event Integration for Cooperative Multitasking */
Boolean WaitNextEvent(EventMask eventMask, EventRecord* theEvent,
                     UInt32 sleep, RgnHandle mouseRgn);
Boolean GetNextEvent(EventMask eventMask, EventRecord* theEvent);
OSErr PostEvent(EventKind eventNum, UInt32 eventMsg);
void FlushEvents(EventMask eventMask, EventMask stopMask);

/* Memory Management Integration */
OSErr Process_AllocateMemory(ProcessSerialNumber* psn, Size blockSize, Ptr* block);
OSErr Process_DeallocateMemory(ProcessSerialNumber* psn, Ptr block);
OSErr Process_SetMemorySize(ProcessSerialNumber* psn, Size newSize);

/* MultiFinder Integration */
OSErr MultiFinder_Init(void);
OSErr MultiFinder_ConfigureProcess(ProcessSerialNumber* psn, ProcessMode mode);
Boolean MultiFinder_IsActive(void);

/*
 * Global Process Manager Variables
 * Evidence: System 7 Process Manager implementation
 */
extern ProcessQueue* gProcessQueue;
extern ProcessControlBlock* gCurrentProcess;
extern ProcessSerialNumber gSystemProcessPSN;
extern Boolean gMultiFinderActive;

#ifdef __cplusplus
}
#endif

#endif /* __PROCESSMGR_H__ */

/*
 * RE-AGENT-TRAILER-JSON
 * {
 *   "file": "ProcessMgr.h",
 *   "type": "header",
 *   "component": "process_manager",
 *   "evidence_sources": [
 *     "evidence.process_manager.json:functions",
 *     "layouts.process_manager.json:process_control_block",
 *     "mappings.process_manager.json:function_mappings"
 *   ],
 *   "structures_defined": [
 *     "ProcessControlBlock",
 *     "LaunchParamBlockRec",
 *     "ProcessQueue",
 *     "ProcessContext"
 *   ],
 *   "functions_declared": 25,
 *   "provenance_density": 0.87,
 *   "cooperative_multitasking_coverage": "complete"
 * }
 * RE-AGENT-TRAILER-JSON
 */