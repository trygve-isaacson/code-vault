/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

#include "vmessage.h"

#include "vmutexlocker.h"
#include "vexception.h"
#include "vsocketthread.h"
#include "vmessagehandler.h"
#include "vmessagepool.h"

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

VMessage::VMessage() :
VBinaryIOStream(mMessageDataBuffer),
mMessageDataBuffer(1024),
mMessageID(0),
mPool(NULL),
mIsBeingBroadcast(false),
mIsReleased(false),
mNumBroadcastTargets(0),
mBroadcastMutex("VMessage()bmtx                  ") // -> unlocked
    {
    ASSERT_INVARIANT();
    }

VMessage::VMessage(VMessageID messageID, VMessagePool* pool, Vs64 initialBufferSize) :
VBinaryIOStream(mMessageDataBuffer),
mMessageDataBuffer(initialBufferSize),
mMessageID(messageID),
mPool(pool),
mIsBeingBroadcast(false),
mIsReleased(false),
mNumBroadcastTargets(0),
mBroadcastMutex(VString("VMessage(%d)bmtx                 ", (int)messageID)) // -> unlocked
    {
    ASSERT_INVARIANT();
    }

VMessage::~VMessage()
    {
    mIsReleased = true;
    }

void VMessage::release()
    {
    ASSERT_INVARIANT();
    mIsReleased = true;
    mBroadcastMutex.setName(VString("VMessage(%d)(released)bmtx        ", (int)mMessageID));
    }

void VMessage::recycle(VMessageID messageID, bool makeEmpty)
    {
    mMessageID = messageID;
    mIsBeingBroadcast = false;
    mIsReleased = false;
    mNumBroadcastTargets = 0;
    mBroadcastMutex.setName(VString("VMessage(%d)(recycled)bmtx        ", (int)messageID));

    if (makeEmpty)
        mMessageDataBuffer.setEOF(CONST_S64(0));
    else
        (void) this->seek(CONST_S64(0), SEEK_SET);

    ASSERT_INVARIANT();
    }

void VMessage::copyMessageData(VMessage& targetMessage) const
    {
    ASSERT_INVARIANT();

    Vs64 savedOffset = mMessageDataBuffer.getIOOffset();
    
    mMessageDataBuffer.seek(0, SEEK_SET);
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
    }

void VMessage::removeBroadcastTarget()
    {
    ASSERT_INVARIANT();

    mNumBroadcastTargets--;
    }

void VMessage::_assertInvariant() const
    {
    V_ASSERT(
        (mNumBroadcastTargets >= 0)
        );
    }

