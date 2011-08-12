/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocketfactory.h"

VSocketFactory::VSocketFactory()
    {
    }

VSocket* VSocketFactory::createSocket(VSocketID socketID)
    {
    VSocket* theSocket = new VSocket(socketID);
    theSocket->discoverHostAndPort();
    theSocket->setDefaultSockOpt();
    
    return theSocket;
    }

VSocket* VSocketFactory::createSocket(const VString& hostName, int portNumber)
    {
    VSocket* theSocket = new VSocket(hostName, portNumber);
    theSocket->connect();

    return theSocket;
    }

