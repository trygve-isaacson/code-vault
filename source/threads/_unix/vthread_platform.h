/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

#ifndef vthread_platform_h
#define vthread_platform_h

/** @file */

// POSIX threads data types.

extern "C" {
#include <pthread.h>
}

typedef pthread_t        ThreadID;
typedef pthread_cond_t    Semaphore;
typedef pthread_mutex_t    Mutex;
typedef struct timespec Timeout;

#endif /* vthread_platform_h */

