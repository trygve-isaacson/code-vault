/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocketfactory.h"

VSocketFactory::VSocketFactory() {
}

VSocket* VSocketFactory::createSocket(VSocketID socketID) {
    VSocket* theSocket = new VSocket(socketID);
    theSocket->discoverHostAndPort();
    theSocket->setDefaultSockOpt();

    return theSocket;
}

VSocket* VSocketFactory::createSocket(const VString& hostName, int portNumber, const VSocketConnectionStrategy& connectionStrategy) {
    VSocket* theSocket = new VSocket();
    theSocket->connectToHostName(hostName, portNumber, connectionStrategy);

    return theSocket;
}

