/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vmutex.h"

#include "vexception.h"
#include "vthread.h"

VMutex::VMutex() :
mLastLockThread((VThreadID_Type) -1)
    {
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
        mLastLockThread = VThread::threadSelf();
    else
        throw VException("VMutex::lock unable to lock mutex.");
    }

void VMutex::unlock()
    {
    if (! VMutex::mutexUnlock(&mMutex))
        throw VException("VMutex::unlock unable to unlock mutex.");
    }

VMutex_Type* VMutex::mutex()
    {
    return &mMutex;
    }


