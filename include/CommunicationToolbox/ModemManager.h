/*
 * ModemManager.h
 * Modem Management API - Portable System 7.1 Implementation
 *
 * Provides modem control, AT command processing, dial-up networking,
 * and modern cellular modem support.
 *
 * Extends Mac OS Communication Toolbox with modern modem functionality.
 */

#ifndef MODEMMANAGER_H
#define MODEMMANAGER_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "CommToolbox.h"
#include "SerialManager.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Modem Manager Version */
#define MODEM_MANAGER_VERSION 1

/* Modem Types */

/* Modem Status */

/* AT Command Response Types */

/* Connection Types */

/* Modem Configuration */

/* Modem Information */

/* Connection Information */

/* Cellular Modem Information */

/* Modem Handle */
/* Handle is defined in MacTypes.h */

/* Callback procedures */

#if GENERATINGCFM
pascal ModemConnectionUPP NewModemConnectionProc(ModemConnectionProcPtr userRoutine);
pascal ModemDataReceivedUPP NewModemDataReceivedProc(ModemDataReceivedProcPtr userRoutine);
pascal ModemATResponseUPP NewModemATResponseProc(ModemATResponseProcPtr userRoutine);
pascal void DisposeModemConnectionUPP(ModemConnectionUPP userUPP);
pascal void DisposeModemDataReceivedUPP(ModemDataReceivedUPP userUPP);
pascal void DisposeModemATResponseUPP(ModemATResponseUPP userUPP);
#else
#define NewModemConnectionProc(userRoutine) (userRoutine)
#define NewModemDataReceivedProc(userRoutine) (userRoutine)
#define NewModemATResponseProc(userRoutine) (userRoutine)
#define DisposeModemConnectionUPP(userUPP)
#define DisposeModemDataReceivedUPP(userUPP)
#define DisposeModemATResponseUPP(userUPP)
#endif

/* Modem Manager API */

/* Initialization */
pascal OSErr InitModemManager(void);

/* Modem Management */
pascal OSErr ModemOpen(ConstStr255Param portName, const ModemConfig *config, ModemHandle *hModem);
pascal OSErr ModemClose(ModemHandle hModem);
pascal Boolean ModemIsOpen(ModemHandle hModem);

/* Configuration */
pascal OSErr ModemSetConfig(ModemHandle hModem, const ModemConfig *config);
pascal OSErr ModemGetConfig(ModemHandle hModem, ModemConfig *config);
pascal OSErr ModemGetInfo(ModemHandle hModem, ModemInfo *info);

/* AT Command Interface */
pascal OSErr ModemSendATCommand(ModemHandle hModem, ConstStr255Param command, Str255 response);
pascal OSErr ModemSendATCommandAsync(ModemHandle hModem, ConstStr255Param command,
                                    ModemATResponseUPP callback, long refCon);
pascal OSErr ModemWaitForResponse(ModemHandle hModem, Str255 response, long timeout);
pascal OSErr ModemFlushResponses(ModemHandle hModem);

/* Connection Management */
pascal OSErr ModemDial(ModemHandle hModem, ConstStr255Param phoneNumber,
                      ModemConnectionUPP callback, long refCon);
pascal OSErr ModemHangup(ModemHandle hModem);
pascal OSErr ModemAnswer(ModemHandle hModem, ModemConnectionUPP callback, long refCon);
pascal OSErr ModemGetConnectionInfo(ModemHandle hModem, ConnectionInfo *info);

/* Data Transfer */
pascal OSErr ModemRead(ModemHandle hModem, void *buffer, long *count);
pascal OSErr ModemWrite(ModemHandle hModem, const void *buffer, long *count);
pascal OSErr ModemSetDataCallback(ModemHandle hModem, ModemDataReceivedUPP callback, long refCon);

/* Status and Control */
pascal OSErr ModemGetStatus(ModemHandle hModem, short *status);
pascal OSErr ModemSetSpeakerVolume(ModemHandle hModem, short volume);
pascal OSErr ModemSetAutoAnswer(ModemHandle hModem, short rings);
pascal OSErr ModemGetSignalQuality(ModemHandle hModem, short *quality);

