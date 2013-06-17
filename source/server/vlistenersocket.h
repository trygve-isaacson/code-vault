/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vlistenersocket_h
#define vlistenersocket_h

/** @file */

#include "vsocket.h"

class VSocketFactory;

/**
    @ingroup vsocket
*/

/**
VListenerSocket is a special kind of socket that can accept incoming
connections and create a new VSocket for each such connection that it
accepts.

Usually you won't have to use this class directly when implementing
a server; you will use VListenerThread to manage things.

There is one counter-intuitive thing about this class: the listen()
method is implemented in its superclass, VSocket. This is because
listen() is a platform-specific bit of code, and VSocket is where
all of the platform-specific socket code lives. This class merely
adds the accept() method.

@see    VListenerThread
*/
class VListenerSocket : public VSocket {
    public:

        /**
        Creates a VListenerSocket to listen on a particular port.
        @param    portNumber    the port to listen on
        @param    bindAddress   if empty, the socket will bind to INADDR_ANY (usually a good
                                default); if a value is supplied the socket will bind to the
                                supplied IP address (can be useful on a multi-homed server)
        @param    factory        a factory that will create a VSocket-derived
                            object for each incoming connection
        @param    backlog        the listen backlog for the socket; this limits
                            the number of pending incoming connections that
                            can be queued up for acceptance
        */
        VListenerSocket(int portNumber, const VString& bindAddress, VSocketFactory* factory, int backlog = 50);
        /**
        Destructor.
        */
        virtual ~VListenerSocket();

        /**
        Blocks until an incoming connection occurs or the timeout interval
        elapses (if a timeout has been specified), returning a new VSocket
        object for the connection.

        If you fail to call listen() before calling accept(), this method
        will throw a VException. The socket cannot accept until it is
        listening.

        @return    the new VSocket object for the accepted connection
        */
        VSocket* accept();

        /**
        Causes the listener to activate by listening for incoming connections;
        generally this is followed by a loop that calls accept() to accept
        inbound connections.
        */
        void listen();

    private:

        // Prevent copy construction and assignment since there is no provision for sharing pointer data,
        // and it is probably an error anyway to try to create multiple listeners on the same port.
        VListenerSocket(const VListenerSocket& other);
        VListenerSocket& operator=(const VListenerSocket& other);

        VString         mBindAddress;   ///< The address that listen() will bind() to; empty means INADDR_ANY.
        int             mBacklog;       ///< The listen backlog value.
        VSocketFactory* mFactory;       ///< The factory for creating new VSocket objects.

};

#endif /* vlistenersocket_h */
