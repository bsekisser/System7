/*
 * ProtocolStack.h
 * Protocol Stack API - Portable System 7.1 Implementation
 *
 * Provides communication protocol implementation including TCP/IP,
 * SSH, Telnet, and other network protocols for modern connectivity.
 *
 * Extends Mac OS Communication Toolbox with modern protocol support.
 */

#ifndef PROTOCOLSTACK_H
#define PROTOCOLSTACK_H

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "CommToolbox.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Protocol Stack Version */
#define PROTOCOL_STACK_VERSION 1

/* Protocol Types */

/* Protocol Status */

/* Protocol Errors */

/* Address Family Types */

/* Socket Types */

/* Network Address Structure */

        struct {
            unsigned char addr[16]; /* IPv6 address */
            unsigned short port;    /* Port number */
            long flowInfo;          /* Flow information */
            long scopeID;           /* Scope ID */
        } ipv6;
        struct {
            Str255 portName;        /* Serial port name */
            long baudRate;          /* Baud rate */
        } serial;
        struct {
            Str255 path;            /* Local socket path */
        } local;
    } address;
} NetworkAddress;

/* Protocol Configuration */

/* Protocol Statistics */

/* Protocol Handle */
/* Handle is defined in MacTypes.h */

/* Callback procedures */

#if GENERATINGCFM
pascal ProtocolConnectUPP NewProtocolConnectProc(ProtocolConnectProcPtr userRoutine);
pascal ProtocolDataUPP NewProtocolDataProc(ProtocolDataProcPtr userRoutine);
pascal ProtocolErrorUPP NewProtocolErrorProc(ProtocolErrorProcPtr userRoutine);
pascal ProtocolStatusUPP NewProtocolStatusProc(ProtocolStatusProcPtr userRoutine);
pascal void DisposeProtocolConnectUPP(ProtocolConnectUPP userUPP);
pascal void DisposeProtocolDataUPP(ProtocolDataUPP userUPP);
pascal void DisposeProtocolErrorUPP(ProtocolErrorUPP userUPP);
pascal void DisposeProtocolStatusUPP(ProtocolStatusUPP userUPP);
#else
#define NewProtocolConnectProc(userRoutine) (userRoutine)
#define NewProtocolDataProc(userRoutine) (userRoutine)
#define NewProtocolErrorProc(userRoutine) (userRoutine)
#define NewProtocolStatusProc(userRoutine) (userRoutine)
#define DisposeProtocolConnectUPP(userUPP)
#define DisposeProtocolDataUPP(userUPP)
#define DisposeProtocolErrorUPP(userUPP)
#define DisposeProtocolStatusUPP(userUPP)
#endif

/* Protocol Stack API */

/* Initialization */
pascal OSErr InitProtocolStack(void);

/* Protocol Management */
pascal OSErr ProtocolCreate(short protocolType, const ProtocolConfig *config, ProtocolHandle *hProtocol);
pascal OSErr ProtocolDispose(ProtocolHandle hProtocol);

/* Connection Management */
pascal OSErr ProtocolConnect(ProtocolHandle hProtocol, Boolean async, ProtocolConnectUPP callback, long refCon);
pascal OSErr ProtocolListen(ProtocolHandle hProtocol, short backlog, ProtocolConnectUPP callback, long refCon);
pascal OSErr ProtocolAccept(ProtocolHandle hProtocol, ProtocolHandle *newProtocol);
pascal OSErr ProtocolDisconnect(ProtocolHandle hProtocol, Boolean graceful);

/* Data Transfer */
pascal OSErr ProtocolSend(ProtocolHandle hProtocol, const void *data, long *size);
pascal OSErr ProtocolReceive(ProtocolHandle hProtocol, void *buffer, long *size);
pascal OSErr ProtocolSendTo(ProtocolHandle hProtocol, const void *data, long size, const NetworkAddress *addr);
pascal OSErr ProtocolReceiveFrom(ProtocolHandle hProtocol, void *buffer, long *size, NetworkAddress *addr);

/* Asynchronous Operations */
pascal OSErr ProtocolSendAsync(ProtocolHandle hProtocol, const void *data, long size,
                              ProtocolDataUPP callback, long refCon);
pascal OSErr ProtocolReceiveAsync(ProtocolHandle hProtocol, void *buffer, long size,
                                 ProtocolDataUPP callback, long refCon);

/* Status and Control */
pascal OSErr ProtocolGetStatus(ProtocolHandle hProtocol, short *status);
pascal OSErr ProtocolGetStats(ProtocolHandle hProtocol, ProtocolStats *stats);
pascal OSErr ProtocolSetConfig(ProtocolHandle hProtocol, const ProtocolConfig *config);
pascal OSErr ProtocolGetConfig(ProtocolHandle hProtocol, ProtocolConfig *config);

/* Event Handling */
pascal OSErr ProtocolSetCallbacks(ProtocolHandle hProtocol, ProtocolDataUPP dataCallback,
                                 ProtocolErrorUPP errorCallback, ProtocolStatusUPP statusCallback,
                                 long refCon);
pascal OSErr ProtocolProcessEvents(ProtocolHandle hProtocol);

/* TCP/IP Specific Functions */

