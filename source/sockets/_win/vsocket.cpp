/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocket.h"
#include "vtypes_internal.h"

#include "vexception.h"

#include <ws2tcpip.h> // For the WSA calls in enumerateNetworkInterfaces()

V_STATIC_INIT_TRACE

// This is to force our staticInit to be called at startup.
bool VSocket::gStaticInited = VSocket::staticInit();

bool VSocket::staticInit()
    {
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

VNetworkInterfaceList VSocketBase::enumerateNetworkInterfaces()
    {
    VNetworkInterfaceList interfaces;

    SOCKET sock = ::WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0);
    if (sock == SOCKET_ERROR)
        throw VException(VString("VSocketBase::enumerateNetworkInterfaces: WSASocket failed with error code %d.", ::WSAGetLastError()));

    INTERFACE_INFO interfaceInfo[20];
    unsigned long numBytesReturned;
    int result = ::WSAIoctl(sock, SIO_GET_INTERFACE_LIST, 0, 0, &interfaceInfo, sizeof(interfaceInfo), &numBytesReturned, 0, 0);
    if (result == SOCKET_ERROR)
        throw VException(VString("VSocketBase::enumerateNetworkInterfaces: WSAIoctl failed with error code %d.", ::WSAGetLastError()));

    int numInterfaces = numBytesReturned / sizeof(INTERFACE_INFO);
    for (int i = 0; i < numInterfaces; ++i)
        {
        // Filter out 127.x.x.x (loopback addresses).
        VString address(::inet_ntoa(((sockaddr_in *) &(interfaceInfo[i].iiAddress))->sin_addr));
        if (! address.startsWith("127."))
            {
            VNetworkInterfaceInfo info;
            info.mAddress = address;
            interfaces.push_back(info);
            }
        }

    return interfaces;
    }

VSocket::VSocket(VSocketID id) :
VSocketBase(id)
    {
    }

VSocket::VSocket(const VString& hostName, int portNumber) :
VSocketBase(hostName, portNumber)
    {
    }

VSocket::~VSocket()
    {
    // Note: base class destructor does a close() of the socket if it is open.
    }

void VSocket::_connect()
    {
    int                 length;
    struct sockaddr_in  address;
    VSocketID           socketID;

    ::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = (u_short) V_BYTESWAP_HTON_S16_GET(static_cast<Vs16>(mPortNumber));
    address.sin_addr.s_addr = ::inet_addr(mHostName);
    length = sizeof(struct sockaddr_in);

    socketID = ::socket(AF_INET, SOCK_STREAM, 0);

    if (socketID != kNoSocketID)
        {
        if (::connect(socketID, (const sockaddr*) &address, length) != 0)
            {
            // failure
            ::closesocket(socketID);
            throw VException(VString("VSocket::connect was unable to connect to %s:%d -- socket %d, errno=%s", mHostName.chars(), mPortNumber, socketID, ::strerror(errno)));
            }
        }

    mSocketID = socketID;
    }

void VSocket::_listen(const VString& bindAddress, int backlog)
    {
    VSocketID           listenSockID = kNoSocketID;
    struct sockaddr_in  info;
    int                 infoLength = sizeof(info);
    const int           on = 1;
    int                 result;

    ::memset(&info, 0, sizeof(info));
    info.sin_family = AF_INET;
    info.sin_port = (u_short) V_BYTESWAP_HTON_S16_GET(static_cast<Vs16>(mPortNumber));

    if (bindAddress.isEmpty())
        info.sin_addr.s_addr = INADDR_ANY;
    else
        info.sin_addr.s_addr = inet_addr(bindAddress);

    listenSockID = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSockID <= 0)
        throw VException(VString("VSocket::listen socket() failed with id %d, errno=%d", listenSockID, ::WSAGetLastError()));

    result = ::setsockopt(listenSockID, SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof(on));
    if (result != 0)
        throw VException(VString("VSocket::listen setsockopt() failed -- socket id %d, result %d, errno=%d", listenSockID, result, ::WSAGetLastError()));

    result = ::bind(listenSockID, (const sockaddr*) &info, infoLength);
    if (result != 0)
        {
        // Bind failed.
        ::closesocket(listenSockID);
        throw VException(VString("VSocket::listen bind() for port %d failed -- socket id %d, errno=%d", mPortNumber, listenSockID, ::WSAGetLastError()));
        }

    result = ::listen(listenSockID, backlog);
    if (result != 0)
        throw VException(VString("VSocket::listen listen() for port %d failed -- socket id %d, result %d, errno=%d", mPortNumber, listenSockID, result, ::WSAGetLastError()));

    mSocketID = listenSockID;
    }

int VSocket::available()
    {
    u_long numBytesAvailable = 0;

    int result = ::ioctlsocket(mSocketID, FIONREAD, &numBytesAvailable);

    if (result != 0)
        throw VException(VString("VSocket::available failed on socket %d, result=%d, error=%s.", mSocketID, result, ::strerror(errno)));

    if (numBytesAvailable == 0)
        {
        //set the socket to be non blocking.
        u_long argp = 1;
        ::ioctlsocket(mSocketID, FIONBIO, &argp);

        //See if there is any data in the buffer.
        result = ::recv(mSocketID, NULL, 0 , MSG_PEEK);

        //restore the blocking
        argp = 1;
        ::ioctlsocket(mSocketID, FIONBIO, &argp);

        switch (result)
            {
            case 0:
            case WSAECONNRESET:
                throw VEOFException("VSocket::available: socket is now available.");
                break;
            case SOCKET_ERROR:
                throw VException(VString("VSocket::available failed on socket %d, result=%d, error=%s.", mSocketID, result, ::strerror(errno)));
                break;
            default:
                break;
            }
        }

    return (int) numBytesAvailable;
    }

