/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocketthread.h"

#include "vlistenerthread.h"

VSocketThread::VSocketThread(const VString& threadBaseName, VSocket* socket, VListenerThread* ownerThread)
    : VThread(
        VSTRING_FORMAT("%s[%s:%d]", threadBaseName.chars(), (socket == NULL ? "?":socket->getHostIPAddress().chars()), (socket == NULL ? 0:socket->getPortNumber())),
        VSTRING_FORMAT("vault.messages.%s.%s", threadBaseName.chars(), (socket == NULL ? "?":(VLogger::getCleansedLoggerName(socket->getHostIPAddress())).chars())),
        kDeleteSelfAtEnd, kCreateThreadDetached, (ownerThread == NULL) ? NULL : ownerThread->getManagementInterface())
    , mSocket(socket)
    , mOwnerThread(ownerThread)
    {
}

VSocketThread::~VSocketThread() {
    if (mOwnerThread != NULL) {
        // Prevent all exceptions from escaping destructor.
        try {
            mOwnerThread->socketThreadEnded(this);
        } catch (...) {}
    }

    delete mSocket;    // socket will close itself on deletion
}

VSocket* VSocketThread::getSocket() const {
    return mSocket;
}

VListenerThread* VSocketThread::getOwnerThread() const {
    return mOwnerThread;
}

void VSocketThread::closeAndStop() {
    if (mSocket != NULL) {
        mSocket->close();
    }

    this->stop();
}

