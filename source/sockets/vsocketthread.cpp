/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocketthread.h"

#include "vlistenerthread.h"

VSocketThread::VSocketThread(const VString& name, VSocket* socket, VListenerThread* ownerThread) :
VThread(name, kDeleteSelfAtEnd, kCreateThreadDetached, (ownerThread == NULL) ? NULL : ownerThread->getManagementInterface()),
mSocket(socket),
mOwnerThread(ownerThread)
    {
    }

VSocketThread::~VSocketThread()
    {
    if (mOwnerThread != NULL)
        {
        // Prevent all exceptions from escaping destructor.
        try
            {
            mOwnerThread->socketThreadEnded(this);
            }
        catch (...) {}
        }

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

