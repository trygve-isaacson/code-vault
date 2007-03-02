/*
Copyright c1997-2007 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

#ifndef vmessageinputthread_h
#define vmessageinputthread_h

/** @file */

#include "vsocketthread.h"
#include "vsocketstream.h"
#include "vserver.h"
#include "vbinaryiostream.h"
#include "vmessage.h"
#include "vmessagehandler.h"

/**
    @ingroup vsocket
*/

/**
VMessageInputThread understands how to perform blocking input reads
of VMessage objects (finding and calling a VMessageHandler) from its
i/o stream. You can also write to its i/o stream, but if you are
doing asynchronous i/o you'll instead post messages to a VMessageOutputThread.
*/
class VMessageInputThread : public VSocketThread
	{
	public:
	
		/**
		Constructs the socket thread with the specified socket, owner thread,
		and message pool.
		
		@param	name		a name for the thread, useful for debugging purposes
		@param	socket		the socket this thread is managing
		@param	ownerThread	the thread that created this one
        @param  server      the server we're running for
		@param	messagePool	the pool from which new VMessage objects are created;
							the caller retains ownership of the pool (it is not
							deleted by this object upon its destruction)
		*/
		VMessageInputThread(const VString& inName, VSocket* inSocket, VListenerThread* ownerThread, VServer* server, VMessagePool* messagePool);
		/**
		Virtual destructor.
		*/
		virtual ~VMessageInputThread();
	
		/**
		Handles requests for the socket; returns only when the thread has
        been stopped, the socket is closed, or an exception is thrown that
        is not properly caught.
		*/
		virtual void run();

		/**
		Attaches the thread to its session, so that message handlers on this
		thread can reference session state.
        @param  session the session on whose behalf we are running
		*/
		void attachSession(VClientSession* session);

    protected:
    
		/**
		Pulls the next message from the socket (blocking until there is data),
		and then calls dispatchMessage() to handle the message.
		*/
		virtual void _processNextRequest();
		/**
		Handles the message by finding or creating a handler and calling it
        to process the message, returning when it's OK to read the next message.
		@param	message	the message to handle
		*/
		virtual void _dispatchMessage(VMessage* message);
        /**
        This method is intended for use by loopback testing, where the test code can
        see (and potentially preprocess) a message that it sent that is about to
        be handled in the normal fashion.
        */
        virtual void _beforeProcessMessage(VMessageHandler* /*handler*/, VMessage* /*message*/) {}
        /**
        This method is intended for use by loopback testing, where the test code can
        see (and potentially post-process) a message that it sent that has just been
        handled in the normal fashion.
        */
        virtual void _afterProcessMessage(VMessageHandler* /*handler*/) {}
	
		VMessagePool*   mMessagePool;	///< The message pool from which new message objects are created.
		VSocketStream   mSocketStream;	///< The underlying raw stream from which data is read.
		VBinaryIOStream mInputStream;	///< The formatted stream from which data is directly read.
		bool			mConnected;		///< True if the client has completed the connection sequence.
		VClientSession*	mSession;		///< The session object we are associated with.
		VServer*        mServer;		///< The server object that owns us.

	};

#endif /* vmessageinputthread_h */
