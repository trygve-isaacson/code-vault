/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocketfactory.h"

VSocketFactory::VSocketFactory()
    {
    }

VSocket* VSocketFactory::createSocket(VSockID sockID)
    {
    VSocket*    theSocket = new VSocket();
    
    theSocket->init(sockID);
    
    return theSocket;
    }

VSocket* VSocketFactory::createSocket(const VString& hostName, int portNumber)
    {
    VSocket*    theSocket = new VSocket();
    
    theSocket->init(hostName, portNumber);
    
    return theSocket;
    }

