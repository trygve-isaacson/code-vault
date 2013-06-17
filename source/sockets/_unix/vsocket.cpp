/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocket.h"
#include "vtypes_internal.h"

#include <signal.h>

#ifdef sun
#include <sys/proc.h>
#endif

#include <sys/ioctl.h>
#include <ifaddrs.h>

#include "vexception.h"
#include "vmutexlocker.h"

V_STATIC_INIT_TRACE

// On Mac OS X, we disable SIGPIPE in VSocketBase::setDefaultSockOpt().
// For other Unix platforms, we specify it in the flags of each send()/recv() call via this parameter.
#ifdef VPLATFORM_MAC
    #define VSOCKET_DEFAULT_SEND_FLAGS 0
    #define VSOCKET_DEFAULT_RECV_FLAGS 0
#else
    #define VSOCKET_DEFAULT_SEND_FLAGS MSG_NOSIGNAL
    #define VSOCKET_DEFAULT_RECV_FLAGS MSG_NOSIGNAL
#endif

// This is to force our staticInit to be called at startup.
bool VSocket::gStaticInited = VSocket::staticInit();

bool VSocket::staticInit() {
    //lint -e421 -e923 " Caution -- function 'signal(int, void (*)(int))' is considered dangerous [MISRA Rule 123]"
    ::signal(SIGPIPE, SIG_IGN);

    // Should log the error if the call failed.

    return true;
}

// This is the one VSocketBase function that must implemented per platform, so we do it here.
// This kind of highlights that the original VSocketBase <- VSocket hierarchy is probably due
// for restructuring so that we simply break the implementation of 1 class into two .cpp files
// where the second file contains the per-platform implementation, but named by the same class,
// as we do now with, for example, VFSNode and its platform-specific parts.
// static
VNetworkInterfaceList VSocketBase::enumerateNetworkInterfaces() {
    VNetworkInterfaceList interfaces;
    struct ifaddrs* interfacesDataPtr = NULL;
    int result = ::getifaddrs(&interfacesDataPtr);
    if (result != 0) {
        throw VStackTraceException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocketBase::enumerateNetworkInterfaces: getifaddrs() failed with result %d.", result));
    }

    struct ifaddrs* intfPtr = interfacesDataPtr;
    while (intfPtr != NULL) {
        if (intfPtr->ifa_addr != NULL) {
            VNetworkInterfaceInfo info;
            info.mFamily = intfPtr->ifa_addr->sa_family;
            info.mName.copyFromCString(intfPtr->ifa_name);
            // AF_INET6 will work just fine here, too. But hold off until we can verify callers can successfully use IPV6 address strings to listen, connect, etc.
            if ((info.mFamily == AF_INET) && (info.mName != "lo0")) { // Internet interfaces only, and skip the loopback address.
                info.mAddress.preflight(255);
                char* buffer = info.mAddress.buffer();
                ::inet_ntop(intfPtr->ifa_addr->sa_family, &((struct sockaddr_in*) intfPtr->ifa_addr)->sin_addr, buffer, 255);
                info.mAddress.postflight((int) ::strlen(buffer));

                // Check for "lo0" above should filter out 127.x.x.x (loopback addresses), but in case it doesn't, check it.
                if (! info.mAddress.startsWith("127.")) {
                    interfaces.push_back(info);
                }
            }
        }

        intfPtr = intfPtr->ifa_next;
    }

    ::freeifaddrs(interfacesDataPtr);
    return interfaces;
}

