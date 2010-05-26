/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0.1
http://www.bombaydigital.com/
*/

/** @file */

#include "vthread.h"

#include <exception>
#include "vexception.h"
#include "vmanagementinterface.h"
#include "vlogger.h"
#include "vmutexlocker.h"

int VThread::gNumVThreads = 0;
int VThread::gNumThreadMains = 0;
int VThread::gNumVThreadsCreated = 0;
int VThread::gNumThreadMainsStarted = 0;
int VThread::gNumVThreadsDestructed = 0;
int VThread::gNumThreadMainsCompleted = 0;
VThreadActionListener* VThread::gActionListener = NULL;

// If we just declare this as an object, not a pointer, static initialization order
// problems can occur. This form ensures it is NULL until properly constructed.
static VMutex* gThreadStatsMutex = new VMutex("gThreadStatsMutex");

VThread::VThread(const VString& name, bool deleteAtEnd, bool createDetached, VManagementInterface* manager) :
mIsDeleted(false),
mName(name),
mDeleteAtEnd(deleteAtEnd),
mCreateDetached(createDetached),
mManager(manager),
mThreadID((VThreadID_Type) -1),
mIsRunning(false)
    {
    VThread::_updateThreadStatistics(VThread::eCreated);

    VLOGGER_TRACE(VString("VThread::VThread: constructed VThread '%s'.", name.chars()));
    }

VThread::~VThread()
    {
    // Detect repeat deletion bug. Can't refer to mName because it's been deleted.
    if (mIsDeleted)
        VLOGGER_ERROR(VString("Thread delete on already-deleted thread @0x%08X.", this));
    else
        VLOGGER_TRACE(VString("VThread::~VThread: destructed VThread '%s'.", mName.chars()));

    mIsDeleted = true;
    mIsRunning = false;
    mThreadID = (VThreadID_Type) -1;

    // Prevent all exceptions from escaping destructor.
    try
        {
        VThread::_updateThreadStatistics(VThread::eDestroyed);
        }
    catch (...) {}
    }

void VThread::start()
    {
    if (mIsRunning)
        return;

    mIsRunning = true;

    try
        {
        VThread::threadCreate(&mThreadID, mCreateDetached, VThread::userThreadMain, (void*) this);
        }
    catch (...)
        {
        mIsRunning = false;
        throw;
        }
    }

void VThread::stop()
    {
    mIsRunning = false;
    }

VThreadID_Type VThread::threadID() const
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

    if (! mIsRunning || (mThreadID == (VThreadID_Type) -1))
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

void* VThread::threadMain(void* arg)
    {
    VException::installWin32SEHandler(); // A no-op if not configured to be used.

    VThread::_updateThreadStatistics(VThread::eMainStarted);

    VThread*    thread = static_cast<VThread*> (arg);
    VString     threadName = thread->getName();
    
    VLOGGER_TRACE(VString("VThread::threadMain: start of thread '%s' id 0x%08X.", threadName.chars(), thread->threadID()));
    
    bool        deleteAtEnd = thread->getDeleteAtEnd();
    
    VManagementInterface* manager = thread->getManagementInterface();

    try
        {
        VThread::_threadStarting(thread);

        if (manager != NULL)
            manager->threadStarting(thread);

        thread->run();
        }
    catch (const VException& ex)
        {
        VLOGGER_ERROR(VString("Thread '%s' main caught exception #%d '%s'.", threadName.chars(), ex.getError(), ex.what()));
        }
    catch (const std::exception& ex)
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
            {
            VLOGGER_TRACE(VString("VThread '%s' notifying manager[0x%08X] of thread end.", threadName.chars(), manager));
            manager->threadEnded(thread);
            }
        }
    catch (...) 
        {
        VLOGGER_ERROR(VString("Thread '%s' main caught exception notifying manager of thread end.", threadName.chars()));
        }

    VThread::_threadEnded(thread);

    if (deleteAtEnd)
        delete thread;

    VThread::_updateThreadStatistics(VThread::eMainCompleted);

    VLOGGER_TRACE(VString("VThread::threadMain: completed thread '%s'.", threadName.chars()));

    return NULL;
    }

// static
void VThread::getThreadStatistics(int& numVThreads, int& numThreadMains, int& numVThreadsCreated, int& numThreadMainsStarted, int& numVThreadsDestructed, int& numThreadMainsCompleted)
    {
    VMutexLocker locker(gThreadStatsMutex, "VThread::getThreadStatistics()");
    
    numVThreads = gNumVThreads;
    numThreadMains = gNumThreadMains;
    numVThreadsCreated = gNumVThreadsCreated;
    numThreadMainsStarted = gNumThreadMainsStarted;
    numVThreadsDestructed = gNumVThreadsDestructed;
    numThreadMainsCompleted = gNumThreadMainsCompleted;
    }

// static
void VThread::setActionListener(VThreadActionListener* listener)
    {
    VMutexLocker locker(gThreadStatsMutex, "VThread::setActionListener()");

    gActionListener = listener;
    }

// static
void VThread::_updateThreadStatistics(eThreadAction action)
    {
    VMutexLocker locker(gThreadStatsMutex, "VThread::_updateThreadStatistics()");

    switch (action)
        {
        case VThread::eCreated:
            ++VThread::gNumVThreads;
            ++VThread::gNumVThreadsCreated;
            break;
        case VThread::eDestroyed:
            --VThread::gNumVThreads;
            ++VThread::gNumVThreadsDestructed;
            break;
        case VThread::eMainStarted:
            ++VThread::gNumThreadMains;
            ++VThread::gNumThreadMainsStarted;
            break;
        case VThread::eMainCompleted:
            --VThread::gNumThreadMains;
            ++VThread::gNumThreadMainsCompleted;
            break;
        }
    }

#ifndef VAULT_USER_STACKCRAWL_SUPPORT
// static
void VThread::logStackCrawl(const VString& headerMessage, VLogger* logger, bool /*verbose*/)
    {
    logger->emitStackCrawlLine(VString("%s (VThread::logStackCrawl: User stack crawl not implemented.)", headerMessage.chars()));
    }
#endif /* VAULT_USER_STACKCRAWL_SUPPORT */


