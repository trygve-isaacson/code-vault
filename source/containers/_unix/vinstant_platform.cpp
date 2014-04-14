/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

#include "vinstant.h"
#include "vtypes_internal.h"
#include "vexception.h"

#include <sys/time.h>

/*
These are the platform-specific implementations of these required
core time functions.
*/

#ifdef VLIBRARY_METROWERKS
static Vs64 _getMSLLocalTimeZoneOffsetMilliseconds() {
    /*
    When compiling with Metrowerks CodeWarrior on Mac, the MSL library's version of mktime() is incorrect
    in that it does not actually do the local time zone conversion. That is, it behaves like timegm().
    We need to work around this bug by subtracting the local time zone offset.
    This is sort of the inverse of the problem with VC++ in the other conditionally compiled code above.
    FIXME: The remaining issue is that we are referencing the CURRENT tz offset, not the offset at the
    time that is being queried; and this seems to always be standard time -- e.g., 8hrs in Oakland,
    never 7hrs even in summertime DST. So in summertime this makes us off by 1 hr.
    */

    int offsetSeconds = 0;

    struct timezone zone;

    if (::gettimeofday(NULL, &zone) == 0) {
        offsetSeconds = (zone.tz_minuteswest * 60);
    }

    return CONST_S64(1000) * static_cast<Vs64>(offsetSeconds);
}
#endif

#define INSTANT_STRUCT_FORMAT "%d-%02d-%02d %02d:%02d:%02d.%03d"

// static
Vs64 VInstantStruct::_platform_offsetFromLocalStruct(const VInstantStruct& when) {
    struct tm fields;

    when.getTmStruct(fields);

    //lint -e421 "Caution -- function 'mktime(struct tm *)' is considered dangerous [MISRA Rule 127]"
    Vs64 mktimeSeconds = static_cast<Vs64>(::mktime(&fields));

    if (mktimeSeconds == CONST_S64(-1)) {
        throw VStackTraceException(VSTRING_FORMAT("VInstantStruct::_platform_offsetFromLocalStruct: time value '" INSTANT_STRUCT_FORMAT "' is out of range.",
            when.mYear, when.mMonth, when.mDay, when.mHour, when.mMinute, when.mSecond, when.mMillisecond));
    }
    
    Vs64 resultOffsetMilliseconds = CONST_S64(1000) * mktimeSeconds;

#ifdef VLIBRARY_METROWERKS
    resultOffsetMilliseconds += _getMSLLocalTimeZoneOffsetMilliseconds();
#endif

    // tm struct has no milliseconds, so restore input value milliseconds
    resultOffsetMilliseconds += (Vs64) when.mMillisecond;

    return resultOffsetMilliseconds;
}

// static
void VInstantStruct::_platform_offsetToLocalStruct(Vs64 offset, VInstantStruct& when) {
    struct tm fields;
    ::memset(&fields, 0, sizeof(fields));

#ifdef VLIBRARY_METROWERKS
    offset -= _getMSLLocalTimeZoneOffsetMilliseconds();
#endif

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

// static
Vs64 VInstant::_platform_snapshot() {
    struct timeval tv;
    (void) ::gettimeofday(&tv, NULL);

    // We need to be careful to cast to Vs64 or we risk truncation to 32 bits.
    return (((Vs64)(tv.tv_sec)) * CONST_S64(1000)) + (Vs64)(tv.tv_usec / 1000);
}

