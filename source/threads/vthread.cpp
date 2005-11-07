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

VThread::VThread(const VString& name, bool deleteAtEnd, VManagementInterface* manager) :
mIsDeleted(false),
mName(name),
mDeleteAtEnd(deleteAtEnd),
mManager(manager),
mThreadID((ThreadID) -1),
mIsRunning(false)
    {
    }

VThread::~VThread()
    {
    // Detect repeat deletion bug. Can't refer to mName because it's been deleted.
    if (mIsDeleted)
        VLogger::getLogger("default")->log(VLogger::kError, NULL, 0, "Thread delete on already-deleted thread @0x%08X.", this);

    mIsDeleted = true;
    mIsRunning = false;
    mThreadID = (ThreadID) -1;
    }

void VThread::start()
    {
    if (mIsRunning)
        return;

    mIsRunning = true;
    
    if (! VThread::threadCreate(&mThreadID, VThread::threadMain, (void*) this))
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
        VLogger::getLogger("default")->log(VLogger::kError, NULL, 0, "Thread '%s' main caught exception #%d '%s'.", threadName.chars(), ex.getError(), ex.what());
        }
    catch (std::exception& ex)
        {
        VLogger::getLogger("default")->log(VLogger::kError, NULL, 0, "Thread '%s' main caught exception '%s'.", threadName.chars(), ex.what());
        }
    catch (...)
        {
        VLogger::getLogger("default")->log(VLogger::kError, NULL, 0, "Thread '%s' main caught unknown exception.", threadName.chars());
        }

    // Let's be bulletproof even on this notification -- use try/catch.
    try
        {
        if (manager != NULL)
            manager->threadEnded(thread);
        }
    catch (...) {}

#ifdef VPLATFORM_WIN
    SetEvent((HANDLE) thread->threadID());    // signal on this thread, so join()ers unblock
#endif
    
    if (deleteAtEnd)
        delete thread;

    VThread::threadExit();
    
    return NULL;
    }

