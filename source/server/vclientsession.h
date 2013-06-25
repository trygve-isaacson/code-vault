/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vclientsession_h
#define vclientsession_h

/** @file */

#include "vstring.h"
#include "vmutex.h"
#include "vmutexlocker.h"
#include "vmessagequeue.h"
#include "vsocketstream.h"
#include "vbinaryiostream.h"

/**
    @ingroup vsocket
*/

class VMessageInputThread;
class VMessageOutputThread;
class VManagementInterface;
class VSocket;
class VSocketThread;
class VListenerThread;
class VMessage;
class VMessageHandlerTask;
class VServer;
class VBentoNode;

typedef std::vector<const VMessageHandlerTask*> SessionTaskList;

/**
This base class provides the API and services general to the various
types of client sessions that may keep a connection alive for a relatively
long time, typically with a user's login credentials. Its primary
functions are to manage the queue of outbound messages, and to ensure
that it is not destructed until pending attached threaded tasks complete
(see _selfDestruct() mechanism).
*/
class VClientSession : public VEnableSharedFromThis<VClientSession> {
    public:

        /**
        Initializes the session object.
        @param  sessionBaseName used to prefix unique info to build mName
        @param  server      the server that keeps track of the session
        @param  clientType  a string to distinguish the type of session
        @param  socket      the socket the session is using
        */
        VClientSession(const VString& sessionBaseName, VServer* server, const VString& clientType, VSocket* socket, const VDuration& standbyTimeLimit, Vs64 maxQueueDataSize);

        const VString& getName() const { return mName; }

        const VString& getClientType() const { return mClientType; }
        VMessageInputThread* getInputThread() const { return mInputThread; }
        VMessageOutputThread* getOutputThread() const { return mOutputThread; }

        /**
        Returns true if the session is "on-line", meaning that messages posted
        to its output queue should be sent; if not on-line, such messages will
        be queued up to be sent once the client becomes on-line. This must be
        implemented by the concrete class per its protocols.
        @return obvious
        */
        virtual bool isClientOnline() const = 0;
        /**
        Returns true fi the session is in the process of shutting down, meaning
        that messages posted to its output queue should be ignored rather than
        sent. This must be implemented by the concrete class per its protocols.
        @return obvious
        */
        virtual bool isClientGoingOffline() const = 0;
        /**
        Triggers a tear-down of the client session, typically in response to
        an i/o thread ending its run() method.
        @param  callingThread   the thread invoking the shutdown (NULL if n/a)
        */
        virtual void shutdown(VThread* callingThread);
        /**
        Posts a message to be sent to the client; if the session is using an
        output thread, the message is posted to the thread's output queue, where
        it will be sent when the output thread wakes up; if the session is NOT
        using an output thread, the message is written to the output stream
        immediately.
        Vault 4.0 change: No longer any need to specify broadcast/non-broadcast
        differences or for caller to handle posting failure.
        @param  message the message to be sent
        */
        void postOutputMessage(VMessagePtr message);
        /**
        Sends a message immediately to the supplied output stream, if the session
        is in a valid state (not in the middle of shutting down). The VMessageOutputThread
        class must use this to send asynchronous output messages in order to guarantee
        that the test for state and the act of sending is done in an atomic fashion with
        respect to the session state. We use our mMutex to protect the send operation
        and the session shutdown procedure from each other. This means that this method
        must not be called from inside one of our own methods that has the mutex locked.
        @param  message         the message to send; the caller still owns it afterward
        @param    sessionLabel    a label to use in log output, to identify the session
        @param    out                the stream to write to
        */
        void sendMessageToClient(VMessagePtr message, const VString& sessionLabel, VBinaryIOStream& out);
        /**
        Returns a string containing the client's address in address:port form.
        @return obvious
        */
        virtual const VString& getClientAddress() const { return mClientAddress; }

        /**
        Returns a new bento node with attributes describing the session. Subclasses
        may override this, call inherited, and then add their own attributes to the
        node. It is recommended to make all attributes strings since this is primarily
        used to display diagnostic information.
        */
        virtual VBentoNode* getSessionInfo() const;

    protected:

        virtual ~VClientSession(); // protected because only friend class VServer may delete us (when garbage collecting)

        void _moveStandbyMessagesToAsyncOutputQueue();  ///< Moves messages from mStartupStandbyQueue to the output queue.
        int _getOutputQueueSize() const; ///< Returns the number of messages currently queued on the output thread.

