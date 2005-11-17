/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vlistenersocket.h"

#include "vexception.h"
#include "vsocketfactory.h"

VListenerSocket::VListenerSocket(int portNumber, VSocketFactory* factory, int backlog)
: VSocket()
    {
    mPortNumber = portNumber;
    mFactory = factory;
    mListenBacklog = backlog;

    /*
    We need to have our listen() calls timeout if we expect to allow
    other threads (e.g., a thread that is handling a remote server
    management command) to shut us down. Otherwise, we'll be blocked
    on listen() and never get a chance to even check isRunning().
    */
    struct timeval    timeout;
    
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    VListenerSocket::setReadTimeOut(timeout);
    }

VListenerSocket::~VListenerSocket()
    {
    }
        
VSocket* VListenerSocket::accept()
    {
    if (mSockID == kNoSockID)
        {
        throw VException("VListenerSocket::accept called before socket is listening.");
        }

    struct sockaddr_in    clientaddr;
    VSocklenT        clientaddrLength = sizeof(clientaddr);
    VSockID            handlerSockID = kNoSockID;
    VSocket*        handlerSocket = NULL;
    bool            shouldAccept = true;
    int                result = -1;

    if (mReadTimeOutActive)
        {
        /* then we need to do a select call */
        struct timeval    timeout = mReadTimeOut;
        fd_set            readset;

        FD_ZERO(&readset);
        //lint -e573 Signed-unsigned mix with divide"
        FD_SET(mSockID, &readset);

        result = ::select(static_cast<int>(mSockID + 1), &readset, NULL, NULL, &timeout);

        if (result == -1)
            {
            throw VException("VListenerSocket::accept select error, errno=%s", ::strerror(errno));
            }

        //lint -e573 Signed-unsigned mix with divide"
        shouldAccept = ((result > 0) && FD_ISSET(mSockID, &readset));
        }

    if (shouldAccept)
        {
        ::memset(&clientaddr, 0, static_cast<Vu32> (clientaddrLength));
        handlerSockID = ::accept(mSockID, (struct sockaddr*) &clientaddr, &clientaddrLength);
        
        if (handlerSockID < 0)
            {
            throw VException("VListenerSocket::accept accept error, errno=%s", ::strerror(errno));
            }
        else
            {
            handlerSocket = mFactory->createSocket(handlerSockID);
            }
        }

    return handlerSocket;
    }

