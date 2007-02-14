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

VMessage::VMessage(Vs64 initialBufferSize) :
VBinaryIOStream(mMessageDataBuffer),
mMessageDataBuffer(initialBufferSize),
mMessageID(0),
mPool(NULL),
mIsBeingBroadcast(false),
mNumBroadcastTargets(0)
	{
	}

VMessage::VMessage(VMessageID messageID, VMessagePool* pool, Vs64 initialBufferSize) :
VBinaryIOStream(mMessageDataBuffer),
mMessageDataBuffer(initialBufferSize),
mMessageID(messageID),
mPool(pool),
mIsBeingBroadcast(false),
mNumBroadcastTargets(0)
	{
	}

void VMessage::recycle(VMessageID messageID, bool makeEmpty)
	{
	mMessageID = messageID;
	mIsBeingBroadcast = false;
	mNumBroadcastTargets = 0;

	if (makeEmpty)
		mMessageDataBuffer.setEOF(CONST_S64(0));
	else
		(void) this->seek(CONST_S64(0), SEEK_SET);
	}

void VMessage::copyMessageData(VMessage& targetMessage) const
	{
	Vs64	savedOffset = mMessageDataBuffer.offset();
	
	mMessageDataBuffer.seek(0, SEEK_SET);
	streamCopy(mMessageDataBuffer, targetMessage, mMessageDataBuffer.eofOffset());
	mMessageDataBuffer.seek(savedOffset, SEEK_SET);
	}

VMessageLength VMessage::getMessageDataLength() const
	{
	return (VMessageLength) mMessageDataBuffer.eofOffset();
	}

Vu8* VMessage::getBuffer() const
	{
	return mMessageDataBuffer.getBuffer();
	}

Vs64 VMessage::getBufferSize() const
	{
	return mMessageDataBuffer.bufferSize();
	}

void VMessage::addBroadcastTarget()
	{
	mNumBroadcastTargets++;
	}

void VMessage::removeBroadcastTarget()
	{
	mNumBroadcastTargets--;
	}

