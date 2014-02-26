/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vmutex.h"

#include "vexception.h"
#include "vthread.h"
#include "vlogger.h"

VDuration VMutex::gVMutexLockDelayLoggingThreshold(100 * VDuration::MILLISECOND());
int VMutex::gVMutexLockDelayLoggingLevel(VLoggerLevel::DEBUG);

VMutex::VMutex(const VString& name, bool suppressLogging)
    : mMutex()
    , mName(name)
    , mSuppressLogging(suppressLogging)
    , mLastLockThread((VThreadID_Type) - 1)
    , mLastLockerName()
    , mLastLockTime(VInstant::NEVER_OCCURRED())
    , mIsLocked(false)
    {

    if (! VMutex::mutexInit(&mMutex))
        throw VStackTraceException(VSTRING_FORMAT("VMutex::VMutex unable to initialize mutex '%s'.", name.chars()));
}

VMutex::~VMutex() {
    VMutex::mutexDestroy(&mMutex);
}

void VMutex::setName(const VString& name) {
    mName = name;
}

VMutex_Type* VMutex::getMutex() {
    return &mMutex;
}

bool VMutex::isLockedByCurrentThread() const {
    return mIsLocked && (mLastLockThread == VThread::threadSelf());
}

void VMutex::_lock(const VString& lockerName) {
#ifdef VAULT_MUTEX_LOCK_DELAY_CHECK
    VInstant start;
#endif
    if (VMutex::mutexLock(&mMutex)) {
        mLastLockTime.setNow();

#ifdef VAULT_MUTEX_LOCK_DELAY_CHECK
        if ((gVMutexLockDelayLoggingThreshold >= VDuration::ZERO()) && ! mSuppressLogging) {
            VInstant end;
            VDuration waitTime = end - start;

            if (waitTime >= gVMutexLockDelayLoggingThreshold) {
                VLOGGER_LEVEL(gVMutexLockDelayLoggingLevel, VSTRING_FORMAT("Delay: '%s' was blocked " VSTRING_FORMATTER_S64 "ms on mutex '%s' released by '%s'.",
                                                                           lockerName.chars(), waitTime.getDurationMilliseconds(), mName.chars(), mLastLockerName.chars()));
            }
        }
#endif

        // Note: These properties are only valid with the understanding that they are not set atomically during lock/unlock.
        // The only guarantee is that they end up set to their new values after the lock is acquired above, and before we return.
        // They may only be used for mutex diagnostics (e.g. isLockedByCurrentThread() and lock delay reporting), not for concurrency control.
        mLastLockThread = VThread::threadSelf();
        mLastLockerName = lockerName;
        mIsLocked = true;
    } else {
        if (mName.isEmpty()) {
            throw VStackTraceException("VMutex::lock unable to lock mutex.");
        } else {
            throw VStackTraceException(VSTRING_FORMAT("VMutex::lock unable to lock mutex '%s'.", mName.chars()));
        }
    }
}

void VMutex::_unlock() {
#ifdef VAULT_MUTEX_LOCK_DELAY_CHECK
    if ((gVMutexLockDelayLoggingThreshold >= VDuration::ZERO()) && ! mSuppressLogging) {
        VInstant now;
        VDuration delay = now - mLastLockTime;
        if (delay >= gVMutexLockDelayLoggingThreshold) {
            VLOGGER_LEVEL(gVMutexLockDelayLoggingLevel, VSTRING_FORMAT("Delay: '%s' is unlocking mutex '%s' after holding it for " VSTRING_FORMATTER_S64 "ms.",
                                                                       mLastLockerName.chars(), mName.chars(), delay.getDurationMilliseconds()));
        }
    }
#endif

    mIsLocked = false; // Note: Must set false *before* unlocking, otherwise we may set it false after another thread jumps in, locks, sets it true, confusing isLockedByCurrentThread(). Part of non-atomicity warning above.
    if (! VMutex::mutexUnlock(&mMutex)) {
        mIsLocked = true; // Restore value since we failed to unlock.
        if (mName.isEmpty()) {
            throw VStackTraceException("VMutex::unlock unable to unlock mutex.");
        } else {
            throw VStackTraceException(VSTRING_FORMAT("VMutex::unlock unable to unlock mutex '%s'.", mName.chars()));
        }
    }
}

