/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocket.h"
#include "vtypes_internal.h"

#include "vexception.h"

VString VSocketBase::gPreferredNetworkInterfaceName("en0");
VString VSocketBase::gPreferredLocalIPAddressPrefix;
VString VSocketBase::gCachedLocalHostIPAddress;

// static
void VSocketBase::setPreferredNetworkInterface(const VString& interfaceName) {
    gPreferredNetworkInterfaceName = interfaceName;
}

// static
void VSocketBase::setPreferredLocalIPAddressPrefix(const VString& addressPrefix) {
    gPreferredLocalIPAddressPrefix = addressPrefix;
}

// static
void VSocketBase::getLocalHostIPAddress(VString& ipAddress, bool refresh) {
    if (refresh || gCachedLocalHostIPAddress.isEmpty()) {
        VNetworkInterfaceList interfaces = VSocketBase::enumerateNetworkInterfaces();
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
VNetAddr VSocketBase::ipAddressStringToNetAddr(const VString& ipAddress) {
    in_addr_t addr = ::inet_addr(ipAddress);
    return (VNetAddr) addr;
}

// static
void VSocketBase::netAddrToIPAddressString(VNetAddr netAddr, VString& ipAddress) {
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
VStringVector VSocketBase::resolveHostName(const VString& hostName) {
    VStringVector resolvedAddresses;

    AddrInfoHintsHelper     hints(AF_UNSPEC, SOCK_STREAM, 0, 0); // accept IPv4 or IPv6, we'll skip any others on receipt; stream connections only, not udp.
    AddrInfoLifeCycleHelper info;
    int result = ::getaddrinfo(hostName.chars(), NULL, &hints.mHints, &info.mInfo); // TODO: iOS solution. If WWAN is asleep, calling getaddrinfo() in isolation may return an error. See CFHost API.
    
    if (result == 0) {
        for (const struct addrinfo* item = info.mInfo; item != NULL; item = item->ai_next) {
            if ((item->ai_family == AF_INET) || (item->ai_family == AF_INET6)) {
                resolvedAddresses.push_back(VSocket::addrinfoToIPAddressString(hostName, item));
            }
        }
    }

    if (result != 0) {
        throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocketBase::resolveHostName(%s): getaddrinfo returned %d.", hostName.chars(), result));
    }
    
    if (resolvedAddresses.empty()) {
        throw VException(VSTRING_FORMAT("VSocketBase::resolveHostName(%s): getaddrinfo did not resolve any addresses.", hostName.chars()));
    }
    
    return resolvedAddresses;
}

/*
This is a somewhat cursory check. The exact sequence and order of dots and decimals is not verified.
*/
// static
bool VSocketBase::isIPv4NumericString(const VString& s) {
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
bool VSocketBase::isIPv6NumericString(const VString& s) {
    int numColons = 0;
    
    for (int i = 0; i < s.length(); ++i) {
        if (! ((s[i] == ':') || (s[i] == '.') || s[i].isHexadecimal())) {
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
bool VSocketBase::isIPNumericString(const VString& s) {
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

VSocketBase::VSocketBase()
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

VSocketBase::VSocketBase(VSocketID id)
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

VSocketBase::~VSocketBase() {
    VSocketBase::close();
}

void VSocketBase::setHostIPAddressAndPort(const VString& hostIPAddress, int portNumber) {
    mHostIPAddress = hostIPAddress;
    mPortNumber = portNumber;
    mSocketName.format("%s:%d", hostIPAddress.chars(), portNumber);
}

void VSocketBase::connectToIPAddress(const VString& ipAddress, int portNumber) {
    this->_connectToIPAddress(ipAddress, portNumber);
    this->setDefaultSockOpt();
}

void VSocketBase::connectToHostName(const VString& hostName, int portNumber) {
    VStringVector ipAddresses = VSocketBase::resolveHostName(hostName);
    this->connectToIPAddress(ipAddresses[0], portNumber);
}

VString VSocketBase::getHostIPAddress() const {
    return mHostIPAddress;
}

int VSocketBase::getPortNumber() const {
    return mPortNumber;
}

void VSocketBase::close() {
    if (mSocketID != kNoSocketID) {
        vault::closeSocket(mSocketID);
        mSocketID = kNoSocketID;
    }
}

void VSocketBase::flush() {
    // If subclass needs to flush, it will override this method.
}

void VSocketBase::setIntSockOpt(int level, int name, int value) {
    int intValue = value;
    this->setSockOpt(level, name, static_cast<void*>(&intValue), sizeof(intValue));
}

void VSocketBase::setLinger(int val) {
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

void VSocketBase::clearReadTimeOut() {
    mReadTimeOutActive = false;
}

void VSocketBase::setReadTimeOut(const struct timeval& timeout) {
    mReadTimeOutActive = true;
    mReadTimeOut = timeout;
}

void VSocketBase::clearWriteTimeOut() {
    mWriteTimeOutActive = false;
}

void VSocketBase::setWriteTimeOut(const struct timeval& timeout) {
    mWriteTimeOutActive = true;
    mWriteTimeOut = timeout;
}

void VSocketBase::setDefaultSockOpt() {
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

Vs64 VSocketBase::numBytesRead() const {
    return mNumBytesRead;
}

Vs64 VSocketBase::numBytesWritten() const {
    return mNumBytesWritten;
}

VDuration VSocketBase::getIdleTime() const {
    VInstant now;
    return now - mLastEventTime;
}

VSocketID VSocketBase::getSockID() const {
    return mSocketID;
}

void VSocketBase::setSockID(VSocketID id) {
    mSocketID = id;
}

VSocketInfo::VSocketInfo(const VSocket& socket)
    : mSocketID(socket.getSockID())
    , mHostIPAddress(socket.getHostIPAddress())
    , mPortNumber(socket.getPortNumber())
    , mNumBytesRead(socket.numBytesRead())
    , mNumBytesWritten(socket.numBytesWritten())
    , mIdleTime(socket.getIdleTime())
    {
}

