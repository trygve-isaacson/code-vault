/*
Copyright c1997-2007 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

#include "vmessage.h"

#include "vmutexlocker.h"
#include "vexception.h"
#include "vsocketthread.h"
#include "vmessagehandler.h"
#include "vmessagepool.h"

// VMessage -------------------------------------------------------------------

const VString VMessage::kMessageLoggerName("messages");

VMessage::VMessage() :
VBinaryIOStream(mMessageDataBuffer),
mMessageDataBuffer(1024),
mMessageID(0),
mPool(NULL),
mIsBeingBroadcast(false),
mIsReleased(false),
mNumBroadcastTargets(0)
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
mNumBroadcastTargets(0)
	{
	ASSERT_INVARIANT();
	}

VMessage::~VMessage()
	{
//	ASSERT_INVARIANT();
	mIsReleased = true;
	}

void VMessage::release()
	{
	ASSERT_INVARIANT();
	mIsReleased = true;
	}

void VMessage::recycle(VMessageID messageID, bool makeEmpty)
	{
	mMessageID = messageID;
	mIsBeingBroadcast = false;
	mIsReleased = false;
	mNumBroadcastTargets = 0;

	if (makeEmpty)
		mMessageDataBuffer.setEOF(CONST_S64(0));
	else
		(void) this->seek(CONST_S64(0), SEEK_SET);

	ASSERT_INVARIANT();
	}

void VMessage::copyMessageData(VMessage& targetMessage) const
	{
	ASSERT_INVARIANT();

	Vs64	savedOffset = mMessageDataBuffer.offset();
	
	mMessageDataBuffer.seek(0, SEEK_SET);
	streamCopy(mMessageDataBuffer, targetMessage, mMessageDataBuffer.eofOffset());
	mMessageDataBuffer.seek(savedOffset, SEEK_SET);
	}

VMessageLength VMessage::getMessageDataLength() const
	{
	ASSERT_INVARIANT();

	return (VMessageLength) mMessageDataBuffer.eofOffset();
	}

Vu8* VMessage::getBuffer() const
	{
	ASSERT_INVARIANT();

	return mMessageDataBuffer.getBuffer();
	}

Vs64 VMessage::getBufferSize() const
	{
	ASSERT_INVARIANT();

	return mMessageDataBuffer.bufferSize();
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

void VMessage::assertInvariant() const
	{
	// 2007.03.22 ranstrom v1.3D ARGO-6599 Performance: each separate V_ASSERT is somewhat slow. Roll in to one call.
	/*
	V_ASSERT(mNumBroadcastTargets >= 0);
	V_ASSERT(!mIsReleased);
	*/
	bool ok = true;
	ok = ok && (mNumBroadcastTargets >= 0);
	ok = ok && (!mIsReleased);
	V_ASSERT(ok);
	}

