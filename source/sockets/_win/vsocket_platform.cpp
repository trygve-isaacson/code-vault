/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocket.h"

#include "vexception.h"

// static
bool VSocket::_platform_staticInit() {
    bool    success = true;
    WORD    versionRequested;
    WSADATA wsaData;
    int     err;

    versionRequested = MAKEWORD(2, 0);

    err = ::WSAStartup(versionRequested, &wsaData);

    success = (err == 0);

    assert(success);

    return success;
}

// static
VNetworkInterfaceList VSocket::_platform_enumerateNetworkInterfaces() {
    VNetworkInterfaceList interfaces;

    SOCKET sock = ::WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0);
    if (sock == SOCKET_ERROR) {
        throw VException(VSystemError::getSocketError(), "VSocket::enumerateNetworkInterfaces: WSASocket failed.");
    }

    INTERFACE_INFO interfaceInfo[20];
    unsigned long numBytesReturned;
    int result = ::WSAIoctl(sock, SIO_GET_INTERFACE_LIST, 0, 0, &interfaceInfo, sizeof(interfaceInfo), &numBytesReturned, 0, 0);
    if (result == SOCKET_ERROR) {
        throw VException(VSystemError::getSocketError(), "VSocket::enumerateNetworkInterfaces: WSAIoctl failed.");
    }

    int numInterfaces = numBytesReturned / sizeof(INTERFACE_INFO);
    for (int i = 0; i < numInterfaces; ++i) {
        // Filter out 127.x.x.x (loopback addresses).
        VString address(::inet_ntoa(((sockaddr_in*) &(interfaceInfo[i].iiAddress))->sin_addr));
        if (! address.startsWith("127.")) {
            VNetworkInterfaceInfo info;
            info.mAddress = address;
            interfaces.push_back(info);
        }
    }

    return interfaces;
}

static const int MAX_ADDRSTRLEN = V_MAX(INET_ADDRSTRLEN, INET6_ADDRSTRLEN);
// static
VString VSocket::_platform_addrinfoToIPAddressString(const VString& hostName, const struct addrinfo* info) {
    VString result;
    result.preflight(MAX_ADDRSTRLEN);

    // WSAAddressToString() works for both IPv4 and IPv6 and is available on "older" versions of Windows.
    DWORD bufferLength = MAX_ADDRSTRLEN;
    int resultCode = ::WSAAddressToStringA(info->ai_addr, (DWORD) info->ai_addrlen, NULL, result.buffer(), &bufferLength);
    if (resultCode != 0) {
        throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket::_platform_addrinfoToIPAddressString(%s): WSAAddressToString() failed.", hostName.chars()));
    }
    result.postflight(bufferLength - 1);

    return result;
}

// static
bool VSocket::_platform_isSocketIDValid(VSocketID socketID) {
    return socketID != INVALID_SOCKET;
}

int VSocket::_platform_available() {
    u_long numBytesAvailable = 0;

    int result = ::v_ioctlsocket(mSocketID, FIONREAD, &numBytesAvailable);

    if (result != 0) {
        throw VStackTraceException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] available: v_ioctlsocket() failed with result %d.", mSocketName.chars(), result));
    }

    if (numBytesAvailable == 0) {
        //set the socket to be non blocking.
        u_long argp = 1;
        ::v_ioctlsocket(mSocketID, FIONBIO, &argp);

        //See if there is any data in the buffer.
        char recvBuffer[4]; // It's unclear if NULL buffer parameter is runtime safe, even with 0 length parameter. Provide valid buffer pointer to be sure.
        result = ::recv(mSocketID, recvBuffer, 0, MSG_PEEK);
        VSystemError theSocketError = VSystemError::getSocketError();

        //restore the blocking
        argp = 0;
        ::v_ioctlsocket(mSocketID, FIONBIO, &argp);

        switch (result) {
            case 0:
                throw VEOFException("VSocket::available: Peer closed connection gracefully.");
                break;
            case SOCKET_ERROR:
                if (theSocketError.getErrorCode() == WSAECONNRESET) {
                    throw VEOFException("VSocket::available: The socket is no longer available.");
                } else {
                    throw VException(theSocketError, VSTRING_FORMAT("VSocket::available failed on socket %d, with SOCKET_ERROR.", mSocketID));
                }
                break;
            default:
                numBytesAvailable = result;
                break;
        }
    }

    return (int) numBytesAvailable;
}

