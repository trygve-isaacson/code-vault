/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

/** @file */

#include "vsocket.h"
#include "vtypes_internal.h"

#include "vexception.h"
#include "vmutexlocker.h"

V_STATIC_INIT_TRACE

// This is to force our _platform_staticInit to be called at startup.
bool VSocket::gStaticInited = VSocket::_platform_staticInit();

// VSocket ----------------------------------------------------------------

VString VSocket::gPreferredNetworkInterfaceName("en0");
VString VSocket::gPreferredLocalIPAddressPrefix;
VString VSocket::gCachedLocalHostIPAddress;

// static
void VSocket::setPreferredNetworkInterface(const VString& interfaceName) {
    gPreferredNetworkInterfaceName = interfaceName;
}

// static
void VSocket::setPreferredLocalIPAddressPrefix(const VString& addressPrefix) {
    gPreferredLocalIPAddressPrefix = addressPrefix;
}

// static
void VSocket::getLocalHostIPAddress(VString& ipAddress, bool refresh) {
    if (refresh || gCachedLocalHostIPAddress.isEmpty()) {
        VNetworkInterfaceList interfaces = VSocket::enumerateNetworkInterfaces();
        for (VNetworkInterfaceList::const_iterator i = interfaces.begin(); i != interfaces.end(); ++i) {
            // We want the first interface, but we keep going and use the preferred one if found.
            if ((i == interfaces.begin()) || ((*i).mName == gPreferredNetworkInterfaceName) || ((*i).mAddress.startsWith(gPreferredLocalIPAddressPrefix))) {
                gCachedLocalHostIPAddress = (*i).mAddress;

                // Break out of search if reason is that we found a preferred address.
                if (((*i).mName == gPreferredNetworkInterfaceName) || ((*i).mAddress.startsWith(gPreferredLocalIPAddressPrefix))) {
                    break;
                }
            }
        }
    }

    ipAddress = gCachedLocalHostIPAddress;
}

// static
VNetAddr VSocket::ipAddressStringToNetAddr(const VString& ipAddress) {
    in_addr_t addr = ::inet_addr(ipAddress);
    return (VNetAddr) addr;
}

// static
void VSocket::netAddrToIPAddressString(VNetAddr netAddr, VString& ipAddress) {
    in_addr addr;

    addr.s_addr = (in_addr_t) netAddr;

    ipAddress.copyFromCString(::inet_ntoa(addr));
}

class AddrInfoLifeCycleHelper {
    public:
        AddrInfoLifeCycleHelper() : mInfo(NULL) {}
        ~AddrInfoLifeCycleHelper() { ::freeaddrinfo(mInfo); }
        struct addrinfo* mInfo;
};

class AddrInfoHintsHelper {
    public:
        AddrInfoHintsHelper(int family, int socktype, int flags, int protocol) : mHints() {
            ::memset(&mHints, 0, sizeof(mHints));
            mHints.ai_family = family;
            mHints.ai_socktype = socktype;
            mHints.ai_flags = flags;
            mHints.ai_protocol = protocol;
        }
        ~AddrInfoHintsHelper() {}
        struct addrinfo mHints;
};

// static
VStringVector VSocket::resolveHostName(const VString& hostName) {
    VStringVector resolvedAddresses;

    AddrInfoHintsHelper     hints(AF_UNSPEC, SOCK_STREAM, 0, 0); // accept IPv4 or IPv6, we'll skip any others on receipt; stream connections only, not udp.
    AddrInfoLifeCycleHelper info;
    int result = ::getaddrinfo(hostName.chars(), NULL, &hints.mHints, &info.mInfo); // TODO: iOS solution. If WWAN is asleep, calling getaddrinfo() in isolation may return an error. See CFHost API.

    if (result == 0) {
        for (const struct addrinfo* item = info.mInfo; item != NULL; item = item->ai_next) {
            if ((item->ai_family == AF_INET) || (item->ai_family == AF_INET6)) {
                resolvedAddresses.push_back(VSocket::_platform_addrinfoToIPAddressString(hostName, item));
            }
        }
    }

    if (result != 0) {
        throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket::resolveHostName(%s): getaddrinfo returned %d.", hostName.chars(), result));
    }

    if (resolvedAddresses.empty()) {
        throw VException(VSTRING_FORMAT("VSocket::resolveHostName(%s): getaddrinfo did not resolve any addresses.", hostName.chars()));
    }

    return resolvedAddresses;
}

/*
This is a somewhat cursory check. The exact sequence and order of dots and decimals is not verified.
*/
// static
bool VSocket::isIPv4NumericString(const VString& s) {
    int numDots = 0;
    int numDecimalDigits = 0;

    for (int i = 0; i < s.length(); ++i) {
        if (s[i] == '.') {
            ++numDots;
            continue;
        }

        if (s[i].isNumeric()) {
            ++numDecimalDigits;
            continue;
        }

        return false; // Some other character that is not part of a numeric IPv4 address.
    }

    // A cursory check of minimum number of dots and digits. Order is not checked.
    return (numDots == 3) && (numDecimalDigits >= 4);
}

