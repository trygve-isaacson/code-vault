/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vthreadsunit.h"
#include "vthread.h"
#include "vmutex.h"
#include "vmutexlocker.h"
#include "vsemaphore.h"
#include "vexception.h"

class TestThreadClass : public VThread
    {
    public:
    
        TestThreadClass(VThreadsUnit* ownerUnit, const VString& namePrefix, int numSecondsToSleep, int numIterations, bool* boolToSetOnCompletion, TestThreadClass** thisPtrToNull, volatile VThreadID_Type* threadIDMember, volatile VThreadID_Type* threadIDSelf, VMutex* mutexToLock);
        ~TestThreadClass();
        
        virtual void run();
        
        VThreadsUnit* mOwnerUnit;
        int     mNumSecondsToSleep;
        int     mNumIterations;
        bool*    mBoolToSetOnCompletion;
        TestThreadClass**    mThisPtrToNull;
        volatile VThreadID_Type* mThreadIDMemberToSet; // stash mThreadID here; unit test will verify it matches threadSelf()
        volatile VThreadID_Type* mThreadIDSelfToSet;   // stash threadSelf() value here in run(); unit test will verify it matches mThreadID
        VMutex* mMutexToLock;

    private:

        TestThreadClass(const TestThreadClass&); // not copyable
        TestThreadClass& operator=(const TestThreadClass&); // not assignable
    };

TestThreadClass::TestThreadClass(VThreadsUnit* ownerUnit, const VString& namePrefix, int numSecondsToSleep, int numIterations, bool* boolToSetOnCompletion, TestThreadClass** thisPtrToNull, volatile VThreadID_Type* threadIDMember, volatile VThreadID_Type* threadIDSelf, VMutex* mutexToLock) :
VThread(VSTRING_FORMAT("TestThreadClass.%s", namePrefix.chars()), thisPtrToNull != NULL, kCreateThreadJoinable, NULL),
mOwnerUnit(ownerUnit),
mNumSecondsToSleep(numSecondsToSleep),
mNumIterations(numIterations),
mBoolToSetOnCompletion(boolToSetOnCompletion),
mThisPtrToNull(thisPtrToNull),
mThreadIDMemberToSet(threadIDMember),
mThreadIDSelfToSet(threadIDSelf),
mMutexToLock(mutexToLock)
    {
    *mBoolToSetOnCompletion = false;
    }

TestThreadClass::~TestThreadClass()
    {
    if (mThisPtrToNull != NULL)
        *mThisPtrToNull = NULL;
    }

void TestThreadClass::run()
    {
    VMutexLocker locker(mMutexToLock, "TestThreadClass::run"); // mMutexToLock can be null, meaning nothing to lock

    while ((mNumIterations > 0) && this->isRunning())
        {
        // We are now running in our own thread. Let's sleep a little...
        VThread::sleep(mNumSecondsToSleep * VDuration::SECOND());

        --mNumIterations;
        }

    if (mMutexToLock != NULL)
        mOwnerUnit->test(mMutexToLock->isLockedByCurrentThread(), "TestThreadClass sees that it has the lock");

    // Now our thread will finish, terminate, and delete this.

    // We set the values the unit test can examine so it can verify behavior.
    *mThreadIDMemberToSet = mThreadID;
    *mThreadIDSelfToSet = VThread::threadSelf();
    *mBoolToSetOnCompletion = true;

    VString info(VSTRING_ARGS("Thread::run completed '%s' : id=%lld self=%lld.", mName.chars(), (Vs64) *mThreadIDMemberToSet, (Vs64) *mThreadIDSelfToSet));
    mOwnerUnit->logStatus(info);
    }

VThreadsUnit::VThreadsUnit(bool logOnSuccess, bool throwOnError) :
VUnit("VThreadsUnit", logOnSuccess, throwOnError)
    {
    }

