/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vlistenerthread.h"
#include "vlistenersocket.h"
#include "vsocketfactory.h"
#include "vsocketthreadfactory.h"
#include "vmanagementinterface.h"
#include "vclientsession.h"
#include "vexception.h"
#include "vmutexlocker.h"
#include "vlogger.h"
#include "vmessageinputthread.h"
#include "vmessageoutputthread.h"
#include <algorithm>

VListenerThread::VListenerThread(const VString& name, bool deleteSelfAtEnd, bool createDetached, VManagementInterface* manager, int portNumber, VSocketFactory* socketFactory, VSocketThreadFactory* threadFactory, VClientSessionFactory* sessionFactory, bool initiallyListening) :
VThread(name, deleteSelfAtEnd, createDetached, manager),
mPortNumber(portNumber),
mShouldListen(initiallyListening),
mSocketFactory(socketFactory),
mThreadFactory(threadFactory),
mSessionFactory(sessionFactory)
// mSocketThreads constructs to empty
// mSocketThreadsMutex constructs to unlocked
    {
    }

VListenerThread::~VListenerThread()
    {
    VLOGGER_DEBUG(VString("VListenerThread '%s' ended.", mName.chars()));
    }

void VListenerThread::stop()
    {
    this->stopListening();
    this->stopAllSocketThreads();

    VThread::stop();
    }

void VListenerThread::run()
    {
    while (this->isRunning())
        {
        if (mShouldListen)
            this->_runListening();
        else
            VThread::sleep(5 * VDuration::SECOND()); // this value limits how quickly we can be shut down
        }
    }

void VListenerThread::socketThreadEnded(VSocketThread* socketThread)
    {
    VMutexLocker                        locker(&mSocketThreadsMutex);
    VSocketThreadPtrVector::iterator    position;
    
    position = std::find(mSocketThreads.begin(), mSocketThreads.end(), socketThread);
    
    if (position != mSocketThreads.end())
        mSocketThreads.erase(position);
    }

int VListenerThread::getPortNumber() const
    {
    return mPortNumber;
    }

VSocketInfoVector VListenerThread::enumerateActiveSockets()
    {
    VSocketInfoVector    info;
    VMutexLocker        locker(&mSocketThreadsMutex);

    for (VSizeType i = 0; i < mSocketThreads.size(); ++i)
        {
        VSocketInfo    oneSocketInfo(*(mSocketThreads[i]->socket()));

        info.push_back(oneSocketInfo);
        }
    
    return info;
    }

void VListenerThread::stopSocketThread(VSocketID socketID, int localPortNumber)
    {
    bool                found = false;
    VMutexLocker        locker(&mSocketThreadsMutex);

    for (VSizeType i = 0; i < mSocketThreads.size(); ++i)
        {
        VSocketThread*    thread = mSocketThreads[i];
        VSocket*        socket = thread->socket();

        if ((socket->getSockID() == socketID) &&
            (socket->getPortNumber() == localPortNumber))
            {
            found = true;
            thread->closeAndStop();
            }
        }
    
    if (! found)
        throw VException("VListenerThread::stopSocketThread did not find a socket with id %d and port %d.", socketID, localPortNumber);
    }

void VListenerThread::stopAllSocketThreads()
    {
    VMutexLocker        locker(&mSocketThreadsMutex);

    for (VSizeType i = 0; i < mSocketThreads.size(); ++i)
        {
        VSocketThread*    thread = mSocketThreads[i];

        thread->closeAndStop();
        }
    }

void VListenerThread::_runListening()
    {
    VListenerSocket*    listenerSocket = NULL;

    if (mManager != NULL)
        mManager->listenerStarting(this);

    try
        {
        listenerSocket = new VListenerSocket(mPortNumber, mSocketFactory);
        listenerSocket->listen();

        if (mManager != NULL)
            mManager->listenerListening(this);

        while (mShouldListen && this->isRunning())
            {
            VSocket*    theSocket = listenerSocket->accept();
            
            if (theSocket != NULL)
                {
                VMutexLocker    locker(&mSocketThreadsMutex);
                
                if (mSessionFactory == NULL)
                    {
                    VSocketThread*    thread = mThreadFactory->createThread(theSocket, this);
                    thread->start();
                    mSocketThreads.push_back(thread);
                    }
                else
                    {
                    VClientSession* session = mSessionFactory->createSession(theSocket, this);
                    // session is responsible for starting the threads it uses
                    VSocketThread* thread;
                    thread = session->getInputThread();
                    if (thread != NULL)
                        mSocketThreads.push_back(thread);
                    thread = session->getOutputThread();
                    if (thread != NULL)
                        mSocketThreads.push_back(thread);

                    mSessionFactory->addSessionToServer(session);
                    }
                }
            else
                {
                /*
                We timed out, which is normal if we have a timeout value.
                As long as we haven't been stopped, we'll try again.
                */
                }
            }
        }
    catch (VException& ex)
        {
        mShouldListen = false;

        VString    message("VListenerThread '%s' _runListening() caught exception #%d '%s'.", mName.chars(), ex.getError(), ex.what());
        VLOGGER_ERROR(message);

        if (mManager != NULL)
            mManager->listenerFailed(this, message);
        }
    catch (...)
        {
        mShouldListen = false;

        VString message("VListenerThread '%s' _runListening() caught unknown exception '%s'.", mName.chars());
        VLOGGER_ERROR(message);

        if (mManager != NULL)
            mManager->listenerFailed(this, message);
        }

    delete listenerSocket;

    if (mManager != NULL)
        mManager->listenerEnded(this);
    }

