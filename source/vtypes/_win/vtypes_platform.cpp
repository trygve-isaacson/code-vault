/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vtypes.h"

#include "vmutex.h"
#include "vmutexlocker.h"
#include "vstring.h"

#include <psapi.h> // for GetProcessMemoryInfo used by VgetMemoryUsage

V_STATIC_INIT_TRACE
    
Vs64 vault::VgetMemoryUsage()
    {
    PROCESS_MEMORY_COUNTERS info;
    BOOL success = ::GetProcessMemoryInfo(::GetCurrentProcess(), &info, sizeof(info));
    if (success)
        return info.WorkingSetSize;
    else
        return 0;
    }
    
static const Vu8 kDOSLineEnding[2] = { 0x0D, 0x0A };

const Vu8* vault::VgetNativeLineEnding(int& numBytes)
    {
    numBytes = 2;
    return kDOSLineEnding;
    }

static void getCurrentTZ(VString& tz)
    {
    char*    tzEnvString = vault::getenv("TZ");
    
    if (tzEnvString == NULL)
        tz = VString::EMPTY();
    else
        tz = tzEnvString;
    }

static void setCurrentTZ(const VString& tz)
    {
    VString    envString("TZ=%s", tz.chars());

    vault::putenv(envString);
    }

static VMutex gTimeGMMutex;

time_t timegm(struct tm* t)
    {
    VMutexLocker    locker(&gTimeGMMutex);
    VString            savedTZ;
    
    getCurrentTZ(savedTZ);
    setCurrentTZ("UTC");

    time_t result = ::mktime(t);

    setCurrentTZ(savedTZ);
    
    return result;
    }

#ifdef VCOMPILER_CODEWARRIOR
#include "vexception.h"
int vault::open(const char* path, int flags, mode_t mode)
    {
    throw VException(VString("Error opening '%s': POSIX open() is not supported by CodeWarrior on Windows.", path));
    }
#endif
