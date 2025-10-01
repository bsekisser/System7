#ifndef __SCSIMANAGER_H__
#define __SCSIMANAGER_H__

#include "SystemTypes.h"

/*
 * SCSIManager.h
 *
 * Portable implementation of Mac OS System 7.1 SCSI Manager
 * Derived from ROM SCSI routines
 *
 * Provides complete SCSI device management including:
 * - SCSI command execution (6, 10, and 12-byte commands)
 * - Device enumeration and identification
 * - Bus management and arbitration
 * - Synchronous and asynchronous I/O operations
 * - Error handling and retry logic
 * - Hardware abstraction for modern storage interfaces
 */

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* SCSI Manager Version */
#define SCSI_VERSION 43

/* Maximum values */
#define MAX_CDB_LENGTH          16      /* Maximum Command Descriptor Block length */
#define MAX_SCSI_BUSES          8       /* Maximum number of SCSI buses */
#define MAX_SCSI_TARGETS        8       /* Maximum targets per bus */
#define MAX_SCSI_LUNS           8       /* Maximum LUNs per target */
#define VENDOR_ID_LENGTH        16      /* ASCII string length for Vendor ID */
#define HANDSHAKE_DATA_LENGTH   8       /* Handshake data length */

/* SCSI Manager Function Codes */

/* SCSI Device Identifier */

/* Command Descriptor Block */
                    /* Pointer to CDB bytes */
    UInt8 cdbBytes[MAX_CDB_LENGTH];   /* Actual CDB to send */
} CDB;

/* Scatter/Gather Record */

/* SCSI Parameter Block Header */

/* SCSI I/O Parameter Block */

/* Bus Inquiry Parameter Block */

/* Abort Command Parameter Block */

/* Terminate I/O Parameter Block */

/* Reset Bus Parameter Block */

/* Reset Device Parameter Block */

/* SCSI Flags */
#define scsiDirectionMask      0x80000000L  /* Data direction mask */
#define scsiDirectionOut       0x80000000L  /* Data out (write) */
#define scsiDirectionIn        0x00000000L  /* Data in (read) */
#define scsiDirectionNone      0x40000000L  /* No data transfer */

#define scsiSIMQHead           0x20000000L  /* Queue at head */
#define scsiSIMQFreeze         0x10000000L  /* Freeze queue */
#define scsiSIMQNoFreeze       0x08000000L  /* Don't freeze on error */
#define scsiDoDisconnect       0x04000000L  /* Enable disconnect */
#define scsiDontDisconnect     0x02000000L  /* Disable disconnect */
#define scsiInitiateWide       0x01000000L  /* Initiate wide negotiation */

/* Result Flags */
#define scsiSIMQFrozen         0x0001       /* SIM queue is frozen */
#define scsiAutosenseValid     0x0002       /* Autosense data valid */
#define scsiBusNotFree         0x0004       /* Bus not free */

/* I/O Flags */
#define scsiNoParityCheck      0x0001       /* Disable parity checking */
#define scsiDisableSelectWAtn  0x0002       /* Disable select with ATN */
#define scsiSavePtrOnDisconnect 0x0004      /* Save pointers on disconnect */
#define scsiNoBucketIn         0x0008       /* No bit bucket in */
#define scsiNoBucketOut        0x0010       /* No bit bucket out */
#define scsiDisableWide        0x0020       /* Disable wide negotiation */
#define scsiInitiateSync       0x0040       /* Initiate sync negotiation */
#define scsiDisableSync        0x0080       /* Disable sync negotiation */

/* Tag Action Values */
#define scsiSimpleQTag         0x20         /* Simple queue tag */
#define scsiHeadQTag           0x21         /* Head of queue tag */
#define scsiOrderedQTag        0x22         /* Ordered queue tag */

/* Data Types */
#define scsiDataBuffer         0x00         /* Regular data buffer */
#define scsiDataTIB            0x01         /* Transfer Information Block */
#define scsiDataSG             0x02         /* Scatter/Gather list */

/* Transfer Types */
#define scsiTransferPolled     0x00         /* Polled transfer */
#define scsiTransferBlind      0x01         /* Blind transfer */
#define scsiTransferDMA        0x02         /* DMA transfer */

