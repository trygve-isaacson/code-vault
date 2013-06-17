/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#include "vmessage.h"

#include "vlogger.h"

// VMessage -------------------------------------------------------------------

const VString VMessage::kMessageLoggerName("messages");
const int VMessage::kMessageContentRecordingLevel  = VLoggerLevel::INFO;
const int VMessage::kMessageHeaderLevel            = VLoggerLevel::DEBUG;
const int VMessage::kMessageContentFieldsLevel     = VLoggerLevel::DEBUG + 1;
const int VMessage::kMessageTrafficDetailsLevel    = VLoggerLevel::DEBUG + 2;
const int VMessage::kMessageHandlerDispatchLevel   = VLoggerLevel::DEBUG + 3;
const int VMessage::kMessageHandlerDetailLevel     = VLoggerLevel::DEBUG + 4;
const int VMessage::kMessageContentHexDumpLevel    = VLoggerLevel::DEBUG + 5;
const int VMessage::kMessageQueueOpsLevel          = VLoggerLevel::DEBUG + 6;
const int VMessage::kMessageTraceDetailLevel       = VLoggerLevel::TRACE;
const int VMessage::kMessageHandlerLifecycleLevel  = VLoggerLevel::TRACE;

VMessage::VMessage()
    : VBinaryIOStream(mMessageDataBuffer)
    , mMessageDataBuffer(1024)
    , mMessageID(0)
    {
}

VMessage::VMessage(VMessageID messageID, Vs64 initialBufferSize)
    : VBinaryIOStream(mMessageDataBuffer)
    , mMessageDataBuffer(initialBufferSize)
    , mMessageID(messageID)
    {
}

void VMessage::recycleForSend(VMessageID messageID) {
    mMessageID = messageID;
    (void) this->seek0();
}

void VMessage::recycleForReceive() {
    mMessageID = 0;
    mMessageDataBuffer.setEOF(CONST_S64(0));
}

void VMessage::copyMessageData(VMessage& targetMessage) const {
    Vs64 savedOffset = mMessageDataBuffer.getIOOffset();

    mMessageDataBuffer.seek0();
    (void) VStream::streamCopy(mMessageDataBuffer, targetMessage, mMessageDataBuffer.getEOFOffset());
    mMessageDataBuffer.seek(savedOffset, SEEK_SET);
}

VMessageLength VMessage::getMessageDataLength() const {
    return (VMessageLength) mMessageDataBuffer.getEOFOffset();
}

Vu8* VMessage::getBuffer() const {
    return mMessageDataBuffer.getBuffer();
}

Vs64 VMessage::getBufferSize() const {
    return mMessageDataBuffer.getBufferSize();
}

