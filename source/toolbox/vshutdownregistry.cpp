/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

#include "vshutdownregistry.h"

#include "vmutexlocker.h"

VMutex VShutdownRegistry::smMutex;
VShutdownRegistry* VShutdownRegistry::smInstance = NULL;

// static
VShutdownRegistry* VShutdownRegistry::instance()
    {
    VMutexLocker    locker(&smMutex);
    
    if (smInstance == NULL)
        smInstance = new VShutdownRegistry();
    
    return smInstance;
    }

// static
void VShutdownRegistry::shutdown()
    {
    VMutexLocker    locker(&smMutex);
    
    if (smInstance != NULL)
        {
        delete smInstance;
        smInstance = NULL;
        }
    }

void VShutdownRegistry::registerHandler(MShutdownHandler* handler)
    {
    VMutexLocker    locker(&smMutex);
    
    mHandlers.push_back(handler);
    }

VShutdownRegistry::~VShutdownRegistry()
    {
    // Note that this is only called via our static shutdown() function,
    // which takes responsibility for thread-safety. Don't lock, or we'll
    // have a deadlock.

    for (ShutdownHandlerList::iterator i = mHandlers.begin(); i != mHandlers.end(); ++i)
        {
        MShutdownHandler*    handler = (*i);

        handler->shutdown();

        if (handler->mDeleteAfterShutdown)
            delete handler;

        (*i) = NULL;
        }
    }

