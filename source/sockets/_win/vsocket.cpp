/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocket.h"

#include "vexception.h"

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
    address.sin_port = V_BYTESWAP_HTONS_GET(mPortNumber);
    address.sin_addr.s_addr = ::inet_addr(mHostName);
    length = sizeof(struct sockaddr_in);

    socketID = ::socket(AF_INET, SOCK_STREAM, 0);

    if (socketID >= 0)
        {
        if (::connect(socketID, (const sockaddr*) &address, length) != 0)
            {
            // failure
            ::closesocket(socketID);
            throw VException("VSocket::connect was unable to connect to %s:%d -- socket %d, errno=%s", mHostName.chars(), mPortNumber, socketID, ::strerror(errno));
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
    info.sin_port = V_BYTESWAP_HTONS_GET(mPortNumber);

    if (bindAddress.isEmpty())
        info.sin_addr.s_addr = INADDR_ANY;
    else
        info.sin_addr.s_addr = inet_addr(bindAddress);

    listenSockID = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSockID <= 0)
        throw VException("VSocket::listen socket() failed with id %d, errno=%d", listenSockID, ::WSAGetLastError());

    result = ::setsockopt(listenSockID, SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof(on));
    if (result != 0)
        throw VException("VSocket::listen setsockopt() failed -- socket id %d, result %d, errno=%d", listenSockID, result, ::WSAGetLastError());

    result = ::bind(listenSockID, (const sockaddr*) &info, infoLength);
    if (result != 0)
        {
        // Bind failed.
        ::closesocket(listenSockID);
        throw VException("VSocket::listen bind() for port %d failed -- socket id %d, errno=%d", mPortNumber, listenSockID, ::WSAGetLastError());
        }

    result = ::listen(listenSockID, backlog);
    if (result != 0)
        throw VException("VSocket::listen listen() for port %d failed -- socket id %d, result %d, errno=%d", mPortNumber, listenSockID, result, ::WSAGetLastError());

    mSocketID = listenSockID;
    }

int VSocket::available()
    {
    u_long numBytesAvailable = 0;

    int result = ::ioctlsocket(mSocketID, FIONREAD, &numBytesAvailable);

    if (result != 0)
        throw VException("VSocket::available failed on socket %d, result=%d, error=%s.", mSocketID, result, ::strerror(errno));

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
                throw VException("VSocket::available failed on socket %d, result=%d, error=%s.", mSocketID, result, ::strerror(errno));
                break;
            default:
                break;
            }
        }

    return (int) numBytesAvailable;
    }

int VSocket::read(Vu8* buffer, int numBytesToRead)
    {
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

                throw VException("VSocket::read select failed on socket %d, result=%d, error=%s.", mSocketID, result, ::strerror(errno));
                }
            else if (result == 0)
                {
                throw VException("VSocket::read select timed out on socket %d.", mSocketID);
                }

            if (!FD_ISSET(mSocketID, &readset))
                {
                throw VException("VSocket::read select got FD_ISSET false on socket %d, errno=%s.", mSocketID, ::strerror(errno));
                }

            }

        numBytesRead = ::recv(mSocketID, (char*) nextBufferPositionPtr, bytesRemainingToRead, 0);

        if (numBytesRead < 0)
            {
            throw VException("VSocket::read recv failed on socket %d, result=%d, errno=%s.", mSocketID, numBytesRead, ::strerror(errno));
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
    const Vu8*  nextBufferPositionPtr = buffer;
    int         bytesRemainingToWrite = numBytesToWrite;
    int         numBytesWritten;
    int         result;
    fd_set      writeset;

    if (mSocketID < 0)
        {
        throw VException("VSocket::write with invalid mSocketID %d", mSocketID);
        }

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

                throw VException("VSocket::write select failed on socket %d, result=%d, errno=%s.", mSocketID, result, ::strerror(errno));
                }
            else if (result == 0)
                {
                throw VException("VSocket::write select timed out on socket %d", mSocketID);
                }

            }

        numBytesWritten = ::send(mSocketID, (const char*) nextBufferPositionPtr, bytesRemainingToWrite, 0);

        if (numBytesWritten <= 0)
            {
            throw VException("VSocket::write send failed on socket %d, errno=%s.", mSocketID, ::strerror(errno));
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

    mPortNumber = ntohs(info.sin_port);
    mHostName = ::inet_ntoa(info.sin_addr);
    }

void VSocket::closeRead()
    {
    int result = ::shutdown(mSocketID, SHUT_RD);

    if (result < 0)
        throw VException("VSocket::closeRead unable to shut down socket %d.", mSocketID);
    }

void VSocket::closeWrite()
    {
    int result = ::shutdown(mSocketID, SHUT_WR);

    if (result < 0)
        throw VException("VSocket::closeWrite unable to shut down socket %d.", mSocketID);
    }

void VSocket::setSockOpt(int level, int name, void* valuePtr, int valueLength)
    {
    ::setsockopt(mSocketID, level, name, (char*) valuePtr, valueLength);
    }

