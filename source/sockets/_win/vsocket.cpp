/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocket.h"

#include "vexception.h"

// This is to force our staticInit to be called at startup.
bool VSocket::smStaticInited = VSocket::staticInit();

bool VSocket::staticInit()
    { 
    bool    success = true;
    WORD    versionRequested;
    WSADATA    wsaData;
    int        err;

    versionRequested = MAKEWORD(2, 0);
 
    err = ::WSAStartup(versionRequested, &wsaData);
    
    success = (err == 0);

    assert(success);

    return success;
    }

VSocket::VSocket()
    {
    }

VSocket::~VSocket()
    {
    }

void VSocket::connect()
    {
    int                    length;
    struct sockaddr_in    address;
    VSockID                sockID;
    
    ::memset(&address, 0, sizeof(address));    // is this needed?
    address.sin_family = AF_INET;
    address.sin_port = htons(mPortNumber);
    address.sin_addr.s_addr = ::inet_addr(mHostName);
    length = sizeof(struct sockaddr_in);
    
    sockID = ::socket(AF_INET, SOCK_STREAM, 0);
        
    if (sockID >= 0)
        {
        if (::connect(sockID, (const sockaddr*) &address, length) != 0)
            {
            // failure
            ::closesocket(sockID);
            throw VException("VSocket::connect failed on socket %d, errno=%s", sockID, ::strerror(errno));
            }
        }

    mSockID = sockID;
    }

void VSocket::listen()
    {
    
    VSockID                listenSockID = kNoSockID;
    struct sockaddr_in    info;
    int                    infoLength = sizeof(info);
    const int            on = 1;
    int                    result;
    
    ::memset(&info, 0, sizeof(info));
    info.sin_family = AF_INET;
    info.sin_port = htons(mPortNumber);
    info.sin_addr.s_addr = INADDR_ANY;//::inet_addr(hostName);
    
    listenSockID = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSockID <= 0)
        throw VException("VSocket::listen socket failed, returning %d, errno=%d", listenSockID, ::WSAGetLastError());

    result = ::setsockopt(listenSockID, SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof(on));
    if (result != 0)
        throw VException("VSocket::listen setsockopt failed, returning %d, errno=%d", result, ::WSAGetLastError());

    result = ::bind(listenSockID, (const sockaddr*) &info, infoLength);
    if (result != 0)
        {
        // Bind failed.
        ::closesocket(listenSockID);
        throw VException("VSocket::listen bind failed on socket %d, errno=%d", listenSockID, ::WSAGetLastError());
        }

    result = ::listen(listenSockID, mListenBacklog);
    if (result != 0)
        throw VException("VSocket::listen listen() failed, returning %d, errno=%d", result, ::WSAGetLastError());

    mSockID = listenSockID;
    }

int VSocket::available()
    {
    u_long    numBytesAvailable;
    
    int    result = ::ioctlsocket(mSockID, FIONREAD, &numBytesAvailable);
    
    if (result != 0)
        throw VException("VSocket::available failed on socket %d, result=%d, error=%s.", mSockID, result, ::strerror(errno));
    
    return (int) numBytesAvailable;
    }

int VSocket::read(Vu8* buffer, int numBytesToRead)
    {
    int        bytesRemainingToRead = numBytesToRead;
    Vu8*    nextBufferPositionPtr = buffer;
    int        result;
    int        numBytesRead = 0;
    fd_set    readset;

    while (bytesRemainingToRead > 0) 
        {
        if (mSockID <= FD_SETSIZE) // FD_SETSIZE is max num open sockets, sockid over that is sign of a big problem
            {
            FD_ZERO(&readset);
            FD_SET(mSockID, &readset);
            result = ::select((int) (mSockID + 1), &readset, NULL, NULL, (mReadTimeOutActive ? &mReadTimeOut : NULL));

            if (result < 0) 
                {
                if (errno == EINTR)
                    {
                    // Debug message: read was interrupted but we will cycle around and try again...
                    continue;
                    }

                throw VException("VSocket::read select failed on socket %d, result=%d, error=%s.", mSockID, result, ::strerror(errno));
                }
            else if (result == 0) 
                {
                throw VException("VSocket::read select timed out on socket %d.", mSockID);
                }

            if (!FD_ISSET(mSockID, &readset)) 
                {
                throw VException("VSocket::read select got FD_ISSET false on socket %d, errno=%s.", mSockID, ::strerror(errno));
                }

            }

        numBytesRead = ::recv(mSockID, (char*) nextBufferPositionPtr, bytesRemainingToRead, 0);

        if (numBytesRead < 0)
            {
            throw VException("VSocket::read recv failed on socket %d, result=%d, errno=%s.", mSockID, numBytesRead, ::strerror(errno));
            }
        else if (numBytesRead == 0)
            {
            if (mRequireReadAll)
                {
                throw VEOFException(VString("VSocket::read: reached EOF unexpectedly on socket %d, recv of length %d returned 0, errno=%s.", mSockID, bytesRemainingToRead, ::strerror(errno)));
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
    Vu8*    nextBufferPositionPtr = (Vu8*) buffer;
    int        bytesRemainingToWrite = numBytesToWrite;
    int        numBytesWritten;
    int        result;
    fd_set    writeset;

    if (mSockID < 0) 
        {
        throw VException("VSocket::write with invalid mSockID %d", mSockID);
        }

    while (bytesRemainingToWrite > 0)
        {
        if (mSockID <= FD_SETSIZE) // FD_SETSIZE is max num open sockets, sockid over that is sign of a big problem
            {
            FD_ZERO(&writeset);
            FD_SET(mSockID, &writeset);
            result = ::select((int) (mSockID + 1), NULL, &writeset, NULL, (mWriteTimeOutActive ? &mWriteTimeOut : NULL));

            if (result < 0) 
                {
                if (errno == EINTR)
                    {
                    // Debug message: write was interrupted but we will cycle around and try again...
                    continue;
                    }
            
                throw VException("VSocket::write select failed on socket %d, result=%d, errno=%s.", mSockID, result, ::strerror(errno));
                }
            else if (result == 0)
                {
                throw VException("VSocket::write select timed out on socket %d", mSockID);
                }

            }

        numBytesWritten = ::send(mSockID, (const char*) nextBufferPositionPtr, bytesRemainingToWrite, 0);

        if (numBytesWritten <= 0)
            {
            throw VException("VSocket::write send failed on socket %d, errno=%s.", mSockID, ::strerror(errno));
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
    VSocklenT            namelen = sizeof(struct sockaddr_in);
    struct sockaddr_in    info;

    ::getpeername(mSockID, (struct sockaddr*) &info, &namelen);

    mPortNumber = ntohs(info.sin_port);
    mHostName = ::inet_ntoa(info.sin_addr);
    }

void VSocket::closeRead()
    {
    int    result = ::shutdown(mSockID, SHUT_RD);
    
    if (result < 0)
        throw VException("VSocket::closeRead unable to shut down socket %d.", mSockID);
    }

void VSocket::closeWrite()
    {
    int    result = ::shutdown(mSockID, SHUT_WR);
    
    if (result < 0)
        throw VException("VSocket::closeWrite unable to shut down socket %d.", mSockID);
    }

void VSocket::setSockOpt(int level, int name, void* valuePtr, int valueLength)
    {
    ::setsockopt(mSockID, level, name, (char*) valuePtr, valueLength);
    }

