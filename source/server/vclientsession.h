/*
Copyright c1997-2007 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

#ifndef vclientsession_h
#define vclientsession_h

/** @file */

#include "vstring.h"
#include "vmutex.h"
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

typedef std::vector<const VMessageHandlerTask*> SessionTaskList;

/**
This base class provides the API and services general to the various
types of client sessions that may keep a connection alive for a relatively
long time, typically with a user's login credentials. Its primary
functions are to manage the queue of outbound messages, and to ensure
that it is not destructed until pending attached threaded tasks complete
(see _selfDestruct() mechanism).
*/
class VClientSession
    {
    public:
    
        /**
        Initializes the session object.
        @param  sessionBaseName used to prefix unique info to build mName
        @param  server      the server that keeps track of the session
        @param  clientType  a string to distinguish the type of session
        @param  socket      the socket the session is using
        */
        VClientSession(const VString& sessionBaseName, VServer* server, const VString& clientType, VSocket* socket);
        
        const VString& getName() const { return mName; }
        
        const VString& getClientType() const { return mClientType; }
        VMessageInputThread* getInputThread() const { return mInputThread; }
        VMessageOutputThread* getOutputThread() const { return mOutputThread; }
        
        /**
        Adds a task to the session's task list; when the session is
        shut down, it will delay destruction until the task list is
        empty. (The tasks presumably point to the session.)
        */
        void attachTask(const VMessageHandlerTask* task);
        /**
        Removes a task from the session's task list, so that the task
        does not cause session shutdown to wait for the task.
        */
        void detachTask(const VMessageHandlerTask* task);

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
        @param  message             the message to be sent
        @param  releaseIfNotPosted  true means that if the message is not posted
                    (for example, if the session is going off-line), it should
                    be released; a broadcast message would specify false because
                    other clients
        @param  queueStandbyIfStartingUp    true means that if the client is not
                    yet online, the message should be queued to be sent upon the
                    client going online; false means if the client is not yet online
                    the message should be ignored (for this client session)
        @return true if the message was posted
        */
		virtual bool postOutputMessage(VMessage* message, bool releaseIfNotPosted=true, bool queueStandbyIfStartingUp=false);
        /**
        Returns a string containing the client's address in address:port form.
        @return obvious
        */
        virtual const VString& getClientAddress() const { return mClientAddress; }
        
    protected:
    
        /**
        This method is called in response to shutdown if both the input and
        output threads have ended; it waits until all attached tasks complete, and then
        calls "delete this". The caller must not refer to the session upon
        return from _selfDestruct(), because the session will be gone.
        */
        void _selfDestruct();
    
        virtual ~VClientSession(); // protected because only our _selfDestruct knows what how to delete us correctly
    
        VString                 mName;          ///< A name for the session to use in logging; built from supplied base name + IP address + port.
        VMutex                  mMutex;         ///< A mutex we use to enforce sequential processing of outbound messages, and to protect our task list.
        VServer*                mServer;        ///< The server that keeps track of this session.
        VString                 mClientType;    ///< A string distinguishing this type of session.
		VString                 mClientIP;      ///< The client's IP address (could be name or number).
		int                     mClientPort;	///< The IP port number of the client session.
        VString                 mClientAddress; ///< The user-visible string we use for logging, contains IP address + port of session.
        VMessageInputThread*    mInputThread;   ///< The thread that is reading inbound messages from the client.
        VMessageOutputThread*   mOutputThread;  ///< If using a separate output thread, this is it (may be NULL for sync i/o model).
        bool                    mIsShuttingDown;///< True if we are in the process of tearing down the session.
        
    private:

        void _moveStandbyMessagesToAsyncOutputQueue();  ///< Moves messages from mStartupStandbyQueue to the output queue.
        void _releaseQueuedClientMessages();            ///< Releases all pending queued message back to the pool (called during shutdown).

		VMessageQueue   mStartupStandbyQueue;	///< A queue we use to hold outbound updates while this client session is starting up.
        SessionTaskList mTasks;                 ///< Tasks currently pointing to this session; shutdown() waits until they're gone.
        
        // We only access the socket i/o stream if postOutputMessage() is called
        // and we are not set up to use a separate output message thread. 
		VSocketStream   mSocketStream;  ///< The underlying raw socket stream over which this thread communicates.
		VBinaryIOStream mIOStream;      ///< The binary-format i/o stream over the raw socket stream.
    };

typedef std::vector<VClientSession*> VClientSessionList;

/**
Implement a subclass of VClientSessionFactory (specifically, the createSession() method)
to create a socket listener that will create the desired type of VClientSession whenever
an incoming connection is accepted on that socket. This class is a parameter to the
VListenerThread constructor.
*/
class VClientSessionFactory
    {
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
        virtual VClientSession* createSession(VSocket* socket, VListenerThread* ownerThread) = 0;
        
        /**
        Adds the specified session to the server; the server keeps track of
        its sessions for purposes of broadcasting, clean shutdown, etc.
        @param  session the session that has been created
        */
        void addSessionToServer(VClientSession* session);

        /**
        Sets the management interface to receive notifications. May be NULL if
        no notifications are to be given. This manager may be passed to the
        sessions created and their threads.
        @param  manager the manager to notify, or NULL
        */
        void setManager(VManagementInterface* manager) { mManager = manager; }
        
    protected:
    
        VManagementInterface*   mManager;   ///< The object that will be notified of session events.
        VServer*                mServer;    ///< The server that will be notified of session creation.
    };

#endif /* vclientsession_h */
