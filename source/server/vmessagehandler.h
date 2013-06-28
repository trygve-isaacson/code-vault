/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vmessagehandler_h
#define vmessagehandler_h

#include "vtypes.h"
#include "vstring.h"
#include "vmutex.h"
#include "vmutexlocker.h"
#include "vlogger.h"
#include "vmessage.h"
#include "vclientsession.h"

/** @file */

/**
    @ingroup vsocket
*/

class VServer;
class VSocketThread;

class VMessageHandlerFactory;
typedef std::map<VMessageID, VMessageHandlerFactory*> VMessageHandlerFactoryMap;

/**
VMessageHandler is the abstract base class for objects that process inbound
messages from various client connections. A VMessageHandler is constructed with
a message to be processed, and the server, session, and thread in which
or on behalf of which the handler is being executed. The base class
constructor is supplied an optional mutex to lock, and it does so using a
VMutexLocker instance variable mLocker, which subclass can choose to unlock
as appropriate. If no lock is needed, the subclass can either pass NULL for
the mutex in the base class constructor, or unlock the locker in the subclass
constructor. The concrete subclass needs to implement
the processMessage() function and do its work there to process the mMessage.
Because the mutex lock is acquired in the constructor, the
implementation may rely on exclusive access to the data that is protected by
the lock. This also means that if the handler runs for a significant amount
of time, it may need to take care to not keep the lock too long. Whether or not
the handler decides to farm its long-running work out to a background task or
do it locally, long-running work will have to release the lock and reacquire
it periodically, or other threads will be blocked for an unacceptably long
time. And if appropriate, a handler or background task should try to release
the lock as soon as possible when it no longer needs it, or if it doesn't
really need it in the first place.
*/
class VMessageHandler {
    public:

        /**
        Returns a message handler suitable for handling the specified
        message. When a message is read from the network, this is what
        is called to find a message handler for it. The message handler
        should then simply be told to processMessage(), and deleted.
        @param    m        the message to supply to the handler
        @param    server    the server to supply to the handler
        @param    session    the session for the client that sent this message, or NULL if n/a
        @param    thread    the thread processing the message (used to
                        correlate the message to a client session)
        */
        static VMessageHandler* get(VMessagePtr m, VServer* server, VClientSessionPtr session, VSocketThread* thread);
        /**
        Registers a message handler factory for a particular
        message ID. When a call is made to get(), the appropriate
        factory function is called to create a handler for the message
        ID.
        */
        static void registerHandlerFactory(VMessageID messageID, VMessageHandlerFactory* factory);

        /**
        Constructs a message handler with a message to handle and the
        server in which it is running.
        @param    name      the handler's name for logger output
        @param    m        the message to process
        @param    server    the server we're running in
        @param    session    the session for the client that sent this message, or NULL if n/a
        @param    thread    the thread processing the message (used to
                        correlate the message to a client session)
        @param    messageFactory  a factory that instantiates messages suitable for this handler
                            when it needs to send a message (assuming all such messages are uniform;
                            if not, messages can be instantiated explicitly) (The caller owns the factory.)
        @param  mutex   if not null, a mutex that will be initially locked by the constructor
                        and unlocked by the destructor
        */
        VMessageHandler(const VString& name, VMessagePtr m, VServer* server, VClientSessionPtr session, VSocketThread* thread, const VMessageFactory* messageFactory, VMutex* mutex);
        /**
        Virtual destructor. VMessageHandler will delete the owned message (setting mMessage to NULL
        will obviously circumvent this).
        */
        virtual ~VMessageHandler();

