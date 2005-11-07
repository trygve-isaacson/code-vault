/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vthread.h"
#include "vmutex.h"
#include "vsemaphore.h"

#include <sys/time.h>
#include <sys/resource.h>

// VThread platform-specific functions ---------------------------------------

// static
bool VThread::threadCreate(ThreadID* threadID, threadMainFunction threadMainProcPtr, void* threadArgument)
    {
    return (::pthread_create(threadID, NULL, threadMainProcPtr, threadArgument) == 0);
    }

// static
void VThread::threadExit()
    {
    ::pthread_exit(NULL);
    }
    
// static
bool VThread::threadJoin(ThreadID threadID, void** value)
    {
    return (::pthread_join(threadID, value) == 0);
    }

// static
void VThread::threadDetach(ThreadID threadID)
    { 
    ::pthread_detach(threadID);
    }
    
// static
ThreadID VThread::threadSelf() 
    {
    return ::pthread_self();
    }

// static
bool VThread::setPriority(int nice)
    {
    return (::setpriority(PRIO_PROCESS, 0, nice) == 0);
    }

// static
void VThread::sleepMilliseconds(int milliseconds)
    {
    struct timeval timeout;
    timeout.tv_sec = milliseconds / 1000;
    timeout.tv_usec = (milliseconds % 1000) * 1000;

    (void) ::select(1, NULL, NULL, NULL, &timeout); // 1 means file descriptor [0], will just timeout
    }

// static
void VThread::yield()
    {
#ifdef sun
    // On Solaris there is no yield function.
    // Simulate by sleeping for 1ms. How to improve?
    VThread::sleepMilliseconds(1);
#else
    ::sched_yield(); 
#endif
    }

// VMutex platform-specific functions ----------------------------------------

// static
bool VMutex::mutexInit(Mutex* mutex)
    {
    return (::pthread_mutex_init(mutex, NULL) == 0);
    }

// static
void VMutex::mutexDestroy(Mutex* mutex)
    {
    (void) ::pthread_mutex_destroy(mutex);
    }

// static
bool VMutex::mutexLock(Mutex* mutex)
    {
    return (::pthread_mutex_lock(mutex) == 0);
    }

// static
bool VMutex::mutexUnlock(Mutex* mutex)
    {
    return (::pthread_mutex_unlock(mutex) == 0);
    }

// VSemaphore platform-specific functions ------------------------------------

// static
bool VSemaphore::semaphoreInit(Semaphore* semaphore)
    {
    return (pthread_cond_init(semaphore, NULL) == 0);
    }

// static
bool VSemaphore::semaphoreDestroy(Semaphore* semaphore)
    {
    return (pthread_cond_destroy(semaphore) == 0);
    }

// static
bool VSemaphore::semaphoreWait(Semaphore* semaphore, Mutex* mutex)
    {
    return (pthread_cond_wait(semaphore, mutex) == 0);
    }

// static
bool VSemaphore::semaphoreSignal(Semaphore* semaphore)
    {
    return (pthread_cond_signal(semaphore) == 0);
    }

// static
bool VSemaphore::semaphoreBroadcast(Semaphore* semaphore)
    {
    return (pthread_cond_broadcast(semaphore) == 0);
    }

