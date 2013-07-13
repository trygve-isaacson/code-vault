/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vsocket_h
#define vsocket_h

/** @file */

#include "vsocketbase.h"

#ifndef PP_Target_Carbon
    // Unix platform includes needed for platform socket implementation.
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
#endif

#include "vmutex.h"

#include <netdb.h>
#include <arpa/inet.h>

/*
There are a couple of Unix APIs we call that take a socklen_t parameter.
Well, on HP-UX the parameter is defined as an int. The cleanest way of dealing
with this is to have our own conditionally-defined type here and use it there.
*/
#ifdef VPLATFORM_UNIX_HPUX
    typedef int VSocklenT;
#else
    typedef socklen_t VSocklenT;
#endif

#endif /* vsocket_h */