/*
There are lots of different forms possible. See RFC 2373.
We know there must be colons present, at least two.
The shortest possible value is "::".
There are usually several hexadecimal segments separated by colons.
There may also be dotted decimal (IPv4) elements at the end.
So we check that every character is a colon, a dot, or a hexadecimal.
And there must be two colons, so an explicit minimum length of 2 test is superfluous.
*/
// static
bool VSocket::isIPv6NumericString(const VString& s) {
    int numColons = 0;

    for (int i = 0; i < s.length(); ++i) {
        if (!((s[i] == ':') || (s[i] == '.') || s[i].isHexadecimal())) {
            return false;
        }

        if (s[i] == ':') {
            ++numColons;
        }
    }

    return (numColons >= 2); // The shortest possible IPv6 string is "::".
}

/*
Scan the string once, looking for signs that it's neither an IPv4 nor IPv6 numeric address.
If checking for either, this is faster than checking separately.
*/
// static
bool VSocket::isIPNumericString(const VString& s) {
    int numColons = 0;
    int numDots = 0;
    int numDecimalDigits = 0;
    int numNonDecimalHexDigits = 0;

    for (int i = 0; i < s.length(); ++i) {
        if (s[i] == ':') {
            ++numColons;
            continue;
        }

        if (s[i] == '.') {
            ++numDots;
            continue;
        }

        if (s[i].isNumeric()) {
            ++numDecimalDigits;
            continue;
        }

        if (s[i].isHexadecimal()) {
            ++numNonDecimalHexDigits;
            continue;
        }

        return false; // Some other character that is not part of a numeric IPv4 or IPv6 address.
    }

    // If we saw no colons (i.e., it's IPv4 dotted decimal) then there must be no A-F hex digits.
    if ((numColons == 0) && (numNonDecimalHexDigits != 0)) {
        return false;
    }

    // If we saw colons, it's IPv6 and the minimum is two colons.
    if (numColons != 0) {
        return (numColons >= 2); // The shortest possible IPv6 string is "::".
    }

    // We saw no colons, so the address should be IPv4. Cursory length check as in isIPv4NumericString().
    return (numDots == 3) && (numDecimalDigits >= 4); // A minimum of 4 digits separated by dots: "1.2.3.4"
}

VSocket::VSocket()
    : mSocketID(kNoSocketID)
    , mHostIPAddress()
    , mPortNumber(0)
    , mReadTimeOutActive(false)
    , mReadTimeOut()
    , mWriteTimeOutActive(false)
    , mWriteTimeOut()
    , mRequireReadAll(true)
    , mNumBytesRead(0)
    , mNumBytesWritten(0)
    , mLastEventTime()
    , mSocketName()
    {
}

VSocket::VSocket(VSocketID id)
    : mSocketID(id)
    , mHostIPAddress()
    , mPortNumber(0)
    , mReadTimeOutActive(false)
    , mReadTimeOut()
    , mWriteTimeOutActive(false)
    , mWriteTimeOut()
    , mRequireReadAll(true)
    , mNumBytesRead(0)
    , mNumBytesWritten(0)
    , mLastEventTime()
    , mSocketName()
    {
}

VSocket::~VSocket() {
    this->close();
}

void VSocket::setHostIPAddressAndPort(const VString& hostIPAddress, int portNumber) {
    mHostIPAddress = hostIPAddress;
    mPortNumber = portNumber;
    mSocketName.format("%s:%d", hostIPAddress.chars(), portNumber);
}

void VSocket::connectToIPAddress(const VString& ipAddress, int portNumber) {
    this->_connectToIPAddress(ipAddress, portNumber);
    this->setDefaultSockOpt();
}

void VSocket::connectToHostName(const VString& hostName, int portNumber) {
    this->connectToHostName(hostName, portNumber, VSocketConnectionStrategySingle());
}

void VSocket::connectToHostName(const VString& hostName, int portNumber, const VSocketConnectionStrategy& connectionStrategy) {
    connectionStrategy.connect(hostName, portNumber, *this);
}

VString VSocket::getHostIPAddress() const {
    return mHostIPAddress;
}

int VSocket::getPortNumber() const {
    return mPortNumber;
}

void VSocket::close() {
    if (mSocketID != kNoSocketID) {
        vault::closeSocket(mSocketID);
        mSocketID = kNoSocketID;
    }
}

void VSocket::flush() {
    // If subclass needs to flush, it will override this method.
}

void VSocket::setIntSockOpt(int level, int name, int value) {
    int intValue = value;
    this->setSockOpt(level, name, static_cast<void*>(&intValue), sizeof(intValue));
}

void VSocket::setLinger(int val) {
    struct linger lingerParam;

    lingerParam.l_onoff = 1;

#ifdef VPLATFORM_WIN
    lingerParam.l_linger = static_cast<u_short>(val); // max linger time while closing
#else
    lingerParam.l_linger = val; // max linger time while closing
#endif

    // turn linger on
    this->setSockOpt(SOL_SOCKET, SO_LINGER, static_cast<void*>(&lingerParam), sizeof(lingerParam));
}

void VSocket::clearReadTimeOut() {
    mReadTimeOutActive = false;
}

