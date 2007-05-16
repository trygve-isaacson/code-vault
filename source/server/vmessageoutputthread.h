/*
Copyright c1997-2007 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

#ifndef vmessageoutputthread_h
#define vmessageoutputthread_h

/** @file */

#include "vsocketthread.h"
#include "vsocketstream.h"
#include "vserver.h"
#include "vbinaryiostream.h"
#include "vmessage.h"
#include "vmessagequeue.h"

/**
    @ingroup vsocket
*/

/**
VMessageOutputThread understands how to maintain and monitor a message
output queue, waking up when a new message has been posted to the queue,
and writing it to the output stream.
*/
class VMessageOutputThread : public VSocketThread
	{
	public:
	
		/**
		Constructs the output thread. The supplied message pool and message
		queue, server, and session are still owned by the caller; this class
		does not delete them in its destructor.
		@param	name		a name for the thread, useful for debugging purposes
		@param	socket		the socket this thread is managing
		@param	ownerThread	the thread that created this one
		@param	messagePool	the pool from which to get and recycle messages
		@param	outputQueue	the message queue where we pull outbound messages from
		@param	server		the server object
		@param	session		the session object
		@param	messagePool	the pool from which new VMessage objects are created;
							the caller retains ownership of the pool (it is not
							deleted by this object upon its destruction)
		*/
		VMessageOutputThread(const VString& name, VSocket* socket, VListenerThread* ownerThread, VServer* server, VClientSession* session, VMessagePool* messagePool);
		/**
		Virtual destructor.
		*/
		virtual ~VMessageOutputThread();
	
		/**
		Handles requests and responses for the socket.
		*/
		virtual void run();

        /**
        Stops the thread; for VMessageOutputThread this calls inherited and
        then wakes up the message queue in case it is blocked.
        */
        virtual void stop();

		/**
		Attaches the thread to its session, so that message handlers on this
		thread can reference session state.
        @param  session the session on whose behalf we are running
		*/
		void attachSession(VClientSession* session);

        /**
        Posts a message to the output thread's output queue; the output thread
        will send the message in order of posting. If the output thread is
        blocked when the message is posted, the posting causes the output
        thread to wake up.
        @param  message the message to post (and send)
        */
		void postOutputMessage(VMessage* message);
        
        /**
        Releases all queued message back to the pool. This is called when
        the session shuts down. That is, any messages sitting on the output
        queue at the time the session shuts down are not sent.
        */
        void releaseAllQueuedMessages();
        
        /**
        Returns the number of messages that are sitting on the output queue
        that have yet to be sent.
        */
        int getOutputQueueSize() const;

	private:
	
        /**
        Processes the next queued message, blocking if there is nothing queued.
        */
		void _processNextOutboundMessage();
		
		/**
        Returns true if the message should actually be sent; typically, the
        only case where this is false is if the session is going offline (in
        the process of shutting down) at the time of the call.
		@param	message	the message to look at
		@return true if the message should be sent; false if not; in either case
				the message will ultimately be released back into the pool
		*/
		bool _shouldSendOutboundMessage(VMessage* message);
	
		VMessagePool*   mMessagePool;	///< The message pool where messages are released after they are sent.
		VMessageQueue   mOutputQueue;	///< The output queue that this thread pulls messages from.
		VSocketStream   mSocketStream;	///< The underlying raw stream the message data is written to.
		VBinaryIOStream mOutputStream;	///< The formatted stream the message data is written to.
		VServer*		mServer;        ///< The server object.
		VClientSession*	mSession;       ///< The session object.
	};

#endif /* vmessageoutputthread_h */
