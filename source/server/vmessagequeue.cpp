/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

#include "vmessagequeue.h"

#include "vmutexlocker.h"
#include "vmessage.h"
#include "vlogger.h"

// VMessageQueue --------------------------------------------------------------

VDuration VMessageQueue::gVMessageQueueLagLoggingThreshold(-1 * VDuration::MILLISECOND()); // -1 means we don't examine the lag time at all
int VMessageQueue::gVMessageQueueLagLoggingLevel(VLogger::kDebug);

VMessageQueue::VMessageQueue() :
mQueuedMessages(), // -> empty
mQueuedMessagesDataSize(0),
mMessageQueueMutex("VMessageQueue::mMessageQueueMutex"), // -> unlocked
mMessageQueueSemaphore(), // -> unsignaled
mLastMessagePostTime()
    {
    }

VMessageQueue::~VMessageQueue()
    {
    // Delete all messages remaining in the queue.

    try {
        VMutexLocker locker(&mMessageQueueMutex, "~VMessageQueue()");

        for (VSizeType i = 0; i < mQueuedMessages.size(); ++i)
            delete mQueuedMessages[i];
        }
    catch (...) { }    // block exceptions from propagating
    }

void VMessageQueue::postMessage(VMessage* message)
    {
    VMutexLocker locker(&mMessageQueueMutex, "VMessageQueue::postMessage()");

    mQueuedMessages.push_back(message);
    mLastMessagePostTime.setNow();

    if (message != NULL)
        mQueuedMessagesDataSize += message->getMessageDataLength();

    locker.unlock();    // otherwise signal() will deadlock
    mMessageQueueSemaphore.signal();
    }

VMessage* VMessageQueue::blockUntilNextMessage()
    {
    // If there is a message on the queue, we can simply return it.
    VMessage* message = this->getNextMessage();
    if (message != NULL)
        return message;

    // There is nothing on the queue, so wait until someone posts a message.
    VMutex    dummy("VMessageQueue::blockUntilNextMessage() dummy");
    mMessageQueueSemaphore.wait(&dummy, 5 * VDuration::SECOND());
    
    return this->getNextMessage();
    }

VMessage* VMessageQueue::getNextMessage()
    {
    VMessage* message = NULL;

    VMutexLocker locker(&mMessageQueueMutex, "VMessageQueue::getNextMessage()");
    
    if (mQueuedMessages.size() > 0)
        {
        message = mQueuedMessages.front();
        mQueuedMessages.pop_front();

        if (message != NULL)
            mQueuedMessagesDataSize -= message->getMessageDataLength();

        }

    if ((message != NULL) && (gVMessageQueueLagLoggingThreshold >= VDuration::ZERO()))
        {
        VInstant now;
        VDuration delayInterval = now - mLastMessagePostTime;
        if (delayInterval >= gVMessageQueueLagLoggingThreshold)
            VLOGGER_LEVEL(gVMessageQueueLagLoggingLevel, VSTRING_FORMAT("VMessageQueue saw a delay of %lldms when getting a message with ID %d.", delayInterval.getDurationMilliseconds(), message->getMessageID()));
        }
    
    return message;
    }

void VMessageQueue::wakeUp()
    {
    this->postMessage(NULL);
    }

VSizeType VMessageQueue::getQueueSize() const
    {
    // No need to lock here, nothing bad can happen underneath us.
    return mQueuedMessages.size();
    }

Vs64 VMessageQueue::getQueueDataSize() const
    {
    // No need to lock here, nothing bad can happen underneath us.
    return mQueuedMessagesDataSize;
    }

void VMessageQueue::releaseAllMessages()
    {
    VMessage* message = NULL;

    VMutexLocker locker(&mMessageQueueMutex, "VMessageQueue::releaseAllMessages()");
    
    while (mQueuedMessages.size() > 0)
        {
        message = mQueuedMessages.front();
        mQueuedMessages.pop_front();
        
        if (message != NULL)
            {
            mQueuedMessagesDataSize -= message->getMessageDataLength();
            VMessage::release(message);
            }
        }
    }

