/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vthread.h"

#include <exception>
#include "vexception.h"
#include "vmanagementinterface.h"
#include "vlogger.h"
#include "vmutexlocker.h"
#include "vbento.h"

// This private map allows us to keep track of all VThread objects, and therefore to
// find the current thread's VThread object from the thread ID. We also have an API
// to get info about all these threads.
typedef std::map<VThreadID_Type, VThread*> VThreadIDToVThreadMap;
VThreadIDToVThreadMap gVThreadIDToVThreadMap;
static VMutex gVThreadMapMutex("gVThreadMapMutex", true/*suppress logging because logging itself uses this*/);

static void _vthreadStarting(VThread* thread)
    {
    VMutexLocker locker(&gVThreadMapMutex, "_vthreadStarting");
    gVThreadIDToVThreadMap[thread->threadID()] = thread;
    }

static void _vthreadEnded(VThread* thread)
    {
    VMutexLocker locker(&gVThreadMapMutex, "_vthreadEnded");
    VThreadIDToVThreadMap::iterator position = gVThreadIDToVThreadMap.find(thread->threadID());
    if (position != gVThreadIDToVThreadMap.end())
        gVThreadIDToVThreadMap.erase(position);
    }

class VStandinThread : public VThread
    {
    public:
        VStandinThread() : VThread("?", false, false, NULL) {}
        virtual ~VStandinThread() {}
        virtual void run() {}
    };

static VStandinThread gStandinThread;

static VThread* _getCurrentVThread()
    {
    VThreadID_Type currentThreadID = VThread::threadSelf();
    VMutexLocker locker(&gVThreadMapMutex, "_getCurrentVThread");
    VThreadIDToVThreadMap::const_iterator position = gVThreadIDToVThreadMap.find(currentThreadID);
    if (position == gVThreadIDToVThreadMap.end())
        return &gStandinThread; // If called from main thread, or non-VThread-derived thread, we won't find a VThread. This allows us to return something workable to any caller.

    return (*position).second;
    // Note: once we return, the thread could stop.
    // But since this is called from the current thread, it really can't disappear while caller lives.
    // It just can't be passed around to other threads!
    }

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

    VLOGGER_TRACE(VSTRING_FORMAT("VThread::VThread: constructed VThread '%s'.", name.chars()));
    }