void VSocket::setReadTimeOut(const struct timeval& timeout) {
    mReadTimeOutActive = true;
    mReadTimeOut = timeout;
}

void VSocket::clearWriteTimeOut() {
    mWriteTimeOutActive = false;
}

void VSocket::setWriteTimeOut(const struct timeval& timeout) {
    mWriteTimeOutActive = true;
    mWriteTimeOut = timeout;
}

void VSocket::setDefaultSockOpt() {
    // set buffer sizes
    this->setIntSockOpt(SOL_SOCKET, SO_RCVBUF, kDefaultBufferSize);
    this->setIntSockOpt(SOL_SOCKET, SO_SNDBUF, kDefaultBufferSize);

#ifndef VPLATFORM_WIN
    // set type of service
    this->setIntSockOpt(IPPROTO_IP, IP_TOS, kDefaultServiceType);
#endif

#ifdef VPLATFORM_MAC
    // Normally, Unix systems will signal SIGPIPE if recv() or send() fails because the
    // other side has closed the socket. Not desirable; we'd rather get back an error code
    // like all other error types, so we can throw an exception. On Mac OS X we make this
    // happen by disabling SIG_PIPE here. On other Unix platforms we pass MSG_NOSIGNAL as
    // flags value for send() and recv() (see /_unix/vsocket.cpp).
    this->setIntSockOpt(SOL_SOCKET, SO_NOSIGPIPE, 1);
#endif

    // set no delay
    this->setIntSockOpt(IPPROTO_TCP, TCP_NODELAY, kDefaultNoDelay);
}

Vs64 VSocket::numBytesRead() const {
    return mNumBytesRead;
}

Vs64 VSocket::numBytesWritten() const {
    return mNumBytesWritten;
}

VDuration VSocket::getIdleTime() const {
    VInstant now;
    return now - mLastEventTime;
}

