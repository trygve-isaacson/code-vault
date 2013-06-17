/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vmessageunit.h"

#include "vmessage.h"
#include "vcompactingdeque.h"

class TestMessage;
typedef boost::shared_ptr<TestMessage> TestMessagePtr;

class TestMessage : public VMessage {
    public:
    
        static TestMessagePtr factory();
        static TestMessagePtr factory(VMessageID messageID);
        virtual ~TestMessage();

        virtual void send(const VString& /*sessionLabel*/, VBinaryIOStream& /*out*/) {}
        virtual void receive(const VString& /*sessionLabel*/, VBinaryIOStream& /*in*/) {}

        static int getNumMessagesConstructed() { return gNumMessagesConstructed; }
        static int getNumMessagesDestructed() { return gNumMessagesDestructed; }
        static void resetCounters() { gNumMessagesConstructed = 0; gNumMessagesDestructed = 0; }

    protected:

        TestMessage();
        TestMessage(VMessageID messageID);

    private:

        static int gNextMessageUniqueID;

        static int gNumMessagesConstructed;
        static int gNumMessagesDestructed;

        int mUniqueID;
};

int TestMessage::gNextMessageUniqueID = 1;
int TestMessage::gNumMessagesConstructed = 0;
int TestMessage::gNumMessagesDestructed = 0;

// static
TestMessagePtr TestMessage::factory() {
    return TestMessagePtr(new TestMessage());
}

// static
TestMessagePtr TestMessage::factory(VMessageID messageID) {
    return TestMessagePtr(new TestMessage(messageID));
}

TestMessage::TestMessage() :
    VMessage(),
    mUniqueID(TestMessage::gNextMessageUniqueID++) {
    ++gNumMessagesConstructed;
}

TestMessage::TestMessage(VMessageID messageID) :
    VMessage(messageID),
    mUniqueID(TestMessage::gNextMessageUniqueID++) {
    ++gNumMessagesConstructed;
}

TestMessage::~TestMessage() {
    ++gNumMessagesDestructed;
}

class TestMessageFactory : public VMessageFactory {
    public:

        TestMessageFactory() {}
        virtual ~TestMessageFactory() {}

        /**
        Must be implemented by subclass, to simply instantiate a
        new VMessage object of a concrete VMessage subclass type.
        @return    pointer to a new message object
        */
        virtual VMessagePtr instantiateNewMessage(VMessageID messageID) const { return TestMessage::factory(messageID); }
};

VMessageUnit::VMessageUnit(bool logOnSuccess, bool throwOnError) :
    VUnit("VMessageUnit", logOnSuccess, throwOnError) {
}

void VMessageUnit::run() {
    // Basic tests of VCompactingDeque, which is used only by VMessageQueue at this time.
    const size_t HWM = 10;
    const size_t LWM = 2;
    VCompactingDeque<int> q(HWM, LWM);
    q.push_back(10);
    q.push_back(20);
    q.push_back(30);
    q.push_back(40);
    q.push_back(50);
    q.push_back(60);
    q.push_back(70);
    q.push_back(80);
    q.push_back(90);
    q.push_back(100);
    q.push_back(110);
    q.push_back(120);
    VUNIT_ASSERT_EQUAL(q.front(), 10);
    VUNIT_ASSERT_EQUAL(q.back(), 120);
    VUNIT_ASSERT_EQUAL(q.size(), (size_t) 12);
    VUNIT_ASSERT_EQUAL(q.mHighWaterMark, (size_t) 0); // <- not a requirement but verifies expected internal behavior; mHighWaterMark only updated on pop
    VUNIT_ASSERT_EQUAL(q.mHighWaterMarkRequired, HWM);
    VUNIT_ASSERT_EQUAL(q.mLowWaterMarkRequired, LWM);

    q.pop_front();
    q.pop_front();
    q.pop_front();
    VUNIT_ASSERT_EQUAL(q.front(), 40);
    VUNIT_ASSERT_EQUAL(q.back(), 120);
    VUNIT_ASSERT_EQUAL(q.size(), (size_t) 9);
    VUNIT_ASSERT_EQUAL(q.mHighWaterMark, (size_t) 12);
    VUNIT_ASSERT_EQUAL(q.mHighWaterMarkRequired, HWM);
    VUNIT_ASSERT_EQUAL(q.mLowWaterMarkRequired, LWM);

    q.pop_back();
    q.pop_back();
    q.pop_back();
    VUNIT_ASSERT_EQUAL(q.front(), 40);
    VUNIT_ASSERT_EQUAL(q.back(), 90);
    VUNIT_ASSERT_EQUAL(q.size(), (size_t) 6);
    VUNIT_ASSERT_EQUAL(q.mHighWaterMark, (size_t) 12);
    VUNIT_ASSERT_EQUAL(q.mHighWaterMarkRequired, HWM);
    VUNIT_ASSERT_EQUAL(q.mLowWaterMarkRequired, LWM);

    q.pop_back();
    q.pop_back();
    q.pop_front();
    q.pop_front();
    VUNIT_ASSERT_EQUAL(q.front(), 60);
    VUNIT_ASSERT_EQUAL(q.back(), 70);
    VUNIT_ASSERT_EQUAL(q.size(), (size_t) 2);
    VUNIT_ASSERT_EQUAL(q.mHighWaterMark, (size_t) 0); // <- verifies that we triggered compaction at LWM == size == 2
    VUNIT_ASSERT_EQUAL(q.mHighWaterMarkRequired, HWM);
    VUNIT_ASSERT_EQUAL(q.mLowWaterMarkRequired, LWM);

    q.push_front(42);
    q.push_back(43);
    q.pop_back();
    VUNIT_ASSERT_EQUAL(q.front(), 42);
    VUNIT_ASSERT_EQUAL(q.back(), 70);
    VUNIT_ASSERT_EQUAL(q.size(), (size_t) 3);
    VUNIT_ASSERT_EQUAL(q.mHighWaterMark, (size_t) 4); // <- verifies that pop_back updated mHighWaterMark to max before pop
    VUNIT_ASSERT_EQUAL(q.mHighWaterMarkRequired, HWM);
    VUNIT_ASSERT_EQUAL(q.mLowWaterMarkRequired, LWM);
}