void VThreadsUnit::run()
    {
    // Test the basic behavior of mutex locking and unlocking.

    VMutex mutex1("mutex1");

        {    
        VMutexLocker locker(&mutex1, "VThreadsUnit locker 1");
        this->test(locker.isLocked(), "mutex locker initial lock");
        }

        {    
        VMutexLocker locker(&mutex1, "VThreadsUnit locker 2", false);
        this->test(! locker.isLocked(), "mutex locker initial unlock");

        locker.lock();
        this->test(locker.isLocked(), "mutex locker explicit lock");

        locker.unlock();
        this->test(! locker.isLocked(), "mutex locker explicit unlock");
        }

    // Test creating a couple of threads, join to them, and verify that they ran.
    // We give each one a different sleep duration, so they behave a little differently.
    // Note that since we don't have additional machinery in place to keep track of
    // threads, we let the thread set our pointer to null when it completes. You must
    // take care not to refer to a thread that has vanished of its own volition. So
    // we check for null before each join, because the thread may be gone by that point.
    // In fact, in our case, thread2 runs for 2 seconds, so by the time thread1 join
    // completes, thread2 is presumably gone.
    bool thread1Flag = false; volatile VThreadID_Type thread1ID = 0; volatile VThreadID_Type thread1Self = 0;
    TestThreadClass* thread1 = new TestThreadClass(this, "1", 4, 1, &thread1Flag, &thread1, &thread1ID, &thread1Self, NULL);
    bool thread2Flag = false; volatile VThreadID_Type thread2ID = 0; volatile VThreadID_Type thread2Self = 0;
    TestThreadClass* thread2 = new TestThreadClass(this, "2", 2, 3, &thread2Flag, &thread2, &thread2ID, &thread2Self, NULL);
    bool thread3Flag = false; volatile VThreadID_Type thread3ID = 0; volatile VThreadID_Type thread3Self = 0;
    TestThreadClass* thread3 = new TestThreadClass(this, "3", 3, 2, &thread3Flag, NULL/*do not self-delete*/, &thread3ID, &thread3Self, NULL);
    
    this->test(thread1->getDeleteAtEnd() == true &&
                thread2->getDeleteAtEnd() == true &&
                thread3->getDeleteAtEnd() == false, "thread delete-at-end flags");
    
    this->test(thread1->isRunning() == false &&
        thread2->isRunning() == false &&
        thread3->isRunning() == false, "thread initial running state");

    (void) thread1->threadID();    // call API to cover it -- result is not of particular use
    
    thread1->start();
    thread2->start();
    thread3->start();
    
    thread2->stop();    // short-circuit its iterations
    
    thread3->setName("thread number 3");
    this->test(thread3->getName() == "thread number 3", "thread naming");
    
    if (thread1 != NULL)
        thread1->join();
    if (thread2 != NULL)
        thread2->join();
    thread3->join();    // thread 3 is set to NOT delete itself
    delete thread3;
    
    this->test(thread1Flag && thread2Flag && thread3Flag, "threads completed");
    
    this->logStatus(VSTRING_FORMAT("thread ids/selfs: %lld/%lld %lld/%lld %lld/%lld",
        (Vs64) thread1ID, (Vs64) thread1Self,
        (Vs64) thread2ID, (Vs64) thread2Self,
        (Vs64) thread3ID, (Vs64) thread3Self));

    VUNIT_ASSERT_EQUAL_LABELED((Vs64) thread1ID, (Vs64) thread1Self, "thread 1 self/id match");
    VUNIT_ASSERT_NOT_EQUAL_LABELED((Vs64) thread1ID, CONST_S64(0), "thread 1 self/id non-zero");
    VUNIT_ASSERT_EQUAL_LABELED((Vs64) thread2ID, (Vs64) thread2Self, "thread 2 self/id match");
    VUNIT_ASSERT_NOT_EQUAL_LABELED((Vs64) thread2ID, CONST_S64(0), "thread 2 self/id non-zero");
    VUNIT_ASSERT_EQUAL_LABELED((Vs64) thread3ID, (Vs64) thread3Self, "thread 3 self/id match");
    VUNIT_ASSERT_NOT_EQUAL_LABELED((Vs64) thread3ID, CONST_S64(0), "thread 3 self/id non-zero");

        { // mutex and locker scope
        // Test the ability to examine a mutex and see if it is locked by the current thread.

        // First, declare and lock a mutex, and see that the API returns true.
        VMutex mutexX("mutexX");
        VUNIT_ASSERT_FALSE_LABELED(mutexX.isLockedByCurrentThread(), "1 - local mutex not locked by current thread");
        mutexX._lock();
        VUNIT_ASSERT_TRUE_LABELED(mutexX.isLockedByCurrentThread(), "2 - local mutex locked by current thread");
        mutexX._unlock();
        VUNIT_ASSERT_FALSE_LABELED(mutexX.isLockedByCurrentThread(), "3 - local mutex not locked by current thread");

        // Next, create another thread that will lock the mutex, and verify that this thread can
        // get the right answer.
        bool threadXFlag = false; volatile VThreadID_Type threadXID = 0; volatile VThreadID_Type threadXSelf = 0;
        TestThreadClass* threadX = new TestThreadClass(this, "4", 4, 1, &threadXFlag, NULL/*do not self-delete*/, &threadXID, &threadXSelf, &mutexX);
        VUNIT_ASSERT_FALSE_LABELED(mutexX.isLockedByCurrentThread(), "4 - shared mutex not locked by current thread");
        
        threadX->start();
        VThread::sleep(VDuration::SECOND()); // Give the thread a second to get going and lock.... it will sleep for 4 seconds once running.
        VUNIT_ASSERT_FALSE_LABELED(mutexX.isLockedByCurrentThread(), "5 - shared mutex not locked by current thread while other thread has it locked");
        
        threadX->join();
        VUNIT_ASSERT_FALSE_LABELED(mutexX.isLockedByCurrentThread(), "6 - shared mutex not locked by current thread after other thread finished");

        // Finally, re-test this thread's ability to detect its own locking of the mutex.
        VUNIT_ASSERT_FALSE_LABELED(mutexX.isLockedByCurrentThread(), "7 - local mutex not locked by current thread");
        mutexX._lock();
        VUNIT_ASSERT_TRUE_LABELED(mutexX.isLockedByCurrentThread(), "8 - local mutex locked by current thread");
        mutexX._unlock();
        VUNIT_ASSERT_FALSE_LABELED(mutexX.isLockedByCurrentThread(), "9 - local mutex not locked by current thread");
        }
    
    }

