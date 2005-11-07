/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vtypes.h"

#include "vmutex.h"
#include "vmutexlocker.h"
#include "vstring.h"

static void getCurrentTZ(VString& tz)
    {
    char*    tzEnvString = getenv("TZ");
    
    if (tzEnvString == NULL)
        tz = "";
    else
        tz = tzEnvString;
    }

static void setCurrentTZ(const VString& tz)
    {
    VString    envString("TZ=%s", tz.chars());

    putenv(envString);
    }

//    extern "C" __time64_t __cdecl _mkgmtime64(struct tm* tb);
//_CRTIMP time_t __cdecl _mkgmtime(struct tm *);

static VMutex gTimeGMMutex;

time_t timegm(struct tm* t)
    {
    //return _mkgmtime(t);
    
    VMutexLocker    locker(&gTimeGMMutex);
    VString            savedTZ;
    
    getCurrentTZ(savedTZ);
    setCurrentTZ("UTC");

    time_t result = ::mktime(t);

    setCurrentTZ(savedTZ);
    
    return result;
    }


