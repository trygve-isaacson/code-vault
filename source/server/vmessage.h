/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

#ifndef vmessage_h
#define vmessage_h

#include "vtypes.h"
#include "vstring.h"
#include "vbinaryiostream.h"
#include "vmutex.h"
#include "vmemorystream.h"
#include "vlogger.h"

/** @file */

/**
    @ingroup vsocket
*/

class VServer;

typedef Vs32 VMessageLength;    ///< The length of a message. Meaning and format on the wire are determined by actual message protocol.
typedef Vs16 VMessageID;        ///< Message identifier (verb) to distinguish it from other messages in the protocol.

class VMessagePool;

/** Emits a message at kFatal level to the message logger. */
#define VLOGGER_MESSAGE_FATAL(message) VLogger::getLogger(VMessage::kMessageLoggerName)->log(VLogger::kFatal, __FILE__, __LINE__, message)
/** Emits a message at kError level to the message logger. */
#define VLOGGER_MESSAGE_ERROR(message) VLogger::getLogger(VMessage::kMessageLoggerName)->log(VLogger::kError, __FILE__, __LINE__, message)
/** Emits a message at kWarn level to the message logger. */
#define VLOGGER_MESSAGE_WARN(message) VLogger::getLogger(VMessage::kMessageLoggerName)->log(VLogger::kWarn, NULL, 0, message)
/** Emits a message at kInfo level to the message logger. */
#define VLOGGER_MESSAGE_INFO(message) VLogger::getLogger(VMessage::kMessageLoggerName)->log(VLogger::kInfo, NULL, 0, message)
/** Emits a message at kDebug level to the message logger. */
#define VLOGGER_MESSAGE_DEBUG(message) VLogger::getLogger(VMessage::kMessageLoggerName)->log(VLogger::kDebug, NULL, 0, message)
/** Emits a message at specified level to the message logger; use the level constants defined in VMessage. */
#define VLOGGER_MESSAGE_LEVEL(level, message) do { VLogger* vmxcond = VLogger::getLoggerConditional(VMessage::kMessageLoggerName, level); if (vmxcond != NULL) vmxcond->log(level, NULL, 0, message); } while (false)
/** Emits a hex dump at a specified level to the specified logger. */
#define VLOGGER_MESSAGE_HEXDUMP(message, buffer, length) do { VLogger* vmxcond = VLogger::getLoggerConditional(VMessage::kMessageLoggerName, VMessage::kMessageContentHexDumpLevel); if (vmxcond != NULL) vmxcond->logHexDump(VMessage::kMessageContentHexDumpLevel, message, buffer, length); } while (false)
/** More efficient macro that avoids building log output unless log level is satisfied; Emits a message at specified level to the message logger; use the level constants defined in VMessage. */
#define VLOGGER_CONDITIONAL_MESSAGE_LEVEL(level, message) do { VLogger* vmxcond = VLogger::getLoggerConditional(VMessage::kMessageLoggerName, level); if (vmxcond != NULL) vmxcond->log(level, NULL, 0, message); } while (false)
// VLOGGER_CONDITIONAL_MESSAGE_LEVEL is no longer necessary since VLOGGER_MESSAGE_LEVEL now does the same conditional check.

