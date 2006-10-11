/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vinstant.h"
#include "vchar.h"
#include "vstring.h"
#include "vexception.h"

#undef sscanf

// VDuration -----------------------------------------------------------------

// The static constants are returned with accessor methods because of the
// unpredictable order of static initialization. So we use local statics
// that are initialized on first use, and return them from these accessors.

// static
const VDuration& VDuration::UNSPECIFIED()
    {
    static const VDuration kUNSPECIFIED(V_MAX_S64);
    return kUNSPECIFIED;
    }

// static
const VDuration& VDuration::NEGATIVE_INFINITY()
    {
    static const VDuration kNEGATIVE_INFINITY(V_MIN_S64);
    return kNEGATIVE_INFINITY;
    }

// static
const VDuration& VDuration::ZERO()
    {
    static const VDuration kZERO(CONST_S64(0));
    return kZERO;
    }

// static
const VDuration& VDuration::MILLISECOND()
    {
    static const VDuration kMILLISECOND(CONST_S64(1));
    return kMILLISECOND;
    }

// static
const VDuration& VDuration::SECOND()
    {
    static const VDuration kSECOND(kMillisecondsPerSecond);
    return kSECOND;
    }

// static
const VDuration& VDuration::MINUTE()
    {
    static const VDuration kMINUTE(kMillisecondsPerMinute);
    return kMINUTE;
    }

// static
const VDuration& VDuration::HOUR()
    {
    static const VDuration kHOUR(kMillisecondsPerHour);
    return kHOUR;
    }

// static
const VDuration& VDuration::DAY()
    {
    static const VDuration kDAY(kMillisecondsPerDay);
    return kDAY;
    }

// static
const VDuration& VDuration::POSITIVE_INFINITY()
    {
    static const VDuration kPOSITIVE_INFINITY(V_MAX_S64 - CONST_S64(1));
    return kPOSITIVE_INFINITY;
    }

VDuration& VDuration::operator+=(const VDuration& forwardOffset)
    {
    // For normal values:
    // Adding +/- infinity results in +/- infinity.
    // All other operations result in no change.
    // For infinite values:
    // Adding the opposite infinity results in zero.
    // All other operations result in no change.
    if (this->isSpecific())
        {
        if (forwardOffset.isSpecific())
            mDurationMilliseconds += forwardOffset.getDurationMilliseconds();
        else if (forwardOffset == VDuration::POSITIVE_INFINITY())
            *this = VDuration::POSITIVE_INFINITY();
        else if (forwardOffset == VDuration::NEGATIVE_INFINITY())
            *this = VDuration::NEGATIVE_INFINITY();
        }
    else if ((*this == VDuration::POSITIVE_INFINITY()) && (forwardOffset == VDuration::NEGATIVE_INFINITY()))
        {
        *this = VDuration::ZERO();
        }
    else if ((*this == VDuration::NEGATIVE_INFINITY()) && (forwardOffset == VDuration::POSITIVE_INFINITY()))
        {
        *this = VDuration::ZERO();
        }

    return *this;
    }

VDuration& VDuration::operator-=(const VDuration& backwardOffset)
    {
    // For normal values:
    // Subtracting +/- infinity results in the opposite +/- infinity.
    // All other operations result in no change.
    // For infinite values:
    // Subtracting the the same infinity results in zero.
    // All other operations result in no change.
    if (this->isSpecific())
        {
        if (backwardOffset.isSpecific())
            mDurationMilliseconds -= backwardOffset.getDurationMilliseconds();
        else if (backwardOffset == VDuration::POSITIVE_INFINITY())
            *this = VDuration::NEGATIVE_INFINITY();
        else if (backwardOffset == VDuration::NEGATIVE_INFINITY())
            *this = VDuration::POSITIVE_INFINITY();
        }
    else if ((*this == VDuration::POSITIVE_INFINITY()) && (backwardOffset == VDuration::POSITIVE_INFINITY()))
        {
        *this = VDuration::ZERO();
        }
    else if ((*this == VDuration::NEGATIVE_INFINITY()) && (backwardOffset == VDuration::NEGATIVE_INFINITY()))
        {
        *this = VDuration::ZERO();
        }

    return *this;
    }

