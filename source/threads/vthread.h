/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

#ifndef vthread_h
#define vthread_h

/** @file */

#include "vthread_platform.h"

#include "vstring.h"
#include "vmutex.h"
#include "vinstant.h"

class VManagementInterface;
class VThreadActionListener;

/**

    @defgroup vthread Vault Threads

    <h3>Overview</h3>
    
    The Vault's thread-related facilities make it easy to create multiple
    threads of concurrent execution, starting with VThread. Care is still
    needed to make those threads safely cooperate with shared data, and you
    can use VMutex and VMutexLocker to implement that consistently and
    safely. Finally, VSemaphore can be used to implement message-passing
    between threads.
    
    <h4>Threads</h4>
    
    VThread is the abstract base class from which you will derive your
    thread classes. Your subclass just needs to override the run()
    method. Your code there is executed in its own thread and that
    thread lives until the run() method returns. After instantiating a
    thread object, call its start()
    method to get it running. Call its join() method to cause the
    calling thread to block until the thread completes. Call its
    stop() method to notify it that it should complete at its next
    opportunity. See the VThread documentation for complete details
    on all that.
    
    <h4>Locking</h4>
    
    VMutex is a mutual exclusion lock for synchronization. Use a mutex to
    prevent multiple threads from accessing the same resource at the same time.
    However, the code that attempts to access the lock should usually do so via
    a VMutexLocker object (see below) to guarantee correct unlocking behavior.
    You'll usually declare a VMutex as an instance variable of a class
    that needs to synchronize access to one of its resources. That mutex
    is associated with a particular resource, and callers must aqcuire
    the lock before accessing the resource. When an attempt is made to
    acquire the lock, the calling thread blocks until it is able to
    acquire the lock; only one thread can own the lock at any given
    moment in time.
    
    VMutexLocker is a helper class for ensuring proper lock/unlock behavior
    around a VMutex object. The code that needs to acquire a VMutex lock
    will usually declare a temporary VMutexLocker object on the stack to
    acquire the lock, and the lock will be released when the VMutexLocker
    destructs, or earlier via an explicit call. This ensures proper unlocking
    even if an exception is thrown by some called function. Otherwise, the
    caller would have to be sure to catch all exceptions and call unlock()
    on the mutex object, which leads to ugly and error-prone code. VMutexLocker
    make the correct behavior trivial. See the VMutexLocker documentation for details.
    
    <h4>Signaling</h4>
    
    VSemaphore is a mechanism for allowing threads to communicate by waiting
    for signals from each other; while a thread is waiting on a semaphore, it
    is blocked and does not consume the CPU. Another thread can signal the
    semaphore, causing exactly one of the waiting threads to wake up. A thread
    that is awakened simply is unblocked from its wait call. A semaphore is
    often used as a way of threads posting messages to each other, such that
    a receiving thread sleeps if there are no messages for it to process, yet
    wakes up the moment a message is available. See the VSemaphore documentation
    for details.
    
*/
    
/**
    @ingroup vthread
*/

