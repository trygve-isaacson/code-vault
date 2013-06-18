/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vmessage_h
#define vmessage_h

#include "vtypes.h"
#include "vstring.h"
#include "vbinaryiostream.h"
#include "vmemorystream.h"
#include "vlogger.h"

#include <boost/shared_ptr.hpp>

/** @file */

/**
    @ingroup vsocket
*/

class VServer;

typedef Vs32 VMessageLength;    ///< The length of a message. Meaning and format on the wire are determined by actual message protocol.
typedef int  VMessageID;        ///< Message identifier (verb) to distinguish it from other messages in the protocol.

/** Emits a message at specified level to the message logger; use the level constants defined in VMessage. */
#define VLOGGER_MESSAGE_LEVEL(level, message) VLOGGER_NAMED_LEVEL(VMessage::kMessageLoggerName, level, message)
/** Emits a message at kFatal level to the message logger. */
#define VLOGGER_MESSAGE_FATAL(message) VLOGGER_NAMED_FATAL(VMessage::kMessageLoggerName, message)
/** Emits a message at kError level to the message logger. */
#define VLOGGER_MESSAGE_ERROR(message) VLOGGER_NAMED_ERROR(VMessage::kMessageLoggerName, message)
/** Emits a message at kWarn level to the message logger. */
#define VLOGGER_MESSAGE_WARN(message) VLOGGER_NAMED_WARN(VMessage::kMessageLoggerName, message)
/** Emits a message at kInfo level to the message logger. */
#define VLOGGER_MESSAGE_INFO(message) VLOGGER_NAMED_INFO(VMessage::kMessageLoggerName, message)
/** Emits a message at kDebug level to the message logger. */
#define VLOGGER_MESSAGE_DEBUG(message) VLOGGER_NAMED_DEBUG(VMessage::kMessageLoggerName, message)
/** Emits a message at kTrace level to the message logger. */
#define VLOGGER_MESSAGE_TRACE(message) VLOGGER_NAMED_TRACE(VMessage::kMessageLoggerName, message)
/** Emits a hex dump at a specified level to the specified logger. */
#define VLOGGER_MESSAGE_HEXDUMP(message, buffer, length) VLOGGER_NAMED_HEXDUMP(VMessage::kMessageLoggerName, VMessage::kMessageContentHexDumpLevel, message, buffer, length)
/** Returns true if the message logger would emit at the specified level. */
#define VLOGGER_MESSAGE_WOULD_LOG(level) VLOGGER_NAMED_WOULD_LOG(VMessage::kMessageLoggerName, level)

/**
VMessage is an abstract base class that implements the basic messaging
capabilities; the concrete subclass must implement the send() and receive()
functions, which know how to read and write the particular message protocol
format (the wire protocol).
*/
class VMessage : public VBinaryIOStream {
    public:

        /**
        Readies the message to be re-used with the existing message data intact,
        for posting to a session or client. The new message ID is applied, and
        some internal bookkeeping may be performed, but the message data is left
        alone to be sent, as if it had just been formed. This is designed for use
        when you receive a message, and then decide to post it or send it without
        modification (except optionally changing the message ID).
        @param    messageID    the message ID to set for the message
        */
        virtual void recycleForSend(VMessageID messageID);
        /**
        Readies the message to be re-used to read another message from a stream,
        as if it had just been instantiated, but without pre/re-allocating the
        data buffer space. This can be effective if you have an input loop
        that reads many messages using a single VMessage on the stack.
        */
        virtual void recycleForReceive();

        /**
        Sets the message ID, which is used when sending.
        @param    messageID    the message ID
        */
        void setMessageID(VMessageID messageID) { mMessageID = messageID; }
        /**
        Returns the message ID.
        */
        VMessageID getMessageID() const { return mMessageID; }

        /**
        Sends the message to the output stream, using the appropriate wire
        protocol message format; for example, it might write the message
        data content length, the message ID, and then the message data. The
        message data content is stored in the mMessageDataBuffer and it is
        typically just copied to the output stream using <code>streamCopy()</code>.
        The data length can be obtained by calling
        <code>this->getMessageDataLength()</code>.
        @param    sessionLabel    a label to use in log output, to identify the session
        @param    out                the stream to write to
        */
        virtual void send(const VString& sessionLabel, VBinaryIOStream& out) = 0;
        /**
        Receives the message from the input stream, using the appropriate wire
        protocol format; for example, it might read the message data content
        length, the message ID, and then the message data. The message data
        should be read into the mMessageDataBuffer, typically by calling
        <code>streamCopy()</code> after you have read the length value.
        @param    sessionLabel    a label to use in log output, to identify the session
        @param    in                the stream to read from
        */
        virtual void receive(const VString& sessionLabel, VBinaryIOStream& in) = 0;
        /**
        Copies this message's data to the target message's data buffer.
        The target's ID and other meta information (such as broadcast
        info) is not altered. This message's i/o offset is restored
        upon return, so its internal state is essentially untouched.
        The target message's offset is honored and altered, so you
        could use this function to append data to the target message
        at its current i/o offset.
        I've declared mMessageDataBuffer mutable so that this function
        can be properly declared const and still call non-const functions
        of mMessageDataBuffer (it saves and restores the buffer's state).
        */
        void copyMessageData(VMessage& targetMessage) const;