/**
VMessage is an abstract base class that implements the basic messaging
capabilities; the concrete subclass must implement the send() and receive()
functions, which know how to read and write the particular message protocol
format (the wire protocol). This class works with VMessagePool by allowing
messages to be "recycled"; recycling takes an existing pooled message and
makes it ready to be used again as if it had been newly instantiated.
*/
class VMessage : public VBinaryIOStream
    {
    public:

        // Constants for the recycle() parameter makeEmpty.
        static const bool kMakeEmpty = true;    ///< (Default) The message buffer length will be set to zero, effectively resetting the message buffer to empty.
        static const bool kKeepData = false;    ///< The message buffer will be left alone so that the existing message data can be retained.

        /**
        Constructs an empty message with no message ID defined,
        suitable for use with receive(). You can also set the
        message ID afterwards with setMessageID().
        */
        VMessage();
        /**
        Constructs a message with a message ID, suitable for use
        with send(), optionally writing message data first.
        @param    messageID        the message ID
        */
        VMessage(VMessageID messageID, VMessagePool* pool, Vs64 initialBufferSize=1024);
        /**
        Virtual destructor.
        */
        virtual ~VMessage();

        /**
        Returns the pool to which this message belongs; when releasing
        a message, it must be released to the correct pool.
        @return the pool this message belongs to
        */
        VMessagePool* getPool() const { return mPool; }

        /**
        Re-initializes the message to be in a usable state as if
        it had just been instantiated; useful when re-using a single
        message object for multiple messages, or when pooling.
        @param    messageID    the message ID to set for the message
        @param    makeEmpty    normally true, which resets the message length to zero; if false,
                            leaves the message buffer alone so the data there remains intact
                            and available as the recycled message's data
        */
        virtual void recycle(VMessageID messageID=0, bool makeEmpty=kMakeEmpty);

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
        /**
        Returns true if this message is being broadcast.
        @return true if this message is being broadcast
        */
        bool isBeingBroadcast() const { return mIsBeingBroadcast; }
        /**
        Marks this message as being for broadcast. This must be called by any broadcast
        functions (i.e., bottlenecked in VServer::postBroadcastMessage) in
        order to mark the message so that during pool release we know to lock the
        broadcast mutex and correctly release the message only when the last broadcast
        target is done with the message.
        @return true if this message is being broadcast
        */
        void markForBroadcast() { mIsBeingBroadcast = true; }
        /**
        Returns the number of outstanding broadcast targets; caller must lock mutex as appropriate.
        @return the number of outstanding broadcast targets
        */
        int numBroadcastTargets() const { return mNumBroadcastTargets; }
        /**
        Returns a pointer to the broadcast mutex, which a caller can
        lock while adding the set of broadcast targets to the message.
        @return a pointer to the broadcast mutex
        */
        VMutex* getBroadcastMutex() { return &mBroadcastMutex; }
        /**
        Increments this message's broadcast target count; caller must lock mutex as appropriate.
        */
        void addBroadcastTarget();
        /**
        Decrements this message's broadcast target count; caller must lock mutex as appropriate.
        */
        void removeBroadcastTarget();
        /**
        The message is no longer used (and is queued).
        */
        void release();
        /*
        These are the log level definitions used for consistent logging
        of message traffic, processing, pooling, and dispatch. Use these
        levels and log to the logger named VMessage::kMessageLoggerName
        in order to be consistent in message logging.

        The format of logged messages should be:
         [session-label] data
        where session-label is:
         id:ip:port
        where id is some descriptive name of the session that does not distinguish which
        client it is (since the ip:port does that)

        Use the macros defined at the top of this file to emit message log output.
        */
        static const VString kMessageLoggerName;
        static const int kMessageContentRecordingLevel  = VLogger::kInfo + 0;   // human-readable single-line form of message content (e.g., bento text format)
        static const int kMessageHeaderLevel            = VLogger::kDebug + 0;  // message meta data such as ID, length, key, etc.
        static const int kMessageContentFieldsLevel     = VLogger::kDebug + 1;  // human-readable multi-line form of message content (e.g., non-bento message fields)
        static const int kMessageTrafficDetailsLevel    = VLogger::kDebug + 2;  // lower level details about message traffic
        static const int kMessageHandlerDispatchLevel   = VLogger::kDebug + 3;  // start and end of every message handler
        static const int kMessageHandlerDetailLevel     = VLogger::kDebug + 4;  // start and end of every message handler task, plus details of broadcast posting
        static const int kMessageContentHexDumpLevel    = VLogger::kDebug + 5;  // hex dump of message content
        static const int kMessageQueueOpsLevel          = VLogger::kDebug + 6;  // low-level operations of message i/o queues
        static const int kMessageTraceDetailLevel       = VLogger::kTrace;      // extremely low-level message processing details
        static const int kMessageHandlerLifecycleLevel  = VLogger::kTrace;      // message handler constructor and destructor
        static const int kMessagePoolTraceLevel         = VLogger::kTrace;      // low-level operations of message reuse pools

    protected:

        mutable VMemoryStream    mMessageDataBuffer;        ///< The buffer that holds the message data. Mutable because copyMessageData needs to touch it and restore it.

    private:

        VMessage(const VMessage&); // not copyable
        VMessage& operator=(const VMessage&); // not assignable

        /** Asserts if any invariant is broken. */
        void _assertInvariant() const;

        VMessageID      mMessageID;             ///< The message ID, either read during receive or to be written during send.
        VMessagePool*   mPool;                  ///< The pool where this message should be released to.
        bool            mIsBeingBroadcast;      ///< True if this message is an outbound broadcast message.
        bool            mIsReleased;            ///< True if this message has been deleted/released
        int             mNumBroadcastTargets;   ///< Number of pending broadcast targets, if for broadcast.
        VMutex          mBroadcastMutex;        ///< Mutex to control multiple threads using this message during broadcasting. This is declared public because the caller is responsible for locking this mutex via a VMutexLocker while posting for broadcast.
    };

/**
VMessageFactory is an abstract base class that you must implement and
supply to a VMessagePool, so that the pool can instantiate new messages
when needed. All you have to do is implement the instantiateNewMessage()
function to return a new VMessage of the desired subclass type.
*/
class VMessageFactory
    {
    public:

        VMessageFactory() {}
        virtual ~VMessageFactory() {}

        /**
        Must be implemented by subclass, to simply instantiate a
        new VMessage object of a concrete VMessage subclass type.
        @param    messageID    the ID to supply to the message constructor
        @return    pointer to a new message object
        */
        virtual VMessage* instantiateNewMessage(VMessageID messageID, VMessagePool* pool) = 0;
    };

#endif /* vmessage_h */
