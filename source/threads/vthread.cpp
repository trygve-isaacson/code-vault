/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vthread.h"

#include <exception>
#include "vexception.h"
#include "vmanagementinterface.h"
#include "vlogger.h"
#include "vmutexlocker.h"

int VThread::smNumVThreads = 0;
int VThread::smNumThreadMains = 0;
int VThread::smNumVThreadsCreated = 0;
int VThread::smNumThreadMainsStarted = 0;
int VThread::smNumVThreadsDestructed = 0;
int VThread::smNumThreadMainsCompleted = 0;

// If we just declare this as an object, not a pointer, static initialization order
// problems can occur. This form ensures it is NULL until properly constructed.
static VMutex* gThreadStatsMutex = new VMutex();

VThread::VThread(const VString& name, bool deleteAtEnd, bool createDetached, VManagementInterface* manager) :
mIsDeleted(false),
mName(name),
mDeleteAtEnd(deleteAtEnd),
mCreateDetached(createDetached),
mManager(manager),
mThreadID((ThreadID) -1),
mIsRunning(false)
    {
    VThread::updateThreadStatistics(VThread::eCreated);
    }

VThread::~VThread()
    {
    // Detect repeat deletion bug. Can't refer to mName because it's been deleted.
    if (mIsDeleted)
        VLOGGER_ERROR(VString("Thread delete on already-deleted thread @0x%08X.", this));

    mIsDeleted = true;
    mIsRunning = false;
    mThreadID = (ThreadID) -1;

    // Prevent all exceptions from escaping destructor.
    try
        {
        VThread::updateThreadStatistics(VThread::eDestroyed);
        }
    catch (...) {}
    }

void VThread::start()
    {
    if (mIsRunning)
        return;

    mIsRunning = true;
    
    if (!VThread::threadCreate(&mThreadID, mCreateDetached, VThread::threadMain, (void*) this))
        {
        mIsRunning = false;
        throw VException("VThread::start failed to start thread.");
        }
    }

void VThread::stop()
    {
    mIsRunning = false;
    }

ThreadID VThread::threadID() const
    {
    return mThreadID;
    }

bool VThread::isRunning() const
    {
    return mIsRunning;
    }

bool VThread::join()
    {
    // FIXME: could complain here if mCreateDetached is true.

    if (! mIsRunning || (mThreadID == (ThreadID) -1))
        return true;    // never started, or was stopped, so we're done
    else
        return VThread::threadJoin(mThreadID, NULL);
    }

bool VThread::getDeleteAtEnd() const
    {
    return mDeleteAtEnd;
    }

VManagementInterface* VThread::getManagementInterface() const
    {
    return mManager;
    }

const VString& VThread::name() const
    {
    return mName;
    }

void VThread::setName(const VString& threadName)
    {
    mName = threadName;
    }

void* VThread::threadMain(void* arg)
    {
    VThread::updateThreadStatistics(VThread::eMainStarted);

    VThread*    thread = static_cast<VThread*> (arg);
    VString        threadName = thread->name();
    
    bool        deleteAtEnd = thread->getDeleteAtEnd();
    
    VManagementInterface*    manager = thread->getManagementInterface();

#ifdef VPLATFORM_WIN
    ResetEvent((HANDLE) thread->threadID());    // remove any signal from this thread
#endif
    
    try
        {
        if (manager != NULL)
            manager->threadStarting(thread);

        thread->run();
        }
    catch (VException& ex)
        {
        VLOGGER_ERROR(VString("Thread '%s' main caught exception #%d '%s'.", threadName.chars(), ex.getError(), ex.what()));
        }
    catch (std::exception& ex)
        {
        VLOGGER_ERROR(VString("Thread '%s' main caught exception '%s'.", threadName.chars(), ex.what()));
        }
    catch (...)
        {
        VLOGGER_ERROR(VString("Thread '%s' main caught unknown exception.", threadName.chars()));
        }

    // Let's be bulletproof even on this notification -- use try/catch.
    try
        {
        if (manager != NULL)
            manager->threadEnded(thread);
        }
    catch (...) 
        {
        VLOGGER_ERROR(VString("Thread '%s' main caught exception notifying manager of thread end.", threadName.chars()));
        }

#ifdef VPLATFORM_WIN
    SetEvent((HANDLE) thread->threadID());    // signal on this thread, so join()ers unblock
#endif


//    VThread::threadExit(); NO! This should only be called for abnormal thread exit.

    if (deleteAtEnd)
        delete thread;

    VThread::updateThreadStatistics(VThread::eMainCompleted);

    return NULL;
    }

// static
void VThread::updateThreadStatistics(eThreadAction action)
    {
    VMutexLocker locker(gThreadStatsMutex);

    switch (action)
        {
        case VThread::eCreated:
            ++VThread::smNumVThreads;
            ++VThread::smNumVThreadsCreated;
            break;
        case VThread::eDestroyed:
            --VThread::smNumVThreads;
            ++VThread::smNumVThreadsDestructed;
            break;
        case VThread::eMainStarted:
            ++VThread::smNumThreadMains;
            ++VThread::smNumThreadMainsStarted;
            break;
        case VThread::eMainCompleted:
            --VThread::smNumThreadMains;
            ++VThread::smNumThreadMainsCompleted;
            break;
        }
    }

// static
void VThread::getThreadStatistics(int& numVThreads, int& numThreadMains, int& numVThreadsCreated, int& numThreadMainsStarted, int& numVThreadsDestructed, int& numThreadMainsCompleted)
    {
    VMutexLocker locker(gThreadStatsMutex);
    
    numVThreads = smNumVThreads;
    numThreadMains = smNumThreadMains;
    numVThreadsCreated = smNumVThreadsCreated;
    numThreadMainsStarted = smNumThreadMainsStarted;
    numVThreadsDestructed = smNumVThreadsDestructed;
    numThreadMainsCompleted = smNumThreadMainsCompleted;
    }


