/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vmessageoutputthread_h
#define vmessageoutputthread_h

/** @file */

#include "vsocketthread.h"
#include "vsocketstream.h"
#include "vbinaryiostream.h"
#include "vmessage.h"
#include "vmessagequeue.h"
#include "vclientsession.h"

class VServer;

/**
    @ingroup vsocket
*/

/**
VMessageOutputThread understands how to maintain and monitor a message
output queue, waking up when a new message has been posted to the queue,
and writing it to the output stream.
*/
class VMessageOutputThread : public VSocketThread {
    public:

        /**
        Constructs the output thread. The supplied message queue,
        server, and session are still owned by the caller; this class
        does not delete them in its destructor.
        @param  threadBaseName  a distinguishing base name for the thread, useful for debugging purposes;
                                the thread name will be composed of this and the socket's IP address and port
        @param    socket        the socket this thread is managing
        @param    ownerThread   the thread that created this one
        @param    server        the server object
        @param    session       the session object
        @param    dependentInputThread   if non-null, the VMessageInputThread that is dependent
                                upon this output thread, and which we must notify before we
                                return from our run() method
        @param maxQueueSize if non-zero, the max number of queued messages allowed; if a call
                            to postOutputMessage() occurs when the limit has been exceeded,
                            the call will just close the socket and return
        @param maxQueueDataSize if non-zero, the max data size of queued messages allowed; if a call
                            to postOutputMessage() occurs when the limit has been exceeded,
                            the call will just close the socket and return
        @param maxQueueGracePeriod how long the maxQueueSize and maxQueueDataSize limits may be exceeded
                            before the socket is closed upon next posted message
        */
        VMessageOutputThread(const VString& threadBaseName, VSocket* socket, VListenerThread* ownerThread, VServer* server, VClientSessionPtr session, VMessageInputThread* dependentInputThread, int maxQueueSize = 0, Vs64 maxQueueDataSize = 0, const VDuration& maxQueueGracePeriod = VDuration::ZERO());
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
        void attachSession(VClientSessionPtr session);

        /**
        Posts a message to the output thread's output queue; the output thread
        will send the message in order of posting. If the output thread is
        blocked when the message is posted, the posting causes the output
        thread to wake up. If the mMaxQueueSize or mMaxQueueDataSize has already
        been exceeded, this method causes the socket to be closed and does not
        post the message.
        @param  message the message to post (and send)
        @param  respectQueueLimits normally true, can be set false to bypass the
                checks on the queue limits
        @return true if the message was successfully posted; false means it was not, so
                caller needs to free the message
        */
        bool postOutputMessage(VMessagePtr message, bool respectQueueLimits = true);

        /**
        Releases/destroys all queued messages. This is called when
        the session shuts down. That is, any messages sitting on the output
        queue at the time the session shuts down are not sent.
        */
        void releaseAllQueuedMessages();

        /**
        Returns the number of messages that are sitting on the output queue
        that have yet to be sent.
        */
        int getOutputQueueSize() const;
        /**
        Returns true if the output queue has exceeded its limits, and returns in
        the input parameters the current queue size information.
        @param currentQueueSize regardless of result, the current queue size is returned here
        @param currentQueueDataSize regardless of result, the current queue data size is returned here
        @return true if the queue is currently over the queue size limits
        */
        bool isOutputQueueOverLimit(int& currentQueueSize, Vs64& currentQueueDataSize) const;

    private:

        VMessageOutputThread(const VMessageOutputThread&); // not copyable
        VMessageOutputThread& operator=(const VMessageOutputThread&); // not assignable

        /**
        Processes the next queued message, blocking if there is nothing queued.
        */
        void _processNextOutboundMessage();

        VMessageQueue           mOutputQueue;       ///< The output queue that this thread pulls messages from.
        VSocketStream           mSocketStream;      ///< The underlying raw stream the message data is written to.
        VBinaryIOStream         mOutputStream;      ///< The formatted stream the message data is written to.
        VServer*                mServer;            ///< The server object.
        VClientSessionPtr       mSession;           ///< The session object.
        VMessageInputThread*    mDependentInputThread;///< If non-null, the input thread we must notify before returning from our run().
        int                     mMaxQueueSize;      ///< If non-zero, if a message is posted when there are already this many messages queued, we close the socket.
        Vs64                    mMaxQueueDataSize;  ///< If non-zero, if a message is posted when there are already this many bytes queued, we close the socket.
        VDuration               mMaxQueueGracePeriod;///< How long we will allow the queue limits to be exceeded before we close the socket.
        VInstant                mWhenMaxQueueSizeWarned;///< Time we last warned about exceeding the queue size; this avoids flood of warnings if condition persists.

        // These are the transient flags we use to enforce and monitor the queue limits.
        bool        mWasOverLimit;      ///< True if the last postOutputMessage() call left us over the limit.
        VInstant    mWhenWentOverLimit; ///< When did we last transition from under-limit to over-limit.
};

#endif /* vmessageoutputthread_h */