/* TCP Socket Options */
pascal OSErr TCPSetNoDelay(ProtocolHandle hProtocol, Boolean enable);
pascal OSErr TCPSetKeepAlive(ProtocolHandle hProtocol, Boolean enable, long interval);
pascal OSErr TCPSetLinger(ProtocolHandle hProtocol, Boolean enable, short timeout);
pascal OSErr TCPGetPeerAddress(ProtocolHandle hProtocol, NetworkAddress *addr);
pascal OSErr TCPGetLocalAddress(ProtocolHandle hProtocol, NetworkAddress *addr);

/* UDP Socket Functions */
pascal OSErr UDPSetBroadcast(ProtocolHandle hProtocol, Boolean enable);
pascal OSErr UDPJoinMulticast(ProtocolHandle hProtocol, const NetworkAddress *group);
pascal OSErr UDPLeaveMulticast(ProtocolHandle hProtocol, const NetworkAddress *group);

/* SSL/TLS Support */

pascal OSErr ProtocolSetSSLConfig(ProtocolHandle hProtocol, const SSLConfig *config);
pascal OSErr ProtocolStartTLS(ProtocolHandle hProtocol);
pascal OSErr ProtocolGetSSLInfo(ProtocolHandle hProtocol, Str255 cipher, short *strength);

/* SSH Support */

pascal OSErr ProtocolSetSSHConfig(ProtocolHandle hProtocol, const SSHConfig *config);
pascal OSErr SSHExecuteCommand(ProtocolHandle hProtocol, ConstStr255Param command,
                              void *output, long *outputSize);
pascal OSErr SSHStartShell(ProtocolHandle hProtocol);
pascal OSErr SSHForwardPort(ProtocolHandle hProtocol, short localPort, ConstStr255Param remoteHost, short remotePort);

/* HTTP/HTTPS Support */

pascal OSErr HTTPSendRequest(ProtocolHandle hProtocol, const HTTPRequest *request);
pascal OSErr HTTPReceiveResponse(ProtocolHandle hProtocol, HTTPResponse *response);
pascal OSErr HTTPSetHeader(Handle headers, ConstStr255Param name, ConstStr255Param value);
pascal OSErr HTTPGetHeader(Handle headers, ConstStr255Param name, Str255 value);

/* FTP Support */
pascal OSErr FTPLogin(ProtocolHandle hProtocol, ConstStr255Param username, ConstStr255Param password);
pascal OSErr FTPChangeDirectory(ProtocolHandle hProtocol, ConstStr255Param path);
pascal OSErr FTPListDirectory(ProtocolHandle hProtocol, Handle *listing);
pascal OSErr FTPUploadFile(ProtocolHandle hProtocol, ConstStr255Param localFile, ConstStr255Param remoteName);
pascal OSErr FTPDownloadFile(ProtocolHandle hProtocol, ConstStr255Param remoteName, ConstStr255Param localFile);
pascal OSErr FTPDeleteFile(ProtocolHandle hProtocol, ConstStr255Param fileName);

/* Telnet Support */

pascal OSErr TelnetSetOptions(ProtocolHandle hProtocol, const TelnetOptions *options);
pascal OSErr TelnetSendCommand(ProtocolHandle hProtocol, unsigned char command, unsigned char option);
pascal OSErr TelnetSendSubnegotiation(ProtocolHandle hProtocol, unsigned char option,
                                     const void *data, short length);

/* Protocol Resolution */
pascal OSErr ResolveHostname(ConstStr255Param hostname, NetworkAddress addresses[], short *count);
pascal OSErr GetHostname(ConstStr255Param address, Str255 hostname);
pascal OSErr GetServicePort(ConstStr255Param service, ConstStr255Param protocol, short *port);

/* Network Interface Information */

pascal OSErr GetNetworkInterfaces(NetworkInterface interfaces[], short *count);
pascal OSErr GetDefaultGateway(NetworkAddress *gateway);

/* Advanced Features */
pascal OSErr ProtocolSetTimeout(ProtocolHandle hProtocol, long timeout);
pascal OSErr ProtocolGetTimeout(ProtocolHandle hProtocol, long *timeout);
pascal OSErr ProtocolSetBufferSize(ProtocolHandle hProtocol, long sendSize, long receiveSize);
pascal OSErr ProtocolFlushBuffers(ProtocolHandle hProtocol);

/* Quality of Service */

pascal OSErr ProtocolSetQoS(ProtocolHandle hProtocol, const QoSConfig *qos);

/* Thread Safety */
pascal OSErr ProtocolLock(ProtocolHandle hProtocol);
pascal OSErr ProtocolUnlock(ProtocolHandle hProtocol);

/* Reference Management */
pascal long ProtocolGetRefCon(ProtocolHandle hProtocol);
pascal void ProtocolSetRefCon(ProtocolHandle hProtocol, long refCon);

/* Debugging and Logging */
pascal OSErr ProtocolStartPacketCapture(ProtocolHandle hProtocol, ConstStr255Param fileName);
pascal OSErr ProtocolStopPacketCapture(ProtocolHandle hProtocol);
pascal OSErr ProtocolSetLogLevel(short level);

/* Log levels */

/* SSL/TLS Versions */

/* SSH Versions */

#ifdef __cplusplus
}
#endif

#endif /* PROTOCOLSTACK_H */