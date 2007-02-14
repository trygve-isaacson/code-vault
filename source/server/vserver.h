/*
Copyright c1997-2007 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

#ifndef vserver_h
#define vserver_h

/** @file */

#include "vmessage.h"

/**
    @ingroup vsocket
*/

class VClientSession;
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
    
        VServer() {}
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
		@param	message	 	the message to be posted
		@param	omitSession	if not NULL, specifies a session the message will NOT
							be posted to
		*/
		virtual void postBroadcastMessage(const VString& clientType, VMessage* message, VClientSession* omitSession) = 0;
    };

#endif /* vserver_h */
