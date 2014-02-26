/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vmutexlocker_h
#define vmutexlocker_h

/** @file */

#include "vtypes.h"

#include "vstring.h"

class VMutex;

/**
    @ingroup vthread
*/

// VMutexLocker ----------------------------------------------------------------

/**
VMutexLocker is a helper class that you can use to make working with
mutexes easier, and more importantly to guarantee proper release behavior
of a VMutex even when exceptions are raised.

VMutexLocker lets you avoid ugly code to properly manage the mutex, and
instead let C++ auto object destruction do the work for you.

Typically, you need to acquire and release a mutex lock in a function in order
to guarantee thread-safety. It is important that the mutex lock gets released
when you are done with it. If you throw an exception, this is ugly to
properly code, but VMutexLocker makes it trivial.

<code>
void doSomethingSafelyToAnObject(MyObject obj)<br>
    {<br>
    VMutexLocker    locker(obj.mMutex);<br>
    <br>
    obj.somethingDangerous(); // might throw an exception!<br>
    <br>
    if (obj.trouble())<br>
    &nbsp;&nbsp;&nbsp;&nbsp;throw VException("Oh no!");<br>
    }
</code>

In the example above, you are guaranteed that the MyObject's mMutex will be
properly unlocked no matter whether you or something you call throws an
exception. This is because the locker object is guaranteed to be properly
destructed when the function scope exits, and the object's destructor
releases the mutex lock.

You can call the lock() method separately if you need to construct the
VMutexLocker without locking right away, but lock it later.

You can call the unlock() method separately if you need to unlock the
mutex before the VMutexLocker is destructed. Another useful technique to cause
the lock to be released earlier than the end of a function is to create a
scope specifically to surround the VMutexLocker's existence.

@see    VMutex
*/
class VMutexLocker {
    public:

        /**
        Constructs the locker, and if specified, acquires the mutex lock. If
        the mutex is already locked by another thread, this call blocks until
        it obtains the lock.

        You can pass NULL to constructor if you don't want anything to
        happen; this can useful if, for example, you allow a NULL VMutex
        pointer to be passed to a routine that needs to lock it if supplied.

        @param    mutex            the VMutex to lock, or NULL if no action is wanted
        @param    lockInitially    true if the lock should be acquired on construction
        */
        VMutexLocker(VMutex* mutex, const VString& name, bool lockInitially = true);
        /**
        Destructor, unlocks the mutex if this object has acquired it.
        */
        virtual ~VMutexLocker();

        /**
        Acquires the mutex lock; if the mutex is currently locked by another
        thread, this call blocks until the mutex lock can be acquired (if
        several threads are competing, the order in which they acquire the
        mutex is not known).
        */
        virtual void lock();
        /**
        Releases the mutex lock; if one or more other threads is waiting on
        the mutex, one of them will unblock and acquire the mutex lock once
        this thread releases it.
        */
        virtual void unlock();
        /**
        Returns true if this object has acquired the lock.
        @return    true if this object has acquired the lock
        */
        bool isLocked() const { return mIsLocked; }
        /**
        Returns a pointer to the VMutex object.
        @return a pointer to the VMutex object (may be NULL)
        */
        VMutex* getMutex() { return mMutex; }
        /**
        Yields the current thread with the mutex unlocked; that is, this function
        unlocks the mutex, yields the current thread, and re-locks the mutex
        before returning. This can be used to make a thread that has a lock
        more "cooperative" with other threads that are competing for the same lock.
        */
        void yield();

    protected:

        VMutex* mMutex;     ///< Pointer to the VMutex object, or NULL.
        bool    mIsLocked;  ///< True if this object has acquired the lock.
        VString mName;      ///< The name of this locker, for diagnostic purposes.

    private:

        // Prevent copy construction and assignment since there is no provision for sharing a mutex.
        VMutexLocker(const VMutexLocker& other);
        VMutexLocker& operator=(const VMutexLocker& other);
};

// VMutexUnlocker --------------------------------------------------------------

/**
VMutexUnlocker is helper class that is the inverse of a VMutexLocker: it
unlocks a mutex upon construction, and locks it upon destruction. The unlocker
presumes that the mutex is locked to begin with.
*/
class VMutexUnlocker : public VMutexLocker {
    public:

        /**
        Constructs the unlocker, and if specified, releases the mutex lock.
        It is presumed by this class that the mutex is locked when this
        object is constructed.

        You can pass NULL to constructor if you don't want anything to
        happen; this can useful if, for example, you allow a NULL VMutex
        pointer to be passed to a routine that needs to unlock it if supplied.

        @param    mutex            the VMutex to unlock, or NULL if no action is wanted
        @param    unlockInitially    true if the lock should be released on construction
        */
        VMutexUnlocker(VMutex* mutex, bool unlockInitially = true);
        /**
        Destructor, re-locks the mutex if this object has released it.
        */
        virtual ~VMutexUnlocker();

    private:

        // Prevent copy construction and assignment since there is no provision for sharing a mutex.
        VMutexUnlocker(const VMutexUnlocker& other);
        VMutexUnlocker& operator=(const VMutexUnlocker& other);
};

#endif /* vmutexlocker_h */

