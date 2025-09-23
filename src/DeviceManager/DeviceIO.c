#include "SuperCompat.h"
#include <stdlib.h>
#include <string.h>
/*
 * DeviceIO.c
 * System 7.1 Device Manager - Device I/O Operations Implementation
 *
 * Implements the core I/O operations including Open, Close, Read, Write,
 * Control, Status, and KillIO operations using parameter blocks.
 *
 */

#include "CompatibilityFix.h"
#include "SystemTypes.h"
#include "System71StdLib.h"

#include "DeviceManager/DeviceIO.h"
#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DriverInterface.h"
#include "DeviceManager/UnitTable.h"
#include "MemoryMgr/memory_manager_types.h"


/* =============================================================================
 * Global Variables
 * ============================================================================= */

static UInt32 g_ioRequestID = 1;
static IOStatistics g_ioStatistics = {0};

/* =============================================================================
 * Internal Function Declarations
 * ============================================================================= */

static SInt16 ValidateIOParam(IOParamPtr pb);
static SInt16 ValidateCntrlParam(CntrlParamPtr pb);
static SInt16 ProcessAsyncIO(void *pb, DCEPtr dce, Boolean isAsync);
static void CompleteIOOperation(IOParamPtr pb, SInt16 result);
static SInt16 HandleFileSystemRedirect(IOParamPtr pb);
/* IsFileRefNum is declared in UnitTable.h */

/* Forward declarations for queue management */
void EnqueueIORequest(DCEPtr dce, IOParamPtr paramBlock);
IOParamPtr DequeueIORequest(DCEPtr dce);

/* =============================================================================
 * Parameter Block I/O Operations
 * ============================================================================= */

SInt16 PBOpen(IOParamPtr paramBlock, Boolean async)
{
    if (paramBlock == NULL) {
        return paramErr;
    }

    SInt16 error = ValidateIOParam(paramBlock);
    if (error != noErr) {
        return error;
    }

    /* Check if this is for a file (positive refNum) */
    if (IsFileRefNum(paramBlock->ioRefNum)) {
        return HandleFileSystemRedirect(paramBlock);
    }

    /* Get DCE for driver */
    DCEHandle dceHandle = GetDCtlEntry(paramBlock->ioRefNum);
    if (dceHandle == NULL) {
        return badUnitErr;
    }

    DCEPtr dce = *dceHandle;
    if (dce == NULL) {
        return unitEmptyErr;
    }

    /* Set up parameter block for processing */
    paramBlock->ioResult = ioInProgress;
    paramBlock->ioActCount = 0;

    /* Process the operation */
    SInt16 result;
    if (async) {
        result = ProcessAsyncIO(paramBlock, dce, true);
    } else {
        result = CallDriverOpen(paramBlock, dce);
        CompleteIOOperation(paramBlock, result);
    }

    /* Update statistics */
    g_ioStatistics.readOperations++; /* Open counts as read operation */

    return result;
}

SInt16 PBClose(IOParamPtr paramBlock, Boolean async)
{
    if (paramBlock == NULL) {
        return paramErr;
    }

    SInt16 error = ValidateIOParam(paramBlock);
    if (error != noErr) {
        return error;
    }

    /* Check if this is for a file */
    if (IsFileRefNum(paramBlock->ioRefNum)) {
        return HandleFileSystemRedirect(paramBlock);
    }

    /* Get DCE for driver */
    DCEHandle dceHandle = GetDCtlEntry(paramBlock->ioRefNum);
    if (dceHandle == NULL) {
        return badUnitErr;
    }

    DCEPtr dce = *dceHandle;
    if (dce == NULL) {
        return unitEmptyErr;
    }

    /* Set up parameter block */
    paramBlock->ioResult = ioInProgress;

    /* Process the operation */
    SInt16 result;
    if (async) {
        result = ProcessAsyncIO(paramBlock, dce, true);
    } else {
        result = CallDriverClose(paramBlock, dce);
        CompleteIOOperation(paramBlock, result);
    }

    return result;
}