        /**
        Processes the message.
        */
        virtual void processMessage() = 0;
        /**
        Returns a newly instantiated message, using the message factory associated with
        this message handler.
        @param    messageID        value with which to init the message's message ID
        @return a message object
        */
        VMessagePtr getMessage(VMessageID messageID);
        /**
        Logs (at the appropriate log level) the supplied information about the
        message being handled. A message handler should call this to log the
        data contained in the inbound message, one whole message at a time. An
        optional facility here is that the caller may supplie the logger object
        to which the output will be written, and may obtain that object via
        a prior call to _getMessageContentRecordLogger(), which may return NULL. This allows
        the caller to: first call _getMessageContentRecordLogger() to obtain the logger
        object; if it's NULL (indicating the log level would emit nothing), it
        can avoid calling logMessageContentRecord() at all and also avoid building
        the log message strings; and if the logger is not NULL (indicating the
        log level would emit data) then it can supply the logger to
        logMessageContentRecord() so that this function doesn't have to keep re-finding
        the logger over repeated calls.
        @param    details    the text to be logged
        @param    logger    the logger to write to, or NULL to force the function to
                        look up the logger
        */
        void logMessageContentRecord(const VString& details, VNamedLoggerPtr logger = VNamedLoggerPtr()) const;
        /**
        Logs (at the appropriate log level) the supplied information about the
        message being handled. A message handler should call this to log the
        data contained in the inbound message, one field at a time. An
        optional facility here is that the caller may supply the logger object
        to which the output will be written, and may obtain that object via
        a prior call to _getMessageContentFieldsLogger(), which may return NULL. This allows
        the caller to: first call _getMessageContentFieldsLogger() to obtain the logger
        object; if it's NULL (indicating the log level would emit nothing), it
        can avoid calling logMessageContentFields() at all and also avoid building
        the log message strings; and if the logger is not NULL (indicating the
        log level would emit data) then it can supply the logger to
        logMessageContentFields() so that this function doesn't have to keep re-finding
        the logger over repeated calls.
        @param    details    the text to be logged
        @param    logger    the logger to write to, or NULL to force the function to
                        look up the logger
        */
        void logMessageContentFields(const VString& details, VNamedLoggerPtr logger = VNamedLoggerPtr()) const;
        /**
        Similar to logMessageContentFields, but for a lower level of messaging.
        */
        void logMessageDetailsFields(const VString& details, VNamedLoggerPtr logger = VNamedLoggerPtr()) const;
        /**
        Logs (at the appropriate log level) the message handler name to
        indicate that the handler has been invoked or has ended.
        */
        virtual void logProcessMessageStart() const;
        /**
        Logs (at the appropriate log level) the message handler name to
        indicate that the handler has been invoked or has ended.
        */
        virtual void logProcessMessageEnd() const;

    protected:

        /**
        Logs (at the appropriate log level) highly detailed status info
        about message handling dispatch.
        @param    dispatchInfo    the info to be logged
        */
        void _logDetailedDispatch(const VString& dispatchInfo) const;
        /**
        Logs (at the appropriate log level) simple content info for a message
        that has been received or will be sent. You should only supply a
        simple form of the data, not a full hex dump (see _logContentHexDump).
        @param    contentInfo    the info to be logged
        */
        void _logMessageContentRecord(const VString& contentInfo) const;
        VNamedLoggerPtr _getMessageContentRecordLogger() const; ///< Returns the logger for message content, or NULL if that level is not enabled.
        /**
        Logs (at the appropriate log level) simple content info for a message
        that has been received or will be sent. You should only supply a
        simple form of the data, not a full hex dump (see _logContentHexDump).
        @param    contentInfo    the info to be logged
        */
        void _logMessageContentFields(const VString& contentInfo) const;
        VNamedLoggerPtr _getMessageContentFieldsLogger() const; ///< Returns the logger for message fields, or NULL if that level is not enabled.
        /**
        Logs (at the appropriate log level) full hex dump content info for a message
        that has been received or will be sent..
        @param    info    informational text to label the output
        @param    buffer    the buffer to dump in hex form
        @param    length    the length of the buffer (or how much of it to dump)
        */
        void _logMessageContentHexDump(const VString& info, const Vu8* buffer, Vs64 length) const;

