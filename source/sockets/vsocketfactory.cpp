/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
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

