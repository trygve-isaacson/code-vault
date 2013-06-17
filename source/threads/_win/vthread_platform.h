/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vthread_platform_h
#define vthread_platform_h

/** @file */

// Win32 threads data types.

#ifndef _WIN32
#define _WIN32
#endif

#include "vtypes_platform.h"

// We define our own names here so that we are independent of the platform names,
// and don't conflict with any similarly-named platform typedefs (such as "ThreadID").

typedef DWORD               VThreadID_Type;
typedef HANDLE              VSemaphore_Type;
typedef CRITICAL_SECTION    VMutex_Type;
typedef long                VTimeout_Type;

#endif /* vthread_platform_h */

