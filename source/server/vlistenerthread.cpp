/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vlistenerthread.h"
#include "vtypes_internal.h"

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

VListenerThread::VListenerThread(const VString& threadBaseName, bool deleteSelfAtEnd, bool createDetached, VManagementInterface* manager, int portNumber, const VString& bindAddress, VSocketFactory* socketFactory, VSocketThreadFactory* threadFactory, VClientSessionFactory* sessionFactory, bool initiallyListening)
    : VThread(threadBaseName, VSTRING_FORMAT("vault.messages.VListenerThread.%s.%d", threadBaseName.chars(), portNumber), deleteSelfAtEnd, createDetached, manager)
    , mPortNumber(portNumber)
    , mBindAddress(bindAddress)
    , mShouldListen(initiallyListening)
    , mSocketFactory(socketFactory)
    , mThreadFactory(threadFactory)
    , mSessionFactory(sessionFactory)
    , mSocketThreads()
    , mSocketThreadsMutex(VSTRING_FORMAT("VListenerThread(%s)::mSocketThreadsMutex", threadBaseName.chars()))
    {
}

VListenerThread::~VListenerThread() {
    VLOGGER_NAMED_DEBUG(mLoggerName, VSTRING_FORMAT("VListenerThread '%s' ended.", mName.chars()));

    // Make sure any of socket threads still alive no longer reference us.
    VMutexLocker locker(&mSocketThreadsMutex, VSTRING_FORMAT("[%s]VListenerThread::socketThreadEnded()", this->getName().chars()));
    for (VSocketThreadPtrVector::const_iterator i = mSocketThreads.begin(); i != mSocketThreads.end(); ++i) {
        (*i)->mOwnerThread = NULL;
    }
}

void VListenerThread::stop() {
    this->stopListening();
    this->stopAllSocketThreads();

    VThread::stop();
}

void VListenerThread::run() {
    while (this->isRunning()) {
        if (mShouldListen) {
            this->_runListening();
        } else {
            VThread::sleep(5 * VDuration::SECOND()); // this value limits how quickly we can be shut down
        }
    }
}

void VListenerThread::socketThreadEnded(VSocketThread* socketThread) {
    VMutexLocker                        locker(&mSocketThreadsMutex, VSTRING_FORMAT("[%s]VListenerThread::socketThreadEnded()", this->getName().chars()));
    VSocketThreadPtrVector::iterator    position;

    position = std::find(mSocketThreads.begin(), mSocketThreads.end(), socketThread);

    if (position != mSocketThreads.end()) {
        mSocketThreads.erase(position);
    }
}

int VListenerThread::getPortNumber() const {
    return mPortNumber;
}

VSocketInfoVector VListenerThread::enumerateActiveSockets() {
    VSocketInfoVector   info;
    VMutexLocker        locker(&mSocketThreadsMutex, VSTRING_FORMAT("[%s]VListenerThread::enumerateActiveSockets()", this->getName().chars()));

    for (VSizeType i = 0; i < mSocketThreads.size(); ++i) {
        VSocketInfo oneSocketInfo(*(mSocketThreads[i]->getSocket()));

        info.push_back(oneSocketInfo);
    }

    return info;
}

void VListenerThread::stopSocketThread(VSocketID socketID, int localPortNumber) {
    bool            found = false;
    VMutexLocker    locker(&mSocketThreadsMutex, VSTRING_FORMAT("[%s]VListenerThread::stopSocketThread()", this->getName().chars()));

    for (VSizeType i = 0; i < mSocketThreads.size(); ++i) {
        VSocketThread*  thread = mSocketThreads[i];
        VSocket*        socket = thread->getSocket();

        if ((socket->getSockID() == socketID) &&
                (socket->getPortNumber() == localPortNumber)) {
            found = true;
            thread->closeAndStop();
        }
    }

    if (! found) {
        throw VStackTraceException(VSTRING_FORMAT("VListenerThread::stopSocketThread did not find a socket with id %d and port %d.", socketID, localPortNumber));
    }
}

void VListenerThread::stopAllSocketThreads() {
    VMutexLocker locker(&mSocketThreadsMutex, VSTRING_FORMAT("[%s]VListenerThread::stopAllSocketThreads()", this->getName().chars()));

    for (VSizeType i = 0; i < mSocketThreads.size(); ++i) {
        VSocketThread* thread = mSocketThreads[i];

        thread->closeAndStop();
    }
}

void VListenerThread::_runListening() {
    VListenerSocket* listenerSocket = NULL;

    if (mManager != NULL) {
        mManager->listenerStarting(this);
    }

    VString exceptionMessage; // filled in if catch block entered
    try {
        listenerSocket = new VListenerSocket(mPortNumber, mBindAddress, mSocketFactory);
        listenerSocket->listen();

        if (mManager != NULL) {
            mManager->listenerListening(this);
        }

        while (mShouldListen && this->isRunning()) {
            VSocket* theSocket = listenerSocket->accept();

            if (theSocket != NULL) {
                try {
                    VMutexLocker locker(&mSocketThreadsMutex, VSTRING_FORMAT("[%s]VListenerThread::_runListening()", this->getName().chars()));

                    if (mSessionFactory == NULL) {
                        VSocketThread* thread = mThreadFactory->createThread(theSocket, this);
                        thread->start(); // throws if can't create OS thread
                        mSocketThreads.push_back(thread);
                    } else {
                        VClientSessionPtr session = mSessionFactory->createSession(theSocket, this); // throws if can't create OS thread(s)
                        VSocketThread* thread;
                        thread = session->getInputThread();
                        if (thread != NULL) {
                            mSocketThreads.push_back(thread);
                        }

                        thread = session->getOutputThread();
                        if (thread != NULL) {
                            mSocketThreads.push_back(thread);
                        }

                        mSessionFactory->addSessionToServer(session);
                    }
                } catch (const VException& ex) {
                    // Likely cause: Failure in starting OS thread. Log, but keep listening.
                    VLOGGER_ERROR(VSTRING_FORMAT("[%s]VListenerThread::_runListening: Unable to create new session: Error %d. %s", this->getName().chars(), ex.getError(), ex.what()));
                    delete theSocket;
                }
            } else {
                /*
                We timed out, which is normal if we have a timeout value.
                As long as we haven't been stopped, we'll try again.
                */
            }
        }
    } catch (const VException& ex) {
        exceptionMessage.format("[%s]VListenerThread::_runListening() caught exception #%d '%s'.", mName.chars(), ex.getError(), ex.what());
    } catch (const std::exception& ex) {
        exceptionMessage.format("[%s]VListenerThread::_runListening() caught exception '%s'.", mName.chars(), ex.what());
    } catch (...) {
        exceptionMessage.format("[%s]VListenerThread::_runListening() caught unknown exception.", mName.chars());
    }

    if (exceptionMessage.isNotEmpty()) {
        mShouldListen = false;
        VLOGGER_NAMED_ERROR(mLoggerName, exceptionMessage);
        if (mManager != NULL) {
            mManager->listenerFailed(this, exceptionMessage);
        }
    }

    delete listenerSocket;

    if (mManager != NULL) {
        mManager->listenerEnded(this);
    }
}

