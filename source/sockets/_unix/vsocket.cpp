/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocket.h"

#ifndef _HPUX_SOURCE
#include <tcpd.h>
#endif

#include <signal.h>

#ifdef sun
#include <sys/proc.h>
#endif

#include <sys/ioctl.h>

#include "vexception.h"
#include "vmutexlocker.h"

V_STATIC_INIT_TRACE
    
// This is to force our staticInit to be called at startup.
bool VSocket::smStaticInited = VSocket::staticInit();

bool VSocket::staticInit()
    {
    //lint -e421 -e923 " Caution -- function 'signal(int, void (*)(int))' is considered dangerous [MISRA Rule 123]"
    ::signal(SIGPIPE, SIG_IGN);

    // Should log the error if the call failed.

    return true;
    }

VSocket::VSocket()
    {
    }

VSocket::~VSocket()
    {
    // Note: base class destructor does a close() of the socket if it is open.
    }

void VSocket::connect()
    {
#ifdef SUSV2
    this->connectStandard();
#else
    this->connectEnhanced();
#endif
    }

void VSocket::listen()
    {
#ifdef SUSV2
    this->listenStandard();
#else
    this->listenEnhanced();
#endif
    }

int VSocket::available()
    {
    int    numBytesAvailable = 0;
    
    int    result = ::ioctl(mSockID, FIONREAD, &numBytesAvailable);
    
    if (result == -1)
        throw VException("VSocket::available failed on socket %d, result=%d, error=%s.", mSockID, result, ::strerror(errno));
    
    return numBytesAvailable;
    }

int VSocket::read(Vu8* buffer, int numBytesToRead)
    {
    int        bytesRemainingToRead = numBytesToRead;
    Vu8*    nextBufferPositionPtr = buffer;
    int        result;
    int        theNumBytesRead = 0;
    fd_set    readset;

    while (bytesRemainingToRead > 0) 
        {
        if (mSockID <= FD_SETSIZE) // FD_SETSIZE is max num open sockets, sockid over that is sign of a big problem
            {
            FD_ZERO(&readset);
            //lint -e573 "Signed-unsigned mix with divide"
            FD_SET(mSockID, &readset);
            result = ::select(mSockID + 1, &readset, NULL, NULL, (mReadTimeOutActive ? &mReadTimeOut : NULL));

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

            //lint -e573 "Signed-unsigned mix with divide"
            if (!FD_ISSET(mSockID, &readset)) 
                {
                throw VException("VSocket::read select got FD_ISSET false on socket %d, errno=%s.", mSockID, ::strerror(errno));
                }

            }

        theNumBytesRead = ::recv(mSockID, (char*) nextBufferPositionPtr, (VSizeType) bytesRemainingToRead, 0);

        if (theNumBytesRead < 0)
            {
            throw VException("VSocket::read recv failed on socket %d, result=%d, errno=%s.", mSockID, theNumBytesRead, ::strerror(errno));
            }
        else if (theNumBytesRead == 0)
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

        bytesRemainingToRead -= theNumBytesRead;
        nextBufferPositionPtr += theNumBytesRead;

        mNumBytesRead += theNumBytesRead;
        }

    mLastEventTime.setNow();

    return (numBytesToRead - bytesRemainingToRead);
    }

int VSocket::write(const Vu8* buffer, int numBytesToWrite)
    {
    Vu8*    nextBufferPositionPtr = (Vu8*) buffer;
    int        bytesRemainingToWrite = numBytesToWrite;
    int        theNumBytesWritten;
    int        result = 0; // avoid compiler warning for possible uninitialized use
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
            //lint -e573 "Signed-unsigned mix with divide"
            FD_SET(mSockID, &writeset);
            result = ::select(mSockID + 1, NULL, &writeset, NULL, (mWriteTimeOutActive ? &mWriteTimeOut : NULL));

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

        theNumBytesWritten = ::send(mSockID, nextBufferPositionPtr, (VSizeType) bytesRemainingToWrite, 0);

        if (theNumBytesWritten <= 0)
            {
            throw VException("VSocket::write send failed on socket %d, errno=%s.", mSockID, ::strerror(errno));
            }
        else if (theNumBytesWritten != bytesRemainingToWrite)
            {
            // Debug message: write was only partially completed so we will cycle around and write the rest...
            }
        else
            {
            // This is where you could put debug/trace-mode socket write logging output....
            }

        bytesRemainingToWrite -= theNumBytesWritten;
        nextBufferPositionPtr += theNumBytesWritten;

        mNumBytesWritten += theNumBytesWritten;
        }

    return (numBytesToWrite - bytesRemainingToWrite);
    }

