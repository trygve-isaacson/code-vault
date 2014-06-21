/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

#include "vmessageoutputthread.h"

#include "vexception.h"
#include "vclientsession.h"
#include "vsocket.h"
#include "vmessageinputthread.h"
#include "vlogger.h"

// VMessageOutputThread -------------------------------------------------------

VMessageOutputThread::VMessageOutputThread(const VString& threadBaseName, VSocket* socket, VListenerThread* ownerThread, VServer* server, VClientSessionPtr session, VMessageInputThread* dependentInputThread, int maxQueueSize, Vs64 maxQueueDataSize, const VDuration& maxQueueGracePeriod)
    : VSocketThread(threadBaseName, socket, ownerThread)
    , mOutputQueue()
    , mSocketStream(socket, "VMessageOutputThread")
    , mOutputStream(mSocketStream)
    , mServer(server)
    , mSession(session)
    , mDependentInputThread(dependentInputThread)
    , mMaxQueueSize(maxQueueSize)
    , mMaxQueueDataSize(maxQueueDataSize)
    , mMaxQueueGracePeriod(maxQueueGracePeriod)
    , mWhenMaxQueueSizeWarned(VInstant() - VDuration::MINUTE()) // one minute ago (past warning throttle threshold)
    , mWasOverLimit(false)
    , mWhenWentOverLimit(VInstant::NEVER_OCCURRED())
    {

    if (mDependentInputThread != NULL) {
        mDependentInputThread->setHasOutputThread(true);
    }
}

VMessageOutputThread::~VMessageOutputThread() {
    mOutputQueue.releaseAllMessages();

    /*
    We share the socket w/ the input thread. We sort of let the input
    thread be the owner. So we don't want our superclass to see
    mSocket and clean it up. Just set it to NULL so that the other
    class will be the one to do so.
    */
    mSocket = NULL;

    mServer = NULL;
    mDependentInputThread = NULL;
}

void VMessageOutputThread::run() {
    try {
        while (this->isRunning()) {
            this->_processNextOutboundMessage();
        }
    } catch (const VSocketClosedException& /*ex*/) {
        VLOGGER_NAMED_DEBUG(mLoggerName, VSTRING_FORMAT("[%s] VMessageOutputThread: Socket has closed, thread will end.", mName.chars()));
    } catch (const VException& ex) {
        /*
        Unlike the input threads, we shouldn't normally get an EOF exception to indicate that the
        connection has been closed normally, because we are an output thread. So any exceptions
        that land here uncaught are socket i/o errors and are logged as such. However, if our thread
        has been told to stop -- is no longer in running state -- then exceptions due to the socket
        being closed programmatically are to be expected, so we check that before logging an error.
        */
        if (this->isRunning()) {
            VLOGGER_NAMED_ERROR(mLoggerName, VSTRING_FORMAT("[%s] VMessageOutputThread::run: Exiting due to top level exception #%d '%s'.", mName.chars(), ex.getError(), ex.what()));
        } else {
            VLOGGER_NAMED_DEBUG(mLoggerName, VSTRING_FORMAT("[%s] VMessageOutputThread: Socket has closed, thread will end.", mName.chars()));
        }
    } catch (const std::exception& ex) {
        if (this->isRunning()) {
            VLOGGER_NAMED_ERROR(mLoggerName, VSTRING_FORMAT("[%s] VMessageOutputThread: Exiting due to top level exception '%s'.", mName.chars(), ex.what()));
        }
    } catch (...) {
        if (this->isRunning()) {
            VLOGGER_NAMED_ERROR(mLoggerName, VSTRING_FORMAT("[%s] VMessageOutputThread: Exiting due to top level unknown exception.", mName.chars()));
        }
    }

    if (mSession != nullptr) {
        mSession->shutdown(this);
    }

    if (mDependentInputThread != NULL) {
        mDependentInputThread->setHasOutputThread(false);
    }
}

void VMessageOutputThread::stop() {
    VSocketThread::stop();
    mOutputQueue.wakeUp(); // if it's blocked, this is needed to kick it back to its run loop
}

void VMessageOutputThread::attachSession(VClientSessionPtr session) {
    mSession = session;
}

