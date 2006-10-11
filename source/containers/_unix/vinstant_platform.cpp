/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

#include "vinstant.h"

#include <sys/time.h>

/*
These are the platform-specific implementations of these required
core time functions.
*/

#ifdef VLIBRARY_METROWERKS
static Vs64 _getMSLLocalTimeZoneOffsetMilliseconds()
    {
    /*
    When compiling with Metrowerks CodeWarrior on Mac, the MSL library's version of mktime() is incorrect
    in that it does not actually do the local time zone conversion. That is, it behaves like timegm().
    We need to work around this bug by subtracting the local time zone offset.
    This is sort of the inverse of the problem with VC++ in the other conditionally compiled code above.
    FIXME: The remaining issue is that we are referencing the CURRENT tz offset, not the offset at the
    time that is being queried; and this seems to always be standard time -- e.g., 8hrs in Oakland,
    never 7hrs even in summertime DST. So in summertime this makes us off by 1 hr.
    */
    
    int    offsetSeconds = 0;

    struct timezone zone;
    
    if (::gettimeofday(NULL, &zone) == 0)
        offsetSeconds = (zone.tz_minuteswest * 60);
    
    return CONST_S64(1000) * static_cast<Vs64>(offsetSeconds);
    }
#endif

static void _setTMStructFromInstantStruct(const VInstantStruct& when, struct tm& fields)
    {
    ::memset(&fields, 0, sizeof(fields));

    fields.tm_year = when.mYear - 1900;    // tm_year field is years since 1900
    fields.tm_mon = when.mMonth - 1;    // tm_mon field is 0..11
    fields.tm_mday = when.mDay;
    fields.tm_hour = when.mHour;
    fields.tm_min = when.mMinute;
    fields.tm_sec = when.mSecond;
    fields.tm_isdst = -1;
    }

static void _setInstantStructFromTMStruct(const struct tm& fields, int millisecond, VInstantStruct& when)
    {
    when.mYear = fields.tm_year + 1900;    // tm_year field is years since 1900
    when.mMonth = fields.tm_mon + 1;    // tm_mon field is 0..11
    when.mDay = fields.tm_mday;
    when.mHour = fields.tm_hour;
    when.mMinute = fields.tm_min;
    when.mSecond = fields.tm_sec;
    when.mMillisecond = millisecond;
    when.mDayOfWeek = fields.tm_wday;
    }

// static
Vs64 VInstant::_platform_now()
    {
#ifdef V_INSTANT_SNAPSHOT_IS_UTC

    // This means we can get millisecond resolution for VInstant values.
    return VInstant::_platform_snapshot();

#else

    // This means we can only get second resolution for VInstant values.
    //lint -e418 -e421 "Passing null pointer to function 'time(long *)'"
    return (CONST_S64(1000) * static_cast<Vs64>(::time(NULL))) + VInstant::gSimulatedClockOffset;

#endif /* V_INSTANT_SNAPSHOT_IS_UTC */
    }

// static
Vs64 VInstant::_platform_offsetFromLocalStruct(const VInstantStruct& when)
    {
    struct tm    fields;
    
    _setTMStructFromInstantStruct(when, fields);

    //lint -e421 "Caution -- function 'mktime(struct tm *)' is considered dangerous [MISRA Rule 127]"
    Vs64    result = CONST_S64(1000) * static_cast<Vs64> (::mktime(&fields));
    
#ifdef VLIBRARY_METROWERKS
        result += _getMSLLocalTimeZoneOffsetMilliseconds();
#endif
        
    // tm struct has no milliseconds, so restore input value milliseconds
    result += (Vs64) when.mMillisecond;
    
    return result;
    }

#ifdef VPLATFORM_UNIX_HPUX

// On HP-UX, there's no API for timegm(), so we have to figure out the TZ offset.
// This is like what we have to do on Windows.

// static
Vs64 VInstant::_platform_offsetFromUTCStruct(const VInstantStruct& when)
    {
    struct tm    fields;

    _setTMStructFromInstantStruct(when, fields);

    //lint -e421 "Caution -- function 'mktime(struct tm *)' is considered dangerous [MISRA Rule 127]"
    Vs64    result = CONST_S64(1000) * static_cast<Vs64> (::mktime(&fields));
    
    // The problem here is that there's no timegm() available.
    // So we first use mktime as if we're getting a local time. Then we try
    // to figure out the correct offset to UTC, and apply it to our result.
    
    time_t x = ::mktime(&fields);        // get a local time_t from supplied broken down time

    struct tm* utcp = ::gmtime(&x);        // convert that back to a GMT time_t
    struct tm utc = *utcp;

    struct tm* locp = ::localtime(&x);    // and also convert it back to a local time_t
    struct tm loc = *locp;

    utc.tm_isdst = -1;                    // must reset dst field so mktime will calculate it for
    loc.tm_isdst = -1;                    //   these particular times, otherwise mktime may be off by DST shift

    time_t utct = ::mktime(&utc);        // get the offset of the utc-based time
    time_t loct = ::mktime(&loc);        // get the offset of the local-based time

    time_t delta = utct - loct;            // get the actual delta (hmmm, what if it spans a ST/DST crossover?)

    result -= (delta * 1000);

    // tm struct has no milliseconds, so restore input value milliseconds
    result += (Vs64) when.mMillisecond;
    
    return result;
    }

#else

// static
Vs64 VInstant::_platform_offsetFromUTCStruct(const VInstantStruct& when)
    {
    struct tm    fields;

    _setTMStructFromInstantStruct(when, fields);

    //lint -e421 "Caution -- function 'mktime(struct tm *)' is considered dangerous [MISRA Rule 127]"
    Vs64    result = CONST_S64(1000) * static_cast<Vs64> (::timegm(&fields));
    
    // tm struct has no milliseconds, so restore input value milliseconds
    result += (Vs64) when.mMillisecond;
    
    return result;
    }

#endif

// static
void VInstant::_platform_offsetToLocalStruct(Vs64 offset, VInstantStruct& when)
    {
    struct tm    fields;
    ::memset(&fields, 0, sizeof(fields));

#ifdef VLIBRARY_METROWERKS
    offset -= _getMSLLocalTimeZoneOffsetMilliseconds();
#endif

    VInstant::threadsafe_localtime(static_cast<time_t> (offset / 1000), &fields);
    
    _setInstantStructFromTMStruct(fields, (int) (offset % 1000), when);
    }

// static
void VInstant::_platform_offsetToUTCStruct(Vs64 offset, VInstantStruct& when)
    {
    struct tm    fields;
    ::memset(&fields, 0, sizeof(fields));

    VInstant::threadsafe_gmtime(static_cast<time_t> (offset / 1000), &fields);
    
    _setInstantStructFromTMStruct(fields, (int) (offset % 1000), when);
    }

// static
Vs64 VInstant::_platform_snapshot()
    {
    struct timeval tv;
    (void) ::gettimeofday(&tv, NULL);
    
    // We need to be careful to cast to Vs64 or we risk truncation to 32 bits.
    return (((Vs64) (tv.tv_sec)) * CONST_S64(1000)) + (Vs64) (tv.tv_usec / 1000) + VInstant::gSimulatedClockOffset;
    }

