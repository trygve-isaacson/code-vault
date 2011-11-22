/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

#ifndef vsemaphore_h
#define vsemaphore_h

/** @file */

#include "vthread.h"

class VMutex;

/**
    @ingroup vthread
*/

/**
VSemaphore implements a platform-independent semaphore that you can embed in an
object or place on the stack to guarantee its cleanup when the VSemaphore object
is destructed.

You can call the wait() and signal() methods to wait for a signal, and to
unblock a waiter, respectively. To wait on a semaphore, you must supply a
pointer to the VMutex that you have already acquired -- a semaphore is
implicitly linked with a mutex.
*/
class VSemaphore
    {
    public:
    
        /**
        Creates and initializes the semaphore.
        */
        VSemaphore();
        /**
        Destructs the semaphore.
        */
        virtual ~VSemaphore();
        
        /**
        Waits until the semaphore is signaled by another thread.
        @param    ownedMutex    the mutex that the caller has already
                            acquired the lock for
        @param    timeoutInterval    zero for no timeout; otherwise, the amount
                            of time after which to timeout
        */
        void wait(VMutex* ownedMutex, const VDuration& timeoutInterval);
        /**
        Signals the semaphore; if one or more other threads is waiting on
        the semaphore, exactly one of them will become unblocked by its
        wait() call returning.
        */
        void signal();
    
        /* PLATFORM-SPECIFIC STATIC FUNCTIONS --------------------------------
        The remaining functions defined here are the low-level interfaces to
        the platform-specific semaphore APIs. These are implemented in each
        platform-specific version of vthread_platform.cpp.
        */
        
        /**
        Initializes the platform semaphore value.
        Wrapper on Unix for pthread_cond_init.
        @param    semaphore    pointer to the platform semaphore
        @return true on success; false on failure
        */
        static bool semaphoreInit(VSemaphore_Type* semaphore);

        /**
        Destroys the platform semaphore value.
        Wrapper on Unix for pthread_cond_semaphore.
        @param    semaphore    pointer to the platform semaphore
        @return true on success; false on failure
        */
        static bool semaphoreDestroy(VSemaphore_Type* semaphore);
        
        /**
        Waits on the platform semaphore value.
        Wrapper on Unix for pthread_cond_wait.
        @param    semaphore    pointer to the platform semaphore
        @param    mutex        pointer to a locked platform mutex that the calling
                            thread had acquired; this function unlocks it while
                            waiting and locks it upon return
        @param    timeoutInterval    zero for no timeout; otherwise, the interval after
                            which to timeout
        @return true on success; false on failure; timeout is considered success
        */
        static bool semaphoreWait(VSemaphore_Type* semaphore, VMutex_Type* mutex, const VDuration& timeoutInterval);

        /**
        Signals the platform semaphore value.
        Wrapper on Unix for pthread_cond_signal.
        @param    semaphore    pointer to the platform semaphore
        @return true on success; false on failure
        */
        static bool semaphoreSignal(VSemaphore_Type* semaphore);

        /**
        Broadcasts a signal the platform semaphore value; all threads waiting
        on the semaphore will unblock.
        Wrapper on Unix for pthread_cond_broadcast.
        @param    semaphore    pointer to the platform semaphore
        @return true on success; false on failure
        */
        static bool semaphoreBroadcast(VSemaphore_Type* semaphore);

    private:
    
        VSemaphore_Type mSemaphore; ///< The OS semaphore handle.
    };

#endif /* vsemaphore_h */

