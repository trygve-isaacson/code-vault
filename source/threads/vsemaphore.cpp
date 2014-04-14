/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

/** @file */

#include "vsemaphore.h"

#include "vexception.h"
#include "vmutex.h"

VSemaphore::VSemaphore()
    : mSemaphore()
    {

    if (! VSemaphore::semaphoreInit(&mSemaphore)) {
        throw VStackTraceException("VSemaphore::VSemaphore unable to initialize semaphore.");
    }
}

VSemaphore::~VSemaphore() {
    (void) VSemaphore::semaphoreDestroy(&mSemaphore);
}

void VSemaphore::wait(VMutex* ownedMutex, const VDuration& timeoutInterval) {
    if (! VSemaphore::semaphoreWait(&mSemaphore, ownedMutex->getMutex(), timeoutInterval)) {
        throw VStackTraceException("VSemaphore::wait unable to wait on semaphore.");
    }
}

void VSemaphore::signal() {
    if (! VSemaphore::semaphoreSignal(&mSemaphore)) {
        throw VStackTraceException("VSemaphore::signal unable to signal semaphore.");
    }
}

