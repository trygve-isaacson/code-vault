/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocket.h"
#include "vtypes_internal.h"

#include "vexception.h"
#include "vmutexlocker.h"

// VSocketBase ----------------------------------------------------------------

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
    this->connectToHostName(hostName, portNumber, VSocketConnectionStrategySingle());
}

void VSocketBase::connectToHostName(const VString& hostName, int portNumber, const VSocketConnectionStrategy& connectionStrategy) {
    connectionStrategy.connect(hostName, portNumber, *this);
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

void VSocketConnectionStrategySingle::connect(const VString& hostName, int portNumber, VSocketBase& socketToConnect) const {
    VStringVector ipAddresses = (mDebugIPAddresses.empty() ? VSocketBase::resolveHostName(hostName) : mDebugIPAddresses);
    socketToConnect.connectToIPAddress(ipAddresses[0], portNumber);
}

// VSocketConnectionStrategyLinear --------------------------------------------

VSocketConnectionStrategyLinear::VSocketConnectionStrategyLinear(const VDuration& timeout)
    : VSocketConnectionStrategy()
    , mTimeout(timeout)
    {
}

void VSocketConnectionStrategyLinear::connect(const VString& hostName, int portNumber, VSocketBase& socketToConnect) const {
    // Timeout should never cause expiration before we do DNS resolution or try the first IP address.
    // Therefore, we calculate the expiration time, but then to DNS first, and check timeout after each failed connect.
    VInstant expirationTime = VInstant() + mTimeout;
    VStringVector ipAddresses = (mDebugIPAddresses.empty() ? VSocketBase::resolveHostName(hostName) : mDebugIPAddresses);
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
    , mConnectedSocketID(VSocketBase::kNoSocketID)
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
        VStringVector ipAddresses = (mDebugIPAddresses.empty() ? VSocketBase::resolveHostName(mHostNameToConnect) : mDebugIPAddresses);
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
        openedSocket.setSockID(VSocketBase::kNoSocketID); // So when it destructs on return from this function, it will NOT close the adopted socket ID.

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

void VSocketConnectionStrategyThreaded::connect(const VString& hostName, int portNumber, VSocketBase& socketToConnect) const {

    VSocketConnectionStrategyThreadedRunner* runner = new VSocketConnectionStrategyThreadedRunner(mTimeoutInterval, mMaxNumThreads, hostName, portNumber, mDebugIPAddresses);
    runner->start();

    while (! runner->hasAnswer()) {
        VThread::sleep(VDuration::MILLISECOND());
    }

    VSocketID sockID = runner->getConnectedSockID();
    if (sockID == VSocketBase::kNoSocketID) {
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