int VSocket::read(Vu8* buffer, int numBytesToRead)
    {
    if (mSocketID == kNoSocketID)
        throw VException(VString("VSocket::read with invalid mSocketID %d", mSocketID));

    int     bytesRemainingToRead = numBytesToRead;
    Vu8*    nextBufferPositionPtr = buffer;
    int     result;
    int     numBytesRead = 0;
    fd_set  readset;

    while (bytesRemainingToRead > 0)
        {
        if (mSocketID <= FD_SETSIZE) // FD_SETSIZE is max num open sockets, sockid over that is sign of a big problem
            {
            FD_ZERO(&readset);
            FD_SET(mSocketID, &readset);
            result = ::select((int) (mSocketID + 1), &readset, NULL, NULL, (mReadTimeOutActive ? &mReadTimeOut : NULL));

            if (result < 0)
                {
                if (errno == EINTR)
                    {
                    // Debug message: read was interrupted but we will cycle around and try again...
                    continue;
                    }

                throw VException(VString("VSocket::read select failed on socket %d, result=%d, error=%s.", mSocketID, result, ::strerror(errno)));
                }
            else if (result == 0)
                {
                throw VException(VString("VSocket::read select timed out on socket %d.", mSocketID));
                }

            if (!FD_ISSET(mSocketID, &readset))
                {
                throw VException(VString("VSocket::read select got FD_ISSET false on socket %d, errno=%s.", mSocketID, ::strerror(errno)));
                }

            }

        numBytesRead = ::recv(mSocketID, (char*) nextBufferPositionPtr, bytesRemainingToRead, 0);

        if (numBytesRead < 0)
            {
            throw VException(VString("VSocket::read recv failed on socket %d, result=%d, errno=%s.", mSocketID, numBytesRead, ::strerror(errno)));
            }
        else if (numBytesRead == 0)
            {
            if (mRequireReadAll)
                {
                throw VEOFException(VString("VSocket::read: reached EOF unexpectedly on socket %d, recv of length %d returned 0, errno=%s.", mSocketID, bytesRemainingToRead, ::strerror(errno)));
                }
            else
                {
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

int VSocket::write(const Vu8* buffer, int numBytesToWrite)
    {
    if (mSocketID == kNoSocketID)
        throw VException(VString("VSocket::write with invalid mSocketID %d", mSocketID));

    const Vu8*  nextBufferPositionPtr = buffer;
    int         bytesRemainingToWrite = numBytesToWrite;
    int         numBytesWritten;
    int         result;
    fd_set      writeset;

    while (bytesRemainingToWrite > 0)
        {
        if (mSocketID <= FD_SETSIZE) // FD_SETSIZE is max num open sockets, sockid over that is sign of a big problem
            {
            FD_ZERO(&writeset);
            FD_SET(mSocketID, &writeset);
            result = ::select((int) (mSocketID + 1), NULL, &writeset, NULL, (mWriteTimeOutActive ? &mWriteTimeOut : NULL));

            if (result < 0)
                {
                if (errno == EINTR)
                    {
                    // Debug message: write was interrupted but we will cycle around and try again...
                    continue;
                    }

                throw VException(VString("VSocket::write select failed on socket %d, result=%d, errno=%s.", mSocketID, result, ::strerror(errno)));
                }
            else if (result == 0)
                {
                throw VException(VString("VSocket::write select timed out on socket %d", mSocketID));
                }

            }

        numBytesWritten = ::send(mSocketID, (const char*) nextBufferPositionPtr, bytesRemainingToWrite, 0);

        if (numBytesWritten <= 0)
            {
            throw VException(VString("VSocket::write send failed on socket %d, errno=%s.", mSocketID, ::strerror(errno)));
            }
        else if (numBytesWritten != bytesRemainingToWrite)
            {
            // Debug message: write was only partially completed so we will cycle around and write the rest...
            }
        else
            {
            // This is where you could put debug/trace-mode socket write logging output....
            }

        bytesRemainingToWrite -= numBytesWritten;
        nextBufferPositionPtr += numBytesWritten;

        mNumBytesWritten += numBytesWritten;
        }

    return (numBytesToWrite - bytesRemainingToWrite);
    }

void VSocket::discoverHostAndPort()
    {
    VSocklenT           namelen = sizeof(struct sockaddr_in);
    struct sockaddr_in  info;

    ::getpeername(mSocketID, (struct sockaddr*) &info, &namelen);

    int portNumber = (int) V_BYTESWAP_NTOH_S16_GET(static_cast<Vs16>(info.sin_port));
    char* name = ::inet_ntoa(info.sin_addr);

    this->setHostAndPort(name, portNumber);
    }

void VSocket::closeRead()
    {
    int result = ::shutdown(mSocketID, SHUT_RD);

    if (result < 0)
        throw VException(VString("VSocket::closeRead unable to shut down socket %d.", mSocketID));
    }

void VSocket::closeWrite()
    {
    int result = ::shutdown(mSocketID, SHUT_WR);

    if (result < 0)
        throw VException(VString("VSocket::closeWrite unable to shut down socket %d.", mSocketID));
    }

void VSocket::setSockOpt(int level, int name, void* valuePtr, int valueLength)
    {
    ::setsockopt(mSocketID, level, name, (char*) valuePtr, valueLength);
    }

