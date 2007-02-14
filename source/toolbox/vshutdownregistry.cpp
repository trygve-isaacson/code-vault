/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5.1
http://www.bombaydigital.com/
*/

#include "vshutdownregistry.h"

#include "vmutexlocker.h"

VShutdownRegistry* VShutdownRegistry::gInstance = NULL;

// This style of static mutex declaration and access ensures correct
// initialization if accessed during the static initialization phase.
static VMutex* _mutexInstance()
    {
    static VMutex gMutex;
    return &gMutex;
    }

// static
VShutdownRegistry* VShutdownRegistry::instance()
    {
    VMutexLocker    locker(_mutexInstance());
    
    if (gInstance == NULL)
        gInstance = new VShutdownRegistry();
    
    return gInstance;
    }

// static
void VShutdownRegistry::shutdown()
    {
    VMutexLocker    locker(_mutexInstance());
    
    if (gInstance != NULL)
        {
        delete gInstance;
        gInstance = NULL;
        }
    }

void VShutdownRegistry::registerHandler(MShutdownHandler* handler)
    {
    VMutexLocker    locker(_mutexInstance());
    
    mHandlers.push_back(handler);
    }

void VShutdownRegistry::registerFunction(shutdownFunction func)
    {
    VMutexLocker    locker(_mutexInstance());
    
    mFunctions.push_back(func);
    }

VShutdownRegistry::~VShutdownRegistry()
    {
    // Note that this is only called via our static shutdown() function,
    // which takes responsibility for thread-safety. Don't lock, or we'll
    // have a deadlock.

    for (ShutdownFunctionList::iterator i = mFunctions.begin(); i != mFunctions.end(); ++i)
        {
        shutdownFunction func = (*i);

        func();
        }

    for (ShutdownHandlerList::iterator i = mHandlers.begin(); i != mHandlers.end(); ++i)
        {
        MShutdownHandler*    handler = (*i);

        handler->_shutdown();

        if (handler->mDeleteAfterShutdown)
            delete handler;

        (*i) = NULL;
        }
    }