VThread::~VThread()
    {
    // Detect repeat deletion bug. Can't refer to mName because it's been deleted.
    if (mIsDeleted)
        VLOGGER_ERROR(VSTRING_FORMAT("Thread delete on already-deleted thread @0x%08X.", this));
    else
        VLOGGER_TRACE(VSTRING_FORMAT("VThread::~VThread: destructed VThread '%s'.", mName.chars()));

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

#ifdef VAULT_SIMPLE_USER_THREAD_MAIN
void* VThread::userThreadMain(void* arg)
    {
    return VThread::threadMain(arg);
    }
#endif

void* VThread::threadMain(void* arg)
    {
    VException::installWin32SEHandler(); // A no-op if not configured to be used.

    VThread::_updateThreadStatistics(VThread::eMainStarted);

    VThread*    thread = static_cast<VThread*> (arg);
    VString     threadName = thread->getName();
    
    VLOGGER_TRACE(VSTRING_FORMAT("VThread::threadMain: start of thread '%s' id 0x%08X.", threadName.chars(), thread->threadID()));
    
    bool        deleteAtEnd = thread->getDeleteAtEnd();
    
    VManagementInterface* manager = thread->getManagementInterface();

    try
        {
        VAutoreleasePool pool;
        _vthreadStarting(thread);
        VThread::_threadStarting(thread);

        if (manager != NULL)
            manager->threadStarting(thread);

        thread->run();
        }
    catch (const VException& ex)
        {
        VLOGGER_ERROR(VSTRING_FORMAT("Thread '%s' main caught exception #%d '%s'.", threadName.chars(), ex.getError(), ex.what()));
        }
    catch (const std::exception& ex)
        {
        VLOGGER_ERROR(VSTRING_FORMAT("Thread '%s' main caught exception '%s'.", threadName.chars(), ex.what()));
        }
    catch (...)
        {
        VLOGGER_ERROR(VSTRING_FORMAT("Thread '%s' main caught unknown exception.", threadName.chars()));
        }

    // Let's be bulletproof even on this notification -- use try/catch.
    try
        {
        if (manager != NULL)
            {
            VLOGGER_TRACE(VSTRING_FORMAT("VThread '%s' notifying manager[0x%08X] of thread end.", threadName.chars(), manager));
            manager->threadEnded(thread);
            }
        }
    catch (...) 
        {
        VLOGGER_ERROR(VSTRING_FORMAT("Thread '%s' main caught exception notifying manager of thread end.", threadName.chars()));
        }

    VThread::_threadEnded(thread);
    _vthreadEnded(thread);

    if (deleteAtEnd)
        delete thread;

    VThread::_updateThreadStatistics(VThread::eMainCompleted);

    VLOGGER_TRACE(VSTRING_FORMAT("VThread::threadMain: completed thread '%s'.", threadName.chars()));

    return NULL;
    }

// static
VThread* VThread::getCurrentThread()
    {
    return _getCurrentVThread();
    }

// static
VString VThread::getCurrentThreadName()
    {
    VThread* currentThread = VThread::getCurrentThread();
    if (currentThread != &gStandinThread)
        return currentThread->getName();

    // It's the stand-in thread for non-VThread threads. Its name is meaningless.
    // Format the current OS thread ID.
    Vs64 id64 = (Vs64) VThread::threadSelf();
    return VSTRING_S64(id64);
    }

// static
void VThread::getThreadsInfo(VBentoNode& bento)
    {
    bento.setName("threads");

    VMutexLocker locker(&gVThreadMapMutex, "VThread::getThreadsInfo");
    for (VThreadIDToVThreadMap::const_iterator i = gVThreadIDToVThreadMap.begin(); i != gVThreadIDToVThreadMap.end(); ++i)
        {
        VThread* thread = (*i).second;
        VBentoNode* child = bento.addNewChildNode("thread");
        child->addString("name", thread->getName());
        child->addInt("threadID", (int) thread->mThreadID);
        child->addBool("isRunning", thread->mIsRunning);
        child->addBool("isDeleted", thread->mIsDeleted);
        child->addBool("deleteAtEnd", thread->mDeleteAtEnd);
        child->addBool("createdDetached", thread->mCreateDetached);
        child->addBool("hasManager", thread->mManager != NULL);
        }
    }

// static
VString VThread::getThreadName(VThreadID_Type threadID)
    {
    VMutexLocker locker(&gVThreadMapMutex, "VThread::getThreadName");
    VThreadIDToVThreadMap::iterator position = gVThreadIDToVThreadMap.find(threadID);
    if (position == gVThreadIDToVThreadMap.end())
        return VString::EMPTY();

    VThread* thread = (*position).second;
    VString threadName = thread->getName();
    return threadName;
    }

// static
void VThread::stopThread(VThreadID_Type threadID)
    {
    VMutexLocker locker(&gVThreadMapMutex, "VThread::stopThread");
    VThreadIDToVThreadMap::iterator position = gVThreadIDToVThreadMap.find(threadID);
    if (position != gVThreadIDToVThreadMap.end())
        {
        VThread* thread = (*position).second;
        thread->stop();
        }
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
    if (logger == NULL)
        VLOGGER_ERROR(VSTRING_FORMAT("%s (VThread::logStackCrawl: User stack crawl not implemented.)", headerMessage.chars()));
    else
        logger->emitStackCrawlLine(VSTRING_FORMAT("%s (VThread::logStackCrawl: User stack crawl not implemented.)", headerMessage.chars()));
    }
#endif /* VAULT_USER_STACKCRAWL_SUPPORT */

// VMainThread ----------------------------------------------------------------

VMainThread::VMainThread()
: VThread("main", kDontDeleteSelfAtEnd, kCreateThreadJoinable/*doesn't matter*/, NULL)
    {
    mThreadID = VThread::threadSelf();
    _vthreadStarting(this); // Register this object for lookup by mThreadID.
    }

VMainThread::~VMainThread()
    {
    _vthreadEnded(this); // De-register this object.
    }

void VMainThread::start()
    {
    VString errorMessage("Error: invalid attempt to start VMainThread.");
    VLOGGER_FATAL(errorMessage);
    throw VStackTraceException(errorMessage);
    }

int VMainThread::execute(int argc, char** argv)
    {
    return VThread::userMain(argc, argv);
    }

// VForeignThread ----------------------------------------------------------------

VForeignThread::VForeignThread(const VString& name)
: VThread(name, kDontDeleteSelfAtEnd, kCreateThreadJoinable/*doesn't matter*/, NULL)
    {
    mThreadID = VThread::threadSelf();
    _vthreadStarting(this); // Register this object for lookup by mThreadID.
    }

VForeignThread::~VForeignThread()
    {
    _vthreadEnded(this); // De-register this object.
    }

void VForeignThread::start()
    {
    VString errorMessage("Error: invalid attempt to start VForeignThread.");
    VLOGGER_FATAL(errorMessage);
    throw VStackTraceException(errorMessage);
    }