/* Error Codes */
#define scsiErrorBase          -7936
#define scsiRequestInProgress  1            /* Request in progress */
#define scsiRequestAborted     (scsiErrorBase + 2)
#define scsiUnableToAbort      (scsiErrorBase + 3)
#define scsiNonZeroStatus      (scsiErrorBase + 4)
#define scsiUnused05           (scsiErrorBase + 5)
#define scsiUnused06           (scsiErrorBase + 6)
#define scsiUnused07           (scsiErrorBase + 7)
#define scsiUnused08           (scsiErrorBase + 8)
#define scsiUnableToTerminate  (scsiErrorBase + 9)
#define scsiSelectTimeout      (scsiErrorBase + 10)
#define scsiCommandTimeout     (scsiErrorBase + 11)
#define scsiIdentifyMessageRejected (scsiErrorBase + 12)
#define scsiMessageRejectReceived   (scsiErrorBase + 13)
#define scsiSCSIBusReset       (scsiErrorBase + 14)
#define scsiParityError        (scsiErrorBase + 15)
#define scsiAutosenseFailed    (scsiErrorBase + 16)
#define scsiUnused11           (scsiErrorBase + 17)
#define scsiDataRunError       (scsiErrorBase + 18)
#define scsiUnexpectedBusFree  (scsiErrorBase + 19)
#define scsiSequenceFail       (scsiErrorBase + 20)
#define scsiWrongDirection     (scsiErrorBase + 21)
#define scsiUnused16           (scsiErrorBase + 22)
#define scsiBDRsent            (scsiErrorBase + 23)
#define scsiTerminated         (scsiErrorBase + 24)
#define scsiNoNexus            (scsiErrorBase + 25)
#define scsiCDBReceived        (scsiErrorBase + 26)
#define scsiTooManyBuses       (scsiErrorBase + 48)
#define scsiNoSuchXref         (scsiErrorBase + 49)
#define scsiXrefNotFound       (scsiErrorBase + 50)
#define scsiBadFunction        (scsiErrorBase + 64)
#define scsiBadParameter       (scsiErrorBase + 65)
#define scsiTIDInvalid         (scsiErrorBase + 66)
#define scsiLUNInvalid         (scsiErrorBase + 67)
#define scsiIDInvalid          (scsiErrorBase + 68)
#define scsiDataTypeInvalid    (scsiErrorBase + 69)
#define scsiTransferTypeInvalid (scsiErrorBase + 70)
#define scsiCDBLengthInvalid   (scsiErrorBase + 71)

/* Function Prototypes */

/* Main SCSI Manager Entry Point */
OSErr SCSIAction(SCSI_IO *ioPtr);

/* Synchronous wrapper for common operations */
OSErr SCSIExecIOSync(SCSI_IO *ioPtr);

/* Bus Management */
OSErr SCSIBusInquirySync(SCSIBusInquiryPB *inquiry);
OSErr SCSIResetBusSync(SCSIResetBusPB *resetBus);
OSErr SCSIResetDeviceSync(SCSIResetDevicePB *resetDevice);

/* I/O Management */
OSErr SCSIAbortCommandSync(SCSIAbortCommandPB *abort);
OSErr SCSITerminateIOSync(SCSITerminateIOPB *terminate);

/* Parameter Block Management */
SCSI_IO *NewSCSI_PB(void);
void DisposeSCSI_PB(SCSI_IO *pb);

/* Device Management */
OSErr SCSIGetVirtualIDInfo(DeviceIdent *device, UInt8 *virtualID);
OSErr SCSICreateRefNumXref(DeviceIdent *device, SInt16 refNum);
OSErr SCSILookupRefNumXref(DeviceIdent *device, SInt16 *refNum);
OSErr SCSIRemoveRefNumXref(DeviceIdent *device);

/* Initialization and Cleanup */
OSErr InitSCSIManager(void);
void ShutdownSCSIManager(void);

/* Hardware Abstraction Layer Interface */

/* Hardware abstraction callbacks */

/* Register hardware abstraction layer */
OSErr SCSIRegisterHAL(SCSIHardwareCallbacks *callbacks, SCSIHardwareInfo *info);

#ifdef __cplusplus
}
#endif

#endif /* __SCSIMANAGER_H__ */