VDuration& VDuration::operator*=(Vs64 multiplier)
    {
    // Normal values use simple multiplication.
    // Anything multiplied by zero is zero.
    // +/- infinity can be flipped to the opposite +/- infinity with a
    // negative multiplier, and set to zero with a zero multiplier.
    // All other operations result in no change.
    if (this->isSpecific())
        {
        mDurationMilliseconds *= multiplier;
        }
    else if (multiplier == 0)
        {
        *this = VDuration::ZERO();
        }
    else if (multiplier < 0)
        {
        if (*this == VDuration::POSITIVE_INFINITY())
            *this = VDuration::NEGATIVE_INFINITY();
        else if (*this == VDuration::NEGATIVE_INFINITY())
            *this = VDuration::POSITIVE_INFINITY();
        }

    return *this;
    }

VDuration& VDuration::operator/=(int divisor)
    {
    // Normal values use simple division, with divide-by-zero yielding
    // +/- infinity.
    // All other operations result in no change.
    if (this->isSpecific())
        {
        if (divisor != 0)
            mDurationMilliseconds /= divisor;
        else
            {
            // Dividing a normal duration by zero.
            if (mDurationMilliseconds >= 0)
                *this = VDuration::POSITIVE_INFINITY();
            else
                *this = VDuration::NEGATIVE_INFINITY();
            }
        }

    return *this;
    }

VDuration& VDuration::operator%=(const VDuration& divisor)
    {
    // Normal values use simple remainder division, with mod-by-zero yielding
    // no change.
    // All other operations result in no change.
    if (this->isSpecific())
        {
        if (divisor.isSpecific() && (divisor != VDuration::ZERO()))
            mDurationMilliseconds %= divisor.mDurationMilliseconds;
        }

    return *this;
    }

VDuration VDuration::operator-() const
    {
    // Negating a normal value is obvious.
    // Negating +/- infinity flips to the opposite +/- infinity.
    // All other operations result in the same value returned.
    if (this->isSpecific())
        return VDuration(-mDurationMilliseconds);
    else if (*this == VDuration::POSITIVE_INFINITY())
        return VDuration::NEGATIVE_INFINITY();
    else if (*this == VDuration::NEGATIVE_INFINITY())
        return VDuration::POSITIVE_INFINITY();
    else
        return *this;
    }

VDuration operator/(const VDuration& d, int divisor)
    {
    // Re-use in-place division code.
    VDuration result = d;
    result /= divisor;
    return result;
    }

VDuration operator%(const VDuration& d, const VDuration& divisor)
    {
    // Re-use in-place mod code.
    VDuration result = d;
    result %= divisor;
    return result;
    }

// static
bool VDuration::_complexGT(const VDuration& d1, const VDuration& d2)
    {
    if (VDuration::canCompareValues(d1, d2))
        return d1.mDurationMilliseconds > d2.mDurationMilliseconds;

    if (d1 == VDuration::POSITIVE_INFINITY())
        return true;

    if ((d2 == VDuration::NEGATIVE_INFINITY()) && (d1 != VDuration::UNSPECIFIED()))
        return true;

    return false;
    }

// static
bool VDuration::_complexGTE(const VDuration& d1, const VDuration& d2)
    {
    if (VDuration::canCompareValues(d1, d2))
        return d1.mDurationMilliseconds >= d2.mDurationMilliseconds;

    if (d1 == VDuration::POSITIVE_INFINITY())
        return d2 != VDuration::POSITIVE_INFINITY();

    if ((d2 == VDuration::NEGATIVE_INFINITY()) && (d1 != VDuration::UNSPECIFIED()))
        return true;

    return false;
    }

// static
bool VDuration::_complexLT(const VDuration& d1, const VDuration& d2)
    {
    if (VDuration::canCompareValues(d1, d2))
        return d1.mDurationMilliseconds < d2.mDurationMilliseconds;

    if (d1 == VDuration::NEGATIVE_INFINITY())
        return true;

    if ((d2 == VDuration::POSITIVE_INFINITY()) && (d1 != VDuration::UNSPECIFIED()))
        return true;

    return false;
    }

// static
bool VDuration::_complexLTE(const VDuration& d1, const VDuration& d2)
    {
    if (VDuration::canCompareValues(d1, d2))
        return d1.mDurationMilliseconds <= d2.mDurationMilliseconds;

    if (d1 == VDuration::NEGATIVE_INFINITY())
        return d2 != VDuration::NEGATIVE_INFINITY();

    if ((d2 == VDuration::POSITIVE_INFINITY()) && (d1 != VDuration::UNSPECIFIED()))
        return true;

    return false;
    }

