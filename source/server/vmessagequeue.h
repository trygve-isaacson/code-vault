/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vmessagequeue_h
#define vmessagequeue_h

#include "vtypes.h"
#include "vmutex.h"
#include "vsemaphore.h"
#include "vcompactingdeque.h"
#include "vmessage.h"

/** @file */

/**
    @ingroup vsocket
*/

/**
VMessageQueue is a thread-safe FIFO queue of messages. Multiple threads may
post messages to the queue (push to the back of the queue) using postMessage()
and pull messages off the queue (pop from the front of the queue) using
blockUntilNextMessage() or getNextMessage(). As its name implies,
blockUntilNextMessage() blocks until a message is available, so it is useful
as a way for a message processing thread to spin, processing each message on
the queue, but blocking if there is nothing for it to do. By constrast,
getNextMessage() simply returns the next message, or NULL, so the caller must
decide how to manage de-queueing messages without chewing up the CPU
needlessly (for UI apps this may mean a notification scheme so that the app's
UI thread only looks at the queue when something gets posted to it).
*/
class VMessageQueue {
    public:

        /**
        Constructs the queue.
        */
        VMessageQueue();
        /**
        Virtual destructor.
        */
        virtual ~VMessageQueue();

        /**
        Posts a message to the back of the queue. May be safely called from
        any thread.
        @param    message    the message object to be posted; the queue becomes
                        owner of the object while it is in the queue
        */
        virtual void postMessage(VMessagePtr message);
        /**
        Returns the message at the front of the queue, blocking if the queue
        is empty. May be safely called from any thread.
        @return the message at the front of the queue; the caller becomes
                        owner of the object
        */
        VMessagePtr blockUntilNextMessage();
        /**
        Returns the message at the front of the queue, or NULL if the queue
        is empty.
        @return the message at the front of the queue, or NULL; the caller
                becomes owner of the object
        */
        VMessagePtr getNextMessage();
        /**
        Wakes up the thread in case it is necessary to let the thread cycle
        even though there are no messages and it is blocked. This is used
        during the shutdown process to allow the blocking thread to notice
        that it has been asked to terminate.
        */
        void wakeUp();
        /**
        Returns the number of messages currently in the queue.
        @return obvious
        */
        VSizeType getQueueSize() const;
        /**
        Returns the number of message bytes currently in the queue.
        @return obvious
        */
        Vs64 getQueueDataSize() const;
        /**
        Releases all messages in the queue.
        */
        void releaseAllMessages();

        /**
        The following methods set and get the configuration for emitting
        a log message if there is a lag between posting to a queue and the
        output thread getting a message. A threshold of 0 means the output
        thread will log on every message. You can set the log leve at which
        the output will be emitted.
        */
        static void setQueueingLagLoggingThreshold(const VDuration& threshold) { gVMessageQueueLagLoggingThreshold = threshold; }
        static VDuration getQueueingLagLoggingThreshold() { return gVMessageQueueLagLoggingThreshold; }
        static void setQueueingLagLoggingLevel(int logLevel) { gVMessageQueueLagLoggingLevel = logLevel; }
        static int getQueueingLagLoggingLevel() { return gVMessageQueueLagLoggingLevel; }

    private:

        VCompactingDeque<VMessagePtr> mQueuedMessages;///< The actual queue of messages.
        Vs64            mQueuedMessagesDataSize;    ///< The number of bytes in the queued messages.
        VMutex          mMessageQueueMutex;         ///< The mutex used to synchronize.
        VSemaphore      mMessageQueueSemaphore;     ///< The semaphore used to block/awaken.
        VInstant        mLastMessagePostTime;       ///< Time most recent message was posted.

        static VDuration gVMessageQueueLagLoggingThreshold; ///< If >=0, queuing lags are logged.
        static int gVMessageQueueLagLoggingLevel;           ///< Log level at which queuing lags are logged.
};

#endif /* vmessagequeue_h */