static const int MAX_ADDRSTRLEN = V_MAX(INET_ADDRSTRLEN, INET6_ADDRSTRLEN);
// Another platform-specific implementation of a function defined by the base class API.
// static
VString VSocketBase::addrinfoToIPAddressString(const VString& hostName, const struct addrinfo* info) {
    void* addr;
    if (info->ai_family == AF_INET) {
        addr = (void*)&(((struct sockaddr_in *)info->ai_addr)->sin_addr);
    } else if (info->ai_family == AF_INET6) {
        addr = (void*)&(((struct sockaddr_in6 *)info->ai_addr)->sin6_addr);
    } else {
        // We don't know how to access the addr for other family types. They could conceivably be added.
        throw VException(VSTRING_FORMAT("VSocketBase::addrinfoToIPAddressString(%s): An invalid family (%d) other than AF_INET or AF_INET6 was specified.", hostName.chars(), info->ai_family));
    }
    
    VString result;
    result.preflight(MAX_ADDRSTRLEN);
    const char* buf = ::inet_ntop(info->ai_family, addr, result.buffer(), MAX_ADDRSTRLEN);
    if (buf == NULL) {
        throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocketBase::addrinfoToIPAddressString(%s): inet_ntop() failed.", hostName.chars()));
    }
    result.postflight(::strlen(buf));

    return result;
}

VSocket::VSocket(VSocketID id)
    : VSocketBase(id)
    {
}

VSocket::VSocket(const VString& hostName, int portNumber)
    : VSocketBase(hostName, portNumber)
    {
}

VSocket::~VSocket() {
    // Note: base class destructor does a close() of the socket if it is open.
}

int VSocket::available() {
    int numBytesAvailable = 0;

    int result = ::ioctl(mSocketID, FIONREAD, &numBytesAvailable);

    if (result == -1) {
        throw VStackTraceException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] available: ioctl() failed.", mSocketName.chars()));
    }

    return numBytesAvailable;
}

