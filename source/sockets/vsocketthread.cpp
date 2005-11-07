/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocketthread.h"

#include "vlistenerthread.h"

VSocketThread::VSocketThread(const VString& name, VSocket* socket, VListenerThread* ownerThread)
: VThread(name, kDeleteSelfAtEnd, (ownerThread == NULL) ? NULL : ownerThread->getManagementInterface())
    {
    mSocket = socket;
    mOwnerThread = ownerThread;
    }

VSocketThread::~VSocketThread()
    {
    if (mOwnerThread != NULL)
        mOwnerThread->socketThreadEnded(this);

    delete mSocket;    // socket will close itself on deletion
    }

VSocket* VSocketThread::socket() const
    {
    return mSocket;
    }

VListenerThread* VSocketThread::getOwnerThread() const
    {
    return mOwnerThread;
    }

void VSocketThread::closeAndStop()
    {
    mSocket->close();
    this->stop();
    }

