/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

#ifndef vthread_platform_h
#define vthread_platform_h

/** @file */

// POSIX threads data types.

extern "C" {
#include <pthread.h>
}

// We define our own names here so that we are independent of the platform names,
// and don't conflict with any similarly-named platform typedefs (such as "ThreadID").
typedef pthread_t       VThreadID_Type;
typedef pthread_cond_t  VSemaphore_Type;
typedef pthread_mutex_t VMutex_Type;
typedef struct timespec VTimeout_Type;

#endif /* vthread_platform_h */