/**
VThread is class that provides an easy way to create a thread of execution.

To start a thread, simply instantiate a VThread (well, actually a subclass
of VThread that contains the desired code), and call its start() method.
The start() method actually invokes the thread's main function, which in
turn invokes the VThread object's run() method, which runs to completion.

If you want to block while a thread is alive, you call the VThread object's
join() method. This is typically used to have a server that creates a bunch
of threads and then runs until they have all completed or been killed; the
server's main thread simply joins to each thread, and this causes it to
block on each join() call until that thread has finished. This is far better
than cycling through a while loop waiting until you notice that all of your
thread objects have been removed. Pass kCreateThreadJoinable for
createDetached in the constructor to make the thread joinable.
        
There is no safe way to remotely "force-kill" another thread. You can only
stop() it and let it notice that it has been stopped. As noted above, if you
need to wait until thread x has completed, call x->join(). Of course, if the
other thread is calling both x->stop() and x->join(), it must call x->stop()
first because x->join() blocks until thread x has been completed, and if you
call x->join() first, you will never reach your call to x->stop().

There are two typical reasons why you would want to stop a thread,
and you need to design such threads to be stoppable:

1. A socket listener thread should be stoppable so that you can
disable the service on the socket without bringing down the entire
server and the ability to remotely re-enable the service. This is
easily accomplished by having the listener thread use a timeout value
for accept(), and check the isRunning() property before cycling
through to accept again. The timeout value you use will become the
latency of the disabling action.

2. A long-running background operation in the UI should often be
stoppable via a progress dialog with a cancel button. This is easily
accomplished by having the operation's thread periodically check the
isRunning() property and cleanly end when it detects that it has
been stopped. If the operation is primarily a network transfer or
file i/o operation, this can often be done in the loop that reads or
writes a chunk of data at a time, when it updates the progress
information.
*/
class VThread
    {
    public:

        // Constants to pass for VThread constructor deleteSelfAtEnd parameter:
        static const bool kDeleteSelfAtEnd = true;        ///< The thread main deletes the VThread at end.
        static const bool kDontDeleteSelfAtEnd = false;    ///< The thread main does not delete the VThread at end.
    
        // Constants to pass for VThread constructor createDetached parameter:
        static const bool kCreateThreadDetached = true;        ///< The thread will be created in detached state.
        static const bool kCreateThreadJoinable = false;    ///< The thread will be created in joinable state.
    
        /**
        Constructs the thread object in stopped state.
        @param    name            a name for the thread, useful for debugging purposes
        @param    deleteSelfAtEnd    true if threadMain() should delete this obj when
                                run() completes; pass kDeleteSelfAtEnd or kDontDeleteSelfAtEnd.
                                If you use kDeleteSelfAtEnd then you need to be careful
                                not to reference the VThread while it is destructing (for
                                example, by joining to it in a non-threadsafe way); if you
                                use kDontDeleteSelfAtEnd, you need to delete the VThread after
                                it has finished running.
        @param    createDetached    true to create the thread in detached state; false if not.
                                Generally, if you aren't joining to a thread, it should be
                                detached, and vice-versa. If a non-detached, non-joined
                                thread ends, it will leak system resources (depending on
                                the platform implementation).
        @param    manager            the object that receives notifications for this thread, or NULL
        */
        VThread(const VString& name, bool deleteSelfAtEnd, bool createDetached, VManagementInterface* manager);
        /**
        Destructor.
        */
        virtual ~VThread();
        
        /**
        Starts the thread by creating whatever OS-specific resources are
        necessary, and invoking the thread main, resulting in the
        VThread subclass' run() method being called.
        */
        virtual void start();
        /**
        Stops the thread by setting its mIsRunning flag to false; it is
        up to the thread's own code to look at this flag and return from
        its run() method.
        */
        virtual void stop();
        /**
        Override required in each VThread subclass, run() does whatever the
        thread is designed to do, and returns when the thread has completed
        its task or detects that isRunning() returns false. If the
        thread is designed to be stoppable from another thread, then its run()
        method must check the isRunning() property periodically and return
        when the property becomes false.
        */
        virtual void run() = 0;
        
        /**
        Returns the underlying OS thread ID. On each OS this type is suitable
        for supplying to OS-specific thread functions.
        @return    the thread ID
        */
        VThreadID_Type threadID() const;
        /**
        Returns true if the thread should still be running, false if it has
        been set to stop and should return from its run() method. Also returns
        false during the initialization time between instantiation and when
        run() is invoked.
        @return    true if the thread is running, false if stopped or stop() has
                been invoked
        */
        bool isRunning() const;
        /**
        Blocks the calling thread until this VThread has completed.
        @return    true if the join call succeeds, false upon error
        */
        bool join();
        /**
        Returns the value of the "delete at end" property. If this property is
        true, then the thread main function will delete the VThread object when
        it has completed.
        @return    the delete at end property
        */
        bool getDeleteAtEnd() const;
        /**
        Returns the thread's management interface object (which may be NULL).
        @return the manager
        */
        VManagementInterface* getManagementInterface() const;
        
        /**
        Returns the thread name (useful for debugging).
        @return the thread name
        */
        const VString& name() const;
        /**
        Sets the thread name (useful for debugging). Sometimes you don't have all
        the information you need to generate a good name at time of construction,
        so you can set it later by calling this method. If you want the name to
        be really useful when debugging (or looking at log files) as a unique
        identifier (to tell threads apart), make it unique; one way would be to
        use the object's address (e.g., VString("0x%08X", aThread)); if the thread
        is related to a socket, a more useful name would be a prefix plus the
        client IP and port (e.g., "INPUT:127.0.0.1:3922" for an input thread).
        @param    name    a name for this thread
        */
        void setName(const VString& threadName);
        
        /**
        The main function that invokes the thread's run() and cleans up when
        it returns.
        @param    arg    we use this parameter for the VThread object pointer
        */
        static void* threadMain(void* arg);
        
        /**
        Returns thread statistics counters for debugging/diagnostic purposes.
        @see the statistics fields comments for meaning of each value
        */
        static void getThreadStatistics(int& numVThreads, int& numThreadMains, int& numVThreadsCreated, int& numThreadMainsStarted, int& numVThreadsDestructed, int& numThreadMainsCompleted);

        /* PLATFORM-SPECIFIC STATIC FUNCTIONS --------------------------------
        The remaining functions defined here are the low-level interfaces to
        the platform-specific threading APIs. These are implemented in each
        platform-specific version of vthread_platform.cpp.
        */
        
        typedef void* (*threadMainFunction)(void* param);

        /**
        Starts up a new running thread.
        Wrapper on Unix for pthread_create.
        @param    threadID            pointer to where to return the new thread's ID
        @param    createDetached        true to create the thread in detached state; false if not.
        @param    threadMainProcPtr    the thread main function that will be invoked
        @param    threadArgument        the argument to be passed to the thread main
        @return true on success; false if there was an error creating the thread
        */
        static bool threadCreate(VThreadID_Type* threadID, bool createDetached, threadMainFunction threadMainProcPtr, void* threadArgument);

        /**
        Terminates the current thread. This could be called from anywhere, but
        the VThread model is to gracefully stop() the thread; so VThread::exit()
        is called by the thread main function at its end, and should probably
        not be called from anywhere else.
        Wrapper on Unix for pthread_exit.
        */
        static void threadExit();

        /**
        Blocks the calling thread until the specified thread ends.
        Wrapper on Unix for pthread_join.
        @param    threadID    ID of the thread to wait on
        @param    exitValue    pointer to storage for thread exit value
        @return true on success; false on failure
        */
        static bool threadJoin(VThreadID_Type threadID, void** value);
        
        /**
        Marks the specified thread's storage to be reclaimed when the thread terminates.
        This function does not terminate the thread.
        Wrapper on Unix for pthread_detach.
        @param    threadID    ID of the thread to detach
        */
        static void threadDetach(VThreadID_Type threadID);

        /**
        Returns the ID of the currently executing thread (the caller's thread).
        Wrapper on Unix for pthread_self.
        @return the current thread ID
        */
        static VThreadID_Type threadSelf();
        
        /**
        Sets the current thread's priority, specifying the Unix nice level.
        Wrapper for Unix setpriority using PRIO_PROCESS.
        @param    level    the nice level
        @return true on success; false on failure
        */
        static bool setPriority(int nice);

        /**
        Blocks the current thread for a specified number of milliseconds.
        The thread will resume execution after approximately that amount
        of time, although the exact runtime behavior depends on the
        system load.
        @param    interval    the amount of time to sleep for
        */
        static void sleep(const VDuration& interval);
        
        /**
        Yields to other threads. This is a way of the calling thread
        being more cooperative with other threads. Unfortunately, the
        behavior is very different on each platform due to different
        facilities available.
        */
        static void yield();
        
        /**
        Defines the actions indicated in calls to updateThreadStats.
        */
        enum eThreadAction { eCreated, eDestroyed, eMainStarted, eMainCompleted };
        
        /**
        Call this to assign a thread action listener; you MUST call this
        later with NULL before exiting if the listener object has a shorter
        lifetime than all thread actions.
        @param listener the listener to be called with thread actions, or NULL
                        to de-register so no actions cause calls to a listener
        */
        static void setActionListener(VThreadActionListener* listener);

    protected:
    
        bool                    mIsDeleted;      ///< For debugging purposes it's useful to detect when an attempt is made to delete a thread twice.
        VString                 mName;           ///< For debugging purposes it's very useful to give each thread a name.
        bool                    mDeleteAtEnd;    ///< True if threadMain should delete this obj when it returns from run().
        bool                    mCreateDetached; ///< True if the thread is created in detached state.
        VManagementInterface*   mManager;        ///< The VManagementInterface that manages us, or NULL.
        VThreadID_Type          mThreadID;       ///< The OS-specific thread ID value.
        bool                    mIsRunning;      ///< The running state of the thread (@see isRunning()).
    
        // These static members track and control the existence of threads we create.
        static int smNumVThreads;               ///< The number of VThread objects that currently exist.
        static int smNumThreadMains;            ///< The number of VThread::main() functions currently underway.
        static int smNumVThreadsCreated;        ///< The total number of VThread objects ever created.
        static int smNumThreadMainsStarted;     ///< The total number of VThread::main() functions ever entered.
        static int smNumVThreadsDestructed;     ///< The total number of VThread objects ever destructed.
        static int smNumThreadMainsCompleted;   ///< The total number of VThread::main() functions ever completed.

        static VThreadActionListener* smActionListener; ///< For now, there can only be 1; if more are needed, make this a collection.

        /**
        Updates the thread statistics based on the specified action happening.
        @param    action    what happened (@see eThreadAction)
        */
        static void _updateThreadStatistics(eThreadAction action);

    private:
    
        // Prevent copy construction and assignment since there is no provision for sharing the underlying thread.
        VThread(const VThread& other);
        VThread& operator=(const VThread& other);
    };

/**
VThreadActionListener is a notification interface that you can implement and
register with VThread::registerActionListener, in order to be notified of
thread lifecycle events, for purposes of tracking thread statistics. The
threadAction() method is called any time a thread is created or destroyed,
or a thread main is started or completes.
*/
class VThreadActionListener
    {
    public:
    
        VThreadActionListener() {}
        virtual ~VThreadActionListener() {}
        
        virtual void threadAction(VThread::eThreadAction action) = 0;
    };

#endif /* vthread_h */