bool VMessageOutputThread::postOutputMessage(VMessagePtr message, bool respectQueueLimits) {
    if (respectQueueLimits) {
        int currentQueueSize = 0;
        Vs64 currentQueueDataSize = 0;
        if (! this->isOutputQueueOverLimit(currentQueueSize, currentQueueDataSize)) {
            mWasOverLimit = false;
        } else {
            VInstant now;
            bool gracePeriodExceeded = false;

            if (mWasOverLimit) {
                // Still over limit. Have we exceeded the grace period?
                VDuration howLongOverLimit = now - mWhenWentOverLimit;
                gracePeriodExceeded = (howLongOverLimit > mMaxQueueGracePeriod);
            } else {
                // We've just gone over the limit.
                // If there is a grace period, note the time.
                if (mMaxQueueGracePeriod == VDuration::ZERO()) {
                    gracePeriodExceeded = true;
                } else {
                    mWhenWentOverLimit = now;
                    mWasOverLimit = true;
                }
            }

            if (gracePeriodExceeded) {
                if (this->isRunning()) { // Only stop() once; we may land here repeatedly under fast queueing, before stop completes.
                    VLOGGER_NAMED_ERROR(mLoggerName, VSTRING_FORMAT("[%s] VMessageOutputThread::postOutputMessage: Closing socket to shut down session because output queue size of %d messages and " VSTRING_FORMATTER_S64 " bytes is over limit.",
                                                 mName.chars(), currentQueueSize, currentQueueDataSize));

                    this->stop();
                }

                return false;
            } else {
                if (now - mWhenMaxQueueSizeWarned > VDuration::MINUTE()) { // Throttle the rate of ongoing warnings.
                    mWhenMaxQueueSizeWarned = now;
                    VDuration gracePeriodRemaining = (mWhenWentOverLimit + mMaxQueueGracePeriod) - now;
                    VLOGGER_NAMED_WARN(mLoggerName, VSTRING_FORMAT("[%s] VMessageOutputThread::postOutputMessage: Posting to queue with excess size of %d messages and " VSTRING_FORMATTER_S64 " bytes. Remaining grace period %d seconds.",
                                                mName.chars(), currentQueueSize, currentQueueDataSize, gracePeriodRemaining.getDurationSeconds()));
                }
            }
        }
    }

    bool posted = false;
    try {
        mOutputQueue.postMessage(message); // can throw bad_alloc if out of memory and queue cannot push_back
        posted = true;
    } catch (...) {
        VLOGGER_NAMED_ERROR(mLoggerName, VSTRING_FORMAT("[%s] VMessageOutputThread::postOutputMessage: Closing socket to shut down session because ran out memory.", mName.chars()));
        this->stop();
    }

    return posted;
}

void VMessageOutputThread::releaseAllQueuedMessages() {
    mOutputQueue.releaseAllMessages();
}

int VMessageOutputThread::getOutputQueueSize() const {
    return static_cast<int>(mOutputQueue.getQueueSize());
}

bool VMessageOutputThread::isOutputQueueOverLimit(int& currentQueueSize, Vs64& currentQueueDataSize) const {
    currentQueueSize = static_cast<int>(mOutputQueue.getQueueSize());
    currentQueueDataSize = mOutputQueue.getQueueDataSize();

    return (((mMaxQueueSize != 0) && (currentQueueSize >= mMaxQueueSize)) ||
            ((mMaxQueueDataSize != 0) && (currentQueueDataSize >= mMaxQueueDataSize)));
}

void VMessageOutputThread::_processNextOutboundMessage() {
    VMessagePtr message = mOutputQueue.blockUntilNextMessage();

    if (message == nullptr) {
        // OK -- means we were awakened from block but w/o a message actually available
    } else {
        if (mSession != nullptr) {
            mSession->sendMessageToClient(message, mName, mOutputStream);
        } else {
            // We are just a client. No "session". Just send.
            VLOGGER_NAMED_LEVEL(mLoggerName, VMessage::kMessageQueueOpsLevel, VSTRING_FORMAT("[%s] VMessageOutputThread::_processNextOutboundMessage: Sending message@0x%08X.", mName.chars(), message.get()));
            message->send(mName, mOutputStream);
        }
    }
}

