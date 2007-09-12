/*
Copyright c1997-2007 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

#include "vmessagequeue.h"

#include "vmutexlocker.h"
#include "vmessage.h"
#include "vmessagepool.h"

// VMessageQueue --------------------------------------------------------------

VMessageQueue::VMessageQueue() :
// mQueuedMessages -> empty
mQueuedMessagesDataSize(0)
// mMessageQueueMutex -> unlocked
// mMessageQueueSemaphore -> unsignaled
	{
	}

VMessageQueue::~VMessageQueue()
	{
	// Delete all messages remaining in the queue.

	try {
		VMutexLocker locker(&mMessageQueueMutex);

		for (VSizeType i = 0; i < mQueuedMessages.size(); ++i)
			delete mQueuedMessages[i];
		}
	catch (...) { }	// block exceptions from propagating
	}

void VMessageQueue::postMessage(VMessage* message)
	{
	VMutexLocker locker(&mMessageQueueMutex);

	mQueuedMessages.push_back(message);

    if (message != NULL)
        mQueuedMessagesDataSize += message->getMessageDataLength();

	locker.unlock();	// otherwise signal() will deadlock
	mMessageQueueSemaphore.signal();
	}

VMessage* VMessageQueue::blockUntilNextMessage()
	{
	// If there is a message on the queue, we can simply return it.
	VMessage* message = this->getNextMessage();
	if (message != NULL)
		return message;

    // There is nothing on the queue, so wait until someone posts a message.
	VMutex	dummy;
	mMessageQueueSemaphore.wait(&dummy, VDuration::ZERO()); // possible FIXME: we may want to timeout after a couple of seconds (supply non-ZERO 2nd parameter)
	
	return this->getNextMessage();
	}

VMessage* VMessageQueue::getNextMessage()
	{
	VMessage*	message = NULL;

	VMutexLocker	locker(&mMessageQueueMutex);
	
	if (mQueuedMessages.size() > 0)
		{
		message = mQueuedMessages.front();
		mQueuedMessages.pop_front();

        if (message != NULL)
            mQueuedMessagesDataSize -= message->getMessageDataLength();
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
	VMessage*	message = NULL;

	VMutexLocker	locker(&mMessageQueueMutex);
	
	while (mQueuedMessages.size() > 0)
		{
		message = mQueuedMessages.front();
		mQueuedMessages.pop_front();
		
        if (message != NULL)
            VMessagePool::releaseMessage(message, message->getPool());
		}
    }

