/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

#include "vserver.h"

#include "vmutexlocker.h"

VServer::VServer()
    : mSessions()
    , mSessionsMutex("VServer::mSessionsMutex")
    {
}

void VServer::addClientSession(VClientSessionPtr session) {
    VMutexLocker locker(&mSessionsMutex, "VServer::addClientSession()");
    mSessions.push_back(session);
}

void VServer::removeClientSession(VClientSessionPtr session) {
    VMutexLocker locker(&mSessionsMutex, "VServer::removeClientSession()");
    for (VClientSessionList::iterator i = mSessions.begin(); i != mSessions.end(); i++) {
        if ((*i) == session) {
            (void) mSessions.erase(i);
            break;
        }
    }
}
