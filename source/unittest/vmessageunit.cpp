/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vmessageunit.h"

#include "vmessage.h"
#include "vmessagepool.h"

class TestMessage : public VMessage
    {
    public:
    
        TestMessage();
        TestMessage(VMessageID messageID, VMessagePool* pool);
        virtual ~TestMessage();

        virtual void send(const VString& /*sessionLabel*/, VBinaryIOStream& /*out*/) {}
        virtual void receive(const VString& /*sessionLabel*/, VBinaryIOStream& /*in*/) {}
        
        static int getNumMessagesConstructed() { return gNumMessagesConstructed; }
        static int getNumMessagesDestructed() { return gNumMessagesDestructed; }
        static void resetCounters() { gNumMessagesConstructed = 0; gNumMessagesDestructed = 0; }

    private:
    
        static int gNextMessageUniqueID;
        
        static int gNumMessagesConstructed;
        static int gNumMessagesDestructed;
        
        int mUniqueID;
    };

int TestMessage::gNextMessageUniqueID = 1;
int TestMessage::gNumMessagesConstructed = 0;
int TestMessage::gNumMessagesDestructed = 0;

TestMessage::TestMessage() :
VMessage(),
mUniqueID(TestMessage::gNextMessageUniqueID++)
    {
    VLOGGER_INFO(VString("TestMessage %d constructed.", mUniqueID));
    ++gNumMessagesConstructed;
    }

TestMessage::TestMessage(VMessageID messageID, VMessagePool* pool) :
VMessage(messageID, pool),
mUniqueID(TestMessage::gNextMessageUniqueID++)
    {
    VLOGGER_INFO(VString("TestMessage %d constructed.", mUniqueID));
    ++gNumMessagesConstructed;
    }

TestMessage::~TestMessage()
    {
    VLOGGER_INFO(VString("TestMessage %d destructed.", mUniqueID));
    ++gNumMessagesDestructed;
    }

class TestMessageFactory : public VMessageFactory
    {
    public:

        TestMessageFactory() {}
        virtual ~TestMessageFactory() {}

        /**
        Must be implemented by subclass, to simply instantiate a
        new VMessage object of a concrete VMessage subclass type.
        @return    pointer to a new message object
        */        
        virtual VMessage* instantiateNewMessage(VMessageID messageID, VMessagePool* pool) { return new TestMessage(messageID, pool); }
    };

typedef std::vector<VMessage*> VMessageList;

static VMessage* _newMessage(VMessagePool* pool, VMessageID messageID)
    {
    return (pool == NULL) ? (new TestMessage(messageID, NULL)) : (pool->get(messageID));
    }

static void _freeMessages(VMessagePool* pool, VMessageList& messages)
    {
    for (VMessageList::const_iterator i = messages.begin(); i != messages.end(); ++i)
        {
        VMessage* m = (*i);
        
        if (pool == NULL)
            delete m;
        else
            VMessagePool::releaseMessage(m, pool);
        }
    
    messages.clear();
    }

static void _testAllocations(VMessagePool* pool, const VString& label)
    {
    TestMessage::resetCounters();
    
    const int PRE_EXTRACT_COUNT = 0;
    const int NUM_MESSAGES_PER_ITERATION = 80; // pool size or larger to cause 
    const int NUM_ITERATIONS = 10000;
    VInstant start;
    
    VMessageList preExtractedMessages;
    VMessageList iterationMessages;

    for (int i = 0; i < PRE_EXTRACT_COUNT; ++i)
        preExtractedMessages.push_back(_newMessage(pool, i));
    
    for (int iterationIndex = 0; iterationIndex < NUM_ITERATIONS; ++iterationIndex)
        {
        for (int messageIndex = 0; messageIndex < NUM_MESSAGES_PER_ITERATION; ++messageIndex)
            {
            iterationMessages.push_back(_newMessage(pool, iterationIndex));
            }
        
        _freeMessages(pool, iterationMessages);
        }

    _freeMessages(pool, preExtractedMessages);

    VInstant end;
    VDuration delta = end - start;
    std::cout << label << delta.getDurationString()
        << " Constructed:" << TestMessage::getNumMessagesConstructed()
        << " Destructed:" << TestMessage::getNumMessagesDestructed()
        << std::endl;

    if (pool != NULL)
        {
        VLogger* cout = new VCoutLogger(VLogger::kTrace, VMessage::kMessageLoggerName, VString::EMPTY());
        VLogger::installLogger(cout);
        pool->printStats();
        }
    }
    
