/*
Copyright c1997-2007 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

#include "vmessagepool.h"

#include "vmutexlocker.h"

// VMessagePool ---------------------------------------------------------------

// static
void VMessagePool::releaseMessage(VMessage* message, VMessagePool* pool)
	{
	if (message != NULL)
		{
		if (message->isBeingBroadcast())
			{
			VMutexLocker locker(message->getBroadcastMutex());
			
			if (message->numBroadcastTargets() != 0)	// don't "remove" if not actually a target
				message->removeBroadcastTarget();

			if (message->numBroadcastTargets() != 0)	// still in someone's output queue
				message = NULL;	// prevent delete/release below; could just return but this seems a little nicer style

			}
		}

	if (message != NULL)
		{
		if (pool == NULL)
			delete message;
		else
			pool->release(message);
		}
	}

VMessagePool::VMessagePool(VMessageFactory* factory, int maxInPool) :
mFactory(factory),
mMaxInPool(maxInPool),
mHighWaterMarkIn(0),
mHighWaterMarkOut(0),
mCurrentOut(0),
mNumMessagesCreated(0),
mNumMessagesDestroyed(0),
mNumMessagesReused(0)
	{
	}

VMessagePool::~VMessagePool()
	{
	try
		{
		VMutexLocker locker(&mMessagePoolMutex);

		for (VSizeType i = 0; i < mPooledMessages.size(); ++i)
			{
			delete mPooledMessages[i];
			mPooledMessages[i] = NULL; // MSVC runtime apparently "tidies up" in dequeue dtor by deleting elements; set to null to prevent double delete
			}
		}
	catch (...) { }	// block exceptions from propagating
	}

VMessage* VMessagePool::get(VMessageID messageID)
	{
	VMessage*		result;
	VMutexLocker	locker(&mMessagePoolMutex);
	
	if (mPooledMessages.empty())
		{
		result = mFactory->instantiateNewMessage(messageID, this);
		mNumMessagesCreated++;

		VLOGGER_LEVEL(VMessage::kMessagePoolTraceLogLevel, VString("VMessagePool::get: created new message @0x%08X", result));
		}
	else
		{
		result = mPooledMessages.front();
		mPooledMessages.pop_front();
		
		result->recycle(messageID);
		
		mNumMessagesReused++;

		VLOGGER_LEVEL(VMessage::kMessagePoolTraceLogLevel, VString("VMessagePool::get: reused pool message @0x%08X", result));
		}

	mCurrentOut++;
	mHighWaterMarkOut = V_MAX(mHighWaterMarkOut, mCurrentOut);

	return result;
	}

void VMessagePool::release(VMessage* message)
	{
	VMutexLocker	locker(&mMessagePoolMutex);

	mCurrentOut--;
	
	if ((mMaxInPool == kUnlimitedPoolSize) || (((int) mPooledMessages.size()) < mMaxInPool))
		{
		VLOGGER_LEVEL(VMessage::kMessagePoolTraceLogLevel, VString("VMessagePool::release: pooling message @0x%08X", message));

		mPooledMessages.push_back(message);
		mHighWaterMarkIn = V_MAX(mHighWaterMarkIn, (int) mPooledMessages.size());
		}
	else
		{
		VLOGGER_LEVEL(VMessage::kMessagePoolTraceLogLevel, VString("VMessagePool::release: deleting message @0x%08X", message));

		delete message;
		mNumMessagesDestroyed++;
		}
	}

void VMessagePool::printStats()
	{
	VLOGGER_LEVEL(VMessage::kMessagePoolTraceLogLevel, VString("VMessagePool.mMaxInPool            = %d", mMaxInPool));
	VLOGGER_LEVEL(VMessage::kMessagePoolTraceLogLevel, VString("VMessagePool.mHighWaterMarkIn      = %d", mHighWaterMarkIn));
	VLOGGER_LEVEL(VMessage::kMessagePoolTraceLogLevel, VString("VMessagePool.mPooledMessages.size  = %d", (int) mPooledMessages.size()));
	VLOGGER_LEVEL(VMessage::kMessagePoolTraceLogLevel, VString("VMessagePool.mHighWaterMarkOut     = %d", mHighWaterMarkOut));
	VLOGGER_LEVEL(VMessage::kMessagePoolTraceLogLevel, VString("VMessagePool.mCurrentOut           = %d", mCurrentOut));
	VLOGGER_LEVEL(VMessage::kMessagePoolTraceLogLevel, VString("VMessagePool.mNumMessagesCreated   = %d", mNumMessagesCreated));
	VLOGGER_LEVEL(VMessage::kMessagePoolTraceLogLevel, VString("VMessagePool.mNumMessagesDestroyed = %d", mNumMessagesDestroyed));
	VLOGGER_LEVEL(VMessage::kMessagePoolTraceLogLevel, VString("VMessagePool.mNumMessagesReused    = %d", mNumMessagesReused));

    Vs64 totalMessageBytes = 0;
	VMutexLocker locker(&mMessagePoolMutex);
	for (VSizeType i = 0; i < mPooledMessages.size(); ++i)
		{
		totalMessageBytes += mPooledMessages[i]->getBufferSize();
		}
    locker.unlock();

	VLOGGER_LEVEL(VMessage::kMessagePoolTraceLogLevel, VString("VMessagePool.mPooledMessages bytes = %lld", totalMessageBytes));
	}

