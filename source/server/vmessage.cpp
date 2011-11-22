/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

#include "vmessage.h"

#include "vmutexlocker.h"
#include "vexception.h"
#include "vsocketthread.h"
#include "vmessagehandler.h"
#include "vassert.h"

// Is ASSERT_INVARIANT enabled/disabled specifically for VMessage?
#ifdef V_ASSERT_INVARIANT_VMESSAGE_ENABLED
    #undef ASSERT_INVARIANT
    #if V_ASSERT_INVARIANT_VMESSAGE_ENABLED == 1
        #define ASSERT_INVARIANT() this->_assertInvariant() ///< Macro to call this->_assertInvariant().
    #else
        #define ASSERT_INVARIANT() ((void) 0) ///< No-op.
    #endif
#endif

// VMessage -------------------------------------------------------------------

const VString VMessage::kMessageLoggerName("messages");
const int VMessage::kMessageContentRecordingLevel;
const int VMessage::kMessageHeaderLevel;
const int VMessage::kMessageContentFieldsLevel;
const int VMessage::kMessageTrafficDetailsLevel;
const int VMessage::kMessageHandlerDispatchLevel;
const int VMessage::kMessageHandlerDetailLevel;
const int VMessage::kMessageContentHexDumpLevel;
const int VMessage::kMessageQueueOpsLevel;
const int VMessage::kMessageTraceDetailLevel;
const int VMessage::kMessageHandlerLifecycleLevel;

// static
void VMessage::release(/* @Nullable */ VMessage* message)
    {
    if (message == NULL)
        return;

    if (message->isBeingBroadcast())
        {
        VMutexLocker locker(message->getBroadcastMutex(), VSTRING_FORMAT("releaseMessage(%d)bmtx.locker    ", (int)message->getMessageID()));
        
        if (message->numBroadcastTargets() != 0)    // don't "remove" if not actually a target
            message->removeBroadcastTarget();

        if (message->numBroadcastTargets() != 0)    // still in someone's output queue, so return w/o deleting
            return;
        }

    VMessage::deleteMessage(message);
    }

// static
void VMessage::deleteMessage(/* @Nullable */ VMessage* message)
    {
    if (message == NULL)
        return;

    const int broadcastCount = message->numBroadcastTargets();
    if (broadcastCount != 0)
        VLOGGER_MESSAGE_ERROR(VSTRING_FORMAT("Message ID=%d @%p deleted with a broadcast count of %d. Risk that a queue still references it.", message->getMessageID(), message, broadcastCount));
    
    delete message;
    }

VMessage::VMessage() :
VBinaryIOStream(mMessageDataBuffer),
mMessageDataBuffer(1024),
mMessageID(0),
mIsBeingBroadcast(false),
mNumBroadcastTargets(0),
mBroadcastMutex("VMessage()") // -> unlocked
    {
    ASSERT_INVARIANT();
    }

VMessage::VMessage(VMessageID messageID, Vs64 initialBufferSize) :
VBinaryIOStream(mMessageDataBuffer),
mMessageDataBuffer(initialBufferSize),
mMessageID(messageID),
mIsBeingBroadcast(false),
mNumBroadcastTargets(0),
mBroadcastMutex(VSTRING_FORMAT("VMessage(%d)", (int)messageID)) // -> unlocked
    {
    ASSERT_INVARIANT();
    }

void VMessage::recycleForSend(VMessageID messageID)
    {
    mMessageID = messageID;
    mIsBeingBroadcast = false;
    mNumBroadcastTargets = 0;
    mBroadcastMutex.setName(VSTRING_FORMAT("VMessage(%d)", (int)messageID));
    (void) this->seek0();

    ASSERT_INVARIANT();
    }

void VMessage::recycleForReceive()
    {
    mMessageID = 0;
    mIsBeingBroadcast = false;
    mNumBroadcastTargets = 0;
    // mBroadcastMutex name remains, since no message ID is involved in its format.
    mMessageDataBuffer.setEOF(CONST_S64(0));

    ASSERT_INVARIANT();
    }

void VMessage::copyMessageData(VMessage& targetMessage) const
    {
    ASSERT_INVARIANT();

    Vs64 savedOffset = mMessageDataBuffer.getIOOffset();
    
    mMessageDataBuffer.seek0();
    (void) VStream::streamCopy(mMessageDataBuffer, targetMessage, mMessageDataBuffer.getEOFOffset());
    mMessageDataBuffer.seek(savedOffset, SEEK_SET);
    }

VMessageLength VMessage::getMessageDataLength() const
    {
    ASSERT_INVARIANT();

    return (VMessageLength) mMessageDataBuffer.getEOFOffset();
    }

Vu8* VMessage::getBuffer() const
    {
    ASSERT_INVARIANT();

    return mMessageDataBuffer.getBuffer();
    }

Vs64 VMessage::getBufferSize() const
    {
    ASSERT_INVARIANT();

    return mMessageDataBuffer.getBufferSize();
    }

void VMessage::addBroadcastTarget()
    {
    ASSERT_INVARIANT();

    mNumBroadcastTargets++;

    ASSERT_INVARIANT();
    }

void VMessage::removeBroadcastTarget()
    {
    ASSERT_INVARIANT();

    mNumBroadcastTargets--;

    ASSERT_INVARIANT();
    }

void VMessage::_assertInvariant() const
    {
    VASSERT_GREATER_THAN_OR_EQUAL(mNumBroadcastTargets, 0);
    }

