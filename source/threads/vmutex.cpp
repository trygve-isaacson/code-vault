/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vmutex.h"

#include "vexception.h"
#include "vthread.h"
#include "vlogger.h"

VDuration VMutex::gVMutexLockDelayLoggingThreshold(100 * VDuration::MILLISECOND());
int VMutex::gVMutexLockDelayLoggingLevel(VLogger::kDebug);

VMutex::VMutex(const VString& name, bool suppressLogging) :
mMutex(),
mName(name),
mSuppressLogging(suppressLogging),
mLastLockThread((VThreadID_Type) -1),
mLastLockerName(),
mLastLockTime(VInstant::NEVER_OCCURRED())
    {
    if (! VMutex::mutexInit(&mMutex))
        throw VException(VString("VMutex::VMutex unable to initialize mutex '%s'.", name.chars()));
    }

VMutex::~VMutex()
    {
    VMutex::mutexDestroy(&mMutex);
    }

void VMutex::setName(const VString& name)
    {
    mName = name;
    }

void VMutex::lock(const VString& lockerName)
    {
#ifdef VAULT_MUTEX_LOCK_DELAY_CHECK
    VInstant start;
#endif
    if (VMutex::mutexLock(&mMutex))
        {
        mLastLockTime.setNow();

#ifdef VAULT_MUTEX_LOCK_DELAY_CHECK
        if ((gVMutexLockDelayLoggingThreshold >= VDuration::ZERO()) && ! mSuppressLogging)
            {
            VInstant end;
            VDuration waitTime = end - start;

            if (waitTime >= gVMutexLockDelayLoggingThreshold)
                {
                VLOGGER_LEVEL(gVMutexLockDelayLoggingLevel, VString("Delay: '%s' was blocked %lldms on mutex '%s' released by '%s'.",
                    lockerName.chars(), waitTime.getDurationMilliseconds(), mName.chars(), mLastLockerName.chars()));
                }
            }
#endif

        mLastLockThread = VThread::threadSelf();
        mLastLockerName = lockerName;
        }
    else
        {
        if (mName.isEmpty())
            throw VException("VMutex::lock unable to lock mutex.");
        else
            throw VException(VString("VMutex::lock unable to lock mutex '%s'.", mName.chars()));
        }
    }

void VMutex::unlock()
    {
#ifdef VAULT_MUTEX_LOCK_DELAY_CHECK
    if ((gVMutexLockDelayLoggingThreshold >= VDuration::ZERO()) && ! mSuppressLogging)
        {
        VInstant now;
        VDuration delay = now - mLastLockTime;
        if (delay >= gVMutexLockDelayLoggingThreshold)
            {
            VLOGGER_LEVEL(gVMutexLockDelayLoggingLevel, VString("Delay: '%s' is unlocking mutex '%s' after holding it for %lldms.",
                mLastLockerName.chars(), mName.chars(), delay.getDurationMilliseconds()));
            }
        }
#endif

    if (! VMutex::mutexUnlock(&mMutex))
        {
        if (mName.isEmpty())
            throw VException("VMutex::unlock unable to unlock mutex.");
        else
            throw VException(VString("VMutex::unlock unable to unlock mutex '%s'.", mName.chars()));
        }
    }

VMutex_Type* VMutex::getMutex()
    {
    return &mMutex;
    }


