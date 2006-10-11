/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vthread.h"
#include "vmutex.h"
#include "vsemaphore.h"
#include "vlogger.h"
#include "vinstant.h"

#include <sys/time.h>
#include <sys/resource.h>

// VThread platform-specific functions ---------------------------------------

// static
bool VThread::threadCreate(VThreadID_Type* threadID, bool createDetached, threadMainFunction threadMainProcPtr, void* threadArgument)
    {
    int                result;
    pthread_attr_t    threadAttributes;
    
    result = ::pthread_attr_init(&threadAttributes);
    
    // FIXME: Throwing an exception here would be preferable. Check calling code
    // to see if it can deal with that.
    if (result != 0)
        {
        VLOGGER_ERROR(VString("VThread::threadCreate: pthread_attr_init returned %d (%s).", result, ::strerror(result)));
        return false;
        }

    result = ::pthread_attr_setdetachstate(&threadAttributes, createDetached ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE);
    
    // FIXME: Throwing an exception here would be preferable.
    if (result != 0)
        {
        VLOGGER_ERROR(VString("VThread::threadCreate: pthread_attr_setdetachstate returned %d (%s).", result, ::strerror(result)));
        return false;
        }

    result = ::pthread_create(threadID, &threadAttributes, threadMainProcPtr, threadArgument);
    
    // FIXME: Throwing an exception here would be preferable.
    if (result != 0)
        {
        // Usually this means we have hit the limit of threads allowed per process.
        // Log our statistics. Maybe we have a thread handle leak.
        int numVThreads;
        int numThreadMains;
        int numVThreadsCreated;
        int numThreadMainsStarted;
        int numVThreadsDestructed;
        int numThreadMainsCompleted;
        
        VThread::getThreadStatistics(numVThreads, numThreadMains, numVThreadsCreated, numThreadMainsStarted, numVThreadsDestructed, numThreadMainsCompleted);

        VLOGGER_ERROR(VString("VThread::threadCreate: pthread_create returned %d (%s).", result, ::strerror(result)));
        VLOGGER_ERROR(VString(" VThread::smNumVThreads             = %d", numVThreads));
        VLOGGER_ERROR(VString(" VThread::smNumThreadMains          = %d", numThreadMains));
        VLOGGER_ERROR(VString(" VThread::smNumVThreadsCreated      = %d", numVThreadsCreated));
        VLOGGER_ERROR(VString(" VThread::smNumThreadMainsStarted   = %d", numThreadMainsStarted));
        VLOGGER_ERROR(VString(" VThread::smNumVThreadsDestructed   = %d", numVThreadsDestructed));
        VLOGGER_ERROR(VString(" VThread::smNumThreadMainsCompleted = %d", numThreadMainsCompleted));
        }

    (void) ::pthread_attr_destroy(&threadAttributes);

    return (result == 0);
    }

// static
void VThread::threadExit()
    {
    ::pthread_exit(NULL);
    }
    
// static
bool VThread::threadJoin(VThreadID_Type threadID, void** value)
    {
    return (::pthread_join(threadID, value) == 0);
    }

// static
void VThread::threadDetach(VThreadID_Type threadID)
    { 
    ::pthread_detach(threadID);
    }
    
// static
VThreadID_Type VThread::threadSelf() 
    {
    return ::pthread_self();
    }

// static
bool VThread::setPriority(int nice)
    {
    return (::setpriority(PRIO_PROCESS, 0, nice) == 0);
    }

// static
void VThread::sleep(const VDuration& interval)
    {
    int milliseconds = static_cast<int>(interval.getDurationMilliseconds());
    struct timeval timeout;
    timeout.tv_sec = interval.getDurationSeconds();
    timeout.tv_usec = (milliseconds % 1000) * 1000;

    (void) ::select(1, NULL, NULL, NULL, &timeout); // 1 means file descriptor [0], will just timeout
    }

// static
void VThread::yield()
    {
#ifdef sun
    // On Solaris there is no yield function.
    // Simulate by sleeping for 1ms. How to improve?
    VThread::sleep(VDuration::MILLISECOND());
#else
    ::sched_yield(); 
#endif
    }

// VMutex platform-specific functions ----------------------------------------

// static
bool VMutex::mutexInit(VMutex_Type* mutex)
    {
    return (::pthread_mutex_init(mutex, NULL) == 0);
    }

// static
void VMutex::mutexDestroy(VMutex_Type* mutex)
    {
    (void) ::pthread_mutex_destroy(mutex);
    }

// static
bool VMutex::mutexLock(VMutex_Type* mutex)
    {
    return (::pthread_mutex_lock(mutex) == 0);
    }

// static
bool VMutex::mutexUnlock(VMutex_Type* mutex)
    {
    return (::pthread_mutex_unlock(mutex) == 0);
    }

// VSemaphore platform-specific functions ------------------------------------

// static
bool VSemaphore::semaphoreInit(VSemaphore_Type* semaphore)
    {
    return (pthread_cond_init(semaphore, NULL) == 0);
    }

// static
bool VSemaphore::semaphoreDestroy(VSemaphore_Type* semaphore)
    {
    return (pthread_cond_destroy(semaphore) == 0);
    }

// static
bool VSemaphore::semaphoreWait(VSemaphore_Type* semaphore, VMutex_Type* mutex, const VDuration& timeoutInterval)
    {
    if (timeoutInterval == VDuration::ZERO())
        return (pthread_cond_wait(semaphore, mutex) == 0);

    // The timespec is an absolute time (base is 1970 UTC), not an
    // offset from the current time.
    VInstant now;
    VInstant timeoutWhen = now + timeoutInterval;
    Vs64     timeoutValue = timeoutWhen.getValue();
    
    struct timespec timeoutSpec;
    
    // Convert milliseconds to seconds.nanoseconds. e.g., 1234ms = 1sec + 234,000,000ns
    timeoutSpec.tv_sec = static_cast<time_t>(timeoutValue / CONST_S64(1000));
    timeoutSpec.tv_nsec = static_cast<time_t>(CONST_S64(1000000) * (timeoutValue % CONST_S64(1000)));

    int result = pthread_cond_timedwait(semaphore, mutex, &timeoutSpec);
    
    return (result == 0) || (result == ETIMEDOUT);
    }

// static
bool VSemaphore::semaphoreSignal(VSemaphore_Type* semaphore)
    {
    return (pthread_cond_signal(semaphore) == 0);
    }

// static
bool VSemaphore::semaphoreBroadcast(VSemaphore_Type* semaphore)
    {
    return (pthread_cond_broadcast(semaphore) == 0);
    }