// static
VDuration VDuration::_complexAdd(const VDuration& d1, const VDuration& d2)
    {
    // Re-use in-place addition code.
    VDuration result = d1;
    result += d2;
    return result;
    }

// static
VDuration VDuration::_complexSubtract(const VDuration& d1, const VDuration& d2)
    {
    // Re-use in-place subtraction code.
    VDuration result = d1;
    result -= d2;
    return result;
    }

// static
VDuration VDuration::_complexMultiply(const VDuration& d, Vs64 multiplier)
    {
    // Re-use in-place multiplication code.
    VDuration result = d;
    result *= multiplier;
    return result;
    }

// static
VDuration VDuration::_complexMin(const VDuration& d1, const VDuration& d2)
    {
    if (VDuration::canCompareValues(d1, d2))
        return (d1 < d2) ? d1 : d2;

    // If either value is UNSPECIFIED, min is UNSPECIFIED
    if ((d1 == VDuration::UNSPECIFIED()) || (d2 == VDuration::UNSPECIFIED()))
        return VDuration::UNSPECIFIED();

    // If either value is -i, min is -i.
    if ((d1 == VDuration::NEGATIVE_INFINITY()) || (d2 == VDuration::NEGATIVE_INFINITY()))
        return VDuration::NEGATIVE_INFINITY();

    // One of them must be +i. min is the other.
    if (d1 == VDuration::POSITIVE_INFINITY())
        return d2;
    else
        return d1;
    }

// static
VDuration VDuration::_complexMax(const VDuration& d1, const VDuration& d2)
    {
    if (VDuration::canCompareValues(d1, d2))
        return (d1 > d2) ? d1 : d2;

    // If either value is UNSPECIFIED, min is UNSPECIFIED
    if ((d1 == VDuration::UNSPECIFIED()) || (d2 == VDuration::UNSPECIFIED()))
        return VDuration::UNSPECIFIED();

    // If either value is +i, max is +i.
    if ((d1 == VDuration::POSITIVE_INFINITY()) || (d2 == VDuration::POSITIVE_INFINITY()))
        return VDuration::POSITIVE_INFINITY();

    // One of them must be -i. max is the other.
    if (d1 == VDuration::NEGATIVE_INFINITY())
        return d2;
    else
        return d1;
    }

// static
VDuration VDuration::_complexAbs(const VDuration& d)
    {
    if (d.isSpecific())
        return (d.mDurationMilliseconds < CONST_S64(0)) ? d : VDuration(-d.mDurationMilliseconds);

    if ((d == VDuration::NEGATIVE_INFINITY()) || (d == VDuration::POSITIVE_INFINITY()))
        return VDuration::POSITIVE_INFINITY();

    return d; // presumably UNSPECIFIED
    }

// VInstantStruct ------------------------------------------------------------

VInstantStruct::VInstantStruct(const VDate& date, const VTimeOfDay& timeOfDay) :
mYear(date.getYear()),
mMonth(date.getMonth()),
mDay(date.getDay()),
mHour(timeOfDay.getHour()),
mMinute(timeOfDay.getMinute()),
mSecond(timeOfDay.getSecond()),
mMillisecond(timeOfDay.getMillisecond()),
mDayOfWeek(0)
    {
    }

// VInstant ------------------------------------------------------------------

static const Vs64 kInfinitePastInternalValue = V_MIN_S64;
static const Vs64 kInfiniteFutureInternalValue = V_MAX_S64 - CONST_S64(1);
static const Vs64 kNeverOccurredInternalValue = V_MAX_S64;

// The static constants are returned with accessor methods because of the
// unpredictable order of static initialization. So we use local statics
// that are initialized on first use, and return them from these accessors.

// static
const VInstant& VInstant::INFINITE_PAST()
    {
    static const VInstant kINFINITE_PAST(kInfinitePastInternalValue);
    return kINFINITE_PAST;
    }

const VInstant& VInstant::INFINITE_FUTURE()
    {
    static const VInstant kINFINITE_FUTURE(kInfiniteFutureInternalValue);
    return kINFINITE_FUTURE;
    }

const VInstant& VInstant::NEVER_OCCURRED()
    {
    static const VInstant kNEVER_OCCURRED(kNeverOccurredInternalValue);
    return kNEVER_OCCURRED;
    }

