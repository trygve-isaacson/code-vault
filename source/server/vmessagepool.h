/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

#ifndef vmessagepool_h
#define vmessagepool_h

#include "vtypes.h"
#include "vmutex.h"
#include "vmessage.h"

/** @file */

/**
    @ingroup vsocket
*/

class VMessageFactory;

typedef std::deque<VMessage*> PooledMessageList;    ///< A deque of VMessage object pointers.

/**
VMessagePool is a thread-safe pool of messages, allowing VMessage objects to
be re-used as much as possible and thus reduce creating and destroying new
ones and incurring that overhead. Multiple threads may request for VMessage
objects from the pool using get(), and put them back into the pool using
release(). If the pool is empty, get() will instantiate a new object. If the
pool is full, release() will delete the object.
All objects in one pool must be of the same concrete class, unless you
don't care about what class an object is when you get it from the pool.
*/
class VMessagePool
    {
    public:
    
        /**
        Helper function to properly release a message back to a pool, but
        handling cases where there is no message (message == NULL, do nothing)
        or there is no pool (pool == NULL, delete the message).
        @param    message    the message to be released or deleted; NULL is allowed
        @param    pool    the pool to release the message to; NULL is allowed
        */
        static void releaseMessage(VMessage* message, VMessagePool* pool);
    
        static const int kDefaultPoolSize = 64;        ///< The default limit on the number of objects in the pool.
        static const int kUnlimitedPoolSize = -1;    ///< Indicates that the pool does not limit the number of objects.

        /**
        Constructs the pool.
        @param    factory        A message factory object. The pool does not take ownership
                            of the factory object, and so does not delete it when the
                            pool is deleted. (You could use the same factory for several
                            pools.)
        @param    maxInPool    The limit on the number of objects in the pool.
        */
        VMessagePool(VMessageFactory* factory, int maxInPool=kDefaultPoolSize);
        /**
        Virtual destructor.
        */
        virtual ~VMessagePool();
        
        /**
        Returns the currently configured max number of objects in the pool.
        A value of -1 means no limit (every released object will be put into the
        pool rather than deleted).
        @return The limit on the number of objects in the pool.
        */
        int getMaxInPool() const { return mMaxInPool; }
        /**
        Sets the limit on the number of objects in the pool. If the pool currently
        exceeds this size, objects are deleted to conform to the new limit. Whenever
        an object is released to the pool, if the pool is already "full" per this
        limit, the object is deleted rather than placed into the pool.
        @param    maxInPool    The limit on the number of objects in the pool; a
                                value of -1 means no limit.
        */
        void setMaxInPool(int maxInPool);
        
        /**
        Returns a message, which is either recycled from the pool, or newly
        instantiated if the pool is empty. Normally you will supply the ID
        if you are preparing a message to be sent (you know the ID), and you
        will omit the ID if you are preparing a message to receive (it will
        read the ID from the input stream).
        @param    messageID        value with which to init the message's message ID
        @return a message object
        */
        VMessage* get(VMessageID messageID=0);
        /**
        Releases a message back to the pool. If the pool is full, the object
        will be deleted.
        @param    message    the message object to be released
        */
        void release(VMessage* message);
        
        /**
        Prints the statistics to the log for diagnostic purposes.
        */
        void printStats();
    
    private:

        VMessagePool(const VMessagePool&); // not copyable
        VMessagePool& operator=(const VMessagePool&); // not assignable
    
        VMessageFactory*    mFactory;           ///< The factory object we use to create new messages.
        int                 mMaxInPool;         ///< The max number of messages kept in the pool.
        PooledMessageList   mPooledMessages;    ///< The actual pool of messages.
        VMutex              mMessagePoolMutex;  ///< The mutex used to synchronize.
        
        // Metrics tracked for possible performance tuning of pool size.
        int mHighWaterMarkIn;       ///< Max number ever kept in this pool at one time.
        int mHighWaterMarkOut;      ///< Max number ever taken out of this pool at one time.
        int mCurrentOut;            ///< Number of msgs currently gotten but not released.
        int mNumMessagesCreated;    ///< Incremented when get() requires new msg.
        int mNumMessagesDestroyed;  ///< Incremented when release() requires freeing msg.
        int mNumMessagesReused;     ///< Incremented when get() returns existing msg.
        
        friend class VMessageUnit;    ///< Allow unit test class to peek at our private instance variables.
    };

#endif /* vmessagepool_h */
