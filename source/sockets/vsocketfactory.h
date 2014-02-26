/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
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
class VSocketFactory {
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
        specified host name, port number, and connection strategy.
        @param    hostName              the host name to pass to connectToHostName()
        @param    portNumber            the port number to pass to connectToHostName()
        @param    connectionStrategy    the strategy object to pass to connectToHostName() (VSocketConnectionStrategySingle is simplest)
        @return the new VSocket object
        */
        virtual VSocket* createSocket(const VString& hostName, int portNumber, const VSocketConnectionStrategy& connectionStrategy);

};

#endif /* vsocketfactory_h */
