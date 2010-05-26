/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

#ifndef vmutex_h
#define vmutex_h

/** @file */

#include "vtypes.h"

#include "vthread_platform.h"
#include "vstring.h"
#include "vinstant.h"

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
        Creates and initializes the mutex with an optional name that can be
        used when debugging mutex and lock behavior.
        @param a name for the mutex; should be unique to avoid confusion
        @param if this mutex is specifically locked during logging, this flag
                must be set so that VMutex doesn't try to log information
                about this mutex (avoids recursive locking deadlock)
        */
        VMutex(const VString& name=VString::EMPTY(), bool suppressLogging=false);
        /**
        Destructs the mutex.
        */
        virtual ~VMutex();
        
        /**
        In some cases it's more convenient to name a mutex after constructing,
        in which case you can call setName(). The name is only used for
        diagnostic purposes when debugging mutex and lock behavior.
        @param a name for the mutex; should be unique to avoid confusion
        */
        void setName(const VString& name);

        /**
        Acquires the mutex lock; if the mutex is currently locked by another
        thread, this call blocks until the mutex lock can be acquired (if
        several threads are competing, the order in which they acquire the
        mutex is not known). You can supply a name to identify who is attempting
        to lock, for diagnostic purposes (the name is stored).
        @param lockerName the name of the caller, for diagnostic purposes
        */
        void lock(const VString& lockerName=VString::EMPTY());
        /**
        Releases the mutex lock; if one or more other threads is waiting on
        the mutex, one of them will unblock and acquire the mutex lock once
        this thread releases it.
        */
        void unlock();

        /**
        Returns a pointer to the raw OS mutex handle.
        @return    a pointer to the raw OS mutex handle
        */
        VMutex_Type* getMutex();

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
        static bool mutexInit(VMutex_Type* mutex);

        /**
        Destroys the platform mutex value.
        Wrapper on Unix for pthread_mutex_destroy.
        @param    mutex    pointer to the platform mutex
        */
        static void mutexDestroy(VMutex_Type* mutex);

        /**
        Locks the platform mutex value.
        Wrapper on Unix for pthread_mutex_lock.
        @return true on success; false on failure
        */
        static bool mutexLock(VMutex_Type* mutex);

        /**
        Unlocks the platform mutex value.
        Wrapper on Unix for pthread_mutex_unlock.
        @return true on success; false on failure
        */
        static bool mutexUnlock(VMutex_Type* mutex);

        /**
        The following methods set and get the configuration for emitting
        a log message if there is a lengthy holding of, or delay in acquiring
        a lock. First, you must compile with VAULT_MUTEX_LOCK_DELAY_CHECK
        defined (presumably in vconfigure.h) to have the delay checking code in place.
        The default delay threshold is 50ms. If you specify 0, every lock will log a message.
        You can set the log leve at which the output will be emitted.
        */
        static void setLockDelayLoggingThreshold(const VDuration& threshold) { gVMutexLockDelayLoggingThreshold = threshold; }
        static VDuration getLockDelayLoggingThreshold() { return gVMutexLockDelayLoggingThreshold; }
        static void setLockDelayLoggingLevel(int logLevel) { gVMutexLockDelayLoggingLevel = logLevel; }
        static int getLockDelayLoggingLevel() { return gVMutexLockDelayLoggingLevel; }
    
    private:
    
        VMutex(const VMutex&); // not copyable
        VMutex& operator=(const VMutex&); // not assignable

        VMutex_Type mMutex;             ///< The OS mutex handle.
        VString mName;                  ///< The name of this mutex for diagnostic purposes.
        bool mSuppressLogging;          ///< True if this VMutex must not call logger functions.
        VThreadID_Type mLastLockThread; ///< If locked, the thread that acquired the lock.
        VString mLastLockerName;        ///< The name of the last (or current) caller of lock().
        VInstant mLastLockTime;         ///< When the last acquisition of the lock occurred.
        
        static VDuration gVMutexLockDelayLoggingThreshold;  ///< If >=0, lock delays are logged.
        static int gVMutexLockDelayLoggingLevel;            ///< Log level at which lock delays are logged.
    };

#endif /* vmutex_h */