        /**
        This function can be overridden if the session needs to filter messages
        being moved from standby queue to output queue. The supplied message must either
        be posted (call inherited) or released. You can also fabricate a replacement
        message and post it instead of the supplied message.
        */
        virtual void _postStandbyMessageToAsyncOutputQueue(VMessagePtr message);

        VString                 mName;          ///< A name for the session to use in logging; built from supplied base name + IP address + port.
        VMutex                  mMutex;         ///< A mutex we use to enforce sequential processing of outbound messages, and to protect our task list.
        VServer*                mServer;        ///< The server that keeps track of this session.
        VString                 mClientType;    ///< A string distinguishing this type of session.
        VString                 mClientIP;      ///< The client's IP address (could be name or number).
        int                     mClientPort;    ///< The IP port number of the client session.
        VString                 mClientAddress; ///< The user-visible string we use for logging, contains IP address + port of session.
        VMessageInputThread*    mInputThread;   ///< The thread that is reading inbound messages from the client.
        VMessageOutputThread*   mOutputThread;  ///< If using a separate output thread, this is it (may be NULL for sync i/o model).
        bool                    mIsShuttingDown;///< True if we are in the process of tearing down the session.

    private:

        VClientSession(const VClientSession&); // not copyable
        VClientSession& operator=(const VClientSession&); // not assignable

        void _releaseQueuedClientMessages();   ///< Releases all pending queued messages (called during shutdown).

        VMessageQueue   mStartupStandbyQueue;   ///< A queue we use to hold outbound updates while this client session is starting up.
        VInstant        mStandbyStartTime;      ///< The time at which we started queueing standby messages; reset by _moveStandbyMessagesToAsyncOutputQueue().
        VDuration       mStandbyTimeLimit;      ///< Once we go to standby, a time limit applies after which posting standby causes session shutdown due to presumed failure.
        Vs64            mMaxClientQueueDataSize;///< If non-zero, if a message is posted when there are already this many bytes queued, we close the socket.

        // We only access the socket i/o stream if postOutputMessage() is called
        // and we are not set up to use a separate output message thread. However, we are responsible
        // for deleting the socket object.
        VSocket*        mSocket;        ///< The socket this session is using.
        VSocketStream   mSocketStream;  ///< The underlying raw socket stream over which this thread communicates.
        VBinaryIOStream mIOStream;      ///< The binary-format i/o stream over the raw socket stream.
};

typedef VSharedPtr<VClientSession> VClientSessionPtr;
typedef VSharedPtr<const VClientSession> VClientSessionConstPtr;
typedef std::vector<VClientSessionPtr> VClientSessionList;

/**
Implement a subclass of VClientSessionFactory (specifically, the createSession() method)
to create a socket listener that will create the desired type of VClientSession whenever
an incoming connection is accepted on that socket. This class is a parameter to the
VListenerThread constructor.
*/
class VClientSessionFactory {
    public:

        /**
        Initializes the factory.
        @param  manager the manager to be supplied to sessions that are created
        @param  server  the server to be supplied to sessions that are created
        */
        VClientSessionFactory(VManagementInterface* manager, VServer* server) : mManager(manager), mServer(server) {}
        virtual ~VClientSessionFactory() {}

        /**
        This is the method you must implement to instantiate the concrete type
        of VClientSession.
        @param  socket      the socket on which the client has connected
        @param  ownerThread the listener thread that accepted the connection
        */
        virtual VClientSessionPtr createSession(VSocket* socket, VListenerThread* ownerThread) = 0;

        /**
        Adds the specified session to the server; the server keeps track of
        its sessions for purposes of broadcasting, clean shutdown, etc.
        @param  session the session that has been created
        */
        void addSessionToServer(VClientSessionPtr session);

        /**
        Sets the management interface to receive notifications. May be NULL if
        no notifications are to be given. This manager may be passed to the
        sessions created and their threads.
        @param  manager the manager to notify, or NULL
        */
        void setManager(VManagementInterface* manager) { mManager = manager; }

    protected:

        VClientSessionFactory(const VClientSessionFactory&); // not copyable
        VClientSessionFactory& operator=(const VClientSessionFactory&); // not assignable

        VManagementInterface*   mManager;   ///< The object that will be notified of session events.
        VServer*                mServer;    ///< The server that will be notified of session creation.
};

#endif /* vclientsession_h */
