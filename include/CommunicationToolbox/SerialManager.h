/*
 * SerialManager.h
 * Serial Port Management API - Portable System 7.1 Implementation
 *
 * Provides cross-platform serial port management with support for
 * modern USB-to-serial adapters, Bluetooth serial, and virtual serial ports.
 *
 * Extends Mac OS Communication Toolbox with modern serial functionality.
 */

#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "CommToolbox.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Serial Manager Version */
#define SERIAL_MANAGER_VERSION 1

/* Serial Port Types */

/* Serial Port Status */

/* Flow Control Types */

/* Parity Types */

/* Stop Bits */

/* Data Bits */

/* Serial Port Configuration */

/* Serial Port Information */

/* Serial Port Statistics */

/* Serial Port Handle */
/* Handle is defined in MacTypes.h */

/* Callback procedures */

#if GENERATINGCFM
pascal SerialDataReceivedUPP NewSerialDataReceivedProc(SerialDataReceivedProcPtr userRoutine);
pascal SerialErrorUPP NewSerialErrorProc(SerialErrorProcPtr userRoutine);
pascal SerialSignalChangedUPP NewSerialSignalChangedProc(SerialSignalChangedProcPtr userRoutine);
pascal void DisposeSerialDataReceivedUPP(SerialDataReceivedUPP userUPP);
pascal void DisposeSerialErrorUPP(SerialErrorUPP userUPP);
pascal void DisposeSerialSignalChangedUPP(SerialSignalChangedUPP userUPP);
#else
#define NewSerialDataReceivedProc(userRoutine) (userRoutine)
#define NewSerialErrorProc(userRoutine) (userRoutine)
#define NewSerialSignalChangedProc(userRoutine) (userRoutine)
#define DisposeSerialDataReceivedUPP(userUPP)
#define DisposeSerialErrorUPP(userUPP)
#define DisposeSerialSignalChangedUPP(userUPP)
#endif

/* Serial Manager API */

/* Initialization */
pascal OSErr InitSerialManager(void);

/* Port Enumeration */
pascal OSErr SerialEnumeratePorts(SerialPortInfo ports[], short *count);
pascal OSErr SerialGetPortInfo(ConstStr255Param portName, SerialPortInfo *info);
pascal OSErr SerialRefreshPortList(void);

/* Port Management */
pascal OSErr SerialOpenPort(ConstStr255Param portName, const SerialConfig *config,
                           SerialPortHandle *hPort);
pascal OSErr SerialClosePort(SerialPortHandle hPort);
pascal Boolean SerialIsPortOpen(SerialPortHandle hPort);

/* Configuration */
pascal OSErr SerialSetConfig(SerialPortHandle hPort, const SerialConfig *config);
pascal OSErr SerialGetConfig(SerialPortHandle hPort, SerialConfig *config);
pascal OSErr SerialSetBaudRate(SerialPortHandle hPort, long baudRate);
pascal OSErr SerialSetDataFormat(SerialPortHandle hPort, short dataBits, short stopBits, short parity);
pascal OSErr SerialSetFlowControl(SerialPortHandle hPort, short flowControl);
pascal OSErr SerialSetTimeouts(SerialPortHandle hPort, long readTimeout, long writeTimeout);

/* Data Transfer */
pascal OSErr SerialRead(SerialPortHandle hPort, void *buffer, long *count);
pascal OSErr SerialWrite(SerialPortHandle hPort, const void *buffer, long *count);
pascal OSErr SerialReadAsync(SerialPortHandle hPort, void *buffer, long count,
                            SerialDataReceivedUPP callback, long refCon);
pascal OSErr SerialWriteAsync(SerialPortHandle hPort, const void *buffer, long count,
                             SerialDataReceivedUPP callback, long refCon);

/* Buffer Management */
pascal OSErr SerialFlushInput(SerialPortHandle hPort);
pascal OSErr SerialFlushOutput(SerialPortHandle hPort);
pascal OSErr SerialGetInputCount(SerialPortHandle hPort, long *count);
pascal OSErr SerialGetOutputCount(SerialPortHandle hPort, long *count);
pascal OSErr SerialSetBufferSizes(SerialPortHandle hPort, long inputSize, long outputSize);