SInt16 PBRead(IOParamPtr paramBlock, Boolean async)
{
    if (paramBlock == NULL) {
        return paramErr;
    }

    SInt16 error = ValidateIOParam(paramBlock);
    if (error != noErr) {
        return error;
    }

    /* Check if this is for a file */
    if (IsFileRefNum(paramBlock->ioRefNum)) {
        return HandleFileSystemRedirect(paramBlock);
    }

    /* Get DCE for driver */
    DCEHandle dceHandle = GetDCtlEntry(paramBlock->ioRefNum);
    if (dceHandle == NULL) {
        return badUnitErr;
    }

    DCEPtr dce = *dceHandle;
    if (dce == NULL) {
        return unitEmptyErr;
    }

    /* Check if driver supports read */
    if (!(dce->dCtlFlags & Read_Enable_Mask)) {
        return readErr;
    }

    /* Set up parameter block */
    paramBlock->ioResult = ioInProgress;
    paramBlock->ioTrap = (paramBlock->ioTrap & 0xFF00) | aRdCmd;
    paramBlock->ioActCount = 0;

    /* Process the operation */
    SInt16 result;
    if (async) {
        result = ProcessAsyncIO(paramBlock, dce, true);
    } else {
        result = CallDriverPrime(paramBlock, dce);
        CompleteIOOperation(paramBlock, result);
    }

    /* Update statistics */
    g_ioStatistics.readOperations++;
    if (result == noErr) {
        g_ioStatistics.bytesRead += paramBlock->ioActCount;
    } else {
        g_ioStatistics.errors++;
    }

    return result;
}

SInt16 PBWrite(IOParamPtr paramBlock, Boolean async)
{
    if (paramBlock == NULL) {
        return paramErr;
    }

    SInt16 error = ValidateIOParam(paramBlock);
    if (error != noErr) {
        return error;
    }

    /* Check if this is for a file */
    if (IsFileRefNum(paramBlock->ioRefNum)) {
        return HandleFileSystemRedirect(paramBlock);
    }

    /* Get DCE for driver */
    DCEHandle dceHandle = GetDCtlEntry(paramBlock->ioRefNum);
    if (dceHandle == NULL) {
        return badUnitErr;
    }

    DCEPtr dce = *dceHandle;
    if (dce == NULL) {
        return unitEmptyErr;
    }

    /* Check if driver supports write */
    if (!(dce->dCtlFlags & Write_Enable_Mask)) {
        return writErr;
    }

    /* Set up parameter block */
    paramBlock->ioResult = ioInProgress;
    paramBlock->ioTrap = (paramBlock->ioTrap & 0xFF00) | aWrCmd;
    paramBlock->ioActCount = 0;

    /* Process the operation */
    SInt16 result;
    if (async) {
        result = ProcessAsyncIO(paramBlock, dce, true);
    } else {
        result = CallDriverPrime(paramBlock, dce);
        CompleteIOOperation(paramBlock, result);
    }

    /* Update statistics */
    g_ioStatistics.writeOperations++;
    if (result == noErr) {
        g_ioStatistics.bytesWritten += paramBlock->ioActCount;
    } else {
        g_ioStatistics.errors++;
    }

    return result;
}

SInt16 PBControl(CntrlParamPtr paramBlock, Boolean async)
{
    if (paramBlock == NULL) {
        return paramErr;
    }

    SInt16 error = ValidateCntrlParam(paramBlock);
    if (error != noErr) {
        return error;
    }

    /* Get DCE for driver */
    DCEHandle dceHandle = GetDCtlEntry(paramBlock->ioCRefNum);
    if (dceHandle == NULL) {
        return badUnitErr;
    }

    DCEPtr dce = *dceHandle;
    if (dce == NULL) {
        return unitEmptyErr;
    }

    /* Check if driver supports control */
    if (!(dce->dCtlFlags & Control_Enable_Mask)) {
        return controlErr;
    }

    /* Set up parameter block */
    paramBlock->ioResult = ioInProgress;

    /* Process the operation */
    SInt16 result;
    if (async) {
        result = ProcessAsyncIO(paramBlock, dce, true);
    } else {
        result = CallDriverControl(paramBlock, dce);
        paramBlock->ioResult = result;
    }

    /* Update statistics */
    g_ioStatistics.controlOperations++;
    if (result != noErr) {
        g_ioStatistics.errors++;
    }

    return result;
}

SInt16 PBStatus(CntrlParamPtr paramBlock, Boolean async)
{
    if (paramBlock == NULL) {
        return paramErr;
    }

    SInt16 error = ValidateCntrlParam(paramBlock);
    if (error != noErr) {
        return error;
    }

    /* Get DCE for driver */
    DCEHandle dceHandle = GetDCtlEntry(paramBlock->ioCRefNum);
    if (dceHandle == NULL) {
        return badUnitErr;
    }

    DCEPtr dce = *dceHandle;
    if (dce == NULL) {
        return unitEmptyErr;
    }

    /* Check if driver supports status */
    if (!(dce->dCtlFlags & Status_Enable_Mask)) {
        return statusErr;
    }

    /* Set up parameter block */
    paramBlock->ioResult = ioInProgress;

    /* Process the operation */
    SInt16 result;
    if (async) {
        result = ProcessAsyncIO(paramBlock, dce, true);
    } else {
        result = CallDriverStatus(paramBlock, dce);
        paramBlock->ioResult = result;
    }

    /* Update statistics */
    g_ioStatistics.statusOperations++;
    if (result != noErr) {
        g_ioStatistics.errors++;
    }

    return result;
}

