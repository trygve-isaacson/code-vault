/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

#include "vmessagequeue.h"

#include "vmutexlocker.h"
#include "vmessage.h"
#include "vlogger.h"

// VMessageQueue --------------------------------------------------------------

VDuration VMessageQueue::gVMessageQueueLagLoggingThreshold(-1 * VDuration::MILLISECOND()); // -1 means we don't examine the lag time at all
int VMessageQueue::gVMessageQueueLagLoggingLevel(VLoggerLevel::DEBUG);

VMessageQueue::VMessageQueue()
    : mQueuedMessages()
    , mQueuedMessagesDataSize(0)
    , mMessageQueueMutex("VMessageQueue::mMessageQueueMutex")
    , mMessageQueueSemaphore()
    , mLastMessagePostTime()
    {
}

VMessageQueue::~VMessageQueue() {
    // 4.0: VMessagePtr means no more need to manually release mQueuedMessages contents.
}

void VMessageQueue::postMessage(VMessagePtr message) {
    VMutexLocker locker(&mMessageQueueMutex, "VMessageQueue::postMessage()");

    mQueuedMessages.push_back(message);
    mLastMessagePostTime.setNow();

    if (message != nullptr) {
        mQueuedMessagesDataSize += message->getMessageDataLength();
    }

    locker.unlock();    // otherwise signal() will deadlock
    mMessageQueueSemaphore.signal();
}

VMessagePtr VMessageQueue::blockUntilNextMessage() {
    // If there is a message on the queue, we can simply return it.
    VMessagePtr message = this->getNextMessage();
    if (message != nullptr) {
        return message;
    }

    // There is nothing on the queue, so wait until someone posts a message.
    VMutex dummy("VMessageQueue::blockUntilNextMessage() dummy");
    mMessageQueueSemaphore.wait(&dummy, 5 * VDuration::SECOND());

    return this->getNextMessage();
}

VMessagePtr VMessageQueue::getNextMessage() {
    VMessagePtr message;

    VMutexLocker locker(&mMessageQueueMutex, "VMessageQueue::getNextMessage()");

    if (mQueuedMessages.size() > 0) {
        message = mQueuedMessages.front();
        mQueuedMessages.pop_front();

        if (message != nullptr) {
            mQueuedMessagesDataSize -= message->getMessageDataLength();
        }

    }

    if ((message != nullptr) && (gVMessageQueueLagLoggingThreshold >= VDuration::ZERO())) {
        VInstant now;
        VDuration delayInterval = now - mLastMessagePostTime;
        if (delayInterval >= gVMessageQueueLagLoggingThreshold) {
            VLOGGER_NAMED_LEVEL("vault.messages.VMessageQueue", gVMessageQueueLagLoggingLevel, VSTRING_FORMAT("VMessageQueue saw a delay of %s when getting a message with ID %d.", delayInterval.getDurationString().chars(), message->getMessageID()));
        }
    }

    return message;
}

void VMessageQueue::wakeUp() {
    mMessageQueueSemaphore.signal();
}

VSizeType VMessageQueue::getQueueSize() const {
    // No need to lock here, nothing bad can happen underneath us.
    return mQueuedMessages.size();
}

Vs64 VMessageQueue::getQueueDataSize() const {
    // No need to lock here, nothing bad can happen underneath us.
    return mQueuedMessagesDataSize;
}

void VMessageQueue::releaseAllMessages() {
    VMutexLocker locker(&mMessageQueueMutex, "VMessageQueue::releaseAllMessages()");

    while (mQueuedMessages.size() > 0) {
        VMessagePtr message = mQueuedMessages.front();
        mQueuedMessages.pop_front();

        if (message != nullptr) {
            mQueuedMessagesDataSize -= message->getMessageDataLength();
        }
    }
}

