/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vmutexlocker.h"

#include "vmutex.h"
#include "vthread.h"

// VMutexLocker ----------------------------------------------------------------

VMutexLocker::VMutexLocker(VMutex* mutex, const VString& name, bool lockInitially) :
mMutex(mutex),
mIsLocked(false),
mName(name)
    {
    if (lockInitially)
        this->lock();
    }

VMutexLocker::~VMutexLocker()
    {
    if (this->isLocked())
        {
        // Prevent all exceptions from escaping destructor.
        try
            {
            this->unlock();
            }
        catch (...) {}
        }
        
    mMutex = NULL;
    }

void VMutexLocker::lock()
    {
    if (mMutex != NULL)
        {
        mMutex->_lock(mName); // specific friend access to private API
        mIsLocked = true;
        }
    }

void VMutexLocker::unlock()
    {
    if ((mMutex != NULL) && this->isLocked())
        {
        mMutex->_unlock(); // specific friend access to private API
        mIsLocked = false;
        }
    }

void VMutexLocker::yield()
    {
    if (this->isLocked())
        {
        this->unlock(); // will allow other threads to acquire the mutex
        VThread::yield(); 
        this->lock(); // will block on the mutex if another thread now has it
        }
    else
        {
        VThread::yield();
        }
    }

// VMutexUnlocker --------------------------------------------------------------

VMutexUnlocker::VMutexUnlocker(VMutex* mutex, bool unlockInitially) :
VMutexLocker(mutex, "VMutexUnlocker", false)
    {
    // We reverse the presumed state and normal construction action vs. VMutexLocker.
    mIsLocked = true; // fool unlock() into letting us unlock it next
    if (unlockInitially)
        this->unlock();
    }

VMutexUnlocker::~VMutexUnlocker()
    {
    // We reverse the normal destruction action vs. VMutexLocker, and must then
    // fake out the mIsLocked state so that the base class destructor won't unlock.
    if (!this->isLocked())
        {
        // Prevent all exceptions from escaping destructor.
        try
            {
            this->lock();
            }
        catch (...) {}
        }
    
    mIsLocked = false; // fool the base class destructor so it doesn't unlock it
    }

