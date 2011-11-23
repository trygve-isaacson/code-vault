/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

/** @file */

#include "vtypes.h"
#include "vtypes_internal.h"

#include "vmutex.h"
#include "vmutexlocker.h"
#include "vstring.h"

#include <psapi.h> // for GetProcessMemoryInfo used by VgetMemoryUsage

V_STATIC_INIT_TRACE

Vs64 vault::VgetMemoryUsage() {
    PROCESS_MEMORY_COUNTERS info;
    BOOL success = ::GetProcessMemoryInfo(::GetCurrentProcess(), &info, sizeof(info));
    if (success)
        return info.WorkingSetSize;
    else
        return 0;
}

static const Vu8 kDOSLineEnding[2] = { 0x0D, 0x0A };

const Vu8* vault::VgetNativeLineEnding(int& numBytes) {
    numBytes = 2;
    return kDOSLineEnding;
}

static void getCurrentTZ(VString& tz) {
    char* tzEnvString = vault::getenv("TZ");

    if (tzEnvString == NULL)
        tz = VString::EMPTY();
    else
        tz = tzEnvString;
}

static void setCurrentTZ(const VString& tz) {
    VString envString(VSTRING_ARGS("TZ=%s", tz.chars()));

    /*
    The IEEE docs describe putenv()'s strange behavior:
    The (char*) we pass becomes owned by the system, until
    we replace it with another. Unless we keep track of
    each call, each call must result in a small leak. So be it.
    But we must allocate a separate buffer from the envString.
    */
    int bufferLength = 1 + envString.length();
    char* orphanBuffer = new char[bufferLength];
    envString.copyToBuffer(orphanBuffer, bufferLength);
    vault::putenv(orphanBuffer);
}

static VMutex gTimeGMMutex("gTimeGMMutex", true/*suppressLogging*/);

time_t timegm(struct tm* t) {
    VMutexLocker    locker(&gTimeGMMutex, "timegm");
    VString         savedTZ;

    getCurrentTZ(savedTZ);
    setCurrentTZ("UTC");

    time_t result = ::mktime(t);

    setCurrentTZ(savedTZ);

    return result;
}

#ifdef VCOMPILER_CODEWARRIOR
#include "vexception.h"
int vault::open(const char* path, int flags, mode_t mode) {
    throw VException(VSTRING_FORMAT("Error opening '%s': POSIX open() is not supported by CodeWarrior on Windows.", path));
}
#endif

// VAutoreleasePool is a no-op on Windows.
VAutoreleasePool::VAutoreleasePool() {}
void VAutoreleasePool::drain() {}
VAutoreleasePool::~VAutoreleasePool() {}

