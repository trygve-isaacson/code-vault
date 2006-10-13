/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
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
bool VSocket::gStaticInited = VSocket::staticInit();

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

int VSocket::available()
    {
    int numBytesAvailable = 0;
    
    int result = ::ioctl(mSocketID, FIONREAD, &numBytesAvailable);
    
    if (result == -1)
        throw VException("VSocket::available failed on socket %d, result=%d, error=%s.", mSocketID, result, ::strerror(errno));
    
    return numBytesAvailable;
    }

int VSocket::read(Vu8* buffer, int numBytesToRead)
    {
    int     bytesRemainingToRead = numBytesToRead;
    Vu8*    nextBufferPositionPtr = buffer;
    int     result;
    int     theNumBytesRead = 0;
    fd_set  readset;

    while (bytesRemainingToRead > 0) 
        {
        if (mSocketID <= FD_SETSIZE) // FD_SETSIZE is max num open sockets, sockid over that is sign of a big problem
            {
            FD_ZERO(&readset);
            //lint -e573 "Signed-unsigned mix with divide"
            FD_SET(mSocketID, &readset);
            result = ::select(mSocketID + 1, &readset, NULL, NULL, (mReadTimeOutActive ? &mReadTimeOut : NULL));

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

            //lint -e573 "Signed-unsigned mix with divide"
            if (!FD_ISSET(mSocketID, &readset)) 
                {
                throw VException("VSocket::read select got FD_ISSET false on socket %d, errno=%s.", mSocketID, ::strerror(errno));
                }

            }

        theNumBytesRead = ::recv(mSocketID, (char*) nextBufferPositionPtr, (VSizeType) bytesRemainingToRead, 0);

        if (theNumBytesRead < 0)
            {
            throw VException("VSocket::read recv failed on socket %d, result=%d, errno=%s.", mSocketID, theNumBytesRead, ::strerror(errno));
            }
        else if (theNumBytesRead == 0)
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

        bytesRemainingToRead -= theNumBytesRead;
        nextBufferPositionPtr += theNumBytesRead;

        mNumBytesRead += theNumBytesRead;
        }

    mLastEventTime.setNow();

    return (numBytesToRead - bytesRemainingToRead);
    }