int VSocket::read(Vu8* buffer, int numBytesToRead) {
    if (! VSocket::_platform_isSocketIDValid(mSocketID)) {
        throw VStackTraceException(VSTRING_FORMAT("VSocket[%s] read: Invalid socket ID %d.", mSocketName.chars(), mSocketID));
    }

    int     bytesRemainingToRead = numBytesToRead;
    Vu8*    nextBufferPositionPtr = buffer;
    fd_set  readset;

    while (bytesRemainingToRead > 0) {

        FD_ZERO(&readset);
        FD_SET(mSocketID, &readset);
        int result = ::select(SelectSockIDTypeCast (mSocketID + 1), &readset, NULL, NULL, (mReadTimeOutActive ? &mReadTimeOut : NULL));

        if (result < 0) {
            VSystemError e = VSystemError::getSocketError();
            if (e.isLikePosixError(EINTR)) {
                // Debug message: read was interrupted but we will cycle around and try again...
                continue;
            }

            if (e.isLikePosixError(EBADF)) {
                throw VSocketClosedException(e, VSTRING_FORMAT("VSocket[%s] read: Socket has closed (EBADF).", mSocketName.chars()));
            } else {
                throw VException(e, VSTRING_FORMAT("VSocket[%s] read: Select failed. Result=%d.", mSocketName.chars(), result));
            }
        } else if (result == 0) {
            throw VException(VSTRING_FORMAT("VSocket[%s] read: Select timed out.", mSocketName.chars()));
        }

        if (!FD_ISSET(mSocketID, &readset)) {
            throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] read: Select got FD_ISSET false.", mSocketName.chars()));
        }

        int theNumBytesRead = SendRecvResultTypeCast ::recv(mSocketID, RecvBufferPtrTypeCast nextBufferPositionPtr, SendRecvByteCountTypeCast bytesRemainingToRead, VSOCKET_DEFAULT_RECV_FLAGS);

        if (theNumBytesRead < 0) {
            VSystemError e = VSystemError::getSocketError();
            if (e.isLikePosixError(EPIPE)) {
                throw VSocketClosedException(e, VSTRING_FORMAT("VSocket[%s] read: Socket has closed (EPIPE).", mSocketName.chars()));
            } else {
                throw VException(e, VSTRING_FORMAT("VSocket[%s] read: recv failed. Result=%d.", mSocketName.chars(), theNumBytesRead));
            }
        } else if (theNumBytesRead == 0) {
            if (mRequireReadAll) {
                throw VEOFException(VSTRING_FORMAT("VSocket[%s] read: recv of %d bytes returned 0 bytes.", mSocketName.chars(), bytesRemainingToRead));
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
    if (! VSocket::_platform_isSocketIDValid(mSocketID)) {
        throw VStackTraceException(VSTRING_FORMAT("VSocket[%s] write: Invalid socket ID %d.", mSocketName.chars(), mSocketID));
    }

    const Vu8*  nextBufferPositionPtr = buffer;
    int         bytesRemainingToWrite = numBytesToWrite;
    fd_set      writeset;

    while (bytesRemainingToWrite > 0) {

        FD_ZERO(&writeset);
        FD_SET(mSocketID, &writeset);
        int result = ::select(SelectSockIDTypeCast (mSocketID + 1), NULL, &writeset, NULL, (mWriteTimeOutActive ? &mWriteTimeOut : NULL));

        if (result < 0) {
            VSystemError e = VSystemError::getSocketError();
            if (e.isLikePosixError(EINTR)) {
                // Debug message: write was interrupted but we will cycle around and try again...
                continue;
            }

            if (e.isLikePosixError(EBADF)) {
                throw VSocketClosedException(e, VSTRING_FORMAT("VSocket[%s] write: Socket has closed (EBADF).", mSocketName.chars()));
            } else {
                throw VException(e, VSTRING_FORMAT("VSocket[%s] write: select() failed. Result=%d.", mSocketName.chars(), result));
            }
        } else if (result == 0) {
            throw VException(VSTRING_FORMAT("VSocket[%s] write: Select timed out.", mSocketName.chars()));
        }

        int theNumBytesWritten = SendRecvResultTypeCast ::send(mSocketID, SendBufferPtrTypeCast nextBufferPositionPtr, SendRecvByteCountTypeCast bytesRemainingToWrite, VSOCKET_DEFAULT_SEND_FLAGS);

        if (theNumBytesWritten <= 0) {
            VSystemError e = VSystemError::getSocketError();
            if (e.isLikePosixError(EPIPE)) {
                throw VSocketClosedException(e, VSTRING_FORMAT("VSocket[%s] write: Socket has closed (EPIPE).", mSocketName.chars()));
            } else {
                throw VException(e, VSTRING_FORMAT("VSocket[%s] write: send() failed.", mSocketName.chars()));
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

    const char* ipAddress = ::inet_ntoa(info.sin_addr);
    this->setHostIPAddressAndPort(VSTRING_COPY(ipAddress), portNumber);
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
    (void) ::setsockopt(mSocketID, level, name, SetSockOptValueTypeCast valuePtr, valueLength);
}

void VSocket::_connectToIPAddress(const VString& ipAddress, int portNumber) {
    this->setHostIPAddressAndPort(ipAddress, portNumber);

    bool        isIPv4 = VSocket::isIPv4NumericString(ipAddress);
    VSocketID   socketID = ::socket((isIPv4 ? AF_INET : AF_INET6), SOCK_STREAM, 0);

    if (VSocket::_platform_isSocketIDValid(socketID)) {

        const sockaddr* infoPtr = NULL;
        socklen_t infoLen = 0;
        struct sockaddr_in infoIPv4;
#ifdef VPLATFORM_WIN
        addrinfo addrInfo;
        AddrInfoLifeCycleHelper addrInfoResults;
#else
        struct sockaddr_in6 infoIPv6;
#endif

        if (isIPv4) {
            ::memset(&infoIPv4, 0, sizeof(infoIPv4));
            infoIPv4.sin_family = AF_INET;
            infoIPv4.sin_port = (in_port_t) V_BYTESWAP_HTON_S16_GET(static_cast<Vs16>(portNumber));
            infoIPv4.sin_addr.s_addr = ::inet_addr(ipAddress);
            infoPtr = (const sockaddr*) &infoIPv4;
            infoLen = sizeof(infoIPv4);
        } else {
#ifdef VPLATFORM_WIN
//#ifdef _WIN32_WINNT <= 0x501 // Cannot use inet_pton until Vista
            VString portString = VSTRING_INT(portNumber);
            ::memset(&addrInfo, 0, sizeof(addrInfo));
            addrInfo.ai_family = AF_INET6;
            addrInfo.ai_flags |= AI_NUMERICHOST;
            int getaddrinfoResult = ::getaddrinfo(ipAddress, portString, &addrInfo, &addrInfoResults.mInfo);
            if (getaddrinfoResult != 0) {
                throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] _connectToIPAddress: getaddrinfo() failed.", mSocketName.chars()));
            }
            infoPtr = (const sockaddr*) addrInfoResults.mInfo->ai_addr;
            infoLen = addrInfoResults.mInfo->ai_addrlen;
#else

            ::memset(&infoIPv6, 0, sizeof(infoIPv6));
//#ifndef VPLATFORM_WIN /* sin6_len is not defined in the Winsock definition! */
            infoIPv6.sin6_len = sizeof(infoIPv6);
//#endif
            infoIPv6.sin6_family = AF_INET6;
            infoIPv6.sin6_port = (in_port_t) V_BYTESWAP_HTON_S16_GET(static_cast<Vs16>(portNumber));
            int ptonResult = ::inet_pton(AF_INET6, ipAddress, &infoIPv6.sin6_addr);
            if (ptonResult != 1) {
                throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] _connectToIPAddress: inet_pton() failed.", mSocketName.chars()));
            }
            infoPtr = (const sockaddr*) &infoIPv6;
            infoLen = sizeof(infoIPv6);
#endif
        }

        int result = ::connect(socketID, infoPtr, infoLen);

        if (result != 0) {
            // Connect failed.
            VSystemError e = VSystemError::getSocketError(); // Call before calling vault::closeSocket(), which will succeed and clear the error code!
            vault::closeSocket(socketID);
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

    ::memset(&info, 0, sizeof(info));
    info.sin_family = AF_INET;
    info.sin_port = (in_port_t) V_BYTESWAP_HTON_S16_GET(static_cast<Vs16>(mPortNumber));

    if (bindAddress.isEmpty()) {
        info.sin_addr.s_addr = INADDR_ANY;
    } else {
        info.sin_addr.s_addr = inet_addr(bindAddress);
    }

    listenSockID = ::socket(AF_INET, SOCK_STREAM, 0);
    if (! VSocket::_platform_isSocketIDValid(listenSockID)) {
        throw VStackTraceException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] listen: socket() failed. Result=%d.", mSocketName.chars(), listenSockID));
    }

    // Once we've successfully called ::socket(), if something else fails here, we need
    // to close that socket. We can just throw upon any failed call, and use a try/catch
    // with re-throw after closure.

    try {
        int result = ::setsockopt(listenSockID, SOL_SOCKET, SO_REUSEADDR, SetSockOptValueTypeCast &on, sizeof(on));
        if (result != 0) {
            throw VStackTraceException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] listen: setsockopt() failed. Result=%d.", mSocketName.chars(), result));
        }

        result = ::bind(listenSockID, (const sockaddr*) &info, infoLength);
        if (result != 0) {
            throw VStackTraceException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] listen: bind() failed. Result=%d.", mSocketName.chars(), result));
        }

        result = ::listen(listenSockID, backlog);
        if (result != 0) {
            throw VStackTraceException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] listen: listen() failed. Result=%d.", mSocketName.chars(), result));
        }

    } catch (...) {
        vault::closeSocket(listenSockID);
        throw;
    }

    mSocketID = listenSockID;
}

VSocketID VSocket::getSockID() const {
    return mSocketID;
}

void VSocket::setSockID(VSocketID id) {
    mSocketID = id;
}

// VSocketInfo ----------------------------------------------------------------

VSocketInfo::VSocketInfo(const VSocket& socket)
    : mSocketID(socket.getSockID())
    , mHostIPAddress(socket.getHostIPAddress())
    , mPortNumber(socket.getPortNumber())
    , mNumBytesRead(socket.numBytesRead())
    , mNumBytesWritten(socket.numBytesWritten())
    , mIdleTime(socket.getIdleTime())
    {
}

// VSocketConnectionStrategySingle --------------------------------------------

void VSocketConnectionStrategySingle::connect(const VString& hostName, int portNumber, VSocket& socketToConnect) const {
    VStringVector ipAddresses = (mDebugIPAddresses.empty() ? VSocket::resolveHostName(hostName) : mDebugIPAddresses);
    socketToConnect.connectToIPAddress(ipAddresses[0], portNumber);
}

// VSocketConnectionStrategyLinear --------------------------------------------

VSocketConnectionStrategyLinear::VSocketConnectionStrategyLinear(const VDuration& timeout)
    : VSocketConnectionStrategy()
    , mTimeout(timeout)
    {
}

void VSocketConnectionStrategyLinear::connect(const VString& hostName, int portNumber, VSocket& socketToConnect) const {
    // Timeout should never cause expiration before we do DNS resolution or try the first IP address.
    // Therefore, we calculate the expiration time, but then to DNS first, and check timeout after each failed connect.
    VInstant expirationTime = VInstant() + mTimeout;
    VStringVector ipAddresses = (mDebugIPAddresses.empty() ? VSocket::resolveHostName(hostName) : mDebugIPAddresses);
    for (VStringVector::const_iterator i = ipAddresses.begin(); i != ipAddresses.end(); ++i) {
        try {
            socketToConnect.connectToIPAddress(*i, portNumber);
            return; // As soon as we succeed, return.
        } catch (const VException& ex) {
            VLOGGER_TRACE(VSTRING_FORMAT("VSocketConnectionStrategyLinear::connect(%s): Failed to connect to '%s'. %s", hostName.chars(), (*i).chars(), ex.what()));
            if (VInstant(/*now*/) >= expirationTime) {
                throw;
            }
        }
    }

    throw VException("VSocketConnectionStrategyLinear::connect: Failed to connect to all resolved names.");
}

// VSocketConnectionStrategyThreadedWorker ------------------------------------

class VSocketConnectionStrategyThreadedRunner;

class VSocketConnectionStrategyThreadedWorker : public VThread {
    public:

        VSocketConnectionStrategyThreadedWorker(VSocketConnectionStrategyThreadedRunner* ownerRunner, const VString& ipAddressToConnect, int portNumberToConnect);
        virtual ~VSocketConnectionStrategyThreadedWorker();

        // VThread implementation:
        virtual void run();

    private:

        // These contain the code to communicate safely with the owner (which is running in another thread),
        // to let it know (if it's still around!) that we are done in either fashion.
        void _handleSuccess(VSocket& openedSocket);
        void _handleFailure(const VException& ex);

        VMutex                                      mMutex;
        VSocketConnectionStrategyThreadedRunner*    mOwnerRunner;
        VString                                     mIPAddressToConnect;
        int                                         mPortNumberToConnect;
};

// VSocketConnectionStrategyThreadedRunner ------------------------------------

/**
Because the strategy involves creating multiple threads but wanting to proceed as
soon as 1 of them succeeds, we need an intermediary thread object that can live
longer and wait around for all of the workers to complete and properly bookkeep
them. This "runner" class manages all communication with the workers, pokes the
strategy object back immediately upon success (at which point the strategy can
let go of the runner and proceed), and hangs around until all worker threads have
communicated their completion.
*/
class VSocketConnectionStrategyThreadedRunner : public VThread {
    public:
        VSocketConnectionStrategyThreadedRunner(const VDuration& timeoutInterval, int maxNumThreads, const VString& hostName, int portNumber, const VStringVector& debugIPAddresses);
        virtual ~VSocketConnectionStrategyThreadedRunner();

        // VThread implementation:
        virtual void run();

        // Caller should start() this thread, and then aggressively check for completion via hasAnswer().
        // Once done, call getConnectedSockID(), and if it's not kNoSockID, it's a connected sockid to take over.
        // Call getConnectedIPAddress() to find out where we got connected to.
        // Finally, call detachFromStrategy() to signal that you will no longer refer to the runner,
        // so that it can self-destruct.
        bool        hasAnswer() const;
        VSocketID   getConnectedSockID() const;
        VString     getConnectedIPAddress() const;
        void        detachFromStrategy();

    private:

        bool _isDone() const;
        bool _isDetachedFromStrategy() const;
        void _lockedStartWorker(const VString& ipAddressToConnect);
        void _lockedForgetOneWorker(VSocketConnectionStrategyThreadedWorker* worker); // forgets one worker but assumes that worker will no longer reference us
        void _lockedForgetAllWorkers(); // forgets all workers and tells them to stop referring to us

        const VInstant      mExpiry;    // Construction time plus timeout interval. After this instant, we stop creating new threads.
        const int           mMaxNumThreads;
        const VString       mHostNameToConnect;
        const int           mPortNumberToConnect;
        const VStringVector mDebugIPAddresses;

        bool            mDetachedFromStrategy;
        mutable VMutex  mMutex;
        VStringVector   mIPAddressesYetToTry;

        bool            mConnectionCompleted;
        bool            mAllWorkersFailed;
        VSocketID       mConnectedSocketID;
        VString         mConnectedSocketIPAddress;

        typedef std::deque<VSocketConnectionStrategyThreadedWorker*> WorkerList;
        WorkerList      mWorkers;

        // Private functions called only by our worker friend class.
        friend class VSocketConnectionStrategyThreadedWorker;
        void _workerSucceeded(VSocketConnectionStrategyThreadedWorker* worker, VSocket& openedSocket);
        void _workerFailed(VSocketConnectionStrategyThreadedWorker* worker, const VException& ex);

};

// VSocketConnectionStrategyThreadedWorker ------------------------------------

VSocketConnectionStrategyThreadedWorker::VSocketConnectionStrategyThreadedWorker(VSocketConnectionStrategyThreadedRunner* ownerRunner, const VString& ipAddressToConnect, int portNumberToConnect)
    : VThread(VSTRING_FORMAT("VSocketConnectionStrategyThreadedWorker.%s:%d", ipAddressToConnect.chars(), portNumberToConnect), "vault.sockets.VSocketConnectionStrategyThreadedWorker", kDeleteSelfAtEnd, kCreateThreadDetached, NULL)
    , mMutex(mName)
    , mOwnerRunner(ownerRunner)
    , mIPAddressToConnect(ipAddressToConnect)
    , mPortNumberToConnect(portNumberToConnect)
    {
    VLOGGER_TRACE(VSTRING_FORMAT("VSocketConnectionStrategyThreadedWorker %s:%d constructor.", mIPAddressToConnect.chars(), mPortNumberToConnect));
}

VSocketConnectionStrategyThreadedWorker::~VSocketConnectionStrategyThreadedWorker() {
    VLOGGER_TRACE(VSTRING_FORMAT("VSocketConnectionStrategyThreadedWorker %s:%d destructor.", mIPAddressToConnect.chars(), mPortNumberToConnect));
}

void VSocketConnectionStrategyThreadedWorker::run() {
    VInstant connectStart;
    try {
        VSocket tempSocket;
        tempSocket.connectToIPAddress(mIPAddressToConnect, mPortNumberToConnect);
        VDuration duration(connectStart);
        VLOGGER_TRACE(VSTRING_FORMAT("VSocketConnectionStrategyThreadedWorker %s:%d run() succeeded with sockid %d in %s.", mIPAddressToConnect.chars(), mPortNumberToConnect, (int) tempSocket.getSockID(), duration.getDurationString().chars()));
        this->_handleSuccess(tempSocket);
    } catch (const VException& ex) {
        VDuration duration(connectStart);
        VLOGGER_TRACE(VSTRING_FORMAT("VSocketConnectionStrategyThreadedWorker %s:%d run() failed in %s.", mIPAddressToConnect.chars(), mPortNumberToConnect, duration.getDurationString().chars()));
        this->_handleFailure(ex);
    }
}

void VSocketConnectionStrategyThreadedWorker::_handleSuccess(VSocket& openedSocket) {
    VMutexLocker locker(&mMutex, "VSocketConnectionStrategyThreadedWorker::_handleSuccess");
    if (mOwnerRunner != NULL) {
        mOwnerRunner->_workerSucceeded(this, openedSocket);
        mOwnerRunner = NULL;
    }
}

void VSocketConnectionStrategyThreadedWorker::_handleFailure(const VException& ex) {
    VMutexLocker locker(&mMutex, "VSocketConnectionStrategyThreadedWorker::_handleFailure");
    if (mOwnerRunner != NULL) {
        mOwnerRunner->_workerFailed(this, ex);
        mOwnerRunner = NULL;
    }
}

// VSocketConnectionStrategyThreadedRunner ------------------------------------

VSocketConnectionStrategyThreadedRunner::VSocketConnectionStrategyThreadedRunner(const VDuration& timeoutInterval, int maxNumThreads, const VString& hostName, int portNumber, const VStringVector& debugIPAddresses)
    : VThread(VSTRING_FORMAT("VSocketConnectionStrategyThreadedRunner.%s:%d", hostName.chars(), portNumber), "vault.sockets.VSocketConnectionStrategyThreadedRunner", kDeleteSelfAtEnd, kCreateThreadDetached, NULL)
    , mExpiry(VInstant() + timeoutInterval)
    , mMaxNumThreads(maxNumThreads)
    , mHostNameToConnect(hostName)
    , mPortNumberToConnect(portNumber)
    , mDebugIPAddresses(debugIPAddresses)
    , mDetachedFromStrategy(false)
    , mMutex(mName)
    , mIPAddressesYetToTry()
    , mConnectionCompleted(false)
    , mAllWorkersFailed(false)
    , mConnectedSocketID(VSocket::kNoSocketID)
    , mConnectedSocketIPAddress()
    , mWorkers()
    {
    VLOGGER_TRACE(VSTRING_FORMAT("VSocketConnectionStrategyThreadedRunner %s:%d constructor.", mHostNameToConnect.chars(), mPortNumberToConnect));
}

VSocketConnectionStrategyThreadedRunner::~VSocketConnectionStrategyThreadedRunner() {
    VLOGGER_TRACE(VSTRING_FORMAT("VSocketConnectionStrategyThreadedRunner %s:%d destructor.", mHostNameToConnect.chars(), mPortNumberToConnect));
}

void VSocketConnectionStrategyThreadedRunner::run() {

    /* locking scope */ {
        VMutexLocker locker(&mMutex, "VSocketConnectionStrategyThreadedRunner::run() starting initial workers");
        VStringVector ipAddresses = (mDebugIPAddresses.empty() ? VSocket::resolveHostName(mHostNameToConnect) : mDebugIPAddresses);
        int numWorkersRemaining = mMaxNumThreads;
        for (size_t i = 0; i < ipAddresses.size(); ++i) {
            //for (size_t i1 = ipAddresses.size(); i1 > 0; --i1) { int i = i1-1; // try backwards to get that google.com IPv6 address
            if (numWorkersRemaining == 0) {
                mIPAddressesYetToTry.push_back(ipAddresses[i]);
            } else {
                this->_lockedStartWorker(ipAddresses[i]);
                --numWorkersRemaining;
            }
        }
    }

    // More workers will be created when and if others complete unsuccessfully.

    while (! this->_isDone()) {
        VThread::sleep(VDuration::MILLISECOND());
    }

    while (! this->_isDetachedFromStrategy()) {
        VThread::sleep(VDuration::MILLISECOND());
    }

}

bool VSocketConnectionStrategyThreadedRunner::hasAnswer() const {
    VMutexLocker locker(&mMutex, "hasAnswer");
    return mConnectionCompleted || mAllWorkersFailed || (VInstant() > mExpiry);
}

VSocketID VSocketConnectionStrategyThreadedRunner::getConnectedSockID() const {
    VMutexLocker locker(&mMutex, "getConnectedSockID");
    return mConnectedSocketID;
}

VString VSocketConnectionStrategyThreadedRunner::getConnectedIPAddress() const {
    VMutexLocker locker(&mMutex, "getConnectedIPAddress");
    return mConnectedSocketIPAddress;
}

void VSocketConnectionStrategyThreadedRunner::detachFromStrategy() {
    VMutexLocker locker(&mMutex, "detachFromStrategy");
    mDetachedFromStrategy = true;
}

bool VSocketConnectionStrategyThreadedRunner::_isDone() const {
    VMutexLocker locker(&mMutex, "_done");
    return mWorkers.empty();
}

bool VSocketConnectionStrategyThreadedRunner::_isDetachedFromStrategy() const {
    VMutexLocker locker(&mMutex, "_isDetachedFromStrategy");
    return mDetachedFromStrategy;
}

void VSocketConnectionStrategyThreadedRunner::_lockedStartWorker(const VString& ipAddressToConnect) {
    VLOGGER_TRACE(VSTRING_FORMAT("VSocketConnectionStrategyThreadedRunner starting worker %s:%d.", ipAddressToConnect.chars(), mPortNumberToConnect));
    VSocketConnectionStrategyThreadedWorker* worker = new VSocketConnectionStrategyThreadedWorker(this, ipAddressToConnect, mPortNumberToConnect);
    mWorkers.push_back(worker);
    worker->start();
}

void VSocketConnectionStrategyThreadedRunner::_workerSucceeded(VSocketConnectionStrategyThreadedWorker* worker, VSocket& openedSocket) {
    VMutexLocker locker(&mMutex, VSTRING_FORMAT("_workerSucceeded(%s)", worker->getName().chars()));
    if (mConnectionCompleted) {
        VLOGGER_TRACE(VSTRING_FORMAT("VSocketConnectionStrategyThreadedRunner %s:%d _workerSucceeded(sockid %d) ignored because another worker has already won.", openedSocket.getHostIPAddress().chars(), mPortNumberToConnect, (int) openedSocket.getSockID()));
    } else {
        VLOGGER_TRACE(VSTRING_FORMAT("VSocketConnectionStrategyThreadedRunner %s:%d _workerSucceeded(sockid %d) wins.", openedSocket.getHostIPAddress().chars(), mPortNumberToConnect, (int) openedSocket.getSockID()));

        mConnectedSocketID = openedSocket.getSockID();
        mConnectedSocketIPAddress = openedSocket.getHostIPAddress();
        openedSocket.setSockID(VSocket::kNoSocketID); // So when it destructs on return from this function, it will NOT close the adopted socket ID.

        mConnectionCompleted = true;
    }

    this->_lockedForgetOneWorker(worker);
}

void VSocketConnectionStrategyThreadedRunner::_workerFailed(VSocketConnectionStrategyThreadedWorker* worker, const VException& ex) {
    VMutexLocker locker(&mMutex, VSTRING_FORMAT("_workerFailed(%s)", worker->getName().chars()));
    this->_lockedForgetOneWorker(worker);

    VLOGGER_ERROR(VSTRING_FORMAT("VSocketConnectionStrategyThreadedRunner::_workerFailed: %s", ex.what()));

    // If we have yet to succeed, start another worker thread if we have more addresses to try.
    if (!mConnectionCompleted) {
        if (mIPAddressesYetToTry.empty()) {
            // Nothing left to try.
        } else if (VInstant() > mExpiry) {
            // Too much time has elapsed. Give up. Don't start a new worker. Clear the "to do" list.
            // Mark failure so that the caller can immediately proceed, not waiting for any other
            // outstanding workers to complete. The presence of an overdue expiry means we failed.
            mIPAddressesYetToTry.clear();
            mAllWorkersFailed = true;
        } else {
            // Pop the next address off and start a worker for it.
            VString nextIPAddressToTry = mIPAddressesYetToTry[0];
            mIPAddressesYetToTry.erase(mIPAddressesYetToTry.begin());
            this->_lockedStartWorker(nextIPAddressToTry);
        }
    }

    // If that failure was the last worker, then now that it's gone there are no more workers,
    // because we didn't just start another one in its place, then we failed.
    if (mWorkers.empty()) {
        mAllWorkersFailed = true;
    }
}

void VSocketConnectionStrategyThreadedRunner::_lockedForgetOneWorker(VSocketConnectionStrategyThreadedWorker* worker) {
    WorkerList::iterator position = std::find(mWorkers.begin(), mWorkers.end(), worker);
    if (position != mWorkers.end()) {
        mWorkers.erase(position);
    }
}

void VSocketConnectionStrategyThreadedRunner::_lockedForgetAllWorkers() {
    mWorkers.clear();
}

// VSocketConnectionStrategyThreaded ------------------------------------------

VSocketConnectionStrategyThreaded::VSocketConnectionStrategyThreaded(const VDuration& timeoutInterval, int maxNumThreads)
    : VSocketConnectionStrategy()
    , mTimeoutInterval(timeoutInterval)
    , mMaxNumThreads(maxNumThreads)
    {
}

void VSocketConnectionStrategyThreaded::connect(const VString& hostName, int portNumber, VSocket& socketToConnect) const {

    VSocketConnectionStrategyThreadedRunner* runner = new VSocketConnectionStrategyThreadedRunner(mTimeoutInterval, mMaxNumThreads, hostName, portNumber, mDebugIPAddresses);
    runner->start();

    while (! runner->hasAnswer()) {
        VThread::sleep(VDuration::MILLISECOND());
    }

    VSocketID sockID = runner->getConnectedSockID();
    if (sockID == VSocket::kNoSocketID) {
        throw VException("VSocketConnectionStrategyThreaded::connect: Failed to connect to all addresses.");
    } else {
        socketToConnect.setSockID(sockID);
        socketToConnect.setHostIPAddressAndPort(runner->getConnectedIPAddress(), portNumber);
    }

    // Finally, let the runner know that it is safe for it to end because we are no longer referring to it.
    // It may still need to bookkeep worker threads that have not yet completed. It will self-delete later.
    runner->detachFromStrategy();
    runner = NULL;

    VLOGGER_TRACE(VSTRING_FORMAT("VSocketConnectionStrategyThreaded::connect(%s, %d) completed successfully at %s.", hostName.chars(), portNumber, socketToConnect.getHostIPAddress().chars()));
}