VMessageUnit::VMessageUnit(bool logOnSuccess, bool throwOnError) :
VUnit("VMessageUnit", logOnSuccess, throwOnError)
    {
    }

void VMessageUnit::run()
    {
    TestMessageFactory    factory;
    VMessagePool    pool(&factory, 5);    // limit of 5 messages in pool
    VMessagePool stats(&factory);    // we just use its stats field to compare to
    
    VMessage*    m1 = pool.get();
    this->test(pool.mPooledMessages.size() == 0, "MessagePool test 1");

    stats.mHighWaterMarkIn = 0;
    stats.mHighWaterMarkOut = 1;
    stats.mCurrentOut = 1;
    stats.mNumMessagesCreated = 1;
    stats.mNumMessagesDestroyed = 0;
    stats.mNumMessagesReused = 0;
    this->_validateStats(pool, stats, 0, "MessagePool test 1 stats");

    VMessage*    m2 = pool.get();
    this->test(pool.mPooledMessages.size() == 0, "MessagePool test 2");

    stats.mHighWaterMarkIn = 0;
    stats.mHighWaterMarkOut = 2;
    stats.mCurrentOut = 2;
    stats.mNumMessagesCreated = 2;
    stats.mNumMessagesDestroyed = 0;
    stats.mNumMessagesReused = 0;
    this->_validateStats(pool, stats, 0, "MessagePool test 2 stats");

    VMessage*    m3 = pool.get();
    this->test(pool.mPooledMessages.size() == 0, "MessagePool test 3");
    
    stats.mHighWaterMarkIn = 0;
    stats.mHighWaterMarkOut = 3;
    stats.mCurrentOut = 3;
    stats.mNumMessagesCreated = 3;
    stats.mNumMessagesDestroyed = 0;
    stats.mNumMessagesReused = 0;
    this->_validateStats(pool, stats, 0, "MessagePool test 3 stats");

    // Let's verify that we get back m3 if we release and get.
    VMessagePool::releaseMessage(m3, &pool);
    this->test(pool.mPooledMessages.size() == 1, "MessagePool test 4");

    stats.mHighWaterMarkIn = 1;
    stats.mHighWaterMarkOut = 3;
    stats.mCurrentOut = 2;
    stats.mNumMessagesCreated = 3;
    stats.mNumMessagesDestroyed = 0;
    stats.mNumMessagesReused = 0;
    this->_validateStats(pool, stats, 1, "MessagePool test 4 stats");

    VMessage*    m4 = pool.get();
    this->test(pool.mPooledMessages.size() == 0, "MessagePool test 5a");
    this->test(m3 == m4, "MessagePool test 5b");
    m3 = NULL;    // we released it previously, shouldn't be using it except in above test

    stats.mHighWaterMarkIn = 1;
    stats.mHighWaterMarkOut = 3;
    stats.mCurrentOut = 3;
    stats.mNumMessagesCreated = 3;
    stats.mNumMessagesDestroyed = 0;
    stats.mNumMessagesReused = 1;
    this->_validateStats(pool, stats, 0, "MessagePool test 5 stats");

    VMessage*    m5 = pool.get();

    stats.mHighWaterMarkIn = 1;
    stats.mHighWaterMarkOut = 4;
    stats.mCurrentOut = 4;
    stats.mNumMessagesCreated = 4;
    stats.mNumMessagesDestroyed = 0;
    stats.mNumMessagesReused = 1;
    this->_validateStats(pool, stats, 0, "MessagePool test 6 stats");

    VMessage*    m6 = pool.get();

    stats.mHighWaterMarkIn = 1;
    stats.mHighWaterMarkOut = 5;
    stats.mCurrentOut = 5;
    stats.mNumMessagesCreated = 5;
    stats.mNumMessagesDestroyed = 0;
    stats.mNumMessagesReused = 1;
    this->_validateStats(pool, stats, 0, "MessagePool test 7 stats");

    VMessage*    m7 = pool.get();
    this->test(pool.mPooledMessages.size() == 0, "MessagePool test 8");

    stats.mHighWaterMarkIn = 1;
    stats.mHighWaterMarkOut = 6;
    stats.mCurrentOut = 6;
    stats.mNumMessagesCreated = 6;
    stats.mNumMessagesDestroyed = 0;
    stats.mNumMessagesReused = 1;
    this->_validateStats(pool, stats, 0, "MessagePool test 8 stats");

    // We've got 6 messages now. Pool is empty. Verify only 5 can be put back in pool.

    VMessagePool::releaseMessage(m1, &pool);
    m1 = NULL;

    stats.mHighWaterMarkIn = 1;
    stats.mHighWaterMarkOut = 6;
    stats.mCurrentOut = 5;
    stats.mNumMessagesCreated = 6;
    stats.mNumMessagesDestroyed = 0;
    stats.mNumMessagesReused = 1;
    this->_validateStats(pool, stats, 1, "MessagePool test 9 stats");

    VMessagePool::releaseMessage(m2, &pool);
    m2 = NULL;

    stats.mHighWaterMarkIn = 2;
    stats.mHighWaterMarkOut = 6;
    stats.mCurrentOut = 4;
    stats.mNumMessagesCreated = 6;
    stats.mNumMessagesDestroyed = 0;
    stats.mNumMessagesReused = 1;
    this->_validateStats(pool, stats, 2, "MessagePool test 10 stats");

    // Note: m3 is null (it's already in the pool), skip it.
    
    VMessagePool::releaseMessage(m4, &pool);
    m4 = NULL;

    stats.mHighWaterMarkIn = 3;
    stats.mHighWaterMarkOut = 6;
    stats.mCurrentOut = 3;
    stats.mNumMessagesCreated = 6;
    stats.mNumMessagesDestroyed = 0;
    stats.mNumMessagesReused = 1;
    this->_validateStats(pool, stats, 3, "MessagePool test 11 stats");

    VMessagePool::releaseMessage(m5, &pool);
    m5 = NULL;

    stats.mHighWaterMarkIn = 4;
    stats.mHighWaterMarkOut = 6;
    stats.mCurrentOut = 2;
    stats.mNumMessagesCreated = 6;
    stats.mNumMessagesDestroyed = 0;
    stats.mNumMessagesReused = 1;
    this->_validateStats(pool, stats, 4, "MessagePool test 12 stats");

    VMessagePool::releaseMessage(m6, &pool);
    m6 = NULL;

    stats.mHighWaterMarkIn = 5;
    stats.mHighWaterMarkOut = 6;
    stats.mCurrentOut = 1;
    stats.mNumMessagesCreated = 6;
    stats.mNumMessagesDestroyed = 0;
    stats.mNumMessagesReused = 1;
    this->_validateStats(pool, stats, 5, "MessagePool test 13 stats");

    VMessagePool::releaseMessage(m7, &pool);
    m7 = NULL;

    stats.mHighWaterMarkIn = 5;
    stats.mHighWaterMarkOut = 6;
    stats.mCurrentOut = 0;
    stats.mNumMessagesCreated = 6;
    stats.mNumMessagesDestroyed = 1;
    stats.mNumMessagesReused = 1;
    this->_validateStats(pool, stats, 5, "MessagePool test 14 stats");

    // Now let's check 3 out, 1 in, and then drain it, verifying each step.
    m1 = pool.get();

    stats.mHighWaterMarkIn = 5;
    stats.mHighWaterMarkOut = 6;
    stats.mCurrentOut = 1;
    stats.mNumMessagesCreated = 6;
    stats.mNumMessagesDestroyed = 1;
    stats.mNumMessagesReused = 2;
    this->_validateStats(pool, stats, 4, "MessagePool test 15 stats");

    m2 = pool.get();

    stats.mHighWaterMarkIn = 5;
    stats.mHighWaterMarkOut = 6;
    stats.mCurrentOut = 2;
    stats.mNumMessagesCreated = 6;
    stats.mNumMessagesDestroyed = 1;
    stats.mNumMessagesReused = 3;
    this->_validateStats(pool, stats, 3, "MessagePool test 16 stats");

    m3 = pool.get();

    stats.mHighWaterMarkIn = 5;
    stats.mHighWaterMarkOut = 6;
    stats.mCurrentOut = 3;
    stats.mNumMessagesCreated = 6;
    stats.mNumMessagesDestroyed = 1;
    stats.mNumMessagesReused = 4;
    this->_validateStats(pool, stats, 2, "MessagePool test 17 stats");

    VMessagePool::releaseMessage(m3, &pool);
    m3 = NULL;

    stats.mHighWaterMarkIn = 5;
    stats.mHighWaterMarkOut = 6;
    stats.mCurrentOut = 2;
    stats.mNumMessagesCreated = 6;
    stats.mNumMessagesDestroyed = 1;
    stats.mNumMessagesReused = 4;
    this->_validateStats(pool, stats, 3, "MessagePool test 18 stats");

    m3 = pool.get();

    stats.mHighWaterMarkIn = 5;
    stats.mHighWaterMarkOut = 6;
    stats.mCurrentOut = 3;
    stats.mNumMessagesCreated = 6;
    stats.mNumMessagesDestroyed = 1;
    stats.mNumMessagesReused = 5;
    this->_validateStats(pool, stats, 2, "MessagePool test 19 stats");

    m4 = pool.get();

    stats.mHighWaterMarkIn = 5;
    stats.mHighWaterMarkOut = 6;
    stats.mCurrentOut = 4;
    stats.mNumMessagesCreated = 6;
    stats.mNumMessagesDestroyed = 1;
    stats.mNumMessagesReused = 6;
    this->_validateStats(pool, stats, 1, "MessagePool test 20 stats");

    m5 = pool.get();

    stats.mHighWaterMarkIn = 5;
    stats.mHighWaterMarkOut = 6;
    stats.mCurrentOut = 5;
    stats.mNumMessagesCreated = 6;
    stats.mNumMessagesDestroyed = 1;
    stats.mNumMessagesReused = 7;
    this->_validateStats(pool, stats, 0, "MessagePool test 21 stats");

    // Now let's check 2 more out that should require creation.
    m6 = pool.get();

    stats.mHighWaterMarkIn = 5;
    stats.mHighWaterMarkOut = 6;
    stats.mCurrentOut = 6;
    stats.mNumMessagesCreated = 7;
    stats.mNumMessagesDestroyed = 1;
    stats.mNumMessagesReused = 7;
    this->_validateStats(pool, stats, 0, "MessagePool test 22 stats");

    m7 = pool.get();

    stats.mHighWaterMarkIn = 5;
    stats.mHighWaterMarkOut = 7;
    stats.mCurrentOut = 7;
    stats.mNumMessagesCreated = 8;
    stats.mNumMessagesDestroyed = 1;
    stats.mNumMessagesReused = 7;
    this->_validateStats(pool, stats, 0, "MessagePool test 23 stats");
    
    // Upon destruction, the pool will delete what it contains.
    // Currently it should only contain our former m3 that we released.
    // To properly clean up, we can either delete the messages we hold here,
    // or release them back to the pool and let the pool delete them.
    delete m1;
    delete m2;
    delete m3; // should be null from assignment above
    delete m4;
    delete m5;
    delete m6;
    delete m7;
    
    if (false)
    {
    // Test speed of allocation with modest sized pool.
        {
        const int POOL_SIZE = 64;
        TestMessageFactory  speedFactory;
        VMessagePool        speedPool(&speedFactory, POOL_SIZE);
        
        _testAllocations(&pool, "Test with pool: ");
        }

    // Test speed of allocation without pool.
        {
        _testAllocations(NULL, "Test without pool: ");
        }
    }
    
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

