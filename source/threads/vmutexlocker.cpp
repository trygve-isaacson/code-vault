/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vmutexlocker.h"

#include "vmutex.h"
#include "vthread.h"

// VMutexLocker ----------------------------------------------------------------

VMutexLocker::VMutexLocker(VMutex* inMutex, bool lockInitially) :
mMutex(inMutex),
mIsLocked(false)
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
        mMutex->lock();
        mIsLocked = true;
        }
    }

void VMutexLocker::unlock()
    {
    if ((mMutex != NULL) && this->isLocked())
        {
        mMutex->unlock();
        mIsLocked = false;
        }
    }

void VMutexLocker::yield()
    {
    bool wasLocked = this->isLocked();
    
    if (wasLocked)
        this->unlock();

    VThread::yield();

    if (wasLocked)
        this->lock();
    }

// VMutexUnlocker --------------------------------------------------------------

VMutexUnlocker::VMutexUnlocker(VMutex* inMutex, bool unlockInitially) :
VMutexLocker(inMutex, false)
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

