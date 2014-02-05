/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

#ifndef vsingleton_h
#define vsingleton_h

#include "vshutdownregistry.h"
#include "vmutexlocker.h"
#include "vexception.h"

/**
    @ingroup toolbox
*/

/**
VSingleton is a template class that provides a "singleton holder" that
implements the "singleton" pattern behavior for a class T you write. As of
this version, the only key requirement of class T is that it has a default
constructor, which will be used when VSingleton instantiates it. The behavior
VSingleton gives you is:
<p>
<ul>
<li>There is only 1 instance of T.
<li>The instance of T is not instantiated until the first time it is accessed
(via the instance() method). This is sometimes called "lazy initialization".
<li>The instance of T can be deleted, at which point another instance of
T may be permitted or prohibited.
<li>The instance of T can be set for (almost) automatic deletion during program
shutdown.
<li>Callers access the instance of T by calling the holder's instance() method.
<li>Callers may destroy the instance of T by calling the holder's deleteInstance()
method.
<li>It makes no sense to create multiple holders of the same class T. There will
still only be one instance of T since the holders will share the same static
variables (such as the instance pointer), but the holders' policy flags could
have differing behavior, which would be strange.
</ul>
<p>
You should define T's constructor and destructor private, and the template
class a friend, so that only the template class can create and destroy the
instance of T. Otherwise, the singleton behavior can be circumvented.
<p>
VSingleton can work with the VShutdownRegistry to make sure that the
instance of T is cleaned up at shutdown, so you don't leak global resources
(even in a simple case, a leak-detecting debugger will infer that there is
a leak if the instance of T exists at program exit).
<p>
When you construct the VSingleton<T>, you pass it four "policy" flags:
<p>
<ul>
<li>HolderDeletionPolicy. If you're using the VShutdownRegistry, you need to
specify whether the VSingleton itself can be deleted. (Note that I am
referring to the VSingleton, not the instance of T.) If it's allocated
via "new", it should be deleted, so pass kDeleteHolderAtShutdown. If it's
a global variable, it must not be deleted, so pass kDontDeleteHolderAtShutdown.
(It would be strange to declare it as a local variable! But if you did, it
would have to be flagged like a global variable since it cannot be deleted
with operator delete.) If you specify kDeleteHolderAtShutdown, the
VShutdownRegistry will delete the VSingleton immediately after it tells it
to shutdown().
<li>ThreadSafetyPolicy. If you know that the instance will be created during
the main thread's startup with no possibility of other threads trying to
access/create the instance at the same time, you can avoid the overhead of
locking, in which case pass kSimpleAccess. However, you should not normally
assume this is the case, and should thus normally pass kThreadSafeAccess.
- ShutdownPolicy. If you want the singleton deleted when the program shuts
down, pass kRegisterForShutdown. Otherwise, pass kDontRegisterForShutdown.
<li>ResurrectionPolicy. If you want to allow the instance of T to be re-created
after it was previously created and destroyed, pass kAllowResurrection.
Otherwise, pass kDontAllowResurrection, which will cause a VException to
be thrown if someone tries to access the instance() after it was previously
destroyed. This situation would typically arise if you invoke the
VShutdownRegistry before all non-main threads have terminated, or if some
singleton's shutdown/cleanup procedure access another deleted singleton's
instance. These are typically design bugs that need to be re-thought.
</ul>
<p>
@see VShutdownRegistry
@see IVShutdownHandler
*/
template <class T>
class VSingleton : public IVShutdownHandler {
    public:

        /** Specifies whether the VSingleton can be deleted. */
        enum HolderDeletionPolicy { kDeleteHolderAtShutdown, kDontDeleteHolderAtShutdown };
        /** Specifies whether a mutex is used to access the instance. */
        enum ThreadSafetyPolicy { kThreadSafeAccess, kSimpleAccess };
        /** Specifies whether the instance is shut down via the shutdown registry. */
        enum ShutdownPolicy { kRegisterForShutdown, kDontRegisterForShutdown };
        /** Specifies whether the instance can be re-created after being previously destroyed. */
        enum ResurrectionPolicy { kDontAllowResurrection, kAllowResurrection };

        /**
        Constructs the singleton holder.
        @param    holderDeletionPolicy  specifies whether the holder can be deleted
        @param    threadSafetyPolicy    specifies whether a mutex is used to access the instance
        @param    shutdownPolicy        specifies whether the instance is shut down via the shutdown registry
        @param    resurrectionPolicy    specifies whether the instance can be re-created after being previously destroyed
        */
        VSingleton(HolderDeletionPolicy holderDeletionPolicy,
                   ThreadSafetyPolicy threadSafetyPolicy,
                   ShutdownPolicy shutdownPolicy,
                   ResurrectionPolicy resurrectionPolicy = kDontAllowResurrection)
            : IVShutdownHandler(holderDeletionPolicy == kDeleteHolderAtShutdown)
            , mThreadSafe(threadSafetyPolicy == kThreadSafeAccess)
            , mWantShutdown(shutdownPolicy == kRegisterForShutdown)
            , mAllowResurrection(resurrectionPolicy == kAllowResurrection)
            {
        }

        ~VSingleton() {}

        /**
        Returns a pointer to the instance of T. If the instance does not yet exist, it
        is created. If the resurrection policy prohibits resurrection and the instance
        was previously deleted, a VException is thrown.
        @return the instance of T
        */
        T* instance() {
            VMutexLocker locker(&gMutex, "VSingleton::instance()", mThreadSafe);

            if (gInstance == NULL) {
                if (gInstanceDeleted && !mAllowResurrection) {
                    throw VStackTraceException("VSingleton called with invalid attempt to get instance of deleted singleton.");
                }

                gInstance = new T();

                if (mWantShutdown) {
                    VShutdownRegistry::instance()->registerHandler(this);
                }
            }

            return gInstance;
        }

        /**
        Deletes the instance of T if it exists.
        */
        void deleteInstance() {
            VMutexLocker locker(&gMutex, "VSingleton::deleteInstance()", mThreadSafe);
            delete gInstance;
            gInstance = NULL;
            gInstanceDeleted = true;
        }

    protected:

        /**
        Implementation of IVShutdownHandler interface.
        To shut down the singleton means to delete the instance.
        */
        virtual void _shutdown() {
            this->deleteInstance();
        }

    private:

        static VMutex   gMutex;             ///< Mutex to protect the instance if the thread safety policy says so.
        static T*       gInstance;          ///< Pointer to the instance; NULL until first call to instance().
        static bool     gInstanceDeleted;   ///< True if deleteInstance() has been called; used with resurrection policy.

        bool mThreadSafe;           ///< True if access to the instance is done via a mutex lock.
        bool mWantShutdown;         ///< True if the holder will register with the shutdown registry upon creating the instance.
        bool mAllowResurrection;    ///< True if the instance is allowed to be created after having been previously destroyed.

};

template <class T> VMutex VSingleton<T>::gMutex("VSingleton::gMutex");
template <class T> T* VSingleton<T>::gInstance = NULL;
template <class T> bool VSingleton<T>::gInstanceDeleted = false;

#endif /* vsingleton_h */