const VString& VInstant::UTC_TIME_ZONE_ID()
    {
    static const VString kUTC_TIME_ZONE_ID("UTC");
    return kUTC_TIME_ZONE_ID;
    }

const VString& VInstant::LOCAL_TIME_ZONE_ID()
    {
    static const VString kLOCAL_TIME_ZONE_ID; // empty string means local time zone
    return kLOCAL_TIME_ZONE_ID;
    }

Vs64 VInstant::gSimulatedClockOffset(0);

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

VInstant::VInstant() :
mValue(0)
    {
    VInstant::setNow();
    }

VInstant& VInstant::operator=(const VInstant& i)
    {
    if (this != &i)
        {
        mValue = i.getValue();
        }

    return *this;
    }

VInstant& VInstant::operator+=(const VDuration& forwardOffsetDuration)
    {
    if (this->isSpecific() && forwardOffsetDuration.isSpecific())
        mValue += forwardOffsetDuration.getDurationMilliseconds();
    else if (this->isSpecific() && (forwardOffsetDuration == VDuration::NEGATIVE_INFINITY()))
        *this = VInstant::INFINITE_PAST();
    else if (this->isSpecific() && (forwardOffsetDuration == VDuration::POSITIVE_INFINITY()))
        *this = VInstant::INFINITE_FUTURE();

    return *this;
    }

VInstant& VInstant::operator-=(const VDuration& backwardOffsetDuration)
    {
    if (this->isSpecific() && backwardOffsetDuration.isSpecific())
        mValue -= backwardOffsetDuration.getDurationMilliseconds();
    else if (this->isSpecific() && (backwardOffsetDuration == VDuration::NEGATIVE_INFINITY()))
        *this = VInstant::INFINITE_FUTURE();
    else if (this->isSpecific() && (backwardOffsetDuration == VDuration::POSITIVE_INFINITY()))
        *this = VInstant::INFINITE_PAST();

    return *this;
    }

void VInstant::setNow()
    {
    mValue = VInstant::_platform_now();
    }

