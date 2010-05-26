/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vsemaphore.h"

#include "vexception.h"
#include "vmutex.h"

VSemaphore::VSemaphore() :
mSemaphore()
    {
    if (! VSemaphore::semaphoreInit(&mSemaphore))
        throw VException("VSemaphore::VSemaphore unable to initialize semaphore.");
    }

VSemaphore::~VSemaphore()
    {
    (void) VSemaphore::semaphoreDestroy(&mSemaphore);
    }

void VSemaphore::wait(VMutex* ownedMutex, const VDuration& timeoutInterval)
    {
    if (! VSemaphore::semaphoreWait(&mSemaphore, ownedMutex->getMutex(), timeoutInterval))
        throw VException("VSemaphore::wait unable to wait on semaphore.");
    }

void VSemaphore::signal()
    {
    if (! VSemaphore::semaphoreSignal(&mSemaphore))
        throw VException("VSemaphore::signal unable to signal semaphore.");
    }