void VSocket::discoverHostAndPort()
    {
    struct sockaddr_in    info;
    VSocklenT            infoLength = sizeof(info);

    int    result = ::getpeername(mSockID, (struct sockaddr*) &info, &infoLength);
    //lint -e550 "Symbol 'e' (line 207) not accessed"
    if (result != 0)
        {
        int    e = errno;    // for debugging purposes, can set a breakpoint here and look at errno
        e = 0;            // avoid unused variable compiler warning
        }

    mPortNumber = ntohs(info.sin_port);

    char*    name = /*::hack_*/::inet_ntoa(info.sin_addr); //somewhat deprecated in favor of addr2ascii
    //char*    name = ::addr2ascii(AF_INET, info.sin_addr, sizeof(info.sin_addr), NULL);
    
    mHostName = name;
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
    (void) ::setsockopt(mSockID, level, name, valuePtr, valueLength);
    }

void VSocket::assertInvariant() const
    {
    }

#ifdef SUSV2

void VSocket::connectStandard()
    {
    struct sockaddr_in    info;
    int                    infoLength = sizeof(info);
    VSockID                sockID;
    
    ::memset(&info, 0, sizeof(info));
    info.sin_family = AF_INET;
    info.sin_port = V_BYTESWAP_HTONS_GET((Vu16) mPortNumber);
    info.sin_addr.s_addr = /*::hack_*/::inet_addr(mHostName);
    
    sockID = ::socket(AF_INET, SOCK_STREAM, 0);
        
    if (sockID >= 0)
        {
        int    result = ::connect(sockID, (const sockaddr*) &info, infoLength);

        if (result != 0)
            {
            // Connect failed.
            ::close(sockID);
            throw VException("VSocket::connectStandard failed on socket %d, errno=%s", sockID, ::strerror(errno));
            }
        }

    mSockID = sockID;
    }

void VSocket::listenStandard()
    {
    VSockID                listenSockID = kNoSockID;
    struct sockaddr_in    info;
    int                    infoLength = sizeof(info);
    const int            on = 1;
    int                    result;
    
    ::memset(&info, 0, sizeof(info));
    info.sin_family = AF_INET;
    info.sin_port = V_BYTESWAP_HTONS_GET((Vu16) mPortNumber);
    info.sin_addr.s_addr = INADDR_ANY;//::hack_inet_addr(hostName);
    
    listenSockID = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSockID < 0)
        throw VException("VSocket::listenStandard socket() failed with id %d, errno=%s", listenSockID, ::strerror(errno));
        
    result = ::setsockopt(listenSockID, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (result != 0)
        throw VException("VSocket::listenStandard setsockopt() failed on socket %d with result %d, errno=%s", listenSockID, result, ::strerror(errno));

    result = ::bind(listenSockID, (const sockaddr*) &info, infoLength);
    if (result != 0)
        {
        ::close(listenSockID);
        throw VException("VSocket::listenStandard bind() for port %d failed on socket %d with result %d, errno=%s", mPortNumber, listenSockID, result, ::strerror(errno));
        }

    (void) ::listen(listenSockID, mListenBacklog);
    if (result != 0)
        {
        ::close(listenSockID);
        throw VException("VSocket::listenStandard listen() failed on socket %d with result %d, errno=%s", listenSockID, result, ::strerror(errno));
        }

    mSockID = listenSockID;
    }

#else /* SUSV2 */

void VSocket::connectEnhanced()
    {
    VSockID            sockID = kNoSockID;
    struct addrinfo    *res = NULL;
    
    this->tcpGetAddrInfo(&res);

    try 
        {    
        sockID = this->tcpConnectWAddrInfo(res);
        }
    catch (VException& ex) 
        {
        ::freeaddrinfo(res);
        throw;
        }

    ::freeaddrinfo(res);
    
    mSockID = sockID;
    }