void VInstant::getUTCString(VString& s, bool fileNameSafe) const
    {
    if (this->isSpecific())
        {
        VInstantStruct    when;

        VInstant::_platform_offsetToUTCStruct(mValue, when);

        if (fileNameSafe)
            s.format("%d%02d%02d%02d%02d%02d%03d",
                when.mYear, when.mMonth, when.mDay, when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
        else
            s.format("%d-%02d-%02d %02d:%02d:%02d.%03d UTC",
                when.mYear, when.mMonth, when.mDay, when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
        }
    else if (*this == VInstant::INFINITE_PAST())
        s = "PAST";
    else if (*this == VInstant::INFINITE_FUTURE())
        s = "FUTURE";
    else /* NEVER_OCCURRED */
        s = "NEVER";
    }

void VInstant::setUTCString(const VString& s)
    {
    if (s == "PAST")
        mValue = kInfinitePastInternalValue;
    else if (s == "FUTURE")
        mValue = kInfiniteFutureInternalValue;
    else if (s == "NEVER")
        mValue = kNeverOccurredInternalValue;
    else
        {
        VInstantStruct when;
        when.mDayOfWeek = 0;

        ::sscanf(s, "%d-%02d-%02d %02d:%02d:%02d.%03d UTC", &when.mYear, &when.mMonth, &when.mDay, &when.mHour, &when.mMinute, &when.mSecond, &when.mMillisecond);

        mValue = VInstant::_platform_offsetFromUTCStruct(when);
        }
    }

void VInstant::getLocalString(VString& s, bool fileNameSafe) const
    {
    if (this->isSpecific())
        {
        VInstantStruct    when;

        VInstant::_platform_offsetToLocalStruct(mValue, when);

        if (fileNameSafe)
            s.format("%d%02d%02d%02d%02d%02d%03d",
                when.mYear, when.mMonth, when.mDay, when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
        else
            s.format("%d-%02d-%02d %02d:%02d:%02d.%03d",
                when.mYear, when.mMonth, when.mDay, when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
        }
    else if (*this == VInstant::INFINITE_PAST())
        s = "PAST";
    else if (*this == VInstant::INFINITE_FUTURE())
        s = "FUTURE";
    else /* NEVER_OCCURRED */
        s = "NEVER";
    }

void VInstant::setLocalString(const VString& s)
    {
    if (s == "PAST")
        mValue = kInfinitePastInternalValue;
    else if (s == "FUTURE")
        mValue = kInfiniteFutureInternalValue;
    else if (s == "NEVER")
        mValue = kNeverOccurredInternalValue;
    else
        {
        VInstantStruct when;
        when.mDayOfWeek = 0;

        ::sscanf(s, "%d-%02d-%02d %02d:%02d:%02d.%03d UTC", &when.mYear, &when.mMonth, &when.mDay, &when.mHour, &when.mMinute, &when.mSecond, &when.mMillisecond);

        mValue = VInstant::_platform_offsetFromLocalStruct(when);
        }
    }

void VInstant::getValues(VDate& date, VTimeOfDay& timeOfDay, const VString& timeZoneID) const
    {
    VInstantStruct    when;

    if (timeZoneID == VInstant::LOCAL_TIME_ZONE_ID())
        VInstant::_platform_offsetToLocalStruct(mValue, when);
    else if (timeZoneID == VInstant::UTC_TIME_ZONE_ID())
        VInstant::_platform_offsetToUTCStruct(mValue, when);
    else if (gRemoteTimeZoneConverter == NULL)
        throw VException("Request for remote time zone conversion (%s) without a converter.", timeZoneID.chars());
    else
        gRemoteTimeZoneConverter->offsetToRTZStruct(mValue, timeZoneID, when);

    date.set(when.mYear, when.mMonth, when.mDay);
    timeOfDay.set(when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
    }

VDate VInstant::getLocalDate() const
    {
    return this->getDate(VInstant::LOCAL_TIME_ZONE_ID());
    }

VDate VInstant::getDate(const VString& timeZoneID) const
    {
    VInstantStruct    when;

    if (timeZoneID == VInstant::LOCAL_TIME_ZONE_ID())
        VInstant::_platform_offsetToLocalStruct(mValue, when);
    else if (timeZoneID == VInstant::UTC_TIME_ZONE_ID())
        VInstant::_platform_offsetToUTCStruct(mValue, when);
    else if (gRemoteTimeZoneConverter == NULL)
        throw VException("Request for remote time zone conversion (%s) without a converter.", timeZoneID.chars());
    else
        gRemoteTimeZoneConverter->offsetToRTZStruct(mValue, timeZoneID, when);

    return VDate(when.mYear, when.mMonth, when.mDay);
    }

VTimeOfDay VInstant::getLocalTimeOfDay() const
    {
    return this->getTimeOfDay(VInstant::LOCAL_TIME_ZONE_ID());
    }

VTimeOfDay VInstant::getTimeOfDay(const VString& timeZoneID) const
    {
    VInstantStruct    when;

    if (timeZoneID == VInstant::LOCAL_TIME_ZONE_ID())
        VInstant::_platform_offsetToLocalStruct(mValue, when);
    else if (timeZoneID == VInstant::UTC_TIME_ZONE_ID())
        VInstant::_platform_offsetToUTCStruct(mValue, when);
    else if (gRemoteTimeZoneConverter == NULL)
        throw VException("Request for remote time zone conversion (%s) without a converter.", timeZoneID.chars());
    else
        gRemoteTimeZoneConverter->offsetToRTZStruct(mValue, timeZoneID, when);

    return VTimeOfDay(when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
    }

VDateAndTime VInstant::getLocalDateAndTime() const
    {
    return this->getDateAndTime(VInstant::LOCAL_TIME_ZONE_ID());
    }

VDateAndTime VInstant::getDateAndTime(const VString& timeZoneID) const
    {
    VInstantStruct    when;

    if (timeZoneID == VInstant::LOCAL_TIME_ZONE_ID())
        VInstant::_platform_offsetToLocalStruct(mValue, when);
    else if (timeZoneID == VInstant::UTC_TIME_ZONE_ID())
        VInstant::_platform_offsetToUTCStruct(mValue, when);
    else if (gRemoteTimeZoneConverter == NULL)
        throw VException("Request for remote time zone conversion (%s) without a converter.", timeZoneID.chars());
    else
        gRemoteTimeZoneConverter->offsetToRTZStruct(mValue, timeZoneID, when);

    return VDateAndTime(when.mYear, when.mMonth, when.mDay, when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
    }

void VInstant::setLocalDateAndTime(const VDateAndTime& dt)
    {
    this->setDateAndTime(dt, VInstant::LOCAL_TIME_ZONE_ID());
    }

void VInstant::setDateAndTime(const VDateAndTime& dt, const VString& timeZoneID)
    {
    VInstantStruct    when(dt.getDate(), dt.getTimeOfDay());

    if (timeZoneID == VInstant::LOCAL_TIME_ZONE_ID())
        mValue = VInstant::_platform_offsetFromLocalStruct(when);
    else if (timeZoneID == VInstant::UTC_TIME_ZONE_ID())
        mValue = VInstant::_platform_offsetFromUTCStruct(when);
    else if (gRemoteTimeZoneConverter == NULL)
        throw VException("Request for remote time zone conversion (%s) without a converter.", timeZoneID.chars());
    else
        mValue = gRemoteTimeZoneConverter->offsetFromRTZStruct(timeZoneID, when);
    }

void VInstant::setValues(const VDate& date, const VTimeOfDay& timeOfDay, const VString& timeZoneID)
    {
    VInstantStruct    when(date, timeOfDay);

    if (timeZoneID == VInstant::LOCAL_TIME_ZONE_ID())
        mValue = VInstant::_platform_offsetFromLocalStruct(when);
    else if (timeZoneID == VInstant::UTC_TIME_ZONE_ID())
        mValue = VInstant::_platform_offsetFromUTCStruct(when);
    else if (gRemoteTimeZoneConverter == NULL)
        throw VException("Request for remote time zone conversion (%s) without a converter.", timeZoneID.chars());
    else
        mValue = gRemoteTimeZoneConverter->offsetFromRTZStruct(timeZoneID, when);
    }

static const int kSecondsPerDay = 86400;

Vs64 VInstant::getLocalOffsetMilliseconds() const
    {
    /*
    Return the offset, in milliseconds, of the local time zone, at this
    indicated instant.
    */

    VInstantStruct    localStruct;
    VInstant::_platform_offsetToLocalStruct(mValue, localStruct);

    VInstantStruct    utcStruct;
    VInstant::_platform_offsetToUTCStruct(mValue, utcStruct);

    // Now we have two structs for the instant.
    // A little math will tell us the difference in h/m/s.
    // We know the delta cannot be more than 24 hours.
    // No need to worry about milliseconds; time zone offset resolutions are only in minutes.
    // (We could presumably even ignore the seconds for that matter.)
    int deltaSeconds;
    int localSecondsOfDay = (3600 * localStruct.mHour) + (60 * localStruct.mMinute) + localStruct.mSecond;
    int utcSecondsOfDay = (3600 * utcStruct.mHour) + (60 * utcStruct.mMinute) + utcStruct.mSecond;

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
bool VInstant::_complexGT(const VInstant& i1, const VInstant& i2)
    {
    // We also handle the simple case correctly.
    if (i1.isSpecific() && i2.isSpecific())
        return i1 > i2;

    // If they're the same internal value, they're equal. That's NOT i1>i2.
    if (i1.mValue == i2.mValue)
        return false;

    // Not the same internal value. If i2 is past, then i1>i2.
    if ((i2 == VInstant::INFINITE_PAST()) && i1.isComparable())
        return true;

    // Not the same internal value. If i1 is future, then i1>i2.
    if ((i1 == VInstant::INFINITE_FUTURE()) && i2.isComparable())
        return true;

    // None the above. Condition does not hold.
    return false;
    }

// static
bool VInstant::_complexGTE(const VInstant& i1, const VInstant& i2)
    {
    // We also handle the simple case correctly.
    if (i1.isSpecific() && i2.isSpecific())
        return i1 >= i2;

    // If they're the same internal value, they're equal. So i1>=i2.
    if (i1.mValue == i2.mValue)
        return true;

    // Not the same internal value. If i2 is past, then i1>i2.
    if ((i2 == VInstant::INFINITE_PAST()) && i1.isComparable())
        return true;

    // Not the same internal value. If i1 is future, then i1>i2.
    if ((i1 == VInstant::INFINITE_FUTURE()) && i2.isComparable())
        return true;

    // None the above. Condition does not hold.
    return false;
    }

// static
bool VInstant::_complexLT(const VInstant& i1, const VInstant& i2)
    {
    // We also handle the simple case correctly.
    if (i1.isSpecific() && i2.isSpecific())
        return i1 < i2;

    // If they're the same internal value, they're equal. That's NOT i1<i2.
    if (i1.mValue == i2.mValue)
        return false;

    // Not the same internal value. If i1 is past, then i1<i2.
    if ((i1 == VInstant::INFINITE_PAST()) && i2.isComparable())
        return true;

    // Not the same internal value. If i2 is future, then i1<i2.
    if ((i2 == VInstant::INFINITE_FUTURE()) && i1.isComparable())
        return true;

    // None the above. Condition does not hold.
    return false;
    }

// static
bool VInstant::_complexLTE(const VInstant& i1, const VInstant& i2)
    {
    // We also handle the simple case correctly.
    if (i1.isSpecific() && i2.isSpecific())
        return i1 <= i2;

    // If they're the same internal value, they're equal. So i1<=i2.
    if (i1.mValue == i2.mValue)
        return true;

    // Not the same internal value. If i1 is past, then i1<i2.
    if ((i1 == VInstant::INFINITE_PAST()) && i2.isComparable())
        return true;

    // Not the same internal value. If i2 is future, then i1<i2.
    if ((i2 == VInstant::INFINITE_FUTURE()) && i1.isComparable())
        return true;

    // None the above. Condition does not hold.
    return false;
    }

// static
Vs64 VInstant::snapshot()
    {
    return VInstant::_platform_snapshot();
    }

// static
VDuration VInstant::snapshotDelta(Vs64 snapshotValue)
    {
    return VDuration::MILLISECOND() * (VInstant::_platform_snapshot() - snapshotValue);
    }

// static
void VInstant::incrementSimulatedClockOffset(Vs64 delta)
    {
    gSimulatedClockOffset += delta;
    }

// static
void VInstant::setSimulatedClockOffset(Vs64 offsetValue)
    {
    gSimulatedClockOffset = offsetValue;
    }

// VDate ---------------------------------------------------------------------

const VChar VDate::kLocalDateSeparator('/');

VDate::VDate() :
mYear(0),
mMonth(1),
mDay(1)
    {
    ASSERT_INVARIANT();
    }

VDate::VDate(const VString& timeZoneID)
    {
    VInstant    now;
    VDate       nowDate = now.getDate(timeZoneID);
    mYear = nowDate.getYear();
    mMonth = nowDate.getMonth();
    mDay = nowDate.getDay();

    ASSERT_INVARIANT();
    }

VDate::VDate(int inYear, int inMonth, int inDay)
    {
    if ((inMonth < 1) || (inMonth > 12) ||
        (inDay   < 1) || (inDay   > 32)) // 32 allowed when incrementing the date
        throw VRangeException(VString("VDate: %d-%02d-%02d is an invalid value.", inYear, inMonth, inDay));

    mYear = inYear;
    mMonth = inMonth;
    mDay = inDay;

    ASSERT_INVARIANT();
    }

int VDate::getYear() const
    {
    ASSERT_INVARIANT();

    return mYear;
    }

int VDate::getMonth() const
    {
    ASSERT_INVARIANT();

    return mMonth;
    }

int VDate::getDay() const
    {
    ASSERT_INVARIANT();

    return mDay;
    }

int VDate::getDayOfWeek() const
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
    Vs64    offset = VInstant::_platform_offsetFromUTCStruct(when);

    // Now reverse to get the mDayOfWeek filled out.
    VInstant::_platform_offsetToUTCStruct(offset, when);

    return when.mDayOfWeek;
    }

void VDate::set(int inYear, int inMonth, int inDay)
    {
    ASSERT_INVARIANT();

    if ((inMonth < 1) || (inMonth > 12) ||
        (inDay   < 1) || (inDay   > 32)) // 32 allowed when incrementing the date
        throw VRangeException(VString("VDate::set: %d-%02d-%02d is an invalid value.", inYear, inMonth, inDay));

    mYear = inYear;
    mMonth = inMonth;
    mDay = inDay;

    ASSERT_INVARIANT();
    }

void VDate::setYear(int year)
    {
    ASSERT_INVARIANT();

    mYear = year;

    ASSERT_INVARIANT();
    }

void VDate::setMonth(int month)
    {
    ASSERT_INVARIANT();

    mMonth = month;

    ASSERT_INVARIANT();
    }

void VDate::setDay(int day)
    {
    ASSERT_INVARIANT();

    mDay = day;

    ASSERT_INVARIANT();
    }

//lint -e421 "Caution -- function 'abort(void)' is considered dangerous [MISRA Rule 126]"
void VDate::assertInvariant() const
    {
    V_ASSERT(mMonth > 0);
    V_ASSERT(mMonth < 13);
    V_ASSERT(mDay > 0);
    V_ASSERT(mDay < 33); // 32 allowed when incrementing the date
    }

// VTimeOfDay ----------------------------------------------------------------

const VChar VTimeOfDay::kLocalTimeSeparator(':');

VTimeOfDay::VTimeOfDay() :
mHour(0),
mMinute(0),
mSecond(0),
mMillisecond(0)
    {
    ASSERT_INVARIANT();
    }

VTimeOfDay::VTimeOfDay(const VString& timeZoneID)
    {
    VInstant    now;
    VTimeOfDay  nowTimeOfDay = now.getTimeOfDay(timeZoneID);
    mHour = nowTimeOfDay.getHour();
    mMinute = nowTimeOfDay.getMinute();
    mSecond = nowTimeOfDay.getSecond();
    mMillisecond = nowTimeOfDay.getMillisecond();

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

int VTimeOfDay::getHour() const
    {
    ASSERT_INVARIANT();

    return mHour;
    }

int VTimeOfDay::getMinute() const
    {
    ASSERT_INVARIANT();

    return mMinute;
    }

int VTimeOfDay::getSecond() const
    {
    ASSERT_INVARIANT();

    return mSecond;
    }

int VTimeOfDay::getMillisecond() const
    {
    ASSERT_INVARIANT();

    return mMillisecond;
    }

void VTimeOfDay::set(int hour, int minute, int second, int millisecond)
    {
    // No point in doing our own ASSERT_INVARIANT here, because each
    // setter we call will do it anyway.
    this->setHour(hour);
    this->setMinute(minute);
    this->setSecond(second);
    this->setMillisecond(millisecond);
    }

void VTimeOfDay::setHour(int hour)
    {
    ASSERT_INVARIANT();

    if ((hour < 0) || (hour > 23))
        throw VRangeException(VString("VTimeOfDay::set/setHour: %d is an invalid value.", hour));

    mHour = hour;

    ASSERT_INVARIANT();
    }

void VTimeOfDay::setMinute(int minute)
    {
    ASSERT_INVARIANT();

    if ((minute < 0) || (minute > 59))
        throw VRangeException(VString("VTimeOfDay::set/setMinute: %d is an invalid value.", minute));

    mMinute = minute;

    ASSERT_INVARIANT();
    }

void VTimeOfDay::setSecond(int second)
    {
    ASSERT_INVARIANT();

    if ((second < 0) || (second > 59))
        throw VRangeException(VString("VTimeOfDay::set/setSecond: %d is an invalid value.", second));

    mSecond = second;

    ASSERT_INVARIANT();
    }

void VTimeOfDay::setMillisecond(int millisecond)
    {
    ASSERT_INVARIANT();

    if ((millisecond < 0) || (millisecond > 999))
        throw VRangeException(VString("VTimeOfDay::set/setMillisecond: %d is an invalid value.", millisecond));

    mMillisecond = millisecond;

    ASSERT_INVARIANT();
    }

void VTimeOfDay::setToStartOfDay()
    {
    // No need for ASSERT_INVARIANT, since set() will call it.
    this->set(0, 0, 0, 0);
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

// VDateAndTime --------------------------------------------------------------

VDateAndTime::VDateAndTime(const VString& timeZoneID)
    {
    // This is more efficient than simply letting both the mDate and
    // mTimeOfDay construct themselves from the timeZoneID, because here we
    // can do a single time conversion, rather than letting each one do the
    // conversion separately.
    VInstant    now;
    VDate       nowDate;
    VTimeOfDay  nowTimeOfDay;

    now.getValues(nowDate, nowTimeOfDay, timeZoneID);

    mDate.setYear(nowDate.getYear());
    mDate.setMonth(nowDate.getMonth());
    mDate.setDay(nowDate.getDay());
    mTimeOfDay.setHour(nowTimeOfDay.getHour());
    mTimeOfDay.setMinute(nowTimeOfDay.getMinute());
    mTimeOfDay.setSecond(nowTimeOfDay.getSecond());
    mTimeOfDay.setMillisecond(nowTimeOfDay.getMillisecond());
    }

