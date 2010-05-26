/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

#ifndef vsocketfactory_h
#define vsocketfactory_h

/** @file */

#include "vsocket.h"

/**
    @ingroup vsocket
*/

/**
VSocketFactory can be used as-is, or can be subclassed to create
special kinds of sockets; normally every socket is just a VSocket,
but it is conceivable to have things like VSecureSocket or such.
*/
class VSocketFactory
    {
    public:
    
        /**
        Constructor, declared for completeness.
        */
        VSocketFactory();
        /**
        Destructor, declared for completeness.
        */
        virtual ~VSocketFactory() {}
        
        /**
        Creates a VSocket object and calls its init() method with the
        specified socket id.
        @param    socketID    the socket id to pass to init()
        @return    the new VSocket object
        */
        virtual VSocket* createSocket(VSocketID socketID);
        /**
        Creates a VSocket object and calls its init() method with the
        specified host name and port number.
        @param    hostName    the host name to pass to init()
        @param    portNumber    the port number to pass to init()
        @return    the new VSocket object
        */
        virtual VSocket* createSocket(const VString& hostName, int portNumber);

    };

#endif /* vsocketfactory_h */
