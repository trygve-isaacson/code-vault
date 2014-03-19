/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#include "vinstant.h"
#include "vtypes_internal.h"

/*
These are the platform-specific implementations of these required
core time functions.
*/

// static
Vs64 VInstantStruct::_platform_offsetFromLocalStruct(const VInstantStruct& when) {
    struct tm fields;

    when.getTmStruct(fields);

    //lint -e421 "Caution -- function 'mktime(struct tm *)' is considered dangerous [MISRA Rule 127]"
#ifdef VCOMPILER_MSVC
    Vs64 result = CONST_S64(1000) * static_cast<Vs64>(::_mktime64(&fields));
#else
    Vs64 result = CONST_S64(1000) * static_cast<Vs64>(::mktime(&fields));
#endif

    // tm struct has no milliseconds, so restore input value milliseconds
    result += (Vs64) when.mMillisecond;

    return result;
}

// static
void VInstantStruct::_platform_offsetToLocalStruct(Vs64 offset, VInstantStruct& when) {
    struct tm fields;
    ::memset(&fields, 0, sizeof(fields));

    VInstantStruct::_threadsafe_localtime(static_cast<time_t>(offset / 1000), &fields);

    when.setFromTmStruct(fields, (int)(offset % 1000));
}

// static
void VInstantStruct::_platform_offsetToUTCStruct(Vs64 offset, VInstantStruct& when) {
    struct tm fields;
    ::memset(&fields, 0, sizeof(fields));

    VInstantStruct::_threadsafe_gmtime(static_cast<time_t>(offset / 1000), &fields);

    when.setFromTmStruct(fields, (int)(offset % 1000));
}

// static
Vs64 VInstant::_platform_now() {
#ifdef V_INSTANT_SNAPSHOT_IS_UTC

    // This means we can get millisecond resolution for VInstant values.
    return VInstant::_platform_snapshot();

#else

    // This means we can only get second resolution for VInstant values.
    //lint -e418 -e421 "Passing null pointer to function 'time(long *)'" [OK: IEEE POSIX disagrees with lint. NULL is meaningful.]
    return (CONST_S64(1000) * static_cast<Vs64>(::time(NULL)));

#endif /* V_INSTANT_SNAPSHOT_IS_UTC */
}

#ifdef VCOMPILER_MSVC
#include <sys/timeb.h>    // required for _ftime64()

// static
Vs64 VInstant::_platform_snapshot() {
    struct __timeb64 tb;
    ::_ftime64(&tb);

    return (tb.time * 1000) + tb.millitm;
}

#else

// static
Vs64 VInstant::_platform_snapshot() {
    // The Windows _FILETIME structure holds a time value
    // with 100-ns resolution, so we can use that a snapshot
    // value that has the required millisecond resolution.
    // (Divide ns by 10,000 to get ms.)
    SYSTEMTIME st;
    GetSystemTime(&st);

    _FILETIME ft;
    (void) SystemTimeToFileTime(&st, &ft);

    Vs64 high32 = ft.dwHighDateTime;
    Vs64 low32 = ft.dwLowDateTime;
    Vs64 milliseconds = ((high32 << 32) | low32) / CONST_S64(10000); // 10000: ns to ms

    return milliseconds;
}

#endif

