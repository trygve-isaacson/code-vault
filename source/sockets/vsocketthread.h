/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vsocketthread_h
#define vsocketthread_h

/** @file */

#include "vthread.h"

class VSocket;
class VListenerThread;

/**
    @ingroup vsocket
*/

/**
VSocketThread is an abstract base class, derived from VThread, that has
both a VSocket, on which it presumably communicates with a client, and
an owner VThread, which is presumably the VListenerThread that it.

If a subclass so chooses, it can arrange for the owner thread class to
keep track of the threads it owns, and for the owned threads to notify
the owner when they terminate. VListenerThread already implements the owner
side of this arrangement.

@see    VListenerThread
*/
class VSocketThread : public VThread {
    public:

        /**
        Constructs the socket thread with the specified socket and
        owner thread.

        I decided to make the ownerThread a VListenerThread rather
        than a generic VThread, because I don't think it's appropriate
        to make VThread have knowledge of sockets. It's less ugly
        to say that VSocketThread knows about listeners, because you
        generally use socket threads with listeners. Either way there
        is some cross-domain knowledge between threads and sockets;
        this seems the cleaner of the two options.

        @param  threadBaseName  a distinguishing base name for the thread, useful for debugging purposes;
                                the thread name will be composed of this and the socket's IP address and port
        @param    socket        the socket this thread is managing
        @param    ownerThread   the thread that created this one
        */
        VSocketThread(const VString& threadBaseName, VSocket* socket, VListenerThread* ownerThread);
        /**
        Virtual destructor.
        */
        virtual ~VSocketThread();

        /**
        Returns this thread's socket object.
        @return    a pointer to the VSocket
        */
        VSocket* getSocket() const;
        /**
        Returns this thread's owner listener thread.
        @return    a pointer to the VListenerThread
        */
        VListenerThread* getOwnerThread() const;

        /**
        Closes the socket and stops the thread (causing it to end) in one shot.
        */
        void closeAndStop();

    protected:

        VSocket*            mSocket;        ///< The socket this thread is managing.
        VListenerThread*    mOwnerThread;   ///< The thread that created this one.

    private:

        // Prevent copy construction and assignment since there is no provision for sharing the underlying thread.
        VSocketThread(const VSocketThread& other);
        VSocketThread& operator=(const VSocketThread& other);

        friend class VListenerThread; // allow it to remove itself as our owner thread when it destructs
};

/**
VSocketThreadPtrVector is simply a vector of VSocketThread pointers.
*/
typedef std::vector<VSocketThread*> VSocketThreadPtrVector;

#endif /* vsocketthread_h */
