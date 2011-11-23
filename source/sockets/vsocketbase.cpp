/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
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

    ipAddress = ::inet_ntoa(addr);
}

VSocketBase::VSocketBase(VSocketID id)
    : mSocketID(id)
    , mHostName()
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

VSocketBase::VSocketBase(const VString& hostName, int portNumber)
    : mSocketID(kNoSocketID)
    , mHostName(hostName)
    , mPortNumber(portNumber)
    , mReadTimeOutActive(false)
    , mReadTimeOut()
    , mWriteTimeOutActive(false)
    , mWriteTimeOut()
    , mRequireReadAll(true)
    , mNumBytesRead(0)
    , mNumBytesWritten(0)
    , mLastEventTime()
    , mSocketName(VSTRING_ARGS("%s:%d", hostName.chars(), portNumber))
    {
}

VSocketBase::~VSocketBase() {
    VSocketBase::close();
}

void VSocketBase::setHostAndPort(const VString& hostName, int portNumber) {
    mHostName = hostName;
    mPortNumber = portNumber;
    mSocketName.format("%s:%d", hostName.chars(), portNumber);
}

void VSocketBase::connect() {
    this->_connect();
    this->setDefaultSockOpt();
}

void VSocketBase::getHostName(VString& address) const {
    address = mHostName;
}

int VSocketBase::getPortNumber() const {
    return mPortNumber;
}

void VSocketBase::close() {
    if (mSocketID != kNoSocketID) {
#ifdef VPLATFORM_WIN
        ::closesocket(mSocketID);
#else
        vault::close(mSocketID);
#endif
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
    , mHostName()
    , mPortNumber(socket.getPortNumber())
    , mNumBytesRead(socket.numBytesRead())
    , mNumBytesWritten(socket.numBytesWritten())
    , mIdleTime(socket.getIdleTime())
    {
    socket.getHostName(mHostName);
}

