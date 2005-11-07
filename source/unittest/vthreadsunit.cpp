/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vthreadsunit.h"
#include "vthread.h"
#include "vmutex.h"
#include "vmutexlocker.h"
#include "vsemaphore.h"

class TestThreadClass : public VThread
    {
    public:
    
        TestThreadClass(int numSecondsToSleep, int numIterations, bool* boolToSetOnCompletion, TestThreadClass** thisPtrToNull);
        ~TestThreadClass();
        
        virtual void run();
        
        int     mNumSecondsToSleep;
        int     mNumIterations;
        bool*    mBoolToSetOnCompletion;
        TestThreadClass**    mThisPtrToNull;
    };

TestThreadClass::TestThreadClass(int numSecondsToSleep, int numIterations, bool* boolToSetOnCompletion, TestThreadClass** thisPtrToNull) :
VThread("TestThreadClass", thisPtrToNull != NULL, kCreateThreadJoinable, NULL),
mNumSecondsToSleep(numSecondsToSleep),
mNumIterations(numIterations),
mBoolToSetOnCompletion(boolToSetOnCompletion),
mThisPtrToNull(thisPtrToNull)
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
    while ((mNumIterations > 0) && this->isRunning())
        {
        // We are now running in our own thread. Let's sleep a little...
        VThread::sleepMilliseconds(mNumSecondsToSleep * 1000);

        --mNumIterations;
        }

    // Now our thread will finish, terminate, and delete this.

    // We set the creator's boolean so it can verify that we got here.
    *mBoolToSetOnCompletion = true;
    }

VThreadsUnit::VThreadsUnit(bool logOnSuccess, bool throwOnError)
: VUnit("VThreadsUnit", logOnSuccess, throwOnError)
    {
    }

void VThreadsUnit::run()
    {
    // Test the basic behavior of mutex locking and unlocking.

    VMutex mutex1;

        {    
        VMutexLocker locker(&mutex1);
        this->test(mutex1.isLocked() && locker.isLocked(), "mutex locker initial lock");
        }

    this->test(! mutex1.isLocked(), "mutex locker scope unlock");

        {    
        VMutexLocker locker(&mutex1, false);
        this->test(! mutex1.isLocked() && ! locker.isLocked(), "mutex locker initial unlock");

        locker.lock();
        this->test(mutex1.isLocked() && locker.isLocked(), "mutex locker explicit lock");

        locker.unlock();
        this->test(! mutex1.isLocked() && ! locker.isLocked(), "mutex locker explicit unlock");
        }

    this->test(! mutex1.isLocked(), "mutex locker scope leave unlock");

    // Test creating a couple of threads, join to them, and verify that they ran.
    // We give each one a different sleep duration, so they behave a little differently.
    // Note that since we don't have additional machinery in place to keep track of
    // threads, we let the thread set our pointer to null when it completes. You must
    // take care not to refer to a thread that has vanished of its own volition. So
    // we check for null before each join, because the thread may be gone by that point.
    // In fact, in our case, thread2 runs for 2 seconds, so by the time thread1 join
    // completes, thread2 is presumably gone.
    bool thread1Flag = false;
    TestThreadClass* thread1 = new TestThreadClass(4, 1, &thread1Flag, &thread1);
    bool thread2Flag = false;
    TestThreadClass* thread2 = new TestThreadClass(2, 3, &thread2Flag, &thread2);
    bool thread3Flag = false;
    TestThreadClass* thread3 = new TestThreadClass(3, 2, &thread3Flag, NULL);
    
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
    this->test(thread3->name() == "thread number 3", "thread naming");
    
    if (thread1 != NULL)
        thread1->join();
    if (thread2 != NULL)
        thread2->join();
    thread3->join();    // thread 3 is set to NOT delete itself
    delete thread3;
    
    this->test(thread1Flag && thread2Flag && thread3Flag, "threads completed");

    }

