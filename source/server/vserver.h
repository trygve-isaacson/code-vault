/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

#ifndef vserver_h
#define vserver_h

/** @file */

#include "vmessage.h"
#include "vclientsession.h"

/**
    @ingroup vsocket
*/

class VSocket;
class VListenerThread;

/**
This abstract base class defines the interface that must be provided by a concrete
server class in order to facilitate interaction with the classes that manage
listeners, i/o threads, and messaging.
*/
class VServer
    {
    public:
    
        VServer();
        virtual ~VServer() {}
        
        /**
        Notifies the server that it should keep track of the specified
        session, for example it will need to post a message to it and
        all other appropriate sessions when postBroadcastMessage is called.
        @param  session the session that has been created
        */
        virtual void addClientSession(VClientSession* session) = 0;
        /**
        Notifies the server that it should no longer reference the specified
        session, presumably because it is about to be deleted.
        @param  session the session that is going away
        */
        virtual void removeClientSession(VClientSession* session) = 0;
        /**
        Posts a broadcast message to all specified client sessions' async output queues; the
        caller must not refer to the message after calling this function, because
        the message will be deleted or recycled after it has been sent.
        @param    message         the message to be posted
        @param    omitSession    if not NULL, specifies a session the message will NOT
                            be posted to
        */
        virtual void postBroadcastMessage(const VString& clientType, VMessage* message, const VClientSession* omitSession) = 0;
        /**
        Notifies the server that the client session is terminating, and should
        be garbage collected as soon as its reference count goes to zero (meaning
        that no i/o threads or message handlers are referring to it).
        @param  session the session to put on the GC list
        */
        void clientSessionTerminating(VClientSession* session);
        /**
        Deletes any terminated sessions that are no longer referenced. This should
        be called periodically from a background thread.
        */
        void garbageCollectTerminatedSessions();
        /**
        This method is intended for use in testing or diagnostic code; it returns true
        if there are any terminated sessions that have not yet been garbage collected.
        Of course, at any moment after return, the state can change.
        @return true if there are currently any uncollected terminated sessions
        */
        bool hasUncollectedTerminatedSessions() const;

    private:
    
        /**
        Deletes any of the terminated sessions that no longer have any references
        to them.
        */
        void _garbageCollectTerminatedSessions();
    
        VClientSessionList mTerminatedSessions; ///< Sessions to be garbage collected (deleted once no longer referenced).
        mutable VMutex mTerminatedSessionsMutex;///< Mutex to protect operations on mTerminatedSessions
    };

#endif /* vserver_h */