int VSocket::read(Vu8* buffer, int numBytesToRead) {
    if (mSocketID < 0) {
        throw VStackTraceException(VSTRING_FORMAT("VSocket[%s] read: Invalid socket ID %d.", mSocketName.chars(), mSocketID));
    }

    int     bytesRemainingToRead = numBytesToRead;
    Vu8*    nextBufferPositionPtr = buffer;
    int     result;
    int     theNumBytesRead = 0;
    fd_set  readset;

    while (bytesRemainingToRead > 0) {
        if (mSocketID <= FD_SETSIZE) { // FD_SETSIZE is max num open sockets, sockid over that is sign of a big problem
            FD_ZERO(&readset);
            //lint -e573 "Signed-unsigned mix with divide"
            FD_SET(mSocketID, &readset);
            result = ::select(mSocketID + 1, &readset, NULL, NULL, (mReadTimeOutActive ? &mReadTimeOut : NULL));

            if (result < 0) {
                if (errno == EINTR) {
                    // Debug message: read was interrupted but we will cycle around and try again...
                    continue;
                }

                if (errno == EBADF) {
                    throw VSocketClosedException(errno, VSTRING_FORMAT("VSocket[%s] read: Socket has closed (EBADF).", mSocketName.chars()));
                } else {
                    throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] read: Select failed. Result=%d.", mSocketName.chars(), result));
                }
            } else if (result == 0) {
                throw VException(VSTRING_FORMAT("VSocket[%s] read: Select timed out.", mSocketName.chars()));
            }

            //lint -e573 "Signed-unsigned mix with divide"
            if (!FD_ISSET(mSocketID, &readset)) {
                throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] read: Select got FD_ISSET false.", mSocketName.chars()));
            }

        }

        theNumBytesRead = (int) ::recv(mSocketID, (char*) nextBufferPositionPtr, (VSizeType) bytesRemainingToRead, VSOCKET_DEFAULT_RECV_FLAGS);

        if (theNumBytesRead < 0) {
            if (errno == EPIPE) {
                throw VSocketClosedException(errno, VSTRING_FORMAT("VSocket[%s] read: Socket has closed (EPIPE).", mSocketName.chars()));
            } else {
                throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] read: recv failed. Result=%d.", mSocketName.chars(), theNumBytesRead));
            }
        } else if (theNumBytesRead == 0) {
            if (mRequireReadAll) {
                throw VSocketClosedException(0, VSTRING_FORMAT("VSocket[%s] read: Socket has closed.", mSocketName.chars()));
            } else {
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

int VSocket::write(const Vu8* buffer, int numBytesToWrite) {
    if (mSocketID < 0) {
        throw VStackTraceException(VSTRING_FORMAT("VSocket[%s] write: Invalid socket ID %d.", mSocketName.chars(), mSocketID));
    }

    const Vu8*  nextBufferPositionPtr = buffer;
    int         bytesRemainingToWrite = numBytesToWrite;
    int         theNumBytesWritten;
    int         result = 0; // avoid compiler warning for possible uninitialized use
    fd_set      writeset;

    while (bytesRemainingToWrite > 0) {
        if (mSocketID <= FD_SETSIZE) { // FD_SETSIZE is max num open sockets, sockid over that is sign of a big problem
            FD_ZERO(&writeset);
            //lint -e573 "Signed-unsigned mix with divide"
            FD_SET(mSocketID, &writeset);
            result = ::select(mSocketID + 1, NULL, &writeset, NULL, (mWriteTimeOutActive ? &mWriteTimeOut : NULL));

            if (result < 0) {
                if (errno == EINTR) {
                    // Debug message: write was interrupted but we will cycle around and try again...
                    continue;
                }

                if (errno == EBADF) {
                    throw VSocketClosedException(errno, VSTRING_FORMAT("VSocket[%s] write: Socket has closed (EBADF).", mSocketName.chars()));
                } else {
                    throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] write: select() failed. Result=%d.", mSocketName.chars(), result));
                }
            } else if (result == 0) {
                throw VException(VSTRING_FORMAT("VSocket[%s] write: Select timed out.", mSocketName.chars()));
            }

        }

        theNumBytesWritten = (int) ::send(mSocketID, nextBufferPositionPtr, (VSizeType) bytesRemainingToWrite, VSOCKET_DEFAULT_SEND_FLAGS);

        if (theNumBytesWritten <= 0) {
            if (errno == EPIPE) {
                throw VSocketClosedException(errno, VSTRING_FORMAT("VSocket[%s] write: Socket has closed (EPIPE).", mSocketName.chars()));
            } else {
                throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] write: send() failed.", mSocketName.chars()));
            }
        } else if (theNumBytesWritten != bytesRemainingToWrite) {
            // Debug message: write was only partially completed so we will cycle around and write the rest...
        } else {
            // This is where you could put debug/trace-mode socket write logging output....
        }

        bytesRemainingToWrite -= theNumBytesWritten;
        nextBufferPositionPtr += theNumBytesWritten;

        mNumBytesWritten += theNumBytesWritten;
    }

    return (numBytesToWrite - bytesRemainingToWrite);
}

void VSocket::discoverHostAndPort() {
    struct sockaddr_in  info;
    VSocklenT           infoLength = sizeof(info);

    int result = ::getpeername(mSocketID, (struct sockaddr*) &info, &infoLength);
    if (result != 0) {
        throw VStackTraceException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] discoverHostAndPort: getpeername() failed.", mSocketName.chars()));
    }

    int portNumber = (int) V_BYTESWAP_NTOH_S16_GET(static_cast<Vs16>(info.sin_port));

    const char* name = ::inet_ntoa(info.sin_addr); // addr2ascii is preferred but is not yet standard:
    //const char* name = ::addr2ascii(AF_INET, info.sin_addr, sizeof(info.sin_addr), NULL);

    this->setHostAndPort(VSTRING_COPY(name), portNumber);
}

void VSocket::closeRead() {
    int result = ::shutdown(mSocketID, SHUT_RD);

    if (result < 0) {
        throw VException(VSTRING_FORMAT("VSocket[%s] closeRead: Unable to shut down socket.", mSocketName.chars()));
    }
}

