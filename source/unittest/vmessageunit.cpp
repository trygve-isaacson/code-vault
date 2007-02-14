/*
Copyright c1997-2007 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

/** @file */

#include "vmessageunit.h"

#include "vmessage.h"
#include "vmessagepool.h"

class TestMessage : public VMessage
	{
	public:
	
		TestMessage() : VMessage() {}
		TestMessage(VMessageID messageID, VMessagePool* pool) : VMessage(messageID, pool) {}
		virtual ~TestMessage() {}

		virtual void send(const VString& /*sessionLabel*/, VBinaryIOStream& /*out*/) {}
		virtual void receive(const VString& /*sessionLabel*/, VBinaryIOStream& /*in*/) {}
	};

class TestMessageFactory : public VMessageFactory
	{
	public:

		TestMessageFactory() {}
		virtual ~TestMessageFactory() {}

		/**
		Must be implemented by subclass, to simply instantiate a
		new VMessage object of a concrete VMessage subclass type.
		@return	pointer to a new message object
		*/		
		virtual VMessage* instantiateNewMessage(VMessageID messageID, VMessagePool* pool) { return new TestMessage(messageID, pool); }
	};

VMessageUnit::VMessageUnit(bool logOnSuccess, bool throwOnError)
: VUnit("VMessageUnit", logOnSuccess, throwOnError)
    {
    }

