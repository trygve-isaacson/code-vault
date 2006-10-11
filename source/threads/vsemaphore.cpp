/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vsemaphore.h"

#include "vexception.h"
#include "vmutex.h"

VSemaphore::VSemaphore()
    {
    if (! VSemaphore::semaphoreInit(&mSemaphore))
        throw VException("VSemaphore::VSemaphore unabled to initialize semaphore.");
    }

VSemaphore::~VSemaphore()
    {
    (void) VSemaphore::semaphoreDestroy(&mSemaphore);
    }

void VSemaphore::wait(VMutex* ownedMutex, const VDuration& timeoutInterval)
    {
    if (! VSemaphore::semaphoreWait(&mSemaphore, ownedMutex->mutex(), timeoutInterval))
        throw VException("VSemaphore::lock unable to wait on semaphore.");
    }

void VSemaphore::signal()
    {
    if (! VSemaphore::semaphoreSignal(&mSemaphore))
        throw VException("VSemaphore::unlock unable to signal semaphore.");
    }