void VSocket::closeWrite() {
    int result = ::shutdown(mSocketID, SHUT_WR);

    if (result < 0) {
        throw VException(VSTRING_FORMAT("VSocket[%s] closeWrite: Unable to shut down socket.", mSocketName.chars()));
    }
}

void VSocket::setSockOpt(int level, int name, void* valuePtr, int valueLength) {
    (void) ::setsockopt(mSocketID, level, name, valuePtr, valueLength);
}

#ifndef V_BSD_ENHANCED_SOCKETS

void VSocket::_connect() {
    struct sockaddr_in  info;
    int                 infoLength = sizeof(info);
    VSocketID           socketID;

    ::memset(&info, 0, sizeof(info));
    info.sin_family = AF_INET;
    info.sin_port = (in_port_t) V_BYTESWAP_HTON_S16_GET(static_cast<Vs16>(mPortNumber));
    info.sin_addr.s_addr = /*::hack_*/::inet_addr(mHostName);

    socketID = ::socket(AF_INET, SOCK_STREAM, 0);

    if (socketID >= 0) {
        int result = ::connect(socketID, (const sockaddr*) &info, infoLength);

        if (result != 0) {
            // Connect failed.
            VSystemError e = VSystemError::getSocketError(); // Call before calling closeSocket(), which will succeed and clear the error code!
            vault::close(socketID);
            throw VException(e, VSTRING_FORMAT("VSocket[%s] _connect: Connect failed.", mSocketName.chars()));
        }
    }

    mSocketID = socketID;
}

void VSocket::_listen(const VString& bindAddress, int backlog) {
    VSocketID           listenSockID = kNoSocketID;
    struct sockaddr_in  info;
    int                 infoLength = sizeof(info);
    const int           on = 1;
    int                 result;

    ::memset(&info, 0, sizeof(info));
    info.sin_family = AF_INET;
    info.sin_port = (in_port_t) V_BYTESWAP_HTON_S16_GET(static_cast<Vs16>(mPortNumber));

    if (bindAddress.isEmpty()) {
        info.sin_addr.s_addr = INADDR_ANY;
    } else {
        info.sin_addr.s_addr = inet_addr(bindAddress);
    }

    listenSockID = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSockID < 0) {
        throw VStackTraceException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] listen: socket() failed. Result=%d.", mSocketName.chars(), listenSockID));
    }

    result = ::setsockopt(listenSockID, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (result != 0) {
        throw VStackTraceException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] listen: setsockopt() failed. Result=%d.", mSocketName.chars(), result));
    }

    result = ::bind(listenSockID, (const sockaddr*) &info, infoLength);
    if (result != 0) {
        vault::close(listenSockID);
        throw VStackTraceException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] listen: bind() failed. Result=%d.", mSocketName.chars(), result));
    }

    (void) ::listen(listenSockID, backlog);
    if (result != 0) {
        vault::close(listenSockID);
        throw VStackTraceException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] listen: listen() failed. Result=%d.", mSocketName.chars(), result));
    }

    mSocketID = listenSockID;
}

#else /* V_BSD_ENHANCED_SOCKETS version follows */

void VSocket::_connect() {
    VSocketID           socketID = kNoSocketID;
    struct addrinfo*    res = NULL;

    this->_tcpGetAddrInfo(&res);

    try {
        socketID = this->_tcpConnectWAddrInfo(res);
    } catch (const VException& ex) {
        ::freeaddrinfo(res);
        throw;
    }

    ::freeaddrinfo(res);

    mSocketID = socketID;
}

