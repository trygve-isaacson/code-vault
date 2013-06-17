/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
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
class VMessageInputThread : public VSocketThread {
    public:

        /**
        Constructs the socket thread with the specified socket, owner thread,
        and server.

        @param  name        a name for the thread, useful for debugging purposes
        @param  socket      the socket this thread is managing
        @param  ownerThread the thread that created this one
        @param  server      the server we're running for
        @param  messageFactory  a factory that instantiates messages suitable for this thread's input
                                (The caller owns the factory.)
        */
        VMessageInputThread(const VString& name, VSocket* socket, VListenerThread* ownerThread, VServer* server, const VMessageFactory* messageFactory);
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
        void attachSession(VClientSessionPtr session);

        /**
        Sets or clears the mHasOutputThread that controls whether this input thread must
        wait before returning from run(). This is used when separate in/out threads are
        handling i/o and the destruction sequence requires the input thread to wait for the
        output thread to die before dying itself.
        */
        void setHasOutputThread(bool hasOutputThread) { mHasOutputThread = hasOutputThread; }

    protected:

        /**
        Pulls the next message from the socket (blocking until there is data),
        and then calls dispatchMessage() to handle the message.
        */
        virtual void _processNextRequest();
        /**
        Handles the message by finding or creating a handler and calling it
        to process the message, returning when it's OK to read the next message.
        @param    message    the message to handle
        */
        virtual void _dispatchMessage(VMessagePtr message);
        /**
        This method is called by _dispatchMessage if it cannot find the handler
        for the message being handled. How to handle this is protocol-specific,
        but a subclass could send an error response back to the sender if the
        protocol allows that. The implementation must NOT release the message,
        and the message WILL be released by _dispatchMessage() upon return.
        */
        virtual void _handleNoMessageHandler(VMessagePtr /*message*/) {}
        /**
        This method is intended for use by loopback testing, where the test code can
        see (and potentially preprocess) a message that it sent that is about to
        be handled in the normal fashion.
        */
        virtual void _beforeProcessMessage(VMessageHandler* /*handler*/, VMessagePtr /*message*/) {}
        /**
        This method is where we actually call the message handler to process the
        message it was constructed with. A subclass might override this to wrap
        the call to super in a try/catch block if it wants to take action other
        than logging in response to an exception.
        */
        virtual void _callProcessMessage(VMessageHandler* handler);
        /**
        This method is intended for use by loopback testing, where the test code can
        see (and potentially post-process) a message that it sent that has just been
        handled in the normal fashion.
        */
        virtual void _afterProcessMessage(VMessageHandler* /*handler*/) {}

        VSocketStream           mSocketStream;      ///< The underlying raw stream from which data is read.
        VBinaryIOStream         mInputStream;       ///< The formatted stream from which data is directly read.
        bool                    mConnected;         ///< True if the client has completed the connection sequence.
        VClientSessionPtr       mSession;           ///< The session object we are associated with.
        VServer*                mServer;            ///< The server object that owns us.
        const VMessageFactory*  mMessageFactory;    ///< Factory for instantiating new messages to read from input stream.
        volatile bool           mHasOutputThread;   ///< True if we are dependent on an output thread completion before returning from run(). (see run() code)

    private:

        VMessageInputThread(const VMessageInputThread&); // not copyable
        VMessageInputThread& operator=(const VMessageInputThread&); // not assignable
};

/**
VBentoMessageInputThread is a VMessageInputThread that can automatically
handle no-such-handler or uncaught message dispatch exceptions, and in
response send a Bento-based error reply back to the sender.
*/
class VBentoMessageInputThread : public VMessageInputThread {
    public:

        VBentoMessageInputThread(const VString& name, VSocket* socket, VListenerThread* ownerThread, VServer* server, const VMessageFactory* messageFactory);
        ~VBentoMessageInputThread() {}

    protected:

        virtual void _handleNoMessageHandler(VMessagePtr message);
        virtual void _callProcessMessage(VMessageHandler* handler);

};

#endif /* vmessageinputthread_h */
