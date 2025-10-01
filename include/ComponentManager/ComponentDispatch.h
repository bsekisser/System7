/*
 * ComponentDispatch.h
 *
 * Component Dispatch API - System 7.1 Portable Implementation
 * Handles component API dispatch and calling conventions
 */

#ifndef COMPONENTDISPATCH_H
#define COMPONENTDISPATCH_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "ComponentManager.h"
#include "ComponentRegistry.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Component call context */

/* Component call flags */
#define kComponentCallFlagAsync         (1<<0)  /* Asynchronous call */
#define kComponentCallFlagDeferred      (1<<1)  /* Deferred call */
#define kComponentCallFlagImmediate     (1<<2)  /* Immediate call */
#define kComponentCallFlagNoResult      (1<<3)  /* Don't wait for result */
#define kComponentCallFlagInterruptible (1<<4)  /* Call can be interrupted */
#define kComponentCallFlagSecure        (1<<5)  /* Secure call context */

/* Dispatch initialization */
OSErr InitComponentDispatch(void);
void CleanupComponentDispatch(void);

/* Main component dispatch function */
ComponentResult ComponentDispatch(ComponentInstance instance, ComponentParameters* params);

/* Component calling conventions */
ComponentResult CallComponent(ComponentInstance instance, SInt16 selector, ...);
ComponentResult CallComponentWithParams(ComponentInstance instance, ComponentParameters* params);
ComponentResult CallComponentFunction(ComponentParameters* params, ComponentFunction func);
ComponentResult CallComponentFunctionWithStorage(Handle storage, ComponentParameters* params, ComponentFunction func);

/* Standard component selectors dispatch */
ComponentResult DispatchComponentOpen(ComponentCallContext* context);
ComponentResult DispatchComponentClose(ComponentCallContext* context);
ComponentResult DispatchComponentCanDo(ComponentCallContext* context);
ComponentResult DispatchComponentVersion(ComponentCallContext* context);
ComponentResult DispatchComponentRegister(ComponentCallContext* context);
ComponentResult DispatchComponentTarget(ComponentCallContext* context);
ComponentResult DispatchComponentUnregister(ComponentCallContext* context);

/* Component delegation support */
ComponentResult DelegateComponentCall(ComponentParameters* originalParams, ComponentInstance ci);
ComponentResult RedirectComponentCall(ComponentCallContext* context, ComponentInstance target);

/* Call stack management */

OSErr InitComponentCallStack(ComponentCallStack* stack, UInt32 maxDepth);
OSErr PushComponentCall(ComponentCallStack* stack, ComponentCallContext* context);
OSErr PopComponentCall(ComponentCallStack* stack, ComponentCallContext** context);
ComponentCallContext* GetCurrentComponentCall(ComponentCallStack* stack);
void CleanupComponentCallStack(ComponentCallStack* stack);

/* Component call monitoring */

OSErr RegisterComponentCallMonitor(ComponentCallMonitor monitor);
OSErr UnregisterComponentCallMonitor(ComponentCallMonitor monitor);

/* Parameter marshaling */

/* Parameter types */
#define kComponentParamTypeVoid         0
#define kComponentParamTypeInt8         1
#define kComponentParamTypeInt16        2
#define kComponentParamTypeInt32        3
#define kComponentParamTypeInt64        4
#define kComponentParamTypeFloat32      5
#define kComponentParamTypeFloat64      6
#define kComponentParamTypePointer      7
#define kComponentParamTypeHandle       8
#define kComponentParamTypeOSType       9
#define kComponentParamTypeString       10
#define kComponentParamTypeRect         11
#define kComponentParamTypeRegion       12

/* Parameter flags */
#define kComponentParamFlagInput        (1<<0)
#define kComponentParamFlagOutput       (1<<1)
#define kComponentParamFlagOptional     (1<<2)
#define kComponentParamFlagArray        (1<<3)

OSErr MarshalParameters(ComponentParameters* params, ComponentParamDescriptor* descriptors, SInt32 count);
OSErr UnmarshalParameters(ComponentParameters* params, ComponentParamDescriptor* descriptors, SInt32 count);

/* Cross-platform calling convention support */
#ifdef PLATFORM_REMOVED_WIN32
#define COMPONENT_CALLING_CONVENTION __stdcall
#elif defined(__APPLE__)
#define COMPONENT_CALLING_CONVENTION
#elif defined(__linux__)
#define COMPONENT_CALLING_CONVENTION
#else
#define COMPONENT_CALLING_CONVENTION
#endif

/* Component function prototypes for different calling conventions */

/* Calling convention detection and adaptation */
ComponentResult AdaptComponentCall(ComponentCallContext* context);
ComponentRoutine WrapComponentRoutine(void* routine, SInt32 callingConvention);

/* Component exception handling */

OSErr RegisterComponentExceptionHandler(ComponentExceptionHandler handler);
OSErr UnregisterComponentExceptionHandler(ComponentExceptionHandler handler);
OSErr HandleComponentException(ComponentException* exception);

/* Component call profiling */

OSErr EnableComponentProfiling(Boolean enable);
OSErr GetComponentCallProfile(ComponentInstance instance, SInt16 selector, ComponentCallProfile* profile);
OSErr ResetComponentProfiling(void);

/* Thread safety support */

OSErr CreateComponentMutex(ComponentMutex** mutex);
OSErr DestroyComponentMutex(ComponentMutex* mutex);
OSErr LockComponentMutex(ComponentMutex* mutex);
OSErr UnlockComponentMutex(ComponentMutex* mutex);

/* Component call queuing for async operations */

OSErr CreateComponentCallQueue(ComponentCallQueue* queue, UInt32 size);
OSErr DestroyComponentCallQueue(ComponentCallQueue* queue);
OSErr QueueComponentCall(ComponentCallQueue* queue, ComponentCallContext* context);
OSErr DequeueComponentCall(ComponentCallQueue* queue, ComponentCallContext** context);
OSErr ProcessComponentCallQueue(ComponentCallQueue* queue);

/* Component debugging support */

OSErr GetComponentDebugInfo(ComponentInstance instance, ComponentDebugInfo* info);
OSErr SetComponentBreakpoint(ComponentInstance instance, SInt16 selector);
OSErr ClearComponentBreakpoint(ComponentInstance instance, SInt16 selector);

/* Component call tracing */

OSErr EnableComponentTracing(Boolean enable);
OSErr RegisterComponentTraceCallback(ComponentTraceCallback callback);
OSErr TraceComponentCall(ComponentCallContext* context, const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTDISPATCH_H */