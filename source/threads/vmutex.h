/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

#ifndef vmutex_h
#define vmutex_h

/** @file */

#include "vthread.h"

/**
    @ingroup vthread
*/

/**
VMutex implements a platform-independent mutex that you can embed in an
object or place on the stack to guarantee its cleanup when the VMutex object
is destructed.

You can call the lock() and unlock() methods to acquire and release
the mutex lock. However, it is recommended that you use the
helper class VMutexLocker to do this, because you can use VMutexLocker
to guarantee the proper release of the mutex when the VMutexLocker
is destructed (typically by going out of scope) regardless of exceptions
being raised.

@see VMutexLocker
*/
class VMutex
    {
    public:
    
        /**
        Creates and initializes the mutex.
        */
        VMutex();
        /**
        Destructs the mutex.
        */
        virtual ~VMutex();
        
        /**
        Acquires the mutex lock; if the mutex is currently locked by another
        thread, this call blocks until the mutex lock can be acquired (if
        several threads are competing, the order in which they acquire the
        mutex is not known).
        */
        void lock();
        /**
        Releases the mutex lock; if one or more other threads is waiting on
        the mutex, one of them will unblock and acquire the mutex lock once
        this thread releases it.
        */
        void unlock();
        /**
        Returns true if this mutex is locked.
        @return    true if this mutex is locked
        */
        bool isLocked() const { return mIsLocked; }
        
        /**
        Returns a pointer to the raw OS mutex handle.
        @return    a pointer to the raw OS mutex handle
        */
        Mutex* mutex();
    
        /* PLATFORM-SPECIFIC STATIC FUNCTIONS --------------------------------
        The remaining functions defined here are the low-level interfaces to
        the platform-specific threading APIs. These are implemented in each
        platform-specific version of vthread_platform.cpp.
        */
        
        /**
        Initializes the platform mutex value.
        Wrapper on Unix for pthread_mutex_init.
        @param    mutex    pointer to the platform mutex
        @return true on success; false on failure
        */
        static bool mutexInit(Mutex* mutex);
        
        /**
        Destroys the platform mutex value.
        Wrapper on Unix for pthread_mutex_destroy.
        @param    mutex    pointer to the platform mutex
        */
        static void mutexDestroy(Mutex* mutex);
        
        /**
        Locks the platform mutex value.
        Wrapper on Unix for pthread_mutex_lock.
        @return true on success; false on failure
        */
        static bool mutexLock(Mutex* mutex);

        /**
        Unlocks the platform mutex value.
        Wrapper on Unix for pthread_mutex_unlock.
        @return true on success; false on failure
        */
        static bool mutexUnlock(Mutex* mutex);

    private:
    
        Mutex    mMutex;        ///< The OS mutex handle.
        bool    mIsLocked;    ///< True if this object has the mutex lock.
    };

#endif /* vmutex_h */

