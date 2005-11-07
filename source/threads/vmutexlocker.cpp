/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vmutexlocker.h"

#include "vmutex.h"

VMutexLocker::VMutexLocker(VMutex* inMutex, bool lockInitially)
    {
    mIsLocked = false;
    mMutex = inMutex;

    if (lockInitially)
        this->lock();
    }

VMutexLocker::~VMutexLocker()
    {
    if (this->isLocked())
        this->unlock();
        
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

