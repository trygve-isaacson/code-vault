/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

#include "vserver.h"

#include "vmutexlocker.h"

VServer::VServer()
    : mTerminatedSessions()
    , mTerminatedSessionsMutex("VServer::mTerminatedSessionsMutex")
    {
}

void VServer::clientSessionTerminating(VClientSession* session) {
    VMutexLocker locker(&mTerminatedSessionsMutex, "VServer::clientSessionTerminating()");

    // If the session isn't already on the list, add it.
    VClientSessionList::const_iterator position = std::find(mTerminatedSessions.begin(), mTerminatedSessions.end(), session);
    if (position == mTerminatedSessions.end()) {
        mTerminatedSessions.push_back(session);
    }
}

void VServer::garbageCollectTerminatedSessions() {
    this->_garbageCollectTerminatedSessions();
}

bool VServer::hasUncollectedTerminatedSessions() const {
    VMutexLocker locker(&mTerminatedSessionsMutex, "VServer::hasUncollectedTerminatedSessions()");
    return (mTerminatedSessions.size() > 0);
}

void VServer::_garbageCollectTerminatedSessions() {
    VMutexLocker locker(&mTerminatedSessionsMutex, "VServer::_garbageCollectTerminatedSessions()");

    // Walk the list backwards so that we can simply erase() as we go.
    int numSessions = static_cast<int>(mTerminatedSessions.size());
    for (int i = numSessions - 1; i >= 0; --i) {
        VClientSession* session = mTerminatedSessions[i];
        VMutexLocker sessionLocker(&session->mReferenceCountMutex, "VServer::_garbageCollectTerminatedSessions()");

        VString sessionName = session->getName(); // local copy since we may be deleting it below

        if (session->mReferenceCount == 0) {
            sessionLocker.unlock();
            delete session;
            (void) mTerminatedSessions.erase(mTerminatedSessions.begin() + i);
            VLOGGER_TRACE(VSTRING_FORMAT("VServer::_garbageCollectTerminatedSessions: Deleted session '%s'.", sessionName.chars()));
        } else {
            VLOGGER_TRACE(VSTRING_FORMAT("VServer::_garbageCollectTerminatedSessions: Session '%s' still has reference count %d.", sessionName.chars(), session->mReferenceCount));
        }
    }
}

