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

VMessageQueue::VMessageQueue()
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

	locker.unlock();	// otherwise signal() will deadlock
	mMessageQueueSemaphore.signal();
	}

VMessage* VMessageQueue::blockUntilNextMessage()
	{
/*
FIXME: Gotta figure out how to normalize Semaphore signal/wait between the
Unix and Win models. For now, what works is:
On Windows, wait on the semaphore and then get the next message.
On Unix, get the next message if there is one; then wait-and-get as w/ Windows.
*/
#ifndef VPLATFORM_WIN
	
	VMessage* message = this->getNextMessage();
	if (message != NULL)
		return message;

#endif

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

