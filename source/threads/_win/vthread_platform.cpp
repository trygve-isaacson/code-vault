/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

/** @file */

#include "vthread.h"
#include "vmutex.h"
#include "vsemaphore.h"
#include "vexception.h"
#include "vmutexlocker.h"

#include <process.h>

// Windows does not provide an API to map thread IDs to thread handles, but requires
// thread IDs for some APIs and thread handles for others. And you can't get the current
// thread handle, only a "pseudo-handle" or the thread ID. So it makes sense to use the
// thread ID as our "VThreadID_Type" but we need a way to map the ID to the handle when
// calling APIs that need a handle (primarily when join() calls WaitForSingleObject()).
// That is why we maintain this map.
typedef std::map<VThreadID_Type, HANDLE> WindowsThreadIDToHandleMap;
WindowsThreadIDToHandleMap gWindowsThreadIDToHandleMap;
static VMutex gWindowsThreadMapMutex("gWindowsThreadMapMutex");

static void _addThreadToMap(VThreadID_Type threadID, HANDLE threadHandle) {
    VMutexLocker locker(&gWindowsThreadMapMutex, "_addThreadToMap");
    gWindowsThreadIDToHandleMap[threadID] = threadHandle;
}

static void _removeThreadFromMap(VThreadID_Type threadID) {
    VMutexLocker locker(&gWindowsThreadMapMutex, "_removeThreadFromMap");
    gWindowsThreadIDToHandleMap[threadID] = 0;
}

static HANDLE _lookupThreadHandle(VThreadID_Type threadID) {
    VMutexLocker locker(&gWindowsThreadMapMutex, "_lookupThreadHandle");
    return gWindowsThreadIDToHandleMap[threadID];
}

// VThread platform-specific functions ---------------------------------------

// static
void VThread::threadCreate(VThreadID_Type* threadID, bool /*createDetached*/, threadMainFunction threadMainProcPtr, void* threadArgument) {
    HANDLE threadHandle = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) threadMainProcPtr, threadArgument, 0, /*(LPDWORD)*/ threadID);

    if (threadHandle == NULL) {
        throw VStackTraceException(VSystemError(), "VThread::threadCreate: CreateThread returned null.");
    }

    _addThreadToMap(*threadID, threadHandle);
}

// static
void VThread::_threadStarting(const VThread* thread) {
    HANDLE threadHandle = _lookupThreadHandle(thread->threadID());
    ResetEvent(threadHandle);    // remove any signal from this thread
}

// static
void VThread::_threadEnded(const VThread* thread) {
    HANDLE threadHandle = _lookupThreadHandle(thread->threadID());
    _removeThreadFromMap(thread->threadID());
    SetEvent(threadHandle);    // signal on this thread, so join()ers unblock
    CloseHandle(threadHandle); // we use CreateThread() so must call CloseHandle() after thread ends
}

// static
void VThread::threadExit() {
    // FIXME:
    // On Windows, normal exit requires nothing.
    // Abnormal requires AfxEndThread(0); -- which #include?
    // Also: investigate the API "ExitThread()"
}

// static
bool VThread::threadJoin(VThreadID_Type threadID, void** /*value*/) {
    HANDLE threadHandle = _lookupThreadHandle(threadID);
    ::WaitForSingleObject(threadHandle, INFINITE);
    return true;
}

// static
void VThread::threadDetach(VThreadID_Type /*threadID*/) {
    // FIXME: tbd - Is there any Windows call necessary?
}

// static
VThreadID_Type VThread::threadSelf() {
    return ::GetCurrentThreadId();
}

// static
bool VThread::setPriority(int /*nice*/) {
    // FIXME: tbd - Is there a Windows API to do this?
    return true;
}

// static
void VThread::sleep(const VDuration& interval) {
    Sleep(static_cast<DWORD>(interval.getDurationMilliseconds()));
}

// static
void VThread::yield() {
    // FIXME: There's no yield function under Windows --
    // Investigate the API "SwitchToThread()"
    VThread::sleep(VDuration::MILLISECOND());
}

// VMutex platform-specific functions ----------------------------------------

// static
bool VMutex::mutexInit(VMutex_Type* mutex) {
    InitializeCriticalSection(mutex);
    return true;
}

// static
void VMutex::mutexDestroy(VMutex_Type* mutex) {
    DeleteCriticalSection(mutex);
}

// static
bool VMutex::mutexLock(VMutex_Type* mutex) {
    EnterCriticalSection(mutex);
    return true;
}

// static
bool VMutex::mutexUnlock(VMutex_Type* mutex) {
    LeaveCriticalSection(mutex);
    return true;
}

// VSemaphore platform-specific functions ------------------------------------

#define kSemaphoreMaxCount 1

// static
bool VSemaphore::semaphoreInit(VSemaphore_Type* semaphore) {
    *semaphore = CreateSemaphore(NULL, 0, kSemaphoreMaxCount, NULL);
    return (*semaphore != NULL);
}

// static
bool VSemaphore::semaphoreDestroy(VSemaphore_Type* semaphore) {
    CloseHandle(*semaphore);
    return true;
}

// static
bool VSemaphore::semaphoreWait(VSemaphore_Type* semaphore, VMutex_Type* /*mutex*/, const VDuration& timeoutInterval) {
    DWORD timeoutMillisecondsDWORD;

    if (timeoutInterval == VDuration::ZERO()) {
        timeoutMillisecondsDWORD = INFINITE;
    } else {
        timeoutMillisecondsDWORD = static_cast<DWORD>(timeoutInterval.getDurationMilliseconds());
    }

    DWORD result = WaitForSingleObject(*semaphore, timeoutMillisecondsDWORD);    // waits until the semaphore's count is > 0, then decrements it
    return (result != WAIT_FAILED);
}

// static
bool VSemaphore::semaphoreSignal(VSemaphore_Type* semaphore) {
    LONG previousCount = 1;
    BOOL result = ReleaseSemaphore(*semaphore, 1, &previousCount);    // increases the semaphore's "count" by 1

    // The only acceptable "error" is an attempt to increment from 1 to 2.
    return (result != 0 || (previousCount == 1));
}

// static
bool VSemaphore::semaphoreBroadcast(VSemaphore_Type* /*semaphore*/) {
    // FIXME: How can this be done on Windows?
    return false;
}

