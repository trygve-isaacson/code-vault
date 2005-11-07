/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
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
        @param    timeoutMilliseconds    zero for no timeout; otherwise, the number of
                            milliseconds after which to timeout
        */
        void wait(VMutex* ownedMutex, Vs64 timeoutMilliseconds=0);
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
        static bool semaphoreInit(Semaphore* semaphore);

        /**
        Destroys the platform semaphore value.
        Wrapper on Unix for pthread_cond_semaphore.
        @param    semaphore    pointer to the platform semaphore
        @return true on success; false on failure
        */
        static bool semaphoreDestroy(Semaphore* semaphore);
        
        /**
        Waits on the platform semaphore value.
        Wrapper on Unix for pthread_cond_wait.
        @param    semaphore    pointer to the platform semaphore
        @param    mutex        pointer to a locked platform mutex that the calling
                            thread had acquired; this function unlocks it while
                            waiting and locks it upon return
        @param    timeoutMilliseconds    zero for no timeout; otherwise, the number of
                            milliseconds after which to timeout
        @return true on success; false on failure; timeout is considered success
        */
        static bool semaphoreWait(Semaphore* semaphore, Mutex* mutex, Vs64 timeoutMilliseconds);

        /**
        Signals the platform semaphore value.
        Wrapper on Unix for pthread_cond_signal.
        @param    semaphore    pointer to the platform semaphore
        @return true on success; false on failure
        */
        static bool semaphoreSignal(Semaphore* semaphore);

        /**
        Broadcasts a signal the platform semaphore value; all threads waiting
        on the semaphore will unblock.
        Wrapper on Unix for pthread_cond_broadcast.
        @param    semaphore    pointer to the platform semaphore
        @return true on success; false on failure
        */
        static bool semaphoreBroadcast(Semaphore* semaphore);

    protected:
    
        Semaphore    mSemaphore;        ///< The OS semaphore handle.
    };

#endif /* vsemaphore_h */

