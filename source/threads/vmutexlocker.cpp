/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vmutexlocker.h"

#include "vmutex.h"
#include "vthread.h"

VMutexLocker::VMutexLocker(VMutex* inMutex, bool lockInitially) :
mIsLocked(false),
mMutex(inMutex)
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
    this->unlock();
    VThread::yield();
    this->lock();
    }