int VSocket::write(const Vu8* buffer, int numBytesToWrite)
    {
    const Vu8*  nextBufferPositionPtr = buffer;
    int         bytesRemainingToWrite = numBytesToWrite;
    int         theNumBytesWritten;
    int         result = 0; // avoid compiler warning for possible uninitialized use
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
            //lint -e573 "Signed-unsigned mix with divide"
            FD_SET(mSocketID, &writeset);
            result = ::select(mSocketID + 1, NULL, &writeset, NULL, (mWriteTimeOutActive ? &mWriteTimeOut : NULL));

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

        theNumBytesWritten = ::send(mSocketID, nextBufferPositionPtr, (VSizeType) bytesRemainingToWrite, 0);

        if (theNumBytesWritten <= 0)
            {
            throw VException("VSocket::write send failed on socket %d, errno=%s.", mSocketID, ::strerror(errno));
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
    struct sockaddr_in  info;
    VSocklenT           infoLength = sizeof(info);

    int    result = ::getpeername(mSocketID, (struct sockaddr*) &info, &infoLength);
    //lint -e550 "Symbol 'e' (line 207) not accessed"
    if (result != 0)
        {
        int e = errno;    // for debugging purposes, can set a breakpoint here and look at errno
        e = 0;            // avoid unused variable compiler warning
        }

    mPortNumber = ntohs(info.sin_port);

    char* name = ::inet_ntoa(info.sin_addr); // addr2ascii is preferred but is not yet standard:
    //char* name = ::addr2ascii(AF_INET, info.sin_addr, sizeof(info.sin_addr), NULL);
    
    mHostName = name;
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
    (void) ::setsockopt(mSocketID, level, name, valuePtr, valueLength);
    }

void VSocket::assertInvariant() const
    {
    }

#ifndef V_BSD_ENHANCED_SOCKETS

void VSocket::connect()
    {
    struct sockaddr_in  info;
    int                 infoLength = sizeof(info);
    VSocketID           socketID;
    
    ::memset(&info, 0, sizeof(info));
    info.sin_family = AF_INET;
    info.sin_port = V_BYTESWAP_HTONS_GET((Vu16) mPortNumber);
    info.sin_addr.s_addr = /*::hack_*/::inet_addr(mHostName);
    
    socketID = ::socket(AF_INET, SOCK_STREAM, 0);
        
    if (socketID >= 0)
        {
        int result = ::connect(socketID, (const sockaddr*) &info, infoLength);

        if (result != 0)
            {
            // Connect failed.
            vault::close(socketID);
            throw VException("VSocket::connect was unable to connect to %s:%d -- socket id %d, errno=%s", mHostName.chars(), mPortNumber, socketID, ::strerror(errno));
            }
        }

    mSocketID = socketID;
    }

void VSocket::listen()
    {
    VSocketID           listenSockID = kNoSocketID;
    struct sockaddr_in  info;
    int                 infoLength = sizeof(info);
    const int           on = 1;
    int                 result;
    
    ::memset(&info, 0, sizeof(info));
    info.sin_family = AF_INET;
    info.sin_port = V_BYTESWAP_HTONS_GET((Vu16) mPortNumber);
    info.sin_addr.s_addr = INADDR_ANY;//::hack_inet_addr(hostName);
    
    listenSockID = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSockID < 0)
        throw VException("VSocket::listen socket() failed with id %d, errno=%s", listenSockID, ::strerror(errno));
        
    result = ::setsockopt(listenSockID, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (result != 0)
        throw VException("VSocket::listen setsockopt() failed -- socket id %d, result %d, errno=%s", listenSockID, result, ::strerror(errno));

    result = ::bind(listenSockID, (const sockaddr*) &info, infoLength);
    if (result != 0)
        {
        vault::close(listenSockID);
        throw VException("VSocket::listen bind() for port %d failed -- socket id %d, result %d, errno=%s", mPortNumber, listenSockID, result, ::strerror(errno));
        }

    (void) ::listen(listenSockID, mListenBacklog);
    if (result != 0)
        {
        vault::close(listenSockID);
        throw VException("VSocket::listen listen() for port %d failed -- socket id %d, result %d, errno=%s", mPortNumber, listenSockID, result, ::strerror(errno));
        }

    mSocketID = listenSockID;
    }

#else /* V_BSD_ENHANCED_SOCKETS version follows */

void VSocket::connect()
    {
    VSocketID           socketID = kNoSocketID;
    struct addrinfo*    res = NULL;
    
    this->_tcpGetAddrInfo(&res);

    try 
        {    
        socketID = this->_tcpConnectWAddrInfo(res);
        }
    catch (VException& ex) 
        {
        ::freeaddrinfo(res);
        throw;
        }

    ::freeaddrinfo(res);
    
    mSocketID = socketID;
    }

void VSocket::listen()
    {
    VSocketID           listenSockID;
    int                 result;
    const int           on = 1;
    struct addrinfo     hints;
    struct addrinfo*    res;
    struct addrinfo*    ressave;
    VString             lastError;

    ::memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    result = this->_getAddrInfo(&hints, &res, false);
    if (result != 0)
        throw VException("VSocket::listen getAddrInfo() failed with result %d, errno=%s", result, ::strerror(errno));

    ressave = res;

    do
        {
        listenSockID = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

        if (listenSockID < 0)
            {
            // Error on this interface, we'll try the next one.
            lastError.format("VSocket::listen socket() failed with id %d, errno=%s", listenSockID, ::strerror(errno));
            continue;
            }
    
        result = ::setsockopt(listenSockID, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if (result != 0)
            {
            // Error on this interface, we'll try the next one.
            lastError.format("VSocket::listen setsockopt() failed on socket %d with result %d, errno=%s", listenSockID, result, ::strerror(errno));
            continue;
            }
    
        result = ::bind(listenSockID, res->ai_addr, res->ai_addrlen);
        
        if (result == 0)
            {
            // Success -- break out of loop to continue setting up listen.
            break;
            }

        // Error on this interface, we'll try the next one.
        lastError.format("VSocket::listen bind() failed on socket %d with result %d, errno=%s", listenSockID, result, ::strerror(errno));
        vault::close(listenSockID);

        } while ((res = res->ai_next) != NULL);

    if (res == NULL)
        throw VException(lastError);

    if (lastError.length() != 0)
        std::cout << "VSocket::listen bind() succeeded after earlier error: " << lastError << "." << std::endl;

    result = ::listen(listenSockID, mListenBacklog);

    ::freeaddrinfo(ressave);

    if (result != 0)
        {
        vault::close(listenSockID);
        throw VException("VSocket::listen listen() failed on socket %d with result %d, errno=%s", listenSockID, result, ::strerror(errno));
        }

    mSocketID = listenSockID;
    }

void VSocket::_tcpGetAddrInfo(struct addrinfo **res)
    {
    struct addrinfo    hints;
    int                result;

    ::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    result = this->_getAddrInfo(&hints, res);

    if (result != 0)
        {
        throw VException("VSocket::_tcpGetAddrInfo failed error for host=%s, port=%d: %s", mHostName.chars(), mPortNumber, ::gai_strerror(result));
        }
    }

int VSocket::_getAddrInfo(struct addrinfo* hints, struct addrinfo** res, bool useHostName)
    {
    VMutexLocker    locker(&gAddrInfoMutex);
    VString         portAsString("%d", mPortNumber);

    return ::getaddrinfo(useHostName ? mHostName.chars() : NULL, portAsString.chars(), hints, res);
    }

VSocketID VSocket::_tcpConnectWAddrInfo(struct addrinfo * const resInput)
    {
    struct addrinfo*    res = resInput;
    VSocketID           socketID;

    do
        {
        socketID = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        
        if (socketID < 0) 
            {
            // Debug message: socket open failed but we will try again if more available...
            continue;
            }
        else if (socketID > FD_SETSIZE) // FD_SETSIZE is max num open sockets, sockid over that is sign of a big problem
            {
            // Debug message: socket id appears to be out of range...
            }

        if (::connect(socketID, res->ai_addr, res->ai_addrlen) == 0)
            break; /* success */

        vault::close(socketID);
        } while((res = res->ai_next) != NULL);

    if (res == NULL)
        {
        throw VException("VSocket::_tcpConnectWAddrInfo failed, errno=%s", ::gai_strerror(errno));
        }

    return socketID;
    }

// The BSD call to getaddrinfo() is not threadsafe so we protect it with a global mutex.
VMutex VSocket::gAddrInfoMutex;

#endif /* V_BSD_ENHANCED_SOCKETS */