SInt16 PBKillIO(IOParamPtr paramBlock, Boolean async)
{
    if (paramBlock == NULL) {
        return paramErr;
    }

    SInt16 error = ValidateIOParam(paramBlock);
    if (error != noErr) {
        return error;
    }

    /* Get DCE for driver */
    DCEHandle dceHandle = GetDCtlEntry(paramBlock->ioRefNum);
    if (dceHandle == NULL) {
        return badUnitErr;
    }

    DCEPtr dce = *dceHandle;
    if (dce == NULL) {
        return unitEmptyErr;
    }

    /* Set up parameter block */
    paramBlock->ioResult = ioInProgress;

    /* Kill pending I/O operations */
    SInt16 result = CallDriverKill(paramBlock, dce);
    paramBlock->ioResult = result;

    /* Update statistics */
    g_ioStatistics.killOperations++;

    return result;
}

/* =============================================================================
 * I/O Parameter Block Management
 * ============================================================================= */

void InitIOParamBlock(IOParamPtr pb, IOOperationType operation, SInt16 refNum)
{
    if (pb == NULL) {
        return;
    }

    memset(pb, 0, sizeof(IOParam));

    /* Set up header */
    pb->qLink = NULL;
    pb->qType = 0;
    pb->ioTrap = 0;
    pb->ioCmdAddr = NULL;
    pb->ioCompletion = NULL;
    pb->ioResult = 0;
    pb->ioNamePtr = NULL;
    pb->ioVRefNum = 0;

    /* Set operation-specific fields */
    pb->ioRefNum = refNum;
    pb->ioVersNum = 0;
    pb->ioPermssn = fsCurPerm;

    switch (operation) {
        case kIOOperationRead:
            pb->ioTrap |= aRdCmd;
            pb->ioPosMode = fsAtMark;
            break;

        case kIOOperationWrite:
            pb->ioTrap |= aWrCmd;
            pb->ioPosMode = fsAtMark;
            break;

        default:
            break;
    }
}

void SetIOBuffer(IOParamPtr pb, void *buffer, SInt32 count)
{
    if (pb == NULL) {
        return;
    }

    pb->ioBuffer = buffer;
    pb->ioReqCount = count;
    pb->ioActCount = 0;
}

void SetIOPosition(IOParamPtr pb, SInt16 mode, SInt32 offset)
{
    if (pb == NULL) {
        return;
    }

    pb->ioPosMode = mode;
    pb->ioPosOffset = offset;
}

void SetIOCompletion(IOParamPtr pb, IOCompletionProc completion)
{
    if (pb == NULL) {
        return;
    }

    pb->ioCompletion = (void*)completion;
}

Boolean IsIOComplete(IOParamPtr pb)
{
    if (pb == NULL) {
        return true;
    }

    return pb->ioResult != ioInProgress;
}

Boolean IsIOInProgress(IOParamPtr pb)
{
    if (pb == NULL) {
        return false;
    }

    return pb->ioResult == ioInProgress;
}

SInt16 GetIOResult(IOParamPtr pb)
{
    if (pb == NULL) {
        return paramErr;
    }

    return pb->ioResult;
}

/* =============================================================================
 * Asynchronous I/O Management
 * ============================================================================= */

AsyncIORequestPtr CreateAsyncIORequest(IOParamPtr pb, UInt32 priority,
                                       AsyncIOCompletionProc completion)
{
    if (pb == NULL) {
        return NULL;
    }

    AsyncIORequestPtr request = (AsyncIORequestPtr)malloc(sizeof(AsyncIORequest));
    if (request == NULL) {
        return NULL;
    }

    memset(request, 0, sizeof(AsyncIORequest));

    /* Copy parameter block */
    memcpy(&request->param, pb, sizeof(IOParam));

    /* Set request properties */
    request->requestID = g_ioRequestID++;
    request->priority = priority;
    request->isCancelled = false;
    request->isCompleted = false;
    request->context = NULL;

    return request;
}

SInt16 CancelAsyncIORequest(AsyncIORequestPtr request)
{
    if (request == NULL) {
        return paramErr;
    }

    if (request->isCompleted) {
        return noErr; /* Already completed */
    }

    request->isCancelled = true;
    request->param.ioResult = abortErr;

    return noErr;
}

SInt16 WaitForAsyncIO(AsyncIORequestPtr request, UInt32 timeout)
{
    if (request == NULL) {
        return paramErr;
    }

    /* In a real implementation, this would wait for completion */
    /* For now, we simulate immediate completion */
    request->isCompleted = true;

    return noErr;  /* Return success for now */
}

