/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vthread.h"
#include "vtypes_internal_platform.h"

#include "vmutex.h"
#include "vsemaphore.h"
#include "vlogger.h"
#include "vinstant.h"
#include "vexception.h"

#include <sys/time.h>
#include <sys/resource.h>

// VThread platform-specific functions ---------------------------------------

// static
void VThread::threadCreate(VThreadID_Type* threadID, bool createDetached, threadMainFunction threadMainProcPtr, void* threadArgument)
    {
    int             result;
    pthread_attr_t  threadAttributes;
    
    result = ::pthread_attr_init(&threadAttributes);
    
    if (result != 0)
        throw VStackTraceException(result, VSTRING_FORMAT("VThread::threadCreate: pthread_attr_init returned %d (%s).", result, ::strerror(result)));

    result = ::pthread_attr_setdetachstate(&threadAttributes, createDetached ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE);
    
    if (result != 0)
        throw VStackTraceException(result, VSTRING_FORMAT("VThread::threadCreate: pthread_attr_setdetachstate returned %d (%s).", result, ::strerror(result)));

    result = ::pthread_create(threadID, &threadAttributes, threadMainProcPtr, threadArgument);
    
    if (result != 0)
        {
        // Usually this means we have hit the limit of threads allowed per process.
        // Log our statistics. Maybe we have a thread handle leak.
        throw VStackTraceException(result, VSTRING_FORMAT("VThread::threadCreate: pthread_create returned %d (%s). Likely due to lack of resources.", result, ::strerror(result)));
        }

    (void) ::pthread_attr_destroy(&threadAttributes);
    }

// static
#ifdef VTHREAD_PTHREAD_SETNAME_SUPPORTED
void VThread::_threadStarting(const VThread* thread)
    {
    // This API lets us associate our thread name with the native thread resource, so that debugger/crashdump/instruments etc. can see our thread name.
    (void)/*int result =*/ ::pthread_setname_np(thread->getName()); // "np" indicates API is non-POSIX
    }
#else
void VThread::_threadStarting(const VThread* /*thread*/)
    {
    // Nothing to do if pthread_setname_np() is not available.
    }
#endif

// static
void VThread::_threadEnded(const VThread* /*thread*/)
    {
    // Nothing to do for unix version.
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
    (void) ::sched_yield(); 
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