void VMessageUnit::run()
    {
	TestMessageFactory	factory;
	VMessagePool	pool(&factory, 5);	// limit of 5 messages in pool
	VMessagePool stats(&factory);	// we just use its stats field to compare to
	
	VMessage*	m1 = pool.get();
	this->test(pool.mPooledMessages.size() == 0, "MessagePool test 1.");

	stats.mHighWaterMarkIn = 0;
	stats.mHighWaterMarkOut = 1;
	stats.mCurrentOut = 1;
	stats.mNumMessagesCreated = 1;
	stats.mNumMessagesDestroyed = 0;
	stats.mNumMessagesReused = 0;
	this->_validateStats(pool, stats, 0, "MessagePool test 1 stats.");

	VMessage*	m2 = pool.get();
	this->test(pool.mPooledMessages.size() == 0, "MessagePool test 2.");

	stats.mHighWaterMarkIn = 0;
	stats.mHighWaterMarkOut = 2;
	stats.mCurrentOut = 2;
	stats.mNumMessagesCreated = 2;
	stats.mNumMessagesDestroyed = 0;
	stats.mNumMessagesReused = 0;
	this->_validateStats(pool, stats, 0, "MessagePool test 2 stats.");

	VMessage*	m3 = pool.get();
	this->test(pool.mPooledMessages.size() == 0, "MessagePool test 3.");
	
	stats.mHighWaterMarkIn = 0;
	stats.mHighWaterMarkOut = 3;
	stats.mCurrentOut = 3;
	stats.mNumMessagesCreated = 3;
	stats.mNumMessagesDestroyed = 0;
	stats.mNumMessagesReused = 0;
	this->_validateStats(pool, stats, 0, "MessagePool test 3 stats.");

	// Let's verify that we get back m3 if we release and get.
	VMessagePool::releaseMessage(m3, &pool);
	this->test(pool.mPooledMessages.size() == 1, "MessagePool test 4.");

	stats.mHighWaterMarkIn = 1;
	stats.mHighWaterMarkOut = 3;
	stats.mCurrentOut = 2;
	stats.mNumMessagesCreated = 3;
	stats.mNumMessagesDestroyed = 0;
	stats.mNumMessagesReused = 0;
	this->_validateStats(pool, stats, 1, "MessagePool test 4 stats.");

	VMessage*	m4 = pool.get();
	this->test(pool.mPooledMessages.size() == 0, "MessagePool test 5a.");
	this->test(m3 == m4, "MessagePool test 5b.");
	m3 = NULL;	// we released it previously, shouldn't be using it except in above test

	stats.mHighWaterMarkIn = 1;
	stats.mHighWaterMarkOut = 3;
	stats.mCurrentOut = 3;
	stats.mNumMessagesCreated = 3;
	stats.mNumMessagesDestroyed = 0;
	stats.mNumMessagesReused = 1;
	this->_validateStats(pool, stats, 0, "MessagePool test 5 stats.");

	VMessage*	m5 = pool.get();

	stats.mHighWaterMarkIn = 1;
	stats.mHighWaterMarkOut = 4;
	stats.mCurrentOut = 4;
	stats.mNumMessagesCreated = 4;
	stats.mNumMessagesDestroyed = 0;
	stats.mNumMessagesReused = 1;
	this->_validateStats(pool, stats, 0, "MessagePool test 6 stats.");

	VMessage*	m6 = pool.get();

	stats.mHighWaterMarkIn = 1;
	stats.mHighWaterMarkOut = 5;
	stats.mCurrentOut = 5;
	stats.mNumMessagesCreated = 5;
	stats.mNumMessagesDestroyed = 0;
	stats.mNumMessagesReused = 1;
	this->_validateStats(pool, stats, 0, "MessagePool test 7 stats.");

	VMessage*	m7 = pool.get();
	this->test(pool.mPooledMessages.size() == 0, "MessagePool test 8.");

	stats.mHighWaterMarkIn = 1;
	stats.mHighWaterMarkOut = 6;
	stats.mCurrentOut = 6;
	stats.mNumMessagesCreated = 6;
	stats.mNumMessagesDestroyed = 0;
	stats.mNumMessagesReused = 1;
	this->_validateStats(pool, stats, 0, "MessagePool test 8 stats.");

	// We've got 6 messages now. Pool is empty. Verify only 5 can be put back in pool.

	VMessagePool::releaseMessage(m1, &pool);

	stats.mHighWaterMarkIn = 1;
	stats.mHighWaterMarkOut = 6;
	stats.mCurrentOut = 5;
	stats.mNumMessagesCreated = 6;
	stats.mNumMessagesDestroyed = 0;
	stats.mNumMessagesReused = 1;
	this->_validateStats(pool, stats, 1, "MessagePool test 9 stats.");

	VMessagePool::releaseMessage(m2, &pool);

	stats.mHighWaterMarkIn = 2;
	stats.mHighWaterMarkOut = 6;
	stats.mCurrentOut = 4;
	stats.mNumMessagesCreated = 6;
	stats.mNumMessagesDestroyed = 0;
	stats.mNumMessagesReused = 1;
	this->_validateStats(pool, stats, 2, "MessagePool test 10 stats.");

	// Note: m3 is null, skip it.
	
	VMessagePool::releaseMessage(m4, &pool);

	stats.mHighWaterMarkIn = 3;
	stats.mHighWaterMarkOut = 6;
	stats.mCurrentOut = 3;
	stats.mNumMessagesCreated = 6;
	stats.mNumMessagesDestroyed = 0;
	stats.mNumMessagesReused = 1;
	this->_validateStats(pool, stats, 3, "MessagePool test 11 stats.");

	VMessagePool::releaseMessage(m4, &pool);

	stats.mHighWaterMarkIn = 4;
	stats.mHighWaterMarkOut = 6;
	stats.mCurrentOut = 2;
	stats.mNumMessagesCreated = 6;
	stats.mNumMessagesDestroyed = 0;
	stats.mNumMessagesReused = 1;
	this->_validateStats(pool, stats, 4, "MessagePool test 12 stats.");

	VMessagePool::releaseMessage(m6, &pool);

	stats.mHighWaterMarkIn = 5;
	stats.mHighWaterMarkOut = 6;
	stats.mCurrentOut = 1;
	stats.mNumMessagesCreated = 6;
	stats.mNumMessagesDestroyed = 0;
	stats.mNumMessagesReused = 1;
	this->_validateStats(pool, stats, 5, "MessagePool test 13 stats.");

	VMessagePool::releaseMessage(m7, &pool);

	stats.mHighWaterMarkIn = 5;
	stats.mHighWaterMarkOut = 6;
	stats.mCurrentOut = 0;
	stats.mNumMessagesCreated = 6;
	stats.mNumMessagesDestroyed = 1;
	stats.mNumMessagesReused = 1;
	this->_validateStats(pool, stats, 5, "MessagePool test 14 stats.");

	// Now let's check 3 out, 1 in, and then drain it, verifying each step.
	m1 = pool.get();

	stats.mHighWaterMarkIn = 5;
	stats.mHighWaterMarkOut = 6;
	stats.mCurrentOut = 1;
	stats.mNumMessagesCreated = 6;
	stats.mNumMessagesDestroyed = 1;
	stats.mNumMessagesReused = 2;
	this->_validateStats(pool, stats, 4, "MessagePool test 15 stats.");

	m2 = pool.get();

	stats.mHighWaterMarkIn = 5;
	stats.mHighWaterMarkOut = 6;
	stats.mCurrentOut = 2;
	stats.mNumMessagesCreated = 6;
	stats.mNumMessagesDestroyed = 1;
	stats.mNumMessagesReused = 3;
	this->_validateStats(pool, stats, 3, "MessagePool test 16 stats.");

	m3 = pool.get();

	stats.mHighWaterMarkIn = 5;
	stats.mHighWaterMarkOut = 6;
	stats.mCurrentOut = 3;
	stats.mNumMessagesCreated = 6;
	stats.mNumMessagesDestroyed = 1;
	stats.mNumMessagesReused = 4;
	this->_validateStats(pool, stats, 2, "MessagePool test 17 stats.");

	VMessagePool::releaseMessage(m3, &pool);

	stats.mHighWaterMarkIn = 5;
	stats.mHighWaterMarkOut = 6;
	stats.mCurrentOut = 2;
	stats.mNumMessagesCreated = 6;
	stats.mNumMessagesDestroyed = 1;
	stats.mNumMessagesReused = 4;
	this->_validateStats(pool, stats, 3, "MessagePool test 18 stats.");

	m3 = pool.get();

	stats.mHighWaterMarkIn = 5;
	stats.mHighWaterMarkOut = 6;
	stats.mCurrentOut = 3;
	stats.mNumMessagesCreated = 6;
	stats.mNumMessagesDestroyed = 1;
	stats.mNumMessagesReused = 5;
	this->_validateStats(pool, stats, 2, "MessagePool test 19 stats.");

	m4 = pool.get();

	stats.mHighWaterMarkIn = 5;
	stats.mHighWaterMarkOut = 6;
	stats.mCurrentOut = 4;
	stats.mNumMessagesCreated = 6;
	stats.mNumMessagesDestroyed = 1;
	stats.mNumMessagesReused = 6;
	this->_validateStats(pool, stats, 1, "MessagePool test 20 stats.");

	m5 = pool.get();

	stats.mHighWaterMarkIn = 5;
	stats.mHighWaterMarkOut = 6;
	stats.mCurrentOut = 5;
	stats.mNumMessagesCreated = 6;
	stats.mNumMessagesDestroyed = 1;
	stats.mNumMessagesReused = 7;
	this->_validateStats(pool, stats, 0, "MessagePool test 21 stats.");

	// Now let's check 2 more out that should require creation.
	m6 = pool.get();

	stats.mHighWaterMarkIn = 5;
	stats.mHighWaterMarkOut = 6;
	stats.mCurrentOut = 6;
	stats.mNumMessagesCreated = 7;
	stats.mNumMessagesDestroyed = 1;
	stats.mNumMessagesReused = 7;
	this->_validateStats(pool, stats, 0, "MessagePool test 22 stats.");

	m7 = pool.get();

	stats.mHighWaterMarkIn = 5;
	stats.mHighWaterMarkOut = 7;
	stats.mCurrentOut = 7;
	stats.mNumMessagesCreated = 8;
	stats.mNumMessagesDestroyed = 1;
	stats.mNumMessagesReused = 7;
	this->_validateStats(pool, stats, 0, "MessagePool test 23 stats.");
	
    }

void VMessageUnit::_validateStats(const VMessagePool& pool, const VMessagePool& stats, int numInPool, const VString& label)
	{
	this->test((pool.mHighWaterMarkIn == stats.mHighWaterMarkIn) &&
				(pool.mHighWaterMarkOut == stats.mHighWaterMarkOut) &&
				(pool.mCurrentOut == stats.mCurrentOut) &&
				(pool.mNumMessagesCreated == stats.mNumMessagesCreated) &&
				(pool.mNumMessagesDestroyed == stats.mNumMessagesDestroyed) &&
				(pool.mNumMessagesReused == stats.mNumMessagesReused) &&
				(((int) pool.mPooledMessages.size()) == numInPool),
				label);
	}