void DestroyAsyncIORequest(AsyncIORequestPtr request)
{
    if (request != NULL) {
        free(request);
    }
}

/* =============================================================================
 * Internal Helper Functions
 * ============================================================================= */

static SInt16 ValidateIOParam(IOParamPtr pb)
{
    if (pb == NULL) {
        return paramErr;
    }

    /* Check reference number */
    if (!IsValidRefNum(pb->ioRefNum) && !IsFileRefNum(pb->ioRefNum)) {
        return badUnitErr;
    }

    return noErr;
}

static SInt16 ValidateCntrlParam(CntrlParamPtr pb)
{
    if (pb == NULL) {
        return paramErr;
    }

    /* Check reference number */
    if (!IsValidRefNum(pb->ioCRefNum)) {
        return badUnitErr;
    }

    return noErr;
}

static SInt16 ProcessAsyncIO(void *pb, DCEPtr dce, Boolean isAsync)
{
    if (!isAsync) {
        return noErr; /* Should not be called for sync operations */
    }

    /* Mark driver as active */
    dce->dCtlFlags |= Is_Active_Mask;

    /* Add to driver's I/O queue */
    EnqueueIORequest(dce, (IOParamPtr)pb);

    /* In a real implementation, this would trigger driver processing */
    /* For now, we simulate immediate completion */
    IOParamPtr nextPB = DequeueIORequest(dce);
    if (nextPB != NULL) {
        SInt16 result;

        /* Determine operation type and call appropriate driver routine */
        if ((nextPB->ioTrap & 0xFF) == aRdCmd) {
            result = CallDriverPrime(nextPB, dce);
        } else if ((nextPB->ioTrap & 0xFF) == aWrCmd) {
            result = CallDriverPrime(nextPB, dce);
        } else {
            result = CallDriverOpen(nextPB, dce); /* Default to open */
        }

        CompleteIOOperation(nextPB, result);
    }

    /* Check if more operations are pending */
    if (!DriverHasPendingIO(dce)) {
        dce->dCtlFlags &= ~Is_Active_Mask;
    }

    return noErr;
}

static void CompleteIOOperation(IOParamPtr pb, SInt16 result)
{
    if (pb == NULL) {
        return;
    }

    /* Set result */
    pb->ioResult = result;

    /* Call completion routine if provided */
    if (pb->ioCompletion != NULL) {
        IOCompletionProc completion = (IOCompletionProc)pb->ioCompletion;
        completion(pb);
    }
}

static SInt16 HandleFileSystemRedirect(IOParamPtr pb)
{
    /* In the original Mac OS, this would redirect to the File System */
    /* For our implementation, we return an error since we don't implement files */
    return fnfErr; /* File not found */
}

Boolean IsFileRefNum(SInt16 refNum)
{
    return refNum > 0;
}

void IODone(IOParamPtr paramBlock)
{
    if (paramBlock == NULL) {
        return;
    }

    /* Mark operation as complete */
    if (paramBlock->ioResult == ioInProgress) {
        paramBlock->ioResult = noErr;
    }

    /* Call completion routine */
    CompleteIOOperation(paramBlock, paramBlock->ioResult);
}

void EnqueueIORequest(DCEPtr dce, IOParamPtr paramBlock)
{
    if (dce == NULL || paramBlock == NULL) {
        return;
    }

    /* Simple queue implementation - add to tail */
    paramBlock->qLink = NULL;

    if (dce->dCtlQHdr.qTail == NULL) {
        /* Empty queue */
        dce->dCtlQHdr.qHead = (QElemPtr)paramBlock;
        dce->dCtlQHdr.qTail = (QElemPtr)paramBlock;
    } else {
        /* Add to tail */
        ((IOParamPtr)dce->dCtlQHdr.qTail)->qLink = (QElemPtr)paramBlock;
        dce->dCtlQHdr.qTail = (QElemPtr)paramBlock;
    }
}

IOParamPtr DequeueIORequest(DCEPtr dce)
{
    if (dce == NULL || dce->dCtlQHdr.qHead == NULL) {
        return NULL;
    }

    /* Remove from head */
    IOParamPtr pb = (IOParamPtr)dce->dCtlQHdr.qHead;
    dce->dCtlQHdr.qHead = ((QElemPtr)pb)->qLink;

    if (dce->dCtlQHdr.qHead == NULL) {
        /* Queue is now empty */
        dce->dCtlQHdr.qTail = NULL;
    }

    pb->qLink = NULL;
    return pb;
}

Boolean DriverHasPendingIO(DCEPtr dce)
{
    if (dce == NULL) {
        return false;
    }

    return dce->dCtlQHdr.qHead != NULL;
}