/*
void VSocket_listenEnhanced()
    {
    VSockID                listenSockID;
    int                    result;
    const int            on = 1;
    struct addrinfo        hints;
    struct addrinfo*    res;
    struct addrinfo*    ressave;

    ::memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    result = this->getAddrInfo(&hints, &res, false);

    if (result != 0)
        return;

    ressave = res;

    do
        {
        listenSockID = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

        if (listenSockID < 0)
            {
            // Error on this interface, we'll try the next one.
            continue;
            }
    
        ::setsockopt(listenSockID, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    
        if (::bind(listenSockID, res->ai_addr, res->ai_addrlen) == 0)
            {
            // Success.
            break;
            }

        ::close(listenSockID);

        } while ((res = res->ai_next) != NULL);

    if (res == NULL)
        return;

    ::listen(listenSockID, mListenBacklog);

    ::freeaddrinfo(ressave);

    mSockID = listenSockID;
    }
*/
void VSocket::listenEnhanced()
    {
    VSockID                listenSockID;
    int                    result;
    const int            on = 1;
    struct addrinfo        hints;
    struct addrinfo*    res;
    struct addrinfo*    ressave;
    VString                lastError;

    ::memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    result = this->getAddrInfo(&hints, &res, false);
    if (result != 0)
        throw VException("VSocket::listenEnhanced getAddrInfo() failed with result %d, errno=%s", result, ::strerror(errno));

    ressave = res;

    do
        {
        listenSockID = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

        if (listenSockID < 0)
            {
            // Error on this interface, we'll try the next one.
            lastError.format("VSocket::listenEnhanced socket() failed with id %d, errno=%s", listenSockID, ::strerror(errno));
            continue;
            }
    
        result = ::setsockopt(listenSockID, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if (result != 0)
            {
            // Error on this interface, we'll try the next one.
            lastError.format("VSocket::listenEnhanced setsockopt() failed on socket %d with result %d, errno=%s", listenSockID, result, ::strerror(errno));
            continue;
            }
    
        result = ::bind(listenSockID, res->ai_addr, res->ai_addrlen);
        
        if (result == 0)
            {
            // Success -- break out of loop to continue setting up listen.
            break;
            }

        // Error on this interface, we'll try the next one.
        lastError.format("VSocket::listenEnhanced bind() failed on socket %d with result %d, errno=%s", listenSockID, result, ::strerror(errno));
        ::close(listenSockID);

        } while ((res = res->ai_next) != NULL);

    if (res == NULL)
        throw VException(lastError);

    if (lastError.length() != 0)
        std::cout << "VSocket::listenEnhanced bind() succeeded after earlier error: " << lastError << "." << std::endl;

    result = ::listen(listenSockID, mListenBacklog);

    ::freeaddrinfo(ressave);

    if (result != 0)
        {
        ::close(listenSockID);
        throw VException("VSocket::listenEnhanced listen() failed on socket %d with result %d, errno=%s", listenSockID, result, ::strerror(errno));
        }

    mSockID = listenSockID;
    }

void VSocket::tcpGetAddrInfo(struct addrinfo **res)
    {
    struct addrinfo    hints;
    int                result;

    ::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    result = this->getAddrInfo(&hints, res);

    if (result != 0)
        {
        throw VException("VSocket::tcpGetAddrInfo failed error for host=%s, port=%d: %s", mHostName.chars(), mPortNumber, ::gai_strerror(result));
        }
    }

int VSocket::getAddrInfo(struct addrinfo* hints, struct addrinfo** res, bool useHostName)
    {
    VMutexLocker    locker(&smAddrInfoMutex);
    VString            portAsString("%d", mPortNumber);

    return ::getaddrinfo(useHostName ? mHostName.chars() : NULL, portAsString.chars(), hints, res);
    }

VSockID    VSocket::tcpConnectWAddrInfo(struct addrinfo * const resInput)
    {
    struct addrinfo*    res = resInput;
    VSockID                sockID;

    do
        {
        sockID = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        
        if (sockID < 0) 
            {
            // Debug message: socket open failed but we will try again if more available...
            continue;
            }
        else if (sockID > FD_SETSIZE) // FD_SETSIZE is max num open sockets, sockid over that is sign of a big problem
            {
            // Debug message: socket id appears to be out of range...
            }

        if (::connect(sockID, res->ai_addr, res->ai_addrlen) == 0)
            break; /* success */

        ::close(sockID);
        } while((res = res->ai_next) != NULL);

    if (res == NULL)
        {
        throw VException("VSocket::tcpConnectWAddrInfo failed, errno=%s", ::gai_strerror(errno));
        }

    return sockID;
    }

// The BSD call to getaddrinfo() is not threadsafe so we protect it with a global mutex.
VMutex VSocket::smAddrInfoMutex;

#endif /* SUSV2 */

