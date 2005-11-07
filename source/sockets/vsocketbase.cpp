/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocket.h"

#include "vexception.h"

// static
void VSocketBase::getLocalHostIPAddress(VString& ipAddress)
    {
    char    name[256];
    int        result = ::gethostname(name, 255);

    if (result != 0)
        throw VException("VSocketBase::getLocalHostIPAddress: gethostname returned %d (errno %d = %s)", result, (int) errno, ::strerror(errno));

    struct hostent* ent = ::gethostbyname(name);
    
    if (ent == NULL)
        throw VException("VSocketBase::getLocalHostIPAddress: gethostbyname returned null (h_errno %d)", (int) h_errno);

    in_addr        addr;
    
    if (ent->h_addr_list[0] == NULL)
        throw VException("VSocketBase::getLocalHostIPAddress: no address entries.");

    addr.s_addr = *((in_addr_t*) (ent->h_addr_list[0]));
    
    ipAddress = ::inet_ntoa(addr);
    }

// static
VNetAddr VSocketBase::ipAddressStringToNetAddr(const VString& ipAddress)
    {
    in_addr_t    addr = ::inet_addr(ipAddress);
    return (VNetAddr) addr;
    }

// static
void VSocketBase::netAddrToIPAddressString(VNetAddr netAddr, VString& ipAddress)
    {
    in_addr    addr;
    
    addr.s_addr = (in_addr_t) netAddr;
    
    ipAddress = ::inet_ntoa(addr);
    }

VSocketBase::VSocketBase()
    {
    mReadTimeOutActive = false;
    mWriteTimeOutActive = false;
    mSockID = kNoSockID;
    mListenBacklog = 0;
    mRequireReadAll = true;
    mNumBytesRead = 0;
    mNumBytesWritten = 0;
    }

VSocketBase::~VSocketBase()
    {
    VSocketBase::close();
    }

void VSocketBase::init(VSockID id)
    {
    mSockID = id;

    this->discoverHostAndPort();
    this->setDefaultSockOpt();
    }

void VSocketBase::init(const VString& hostName, int portNumber)
    {
    mHostName = hostName;
    mPortNumber = portNumber;
    
    this->connect();
    this->setDefaultSockOpt();
    }

void VSocketBase::getHostName(VString& address) const
    {
    address = mHostName;
    }

int VSocketBase::getPortNumber() const
    {
    return mPortNumber;
    }

void VSocketBase::close()
    {
    if (mSockID >= 0)
        {
#ifdef VPLATFORM_WIN
        ::closesocket(mSockID);
#else
        ::close(mSockID);
#endif
        mSockID = kNoSockID;
        }
    }

void VSocketBase::flush()
    {
    // If subclass needs to flush, it will override this method.
    }

void VSocketBase::setLinger(int val)
    {
    struct linger    lingerParam;

    lingerParam.l_onoff = 1;
    lingerParam.l_linger = val; // max linger time while closing

    // turn linger on
    this->setSockOpt(SOL_SOCKET, SO_LINGER, static_cast<void*> (&lingerParam), sizeof(lingerParam));
    }

void VSocketBase::clearReadTimeOut()
    {
    mReadTimeOutActive = false;
    }

void VSocketBase::setReadTimeOut(const struct timeval& timeout)
    {
    mReadTimeOutActive = true;
    mReadTimeOut = timeout;
    }

void VSocketBase::clearWriteTimeOut()
    {
    mWriteTimeOutActive = false;
    }

void VSocketBase::setWriteTimeOut(const struct timeval& timeout)
    {
    mWriteTimeOutActive = true;
    mWriteTimeOut = timeout;
    }

void VSocketBase::setDefaultSockOpt()
    {
    int    value;

    // set buffer sizes
    value = kDefaultBufferSize;
    this->setSockOpt(SOL_SOCKET, SO_RCVBUF, static_cast<void*> (&value), sizeof(int));
    this->setSockOpt(SOL_SOCKET, SO_SNDBUF, static_cast<void*> (&value), sizeof(int));

#ifndef VPLATFORM_WIN
    // set type of service
    value = kDefaultServiceType;
    this->setSockOpt(IPPROTO_IP, IP_TOS, static_cast<void*> (&value), sizeof(int));
#endif

    // set no delay
    value = kDefaultNoDelay;
    this->setSockOpt(IPPROTO_TCP, TCP_NODELAY, static_cast<void*> (&value), sizeof(int));
    }

Vs64 VSocketBase::numBytesRead() const
    {
    return mNumBytesRead;
    }

Vs64 VSocketBase::numBytesWritten() const
    {
    return mNumBytesWritten;
    }

Vs64 VSocketBase::getIdleTime() const
    {
    VInstant    now;
    return now - mLastEventTime;
    }

VSockID VSocketBase::getSockID() const
    {
    return mSockID;
    }

void VSocketBase::setSockID(VSockID id)
    {
    mSockID = id;
    }

void VSocketBase::assertInvariant() const
    {
    }

VSocketInfo::VSocketInfo(const VSocket& socket)
    {
    socket.getHostName(mHostName);
    mSockID = socket.getSockID();
    mPortNumber = socket.getPortNumber();
    mNumBytesRead = socket.numBytesRead();
    mNumBytesWritten = socket.numBytesWritten();
    mIdleTime = socket.getIdleTime();
    }

