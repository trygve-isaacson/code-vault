/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

#ifndef vshutdownregistry_h
#define vshutdownregistry_h

#include "vtypes.h"
#include "vmutex.h"

#include <vector>

/**
    @ingroup toolbox
*/

/**
MShutdownHandler is the mix-in class (interface) for objects
that are registered with the shutdown registry. The concrete
subclass must simply implement the _shutdown() function and
do its cleanup there.
@see VShutdownRegistry
*/
class MShutdownHandler
    {
    public:
        
        MShutdownHandler(bool deleteHandlerAfterShutdown=true) :
            mDeleteAfterShutdown(deleteHandlerAfterShutdown) {}
        
        virtual ~MShutdownHandler() {}

    protected:

        /**
        Called by the shutdown registry to tell the object to clean up
        as part of shutdown. The typical usage is to free any class
        static (global) data owned by some class. See the
        VSingletonShutdownHandler template class for an example.
        */        
        virtual void _shutdown() = 0;

    private:
    
        bool mDeleteAfterShutdown;
        
        friend class VShutdownRegistry; // give it access to _shutdown() and mDeleteAfterShutdown
    };

/** This type stores a list of shutdown handler pointers. */
typedef std::vector<MShutdownHandler*> ShutdownHandlerList;

/** This defines the function type that can be installed as a simple
shutdown hook. */
typedef void (*shutdownFunction)();
/** This type stores a list of shutdown function pointers. */
typedef std::vector<shutdownFunction> ShutdownFunctionList;

/**
The shutdown registry allows you to register a handler object that
will be called to clean up when the program shuts down. This is intended
to be used as a clean way of allow you to free any "global" resources
you've allocated, that otherwise would have to be automatically deleted
by the operating system as part of program termination. That is, it's
for things like freeing memory that you want to not look like a memory leak
by any diagnostics that check for unfreed memory on quit.
*/
class VShutdownRegistry
    {
    public:
    
        /**
        Returns the global shutdown registry instance, creating it if
        this is the first call to access it.
        @return a pointer to the global registry instance
        */
        static VShutdownRegistry* instance();
        
        /**
        Invokes shutdown by deleting the registry. This is the better than
        simply deleting the registry instance because it's done in a threadsafe
        fashion that guarantees singleton behavior even if inadvertently called
        from multiple places and multiple threads.
        */
        static void shutdown();
        
        /**
        Registers a shutdown handler with the shutdown registry. The
        handler's _shutdown() will be called to shut down when the
        registry is told to shut down.
        */
        void registerHandler(MShutdownHandler* handler);
    
        /**
        Registers a shutdown function with the shutdown registry. The
        function will be called to shut down when the registry is
        told to shut down.
        */
        void registerFunction(shutdownFunction func);
    
    private:
    
        // Declare our constructor and destructor private so that they
        // can only be called via our static functions (which are thread-safe).
        VShutdownRegistry() : mHandlers(), mFunctions() {}
        ~VShutdownRegistry();
        
        static VMutex                gMutex;    ///< Mutex to make our list threadsafe.
        static VShutdownRegistry*    gInstance; ///< The registry as a singleton object.
        
        ShutdownHandlerList     mHandlers;  ///< The handlers that have been registered with us.
        ShutdownFunctionList    mFunctions; ///< The functions that have been registered with us.
    };

/**
This template class allows you to easily register a singleton class T with the
shutdown registry, so that it will get cleaned up at shutdown. The requirements
for the class T are simple:
<p>
<ol>
<li>The class must implement this (public or friend-accessible) function:<br>
<code>
static void deleteInstance();
</code>
<li>When you create the instance, also create the shutdown handler; that is, you only
create one shutdown handler, so you may as well do it when you create the singleton
instance:<br>
<code>    
gInstance = new MyClass(params);
new VSingletonShutdownHandler<MyClass>();
</code>
<li>If you wish to allow deleteInstance() or instance() to be thread-safe, you need to
protect the instance with a VMutex. If you can be sure the first
call to instance() will be from the main startup thread, you don't need a mutex for it;
if you trigger the shutdown registry cleanup after all threads but main have terminated,
then you don't need a mutex for deleteInstance() either. Generally I would suggest
using a mutex unless you know that removing it is a useful performance optimization and
is safe to do.
</ol>
*/
template <class T>
class VSingletonShutdownHandler : public MShutdownHandler
    {
    public:
    
        /**
        Constructs the handler.
        @param    deleteHandlerAfterShutdown    true if the handler (not the T instance)
            can be deleted after _shutdown() is called; this is true for
            heap objects, false for global variables
        */
        VSingletonShutdownHandler(bool deleteHandlerAfterShutdown=true) :
        MShutdownHandler(deleteHandlerAfterShutdown)
            {
            VShutdownRegistry::instance()->registerHandler(this);
            }

        virtual ~VSingletonShutdownHandler() {}

    protected:

        /**
        Implementation of MShutdownHandler interface.
        To shut down the singleton means to delete the instance.
        */
        virtual void _shutdown() { T::deleteInstance(); }
    };

#endif /* vshutdownregistry_h */