/* Signal Control */
pascal OSErr SerialSetDTR(SerialPortHandle hPort, Boolean state);
pascal OSErr SerialSetRTS(SerialPortHandle hPort, Boolean state);
pascal OSErr SerialSetBreak(SerialPortHandle hPort, Boolean state);
pascal OSErr SerialGetSignals(SerialPortHandle hPort, long *signals);

/* Status and Statistics */
pascal OSErr SerialGetStatus(SerialPortHandle hPort, short *status);
pascal OSErr SerialGetStats(SerialPortHandle hPort, SerialStats *stats);
pascal OSErr SerialResetStats(SerialPortHandle hPort);

/* Event Handling */
pascal OSErr SerialSetCallbacks(SerialPortHandle hPort, SerialDataReceivedUPP dataCallback,
                               SerialErrorUPP errorCallback, SerialSignalChangedUPP signalCallback,
                               long refCon);
pascal OSErr SerialProcessEvents(SerialPortHandle hPort);

/* Modern Extensions */

/* USB Serial Support */

pascal OSErr SerialGetUSBInfo(SerialPortHandle hPort, USBSerialInfo *info);
pascal OSErr SerialEnumerateUSBPorts(USBSerialInfo ports[], short *count);

/* Bluetooth Serial Support */

pascal OSErr SerialGetBluetoothInfo(SerialPortHandle hPort, BluetoothSerialInfo *info);
pascal OSErr SerialEnumerateBluetoothPorts(BluetoothSerialInfo ports[], short *count);
pascal OSErr SerialPairBluetoothDevice(const BluetoothSerialInfo *info);

/* Virtual Serial Port Support */

pascal OSErr SerialCreateVirtualPort(ConstStr255Param portName, const VirtualSerialConfig *config);
pascal OSErr SerialDestroyVirtualPort(ConstStr255Param portName);
pascal OSErr SerialConnectVirtualPorts(ConstStr255Param port1, ConstStr255Param port2);

/* Network Serial Support */

pascal OSErr SerialOpenNetworkPort(const NetworkSerialConfig *config, SerialPortHandle *hPort);

/* Advanced Features */
pascal OSErr SerialSetCustomBaudRate(SerialPortHandle hPort, long baudRate);
pascal OSErr SerialGetSupportedBaudRates(SerialPortHandle hPort, long rates[], short *count);
pascal OSErr SerialSetLineCoding(SerialPortHandle hPort, const SerialConfig *config);
pascal OSErr SerialGetLineCoding(SerialPortHandle hPort, SerialConfig *config);

/* Error Recovery */
pascal OSErr SerialRecoverFromError(SerialPortHandle hPort);
pascal OSErr SerialGetLastError(SerialPortHandle hPort, OSErr *error, Str255 description);

/* Power Management */
pascal OSErr SerialSetPowerState(SerialPortHandle hPort, Boolean active);
pascal OSErr SerialGetPowerState(SerialPortHandle hPort, Boolean *active);

/* Thread Safety */
pascal OSErr SerialLockPort(SerialPortHandle hPort);
pascal OSErr SerialUnlockPort(SerialPortHandle hPort);

/* Debugging and Diagnostics */
pascal OSErr SerialStartLogging(SerialPortHandle hPort, ConstStr255Param logFile);
pascal OSErr SerialStopLogging(SerialPortHandle hPort);
pascal OSErr SerialRunDiagnostics(SerialPortHandle hPort, short *result);

/* Port Reference Management */
pascal long SerialGetRefCon(SerialPortHandle hPort);
pascal void SerialSetRefCon(SerialPortHandle hPort, long refCon);

/* Signal definitions for GetSignals/SetSignals */

#ifdef __cplusplus
}
#endif

#endif /* SERIALMANAGER_H */