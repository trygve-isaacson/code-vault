/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vmutex.h"

#include "vexception.h"

VMutex::VMutex()
    {
    mIsLocked = false;
    if (! VMutex::mutexInit(&mMutex))
        throw VException("VMutex::VMutex unable to initialize mutex.");
    }

VMutex::~VMutex()
    {
    VMutex::mutexDestroy(&mMutex);
    }

void VMutex::lock()
    {
    if (VMutex::mutexLock(&mMutex))
        mIsLocked = true;
    else
        throw VException("VMutex::lock unable to lock mutex.");
    }

void VMutex::unlock()
    {
    if (VMutex::mutexUnlock(&mMutex))
        mIsLocked = false;
    else
        throw VException("VMutex::unlock unable to unlock mutex.");
    }

Mutex* VMutex::mutex()
    {
    return &mMutex;
    }