        /**
        Returns the message data length (does not include the length of
        the message ID nor the message length indicator itself).
        @return the message data length
        */
        VMessageLength getMessageDataLength() const;
        /**
        Returns a pointer to the raw message data buffer -- should only be used
        for debugging and logging purposes. The length of the valid data in the buffer
        is getMessageDataLength(). The returned pointer is only guaranteed to be
        valid as long as the message itself exists and is not written to (writing
        may require the buffer to be reallocated).
        @return a pointer to the raw message data buffer
        */
        Vu8* getBuffer() const;
        /**
        Returns the total size of the memory buffer space consumed by this
        message; this is mainly for use in logging and debugging.
        @return the message data length
        */
        Vs64 getBufferSize() const;
        /*
        These are the log level definitions used for consistent logging
        of message traffic, processing, and dispatch. Use these
        levels and log to the logger named VMessage::kMessageLoggerName
        in order to be consistent in message logging.
        There are issues with some compilers and lazy constant evaluation even
        though these are ints, so we define the actual values in the .cpp file
        separately.

        The format of logged messages should be:
         [session-label] data
        where session-label is:
         id:ip:port
        where id is some descriptive name of the session that does not distinguish which
        client it is (since the ip:port does that)

        Use the macros defined at the top of this file to emit message log output.
        */
        static const VString kMessageLoggerName;
        static const int kMessageContentRecordingLevel; ///< VLoggerLevel::INFO      -- human-readable single-line form of message content (e.g., bento text format)
        static const int kMessageHeaderLevel;           ///< VLoggerLevel::DEBUG     -- message meta data such as ID, length, key, etc.
        static const int kMessageContentFieldsLevel;    ///< VLoggerLevel::DEBUG + 1 -- human-readable multi-line form of message content (e.g., non-bento message fields)
        static const int kMessageTrafficDetailsLevel;   ///< VLoggerLevel::DEBUG + 2 -- lower level details about message traffic
        static const int kMessageHandlerDispatchLevel;  ///< VLoggerLevel::DEBUG + 3 -- start and end of every message handler
        static const int kMessageHandlerDetailLevel;    ///< VLoggerLevel::DEBUG + 4 -- start and end of every message handler task, plus details of broadcast posting
        static const int kMessageContentHexDumpLevel;   ///< VLoggerLevel::DEBUG + 5 -- hex dump of message content
        static const int kMessageQueueOpsLevel;         ///< VLoggerLevel::DEBUG + 6 -- low-level operations of message i/o queues
        static const int kMessageTraceDetailLevel;      ///< VLoggerLevel::TRACE     -- extremely low-level message processing details
        static const int kMessageHandlerLifecycleLevel; ///< VLoggerLevel::TRACE     -- message handler constructor and destructor

    protected:

        /**
        Constructs an empty message with no message ID defined,
        suitable for use with receive(). You can also set the
        message ID afterwards with setMessageID().
        */
        VMessage();
        /**
        Constructs a message with a message ID, suitable for use
        with send(), optionally writing message data first.
        @param  messageID           the message ID
        @param  initialBufferSize   if specified, the size of data buffer to preallocate
        */
        VMessage(VMessageID messageID, Vs64 initialBufferSize = 1024);
        /**
        Virtual destructor.
        */
        virtual ~VMessage() {}

        mutable VMemoryStream    mMessageDataBuffer;        ///< The buffer that holds the message data. Mutable because copyMessageData needs to touch it and restore it.

    private:

        VMessage(const VMessage&); // not copyable
        VMessage& operator=(const VMessage&); // not assignable

        VMessageID      mMessageID;             ///< The message ID, either read during receive or to be written during send.
};

typedef boost::shared_ptr<VMessage> VMessagePtr;
typedef boost::shared_ptr<const VMessage> VMessageConstPtr;

/**
VMessageFactory is an abstract base class that you must implement for purposes
of giving an input thread a way to instantiate the correct concrete type of
message. All you have to do is implement the instantiateNewMessage()
function to return a new VMessage of the desired subclass type.
*/
class VMessageFactory {
    public:

        VMessageFactory() {}
        virtual ~VMessageFactory() {}

        /**
        Must be implemented by subclass, to simply instantiate a
        new VMessage object of a concrete VMessage subclass type.
        @param    messageID    the ID to supply to the message constructor
        @return    pointer to a new message object
        */
        virtual VMessagePtr instantiateNewMessage(VMessageID messageID = 0) const = 0;
};

#endif /* vmessage_h */
