/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocketfactory.h"

VSocketFactory::VSocketFactory()
    {
    }

VSocket* VSocketFactory::createSocket(VSocketID socketID)
    {
    VSocket*    theSocket = new VSocket();
    
    theSocket->init(socketID);
    
    return theSocket;
    }

VSocket* VSocketFactory::createSocket(const VString& hostName, int portNumber)
    {
    VSocket*    theSocket = new VSocket();
    
    theSocket->init(hostName, portNumber);
    
    return theSocket;
    }

