/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vinstant.h"
#include "vchar.h"
#include "vstring.h"
#include "vexception.h"

#undef sscanf

// VInstantStruct ------------------------------------------------------------

VInstantStruct::VInstantStruct(const VDate& date, const VTimeOfDay& timeOfDay) :
mYear(date.year()),
mMonth(date.month()),
mDay(date.day()),
mHour(timeOfDay.hour()),
mMinute(timeOfDay.minute()),
mSecond(timeOfDay.second()),
mMillisecond(timeOfDay.millisecond()),
mDayOfWeek(0)
    {
    }

// VInstant ------------------------------------------------------------------

const VString VInstant::kUTCTimeZoneID("UTC");
const VString VInstant::kLocalTimeZoneID; // empty string means local time zone

MRemoteTimeZoneConverter* VInstant::gRemoteTimeZoneConverter = NULL;

// static
void VInstant::setRemoteTimeZoneConverter(MRemoteTimeZoneConverter* converter)
    {
    gRemoteTimeZoneConverter = converter;
    }

// static
MRemoteTimeZoneConverter* VInstant::getRemoteTimeZoneConverter()
    {
    return gRemoteTimeZoneConverter;
    }

VInstant::VInstant()
    {
    mKind = kActualValue;
    mValue = 0;

    VInstant::setNow();
    
    ASSERT_INVARIANT();
    }

VInstant::VInstant(int kind, Vs64 value)
    {
    mKind = kind;
    mValue = value;

    ASSERT_INVARIANT();
    }

VInstant::VInstant(time_t inTime)
    {
    mKind = kActualValue;
    mValue = CONST_S64(1000) * static_cast<Vs64> (inTime);    // careful with 32-to-64 bit conversion
    
    ASSERT_INVARIANT();
    }

VInstant& VInstant::operator=(const VInstant& i)
    {
    if (this != &i)
        {
        mKind = i.getKind();
        mValue = i.getValue();
        }
    
    ASSERT_INVARIANT();

    return *this;
    }

VInstant& VInstant::operator+=(Vs64 forwardOffsetMilliseconds)
    {
    ASSERT_INVARIANT();

    if (mKind == kActualValue)
        {
        mValue += forwardOffsetMilliseconds;
        }

    ASSERT_INVARIANT();

    return *this;
    }

VInstant& VInstant::operator-=(Vs64 backwardOffsetMilliseconds)
    {
    ASSERT_INVARIANT();

    if (mKind == kActualValue)
        {
        mValue -= backwardOffsetMilliseconds;
        }

    ASSERT_INVARIANT();

    return *this;
    }

void VInstant::setNow()
    {
    mKind = kActualValue;
    mValue = VInstant::platform_now();

    ASSERT_INVARIANT();
    }

void VInstant::deltaOffset(Vs64 offsetMilliseconds)
    {
    ASSERT_INVARIANT();

    if (mKind == kActualValue)
        {
        mValue += offsetMilliseconds;
        }

    ASSERT_INVARIANT();
    }