        VString                 mName;          ///< The name to identify this handler type in log output.
        VMessagePtr             mMessage;       ///< The message this handler is to process.
        VServer*                mServer;        ///< The server in which we are running.
        VClientSessionPtr       mSession;       ///< The session reference for which we are running, which holds NULL if n/a.
        VSocketThread*          mThread;        ///< The thread in which we are running.
        const VMessageFactory*  mMessageFactory;///< Factory for instantiating new messages this handler wants to send.
        VInstant                mStartTime;     ///< The time at which this handler was instantiated (message receipt). MUST BE DECLARED BEFORE mLocker.
        VMutexLocker            mLocker;        ///< The mutex locker for the mutex we were given.
        VInstant                mUnblockTime;   ///< The time at which this handler obtained the mLocker lock. MUST BE DECLARED AFTER mLocker.
        VString                 mSessionName;   ///< The name to identify this handler's session in log output.

    private:

        VMessageHandler(const VMessageHandler&); // not copyable
        VMessageHandler& operator=(const VMessageHandler&); // not assignable

        static VMessageHandlerFactoryMap* mapInstance();

        static VMessageHandlerFactoryMap* gFactoryMap;    ///< The factories that create handlers for each ID.
};

/**
VMessageHandlerFactory defines the interface for factory objects that know
how to create the appropriate type of concrete VMessageHandler subclass for
a particular message ID or set of message IDs. Normally you can just place
the DEFINE_MESSAGE_HANDLER_FACTORY macro in your message handler
implementation file.
*/
class VMessageHandlerFactory {
    public:

        VMessageHandlerFactory() {}
        virtual ~VMessageHandlerFactory() {}

        /**
        Instantiates a new message handler for the specified message's ID;
        must be overridden by the concrete subclass.
        @param    m        the message to be passed thru to the handler constructor
        @param    server    the serer to be passed thru to the handler constructor
        @param    session    the session to be passed thru to the handler constructor
        @param    thread    the thread to be passed thru to the handler constructor
        */
        virtual VMessageHandler* createHandler(VMessagePtr m, VServer* server, VClientSessionPtr session, VSocketThread* thread) = 0;
};

// This macro goes in the handler's .h file to define the handler's factory.
#define DEFINE_MESSAGE_HANDLER_FACTORY(messageid, factoryclassname, handlerclassname, descriptivename) \
class factoryclassname : public VMessageHandlerFactory { \
    public: \
    \
        factoryclassname() : VMessageHandlerFactory(), mName(VSTRING_ARGS("%s (%s)",#handlerclassname,descriptivename)) { VMessageHandler::registerHandlerFactory(messageid, this); } \
        virtual ~factoryclassname() {} \
        \
        virtual VMessageHandler* createHandler(VMessagePtr m, VServer* server, VClientSessionPtr session, VSocketThread* thread) \
            { return new handlerclassname(mName, m, server, session, thread); } \
    \
    private: \
    \
        static factoryclassname gFactory; \
        VString mName; \
    \
}

// This macro goes in the handler's .cpp file to declare the handler's factory.
#define DECLARE_MESSAGE_HANDLER_FACTORY(factoryclassname) \
factoryclassname factoryclassname::gFactory

// This macro goes in a global init function to prevent linker dead-stripping
// of the static initialization in the previous macro.
#define FORCE_LINK_MESSAGE_HANDLER_FACTORY(factoryclassname) \
if (false) { factoryclassname dummy; }

/**
This interface defines a background task object that can be attached to
a VClientSession, such that the session will not destruct until all attached
tasks have ended. If your VMessageHandler creates a task to perform work on
a separate thread, it should derive from VMessageHandlerTask so that the
session will know of its existence.
*/
class VMessageHandlerTask {
    public:

        VMessageHandlerTask(VClientSessionPtr session) : mSession(session) {}
        virtual ~VMessageHandlerTask() {}

    protected:

        VClientSessionPtr mSession;  ///< The session object we are associated with.

};

#endif /* vmessagehandler_h */
