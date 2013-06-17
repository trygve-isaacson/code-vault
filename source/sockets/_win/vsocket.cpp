/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocket.h"
#include "vtypes_internal.h"

#include "vexception.h"

V_STATIC_INIT_TRACE

// This is to force our staticInit to be called at startup.
bool VSocket::gStaticInited = VSocket::staticInit();

bool VSocket::staticInit() {
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

VNetworkInterfaceList VSocketBase::enumerateNetworkInterfaces() {
    VNetworkInterfaceList interfaces;

    SOCKET sock = ::WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0);
    if (sock == SOCKET_ERROR) {
        throw VException(VSystemError::getSocketError(), "VSocketBase::enumerateNetworkInterfaces: WSASocket failed.");
    }

    INTERFACE_INFO interfaceInfo[20];
    unsigned long numBytesReturned;
    int result = ::WSAIoctl(sock, SIO_GET_INTERFACE_LIST, 0, 0, &interfaceInfo, sizeof(interfaceInfo), &numBytesReturned, 0, 0);
    if (result == SOCKET_ERROR) {
        throw VException(VSystemError::getSocketError(), "VSocketBase::enumerateNetworkInterfaces: WSAIoctl failed.");
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
VString VSocketBase::addrinfoToIPAddressString(const VString& hostName, const struct addrinfo* info) {
    VString result;
    result.preflight(MAX_ADDRSTRLEN);
    
    // WSAAddressToString() works for both IPv4 and IPv6 and is available on "older" versions of Windows.
    DWORD bufferLength = MAX_ADDRSTRLEN;
    int resultCode = ::WSAAddressToStringA(info->ai_addr, (DWORD) info->ai_addrlen, NULL, result.buffer(), &bufferLength);
    if (resultCode != 0) {
        throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocketBase::addrinfoToIPAddressString(%s): WSAAddressToString() failed.", hostName.chars()));
    }
    result.postflight(bufferLength-1);

    return result;
}

VSocket::VSocket()
    : VSocketBase()
    {
}

VSocket::VSocket(VSocketID id)
    : VSocketBase(id)
    {
}

VSocket::~VSocket() {
    // Note: base class destructor does a close() of the socket if it is open.
}

void VSocket::_connectToIPAddress(const VString& ipAddress, int portNumber) {
    this->setHostIPAddressAndPort(ipAddress, portNumber);

    int                 length;
    struct sockaddr_in  address;
    VSocketID           socketID;

    ::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = (u_short) V_BYTESWAP_HTON_S16_GET(static_cast<Vs16>(portNumber));
    address.sin_addr.s_addr = ::inet_addr(ipAddress);
    length = sizeof(struct sockaddr_in);

    socketID = ::socket(AF_INET, SOCK_STREAM, 0);

    if (socketID != kNoSocketID) {
        if (::connect(socketID, (const sockaddr*) &address, length) != 0) {
            // failure
            VSystemError e = VSystemError::getSocketError(); // Call before calling closeSocket(), which will succeed and clear the error code!
            vault::closeSocket(socketID);
            throw VException(e, VSTRING_FORMAT("VSocket[%s] _connectToIPAddress: Connect failed.", mSocketName.chars()));
        }
    }

    mSocketID = socketID;
}

void VSocket::_listen(const VString& bindAddress, int backlog) {
    // TODO: This function is now identical to the _unix version. Consolidate.
    VSocketID           listenSockID = kNoSocketID;
    struct sockaddr_in  info;
    int                 infoLength = sizeof(info);
    const int           on = 1;

    ::memset(&info, 0, sizeof(info));
    info.sin_family = AF_INET;
    info.sin_port = (u_short) V_BYTESWAP_HTON_S16_GET(static_cast<Vs16>(mPortNumber));

    if (bindAddress.isEmpty()) {
        info.sin_addr.s_addr = INADDR_ANY;
    } else {
        info.sin_addr.s_addr = inet_addr(bindAddress);
    }

    listenSockID = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSockID <= 0) {
        throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket::listen socket() failed with id %d.", listenSockID));
    }

    // Once we've successfully called ::socket(), if something else fails here, we need
    // to close that socket. We can just throw upon any failed call, and use a try/catch
    // with re-throw after closure.
    try {
        int result = ::setsockopt(listenSockID, SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof(on));
        if (result != 0) {
            throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket::listen setsockopt() failed -- socket id %d, result %d.", listenSockID, result));
        }

        result = ::bind(listenSockID, (const sockaddr*) &info, infoLength);
        if (result != 0) {
            vault::closeSocket(listenSockID);
            throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket::listen bind() for port %d failed -- socket id %d.", mPortNumber, listenSockID));
        }

        result = ::listen(listenSockID, backlog);
        if (result != 0) {
            throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket::listen listen() for port %d failed -- socket id %d, result %d.", mPortNumber, listenSockID, result));
        }

    } catch (...) {
        vault::closeSocket(listenSockID);
        throw;
    }

    mSocketID = listenSockID;
}

int VSocket::available() {
    u_long numBytesAvailable = 0;

    int result = ::ioctlsocket(mSocketID, FIONREAD, &numBytesAvailable);

    if (result != 0) {
        throw VStackTraceException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] available: ioctlsocket() failed with result %d.", mSocketName.chars(), result));
    }

    if (numBytesAvailable == 0) {
        //set the socket to be non blocking.
        u_long argp = 1;
        ::ioctlsocket(mSocketID, FIONBIO, &argp);

        //See if there is any data in the buffer.
        char recvBuffer[4]; // It's unclear if NULL buffer parameter is runtime safe, even with 0 length parameter. Provide valid buffer pointer to be sure.
        result = ::recv(mSocketID, recvBuffer, 0 , MSG_PEEK);
        VSystemError theSocketError = VSystemError::getSocketError();

        //restore the blocking
        argp = 1;
        ::ioctlsocket(mSocketID, FIONBIO, &argp);

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

int VSocket::read(Vu8* buffer, int numBytesToRead) {
    if (mSocketID == kNoSocketID) {
        throw VException(VSTRING_FORMAT("VSocket::read with invalid mSocketID %d", mSocketID));
    }

    int     bytesRemainingToRead = numBytesToRead;
    Vu8*    nextBufferPositionPtr = buffer;
    int     result;
    int     numBytesRead = 0;
    fd_set  readset;

    while (bytesRemainingToRead > 0) {
        // Note that unlike on Unix, verifying mSocketID <= FD_SETSIZE here is inappropriate because
        // of different Winsock fd_set internals, ID range, and select() API behavior. mSocketID may well be
        // larger than FD_SETSIZE, and that is OK.

        FD_ZERO(&readset);
        FD_SET(mSocketID, &readset);
        result = ::select((int)(mSocketID + 1), &readset, NULL, NULL, (mReadTimeOutActive ? &mReadTimeOut : NULL));

        if (result < 0) {
            if (errno == EINTR) {
                // Debug message: read was interrupted but we will cycle around and try again...
                continue;
            }

            throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket::read select failed on socket %d with result = %d.", mSocketID, result));
        } else if (result == 0) {
            throw VException(VSTRING_FORMAT("VSocket::read select timed out on socket %d.", mSocketID));
        }

        if (!FD_ISSET(mSocketID, &readset)) {
            throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket::read select got FD_ISSET false on socket %d.", mSocketID));
        }

        numBytesRead = ::recv(mSocketID, (char*) nextBufferPositionPtr, bytesRemainingToRead, 0);

        if (numBytesRead < 0) {
            throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket::read recv failed on socket %d with result %d.", mSocketID, numBytesRead));
        } else if (numBytesRead == 0) {
            if (mRequireReadAll) {
                throw VEOFException(VSTRING_FORMAT("VSocket::read: reached EOF unexpectedly on socket %d, recv of length %d returned 0.", mSocketID, bytesRemainingToRead));
            } else {
                break;    // got successful but partial read, caller will have to keep reading
            }
        }

        bytesRemainingToRead -= numBytesRead;
        nextBufferPositionPtr += numBytesRead;

        mNumBytesRead += numBytesRead;
    }

    mLastEventTime.setNow();

    return (numBytesToRead - bytesRemainingToRead);
}

int VSocket::write(const Vu8* buffer, int numBytesToWrite) {
    if (mSocketID == kNoSocketID) {
        throw VException(VSTRING_FORMAT("VSocket::write with invalid mSocketID %d", mSocketID));
    }

    const Vu8*  nextBufferPositionPtr = buffer;
    int         bytesRemainingToWrite = numBytesToWrite;
    int         numBytesWritten;
    int         result;
    fd_set      writeset;

    while (bytesRemainingToWrite > 0) {
        // See comment at same location in read() above, regarding mSocketID and FD_SETSIZE.

        FD_ZERO(&writeset);
        FD_SET(mSocketID, &writeset);
        result = ::select((int)(mSocketID + 1), NULL, &writeset, NULL, (mWriteTimeOutActive ? &mWriteTimeOut : NULL));

        if (result < 0) {
            if (errno == EINTR) {
                // Debug message: write was interrupted but we will cycle around and try again...
                continue;
            }

            throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket::write select failed on socket %d, result=%d.", mSocketID, result));
        } else if (result == 0) {
            throw VException(VSTRING_FORMAT("VSocket::write select timed out on socket %d", mSocketID));
        }

        numBytesWritten = ::send(mSocketID, (const char*) nextBufferPositionPtr, bytesRemainingToWrite, 0);

        if (numBytesWritten <= 0) {
            throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket::write send failed on socket %d.", mSocketID));
        } else if (numBytesWritten != bytesRemainingToWrite) {
            // Debug message: write was only partially completed so we will cycle around and write the rest...
        } else {
            // This is where you could put debug/trace-mode socket write logging output....
        }

        bytesRemainingToWrite -= numBytesWritten;
        nextBufferPositionPtr += numBytesWritten;

        mNumBytesWritten += numBytesWritten;
    }

    return (numBytesToWrite - bytesRemainingToWrite);
}

void VSocket::discoverHostAndPort() {
    struct sockaddr_in  info;
    VSocklenT           infoLength = sizeof(info);

    ::getpeername(mSocketID, (struct sockaddr*) &info, &infoLength);

    int portNumber = (int) V_BYTESWAP_NTOH_S16_GET(static_cast<Vs16>(info.sin_port));
    const char* ipAddress = ::inet_ntoa(info.sin_addr);

    this->setHostIPAddressAndPort(VSTRING_COPY(ipAddress), portNumber);
}

void VSocket::closeRead() {
    int result = ::shutdown(mSocketID, SHUT_RD);

    if (result < 0) {
        throw VException(VSTRING_FORMAT("VSocket::closeRead unable to shut down socket %d.", mSocketID));
    }
}

void VSocket::closeWrite() {
    int result = ::shutdown(mSocketID, SHUT_WR);

    if (result < 0) {
        throw VException(VSTRING_FORMAT("VSocket::closeWrite unable to shut down socket %d.", mSocketID));
    }
}

void VSocket::setSockOpt(int level, int name, void* valuePtr, int valueLength) {
    ::setsockopt(mSocketID, level, name, (char*) valuePtr, valueLength);
}