void VInstant::getUTCString(VString& s, bool fileNameSafe) const
    {
    ASSERT_INVARIANT();

    switch (mKind)
        {
        case kInfinitePast:
            s = "PAST";
            break;

        case kInfiniteFuture:
            s = "FUTURE";
            break;

        case kNeverOccurred:
            s = "NEVER";
            break;

        case kActualValue:
        default:
            {
            VInstantStruct    when;
            
            VInstant::platform_offsetToUTCStruct(mValue, when);

            if (fileNameSafe)
                s.format("%d%02d%02d%02d%02d%02d%03d",
                    when.mYear, when.mMonth, when.mDay, when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
            else
                s.format("%d-%02d-%02d %02d:%02d:%02d.%03d UTC",
                    when.mYear, when.mMonth, when.mDay, when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
            }
            break;
        }
    }

void VInstant::setUTCString(const VString& s)
    {
    ASSERT_INVARIANT();

    if (s == "PAST")
        {
        mKind = kInfinitePast;
        //lint -e961 "Violates MISRA Advisory Rule 18, Numerical constant requires suffix"
        mValue = -(CONST_S64(0x7FFFFFFFFFFFFFFF));    // for purposes of comparison operators
        }
    else if (s == "FUTURE")
        {
        mKind = kInfiniteFuture;
        //lint -e961 "Violates MISRA Advisory Rule 18, Numerical constant requires suffix"
        mValue = CONST_S64(0x7FFFFFFFFFFFFFFF);    // for purposes of comparison operators
        }
    else if (s == "NEVER")
        {
        mKind = kNeverOccurred;
        mValue = 0;    // it is meaningless to compare nevers; could imply past (when did it happen) or future (when will it happen)
        }
    else
        {
        mKind = kActualValue;

        VInstantStruct when;
        when.mDayOfWeek = 0;
        
        ::sscanf(s, "%d-%02d-%02d %02d:%02d:%02d.%03d UTC", &when.mYear, &when.mMonth, &when.mDay, &when.mHour, &when.mMinute, &when.mSecond, &when.mMillisecond);
        
        mValue = VInstant::platform_offsetFromUTCStruct(when);
        }

    ASSERT_INVARIANT();
    }

void VInstant::getLocalString(VString& s, bool fileNameSafe) const
    {
    ASSERT_INVARIANT();

    switch (mKind)
        {
        case kInfinitePast:
            s = "PAST";
            break;

        case kInfiniteFuture:
            s = "FUTURE";
            break;

        case kNeverOccurred:
            s = "NEVER";
            break;

        case kActualValue:
        default:
            {
            VInstantStruct    when;
            
            VInstant::platform_offsetToLocalStruct(mValue, when);

            if (fileNameSafe)
                s.format("%d%02d%02d%02d%02d%02d%03d",
                    when.mYear, when.mMonth, when.mDay, when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
            else
                s.format("%d-%02d-%02d %02d:%02d:%02d.%03d",
                    when.mYear, when.mMonth, when.mDay, when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
            }
            break;
        }
    }

void VInstant::setLocalString(const VString& s)
    {
    ASSERT_INVARIANT();

    if (s == "Infinite Past")
        {
        mKind = kInfinitePast;
        mValue = 0;
        }
    else if (s == "Infinite Future")
        {
        mKind = kInfiniteFuture;
        mValue = 0;
        }
    else if (s == "Never Occurred")
        {
        mKind = kNeverOccurred;
        mValue = 0;
        }
    else
        {
        mKind = kActualValue;

        VInstantStruct when;
        when.mDayOfWeek = 0;
        
        ::sscanf(s, "%d-%02d-%02d %02d:%02d:%02d.%03d UTC", &when.mYear, &when.mMonth, &when.mDay, &when.mHour, &when.mMinute, &when.mSecond, &when.mMillisecond);
        
        mValue = VInstant::platform_offsetFromLocalStruct(when);
        }

    ASSERT_INVARIANT();
    }

int VInstant::getKind() const
    {
    ASSERT_INVARIANT();

    return mKind;
    }

Vs64 VInstant::getValue() const
    {
    ASSERT_INVARIANT();

    return mValue;
    }

Vs32 VInstant::getValueSeconds() const
    {
    ASSERT_INVARIANT();

    return static_cast<Vs32> (mValue/1000);    // convert milliseconds to seconds
    }

void VInstant::setKind(int kind)
    {
    ASSERT_INVARIANT();

    mKind = kind;
    
    // For non-actual kinds, we always set value to 0 so that comparisons work simply.
    if (mKind != kActualValue)
        mValue = CONST_S64(0);

    ASSERT_INVARIANT();
    }

void VInstant::setValue(Vs64 value)
    {
    ASSERT_INVARIANT();

    mValue = value;

    ASSERT_INVARIANT();
    }

void VInstant::setValueSeconds(Vs32 value)
    {
    ASSERT_INVARIANT();

    mValue = static_cast<Vs64>(value) * CONST_S64(1000);    // convert seconds to milliseconds

    ASSERT_INVARIANT();
    }

void VInstant::getStreamValues(Vs32& kind, Vs64& value) const
    {
    ASSERT_INVARIANT();

    kind = mKind;
    value = mValue;
    }

void VInstant::setStreamValues(Vs32 kind, Vs64 value)
    {
    ASSERT_INVARIANT();

    mKind = kind;
    mValue = value;

    ASSERT_INVARIANT();
    }

void VInstant::getValues(VDate& date, VTimeOfDay& timeOfDay, const VString& timeZoneID) const
    {
    ASSERT_INVARIANT();

    VInstantStruct    when;
    
    if (timeZoneID == kLocalTimeZoneID)
        VInstant::platform_offsetToLocalStruct(mValue, when);
    else if (timeZoneID == kUTCTimeZoneID)
        VInstant::platform_offsetToUTCStruct(mValue, when);
    else if (gRemoteTimeZoneConverter == NULL)
        throw VException("Request for remote time zone conversion (%s) without a converter.", timeZoneID.chars());
    else
        gRemoteTimeZoneConverter->offsetToRTZStruct(mValue, timeZoneID, when);
    
    date.set(when.mYear, when.mMonth, when.mDay);
    timeOfDay.set(when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
    }

void VInstant::setValues(const VDate& date, const VTimeOfDay& timeOfDay, const VString& timeZoneID)
    {
    ASSERT_INVARIANT();
    
    VInstantStruct    when(date, timeOfDay);

    mKind = kActualValue;
    
    if (timeZoneID == kLocalTimeZoneID)
        mValue = VInstant::platform_offsetFromLocalStruct(when);
    else if (timeZoneID == kUTCTimeZoneID)
        mValue = VInstant::platform_offsetFromUTCStruct(when);
    else if (gRemoteTimeZoneConverter == NULL)
        throw VException("Request for remote time zone conversion (%s) without a converter.", timeZoneID.chars());
    else
        mValue = gRemoteTimeZoneConverter->offsetFromRTZStruct(timeZoneID, when);

    ASSERT_INVARIANT();
    }

void VInstant::getDate(VDate& date, const VString& timeZoneID) const
    {
    ASSERT_INVARIANT();
    
    VInstantStruct    when;
    
    if (timeZoneID == kLocalTimeZoneID)
        VInstant::platform_offsetToLocalStruct(mValue, when);
    else if (timeZoneID == kUTCTimeZoneID)
        VInstant::platform_offsetToUTCStruct(mValue, when);
    else if (gRemoteTimeZoneConverter == NULL)
        throw VException("Request for remote time zone conversion (%s) without a converter.", timeZoneID.chars());
    else
        gRemoteTimeZoneConverter->offsetToRTZStruct(mValue, timeZoneID, when);
    
    date.set(when.mYear, when.mMonth, when.mDay);
    }

void VInstant::getTimeOfDay(VTimeOfDay& timeOfDay, const VString& timeZoneID) const
    {
    ASSERT_INVARIANT();

    VInstantStruct    when;
    
    if (timeZoneID == kLocalTimeZoneID)
        VInstant::platform_offsetToLocalStruct(mValue, when);
    else if (timeZoneID == kUTCTimeZoneID)
        VInstant::platform_offsetToUTCStruct(mValue, when);
    else if (gRemoteTimeZoneConverter == NULL)
        throw VException("Request for remote time zone conversion (%s) without a converter.", timeZoneID.chars());
    else
        gRemoteTimeZoneConverter->offsetToRTZStruct(mValue, timeZoneID, when);
    
    timeOfDay.set(when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
    }

static const int kSecondsPerDay = 86400;

Vs64 VInstant::getLocalOffsetMilliseconds() const
    {
    /*
    Return the offset, in milliseconds, of the local time zone, at this
    indicated instant.
    */
    
    VInstantStruct    localStruct;
    VInstant::platform_offsetToLocalStruct(mValue, localStruct);
    
    VInstantStruct    utcStruct;
    VInstant::platform_offsetToUTCStruct(mValue, utcStruct);
    
    // Now we have two structs for the instant.
    // A little math will tell us the difference in h/m/s.
    // We know the delta cannot be more than 24 hours.
    // No need to worry about milliseconds; time zone offset resolutions are only in minutes.
    // (We could presumably even ignore the seconds for that matter.)
    int deltaSeconds;
    int    localSecondsOfDay = (3600 * localStruct.mHour) + (60 * localStruct.mMinute) + localStruct.mSecond;
    int    utcSecondsOfDay = (3600 * utcStruct.mHour) + (60 * utcStruct.mMinute) + utcStruct.mSecond;

    if (localStruct.mDay == utcStruct.mDay)
        {
        // Same date. Just a difference in hours/minutes.
        
        deltaSeconds = localSecondsOfDay - utcSecondsOfDay;
        }
    else if ((localStruct.mDay == utcStruct.mDay+1) ||
            ((localStruct.mDay == 1) && (utcStruct.mDay > 27))) // detect wrap-around day 1 and day 28-31
        {
        // We are ahead of GMT and we are already on the "next" calendar date.
        // localSecondsOfDay is therefore smaller, so add a day's worth of seconds it to compensate.
        
        deltaSeconds = (localSecondsOfDay + kSecondsPerDay) - utcSecondsOfDay;
        }
    else
        {
        // We are behind GMT and are still on the "previous" calendar date.
        // utcSecondsOfDay is therefore smaller, so add a day's worth of seconds it to compensate.

        deltaSeconds = localSecondsOfDay - (utcSecondsOfDay + kSecondsPerDay);
        }
    
    /*
    Debugging notes:
    For a value in Pacific Standard Time (winter time), we should return -8 hrs, which is -28800 * 1000 = -28,800,000ms
    For a value in Pacific Daylight Time (summer time), we should return -7 hrs, which is -25200 * 1000 = -25,200,000ms
    For a value in India Standard Time, we should return +5 hrs 30 minutes, which is +19800 * 1000 = +19,800,000ms
    */

    return (Vs64) (deltaSeconds * 1000);
    }

void VInstant::assertInvariant() const
    {
    V_ASSERT(mKind >= 0);            // mKind must be in range
    V_ASSERT(mKind < kNumKinds);    // mKind must be in range
    }

// static
void VInstant::threadsafe_localtime(const time_t epochOffset, struct tm* resultStorage)
    {
    time_t        offset = epochOffset;
    struct tm*    result;
    
#ifdef V_HAVE_REENTRANT_TIME            
    result = ::localtime_r(&offset, resultStorage);
#else
    result = ::localtime(&offset);
#endif

    if (result == NULL)
        throw VException("VInstant::threadsafe_localtime: input time value %d is out of range.", (int) offset);
    
// Only copy result if we're NOT using reentrant version that already wrote result.
#ifndef V_HAVE_REENTRANT_TIME            
    *resultStorage = *result;
#endif
    }
    
// static
void VInstant::threadsafe_gmtime(const time_t epochOffset, struct tm* resultStorage)
    {
    struct tm* result;

#ifdef V_HAVE_REENTRANT_TIME            
    result = ::gmtime_r(&epochOffset, resultStorage);
#else
    result = ::gmtime(&epochOffset);
#endif

    if (result == NULL)
        throw VException("VInstant::threadsafe_gmtime: input time value %d is out of range.", (int) epochOffset);

// Only copy result if we're NOT using reentrant version that already wrote result.
#ifndef V_HAVE_REENTRANT_TIME            
    *resultStorage = *result;
#endif
    }

// static
bool VInstant::complexGT(const VInstant& i1, const VInstant& i2)
    {
    // We also handle the simple case correctly.
    if ((i1.mKind == kActualValue) && (i2.mKind == kActualValue))
        return i1 > i2;
    
    // If they're the same kind, they're equal. That's NOT i1>i2.
    if (i1.mKind == i2.mKind)
        return false;
    
    // Not the same kind. If i2 is past, then i1>i2.
    if (i2.mKind == kInfinitePast)
        return true;
    
    // Not the same kind. If i1 is future, then i1>i2.
    if (i1.mKind == kInfiniteFuture)
        return true;
    
    // None the above. Condition does not hold.
    return false;
    }

// static
bool VInstant::complexGTE(const VInstant& i1, const VInstant& i2)
    {
    // We also handle the simple case correctly.
    if ((i1.mKind == kActualValue) && (i2.mKind == kActualValue))
        return i1 >= i2;
    
    // If they're the same kind, they're equal. So i1>=i2.
    if (i1.mKind == i2.mKind)
        return true;
    
    // Not the same kind. If i2 is past, then i1>i2.
    if (i2.mKind == kInfinitePast)
        return true;
    
    // Not the same kind. If i1 is future, then i1>i2.
    if (i1.mKind == kInfiniteFuture)
        return true;
    
    // None the above. Condition does not hold.
    return false;
    }

// static
bool VInstant::complexLT(const VInstant& i1, const VInstant& i2)
    {
    // We also handle the simple case correctly.
    if ((i1.mKind == kActualValue) && (i2.mKind == kActualValue))
        return i1 < i2;
    
    // If they're the same kind, they're equal. That's NOT i1<i2.
    if (i1.mKind == i2.mKind)
        return false;
    
    // Not the same kind. If i1 is past, then i1<i2.
    if (i1.mKind == kInfinitePast)
        return true;
    
    // Not the same kind. If i2 is future, then i1<i2.
    if (i2.mKind == kInfiniteFuture)
        return true;
    
    // None the above. Condition does not hold.
    return false;
    }

// static
bool VInstant::complexLTE(const VInstant& i1, const VInstant& i2)
    {
    // We also handle the simple case correctly.
    if ((i1.mKind == kActualValue) && (i2.mKind == kActualValue))
        return i1 <= i2;
    
    // If they're the same kind, they're equal. So i1<=i2.
    if (i1.mKind == i2.mKind)
        return true;
    
    // Not the same kind. If i1 is past, then i1<i2.
    if (i1.mKind == kInfinitePast)
        return true;
    
    // Not the same kind. If i2 is future, then i1<i2.
    if (i2.mKind == kInfiniteFuture)
        return true;
    
    // None the above. Condition does not hold.
    return false;
    }

// static
Vs64 VInstant::snapshot()
    {
    return VInstant::platform_snapshot();
    }

// static
Vs64 VInstant::snapshotDelta(Vs64 snapshotValue)
    {
    return VInstant::platform_snapshot() - snapshotValue;
    }

// VDate ---------------------------------------------------------------------

const VChar VDate::kLocalDateSeparator('/');

VDate::VDate(const VString& timeZoneID)
    {
    // First, satisfy the invariant since we'll call set() via VInstant::getDate().
    mYear = 1970;    // may as well make it 0 UTC to start
    mMonth = 1;
    mDay = 1;

    VInstant    now;
    now.getDate(*this, timeZoneID);

    ASSERT_INVARIANT();
    }

VDate::VDate(int inYear, int inMonth, int inDay)
    {
    if ((inMonth < 1) || (inMonth > 12) ||
        (inDay   < 1) || (inDay   > 31))
        throw VRangeException(VString("VDate: %d-%02d-%02d is an invalid value.", inYear, inMonth, inDay));

    mYear = inYear;
    mMonth = inMonth;
    mDay = inDay;

    ASSERT_INVARIANT();
    }

int VDate::year() const
    {
    ASSERT_INVARIANT();

    return mYear;
    }

int VDate::month() const
    {
    ASSERT_INVARIANT();

    return mMonth;
    }

int VDate::day() const
    {
    ASSERT_INVARIANT();

    return mDay;
    }

int VDate::dayOfWeek() const
    {
    ASSERT_INVARIANT();
    
    VInstantStruct    when;

    when.mYear = mYear;
    when.mMonth = mMonth;
    when.mDay = mDay;
    when.mHour = 12;    // noon, smack dab in middle of day
    when.mMinute = 0;
    when.mSecond = 0;
    when.mMillisecond = 0;
    
    // First get the UTC offset of that UTC date.
    Vs64    offset = VInstant::platform_offsetFromUTCStruct(when);
    
    // Now reverse to get the mDayOfWeek filled out.
    VInstant::platform_offsetToUTCStruct(offset, when);
    
    return when.mDayOfWeek;
    }

void VDate::set(int inYear, int inMonth, int inDay)
    {
    ASSERT_INVARIANT();
    
    if ((inMonth < 1) || (inMonth > 12) ||
        (inDay   < 1) || (inDay   > 31))
        throw VRangeException(VString("VDate::set: %d-%02d-%02d is an invalid value.", inYear, inMonth, inDay));

    mYear = inYear;
    mMonth = inMonth;
    mDay = inDay;

    ASSERT_INVARIANT();
    }

//lint -e421 "Caution -- function 'abort(void)' is considered dangerous [MISRA Rule 126]"
void VDate::assertInvariant() const
    {
    V_ASSERT(mMonth > 0);
    V_ASSERT(mMonth < 13);
    V_ASSERT(mDay > 0);
    V_ASSERT(mDay < 32);
    }

// VTimeOfDay ----------------------------------------------------------------

const VChar VTimeOfDay::kLocalTimeSeparator(':');

VTimeOfDay::VTimeOfDay(const VString& timeZoneID)
    {
    // First, satisfy the invariant since we'll call set() via VInstant::getTimeOfDay().
    mHour = 0;
    mMinute = 0;
    mSecond = 0;
    mMillisecond = 0;

    VInstant    now;
    now.getTimeOfDay(*this, timeZoneID);

    ASSERT_INVARIANT();
    }

VTimeOfDay::VTimeOfDay(int inHour, int inMinute, int inSecond, int inMillisecond)
    {
    if ((inHour   < 0) || (inHour   > 23) ||
        (inMinute < 0) || (inMinute > 59) ||
        (inSecond < 0) || (inSecond > 59) ||
        (inMillisecond < 0) || (inMillisecond > 999))
        throw VRangeException(VString("VTimeOfDay: %02d:%02d:%02d.%03d is an invalid value.", inHour, inMinute, inSecond, inMillisecond));

    mHour = inHour;
    mMinute = inMinute;
    mSecond = inSecond;
    mMillisecond = inMillisecond;

    ASSERT_INVARIANT();
    }

int VTimeOfDay::hour() const
    {
    ASSERT_INVARIANT();

    return mHour;
    }

int VTimeOfDay::minute() const
    {
    ASSERT_INVARIANT();

    return mMinute;
    }

int VTimeOfDay::second() const
    {
    ASSERT_INVARIANT();

    return mSecond;
    }

int VTimeOfDay::millisecond() const
    {
    ASSERT_INVARIANT();

    return mMillisecond;
    }

void VTimeOfDay::set(int inHour, int inMinute, int inSecond, int inMillisecond)
    {
    ASSERT_INVARIANT();

    if ((inHour   < 0) || (inHour   > 23) ||
        (inMinute < 0) || (inMinute > 59) ||
        (inSecond < 0) || (inSecond > 59) ||
        (inMillisecond < 0) || (inMillisecond > 999))
        throw VRangeException(VString("VTimeOfDay::set: %02d:%02d:%02d.%03d is an invalid value.", inHour, inMinute, inSecond, inMillisecond));

    mHour = inHour;
    mMinute = inMinute;
    mSecond = inSecond;
    mMillisecond = inMillisecond;

    ASSERT_INVARIANT();
    }

//lint -e421 "Caution -- function 'abort(void)' is considered dangerous [MISRA Rule 126]"
void VTimeOfDay::assertInvariant() const
    {
    V_ASSERT(mHour >= 0);
    V_ASSERT(mHour < 24);
    V_ASSERT(mMinute >= 0);
    V_ASSERT(mMinute < 60);
    V_ASSERT(mSecond >= 0);
    V_ASSERT(mSecond < 60);
    V_ASSERT(mMillisecond >= 0);
    V_ASSERT(mMillisecond < 1000);
    }

