/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vthread.h"
#include "vmutex.h"
#include "vsemaphore.h"

#include <process.h>

// VThread platform-specific functions ---------------------------------------

// static
bool VThread::threadCreate(VThreadID_Type* threadID, bool /*createDetached*/, threadMainFunction threadMainProcPtr, void* threadArgument)
    {
#ifdef __MWERKS__
    unsigned int    id;
    *threadID = (VThreadID_Type) _beginthreadex((void*) NULL, (unsigned int) 0, (Win32ThreadMainFunctionEx) threadMainProcPtr, threadArgument, (unsigned int) 0, &id);
#else
    *threadID = (VThreadID_Type) _beginthread((Win32ThreadMainFunction) threadMainProcPtr, (unsigned int) 0, threadArgument);
#endif

    return ((*threadID != 0) && (*threadID != -1));
    }

// static
void VThread::threadExit()
    {
    // FIXME:
    // On Windows, normal exit requires nothing.
    // Abnormal requires AfxEndThread(0); -- which #include?
    // Also: investigate the API "ExitThread()"
    }

// static
bool VThread::threadJoin(VThreadID_Type threadID, void** /*value*/)
    {
    // FIXME: investigate the API "GetExitCodeThread()"
    WaitForSingleObject((HANDLE) threadID, INFINITE);
    return true;
    }

// static
void VThread::threadDetach(VThreadID_Type /*threadID*/)
    { 
    // FIXME: tbd - Is there any Windows call necessary?
    }
    
// static
VThreadID_Type VThread::threadSelf() 
    {
    // FIXME: tbd - Investigate the APIs "GetCurrentThreadId()" and "GetCurrentThread()"
    // It seems that GetCurrentThreadId() does not return the same value as
    // what _beginThread gave us.
    // Nor does GetCurrentThread() and DuplicateHandle() as suggested in the Win SDK docs.
    return -1;
    }

// static
bool VThread::setPriority(int /*nice*/)
    {
    // FIXME: tbd - Is there a Windows API to do this?
    return true;
    }

// static
void VThread::sleepMilliseconds(int milliseconds)
    {
    Sleep(milliseconds);
    }

// static
void VThread::yield()
    {
    // FIXME: There's no yield function under Windows --
    // Investigate the API "SwitchToThread()"
    VThread::sleepMilliseconds(1);
    }

// VMutex platform-specific functions ----------------------------------------

// static
bool VMutex::mutexInit(VMutex_Type* mutex)
    {
    InitializeCriticalSection(mutex);
    return true;
    }

// static
void VMutex::mutexDestroy(VMutex_Type* mutex)
    {
    DeleteCriticalSection(mutex);
    }

// static
bool VMutex::mutexLock(VMutex_Type* mutex)
    {
    EnterCriticalSection(mutex);
    return true;
    }

// static
bool VMutex::mutexUnlock(VMutex_Type* mutex)
    {
    LeaveCriticalSection(mutex);
    return true;
    }

// VSemaphore platform-specific functions ------------------------------------

#define kSemaphoreMaxCount    0x7FFFFFFF

// static
bool VSemaphore::semaphoreInit(VSemaphore_Type* semaphore)
    {
    *semaphore = CreateSemaphore(NULL, 0, kSemaphoreMaxCount, NULL);
    return (*semaphore != NULL);
    }

// static
bool VSemaphore::semaphoreDestroy(VSemaphore_Type* semaphore)
    {
    CloseHandle(*semaphore);
    return true;
    }

// static
bool VSemaphore::semaphoreWait(VSemaphore_Type* semaphore, VMutex_Type* /*mutex*/, Vs64 timeoutMilliseconds)
    {
    DWORD    timeoutMillisecondsDWORD = (timeoutMilliseconds == 0) ? INFINITE : ((DWORD) timeoutMilliseconds);
    DWORD    result = WaitForSingleObject(*semaphore, timeoutMillisecondsDWORD);    // waits until the semaphore's count is > 0, then decrements it
    return (result != WAIT_FAILED);
    }

// static
bool VSemaphore::semaphoreSignal(VSemaphore_Type* semaphore)
    {
    BOOL    result = ReleaseSemaphore(*semaphore, 1, NULL);    // increases the semaphore's "count" by 1
    return (result != 0);
    }

// static
bool VSemaphore::semaphoreBroadcast(VSemaphore_Type* /*semaphore*/)
    {
    // FIXME: How can this be done on Windows?
    return false;
    }