void VSocket::_listen(const VString& bindAddress, int backlog) {
    VSocketID           listenSockID;
    int                 result;
    const int           on = 1;
    struct addrinfo     hints;
    struct addrinfo*    res;
    struct addrinfo*    ressave;
    VString             lastErrorText;
    VSystemError        lastSystemError(0, VString::EMPTY());

    ::memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    result = this->_getAddrInfo(&hints, &res, false);
    if (result != 0) {
        throw VStackTraceException(VSystemError(errno, ::gai_strerror(errno)), VSTRING_FORMAT("VSocket[%s] listen: getaddrinfo() failed. Result=%d.", mSocketName.chars(), result));
    }

    ressave = res;

    do {
        listenSockID = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

        if (listenSockID < 0) {
            // Error on this interface, we'll try the next one.
            lastErrorText.format("VSocket[%s] listen: Socket failed. ID=%d.", mSocketName.chars(), listenSocketID);
            lastSystemError = VSystemError::getSocketError();
            continue;
        }

        result = ::setsockopt(listenSockID, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if (result != 0) {
            // Error on this interface, we'll try the next one.
            lastErrorText.format("VSocket[%s] listen: Setsockopt failed. Result=%d.", mSocketName.chars(), result);
            lastSystemError = VSystemError::getSocketError();
            continue;
        }

        result = ::bind(listenSockID, res->ai_addr, res->ai_addrlen);

        if (result == 0) {
            // Success -- break out of loop to continue setting up listen.
            break;
        }

        // Error on this interface, we'll try the next one.
        lastErrorText.format("VSocket[%s] listen: Bind failed. Result=%d.", mSocketName.chars(), result);
        lastSystemError = VSystemError::getSocketError();
        vault::close(listenSockID);

    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        throw VStackTraceException(lastSystemError, lastErrorText);
    }

    if (lastErrorText.isNotEmpty()) {
        VString s(VSTRING_ARGS("VSocket[%s] listen: Bind succeeded after earlier error: ", mSocketName.chars()));
        std::cout << s << lastErrorText << std::endl;
    }

    result = ::listen(listenSockID, backlog);

    ::freeaddrinfo(ressave);

    if (result != 0) {
        vault::close(listenSockID);
        throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] listen: Listen failed. Result=%d", mSocketName.chars(), result));
    }

    mSocketID = listenSockID;
}

void VSocket::_tcpGetAddrInfo(struct addrinfo** res) {
    struct addrinfo hints;
    int             result;

    ::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    result = this->_getAddrInfo(&hints, res);

    if (result != 0) {
        throw VException(VSystemError(errno, ::gai_strerror(errno)), VSTRING_FORMAT("VSocket[%s] _tcpGetAddrInfo: GetAddrInfo failed. Result=%d.", mSocketName.chars(), result));
    }
}

int VSocket::_getAddrInfo(struct addrinfo* hints, struct addrinfo** res, bool useHostName) {
    VMutexLocker    locker(&gAddrInfoMutex, "VSocket::_getAddrInfo()");
    VString         portAsString(VSTRING_FORMATTER_INT, mPortNumber);

    return ::getaddrinfo(useHostName ? mHostName.chars() : NULL, portAsString.chars(), hints, res);
}

VSocketID VSocket::_tcpConnectWAddrInfo(struct addrinfo* const resInput) {
    struct addrinfo*    res = resInput;
    VSocketID           socketID;

    do {
        socketID = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

        if (socketID < 0) {
            // Debug message: socket open failed but we will try again if more available...
            continue;
        } else if (socketID > FD_SETSIZE) { // FD_SETSIZE is max num open sockets, sockid over that is sign of a big problem
            // Debug message: socket id appears to be out of range...
        }

        if (::connect(socketID, res->ai_addr, res->ai_addrlen) == 0) {
            break; /* success */
        }

        vault::close(socketID);
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        throw VException(VSystemError(errno, ::gai_strerror(errno)), VSTRING_FORMAT("VSocket[%s] _tcpConnectWAddrInfo: Socket/Connect failed.", mSocketName.chars()));
    }

    return socketID;
}

// The BSD call to getaddrinfo() is not threadsafe so we protect it with a global mutex.
VMutex VSocket::gAddrInfoMutex("VSocket::gAddrInfoMutex");

#endif /* V_BSD_ENHANCED_SOCKETS */