/* Modern Extensions */

/* Cellular Modem Support */
pascal OSErr ModemGetCellularInfo(ModemHandle hModem, CellularInfo *info);
pascal OSErr ModemSetAPN(ModemHandle hModem, ConstStr255Param apn, ConstStr255Param username,
                        ConstStr255Param password);
pascal OSErr ModemActivatePDPContext(ModemHandle hModem);
pascal OSErr ModemDeactivatePDPContext(ModemHandle hModem);

/* SMS Support */

pascal OSErr ModemSendSMS(ModemHandle hModem, ConstStr255Param recipient, ConstStr255Param text);
pascal OSErr ModemReceiveSMS(ModemHandle hModem, SMSMessage messages[], short *count);
pascal OSErr ModemDeleteSMS(ModemHandle hModem, short index);

/* Voice Call Support */
pascal OSErr ModemMakeVoiceCall(ModemHandle hModem, ConstStr255Param phoneNumber);
pascal OSErr ModemAnswerVoiceCall(ModemHandle hModem);
pascal OSErr ModemHangupVoiceCall(ModemHandle hModem);
pascal OSErr ModemSetVoiceVolume(ModemHandle hModem, short volume);

/* GPS Support (for cellular modems) */

pascal OSErr ModemGetGPSPosition(ModemHandle hModem, GPSInfo *position);
pascal OSErr ModemEnableGPS(ModemHandle hModem, Boolean enable);

/* Network Registration */

pascal OSErr ModemGetNetworkInfo(ModemHandle hModem, NetworkInfo *info);
pascal OSErr ModemScanNetworks(ModemHandle hModem, NetworkInfo networks[], short *count);
pascal OSErr ModemSelectNetwork(ModemHandle hModem, ConstStr255Param operatorCode);

/* SIM Card Support */

pascal OSErr ModemGetSIMInfo(ModemHandle hModem, SIMInfo *info);
pascal OSErr ModemEnterPIN(ModemHandle hModem, ConstStr255Param pin);
pascal OSErr ModemChangePIN(ModemHandle hModem, ConstStr255Param oldPIN, ConstStr255Param newPIN);

/* Data Usage Monitoring */

pascal OSErr ModemGetDataUsage(ModemHandle hModem, DataUsage *usage);
pascal OSErr ModemResetDataUsage(ModemHandle hModem);

/* Profile Management */
pascal OSErr ModemSaveProfile(ModemHandle hModem, ConstStr255Param profileName);
pascal OSErr ModemLoadProfile(ModemHandle hModem, ConstStr255Param profileName);
pascal OSErr ModemDeleteProfile(ModemHandle hModem, ConstStr255Param profileName);
pascal OSErr ModemGetProfileList(ModemHandle hModem, Str255 profiles[], short *count);

/* Diagnostics */
pascal OSErr ModemRunSelfTest(ModemHandle hModem, short *result);
pascal OSErr ModemGetLastError(ModemHandle hModem, OSErr *error, Str255 description);
pascal OSErr ModemStartLogging(ModemHandle hModem, ConstStr255Param logFile);
pascal OSErr ModemStopLogging(ModemHandle hModem);

/* Advanced AT Commands */
pascal OSErr ModemQueryCapabilities(ModemHandle hModem, Str255 capabilities);
pascal OSErr ModemSetEchoMode(ModemHandle hModem, Boolean echo);
pascal OSErr ModemSetVerboseMode(ModemHandle hModem, Boolean verbose);
pascal OSErr ModemFactoryReset(ModemHandle hModem);

/* Thread Safety */
pascal OSErr ModemLock(ModemHandle hModem);
pascal OSErr ModemUnlock(ModemHandle hModem);

/* Reference Management */
pascal long ModemGetRefCon(ModemHandle hModem);
pascal void ModemSetRefCon(ModemHandle hModem, long refCon);

/* Constants for various enumerations */

/* Speaker volume levels */

/* Access technologies */

/* Registration status */

/* SIM status */

#ifdef __cplusplus
}
#endif

#endif /* MODEMMANAGER_H */