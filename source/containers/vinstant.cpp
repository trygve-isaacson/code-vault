/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vinstant.h"
#include "vtypes_internal.h"

#include "vchar.h"
#include "vstring.h"
#include "vexception.h"

#undef sscanf

// VDuration -----------------------------------------------------------------

// The static constants are returned with accessor methods because of the
// unpredictable order of static initialization. So we use local statics
// that are initialized on first use, and return them from these accessors.

// static
const VDuration& VDuration::UNSPECIFIED() {
    static const VDuration kUNSPECIFIED(V_MAX_S64);
    return kUNSPECIFIED;
}

// static
const VDuration& VDuration::NEGATIVE_INFINITY() {
    static const VDuration kNEGATIVE_INFINITY(V_MIN_S64);
    return kNEGATIVE_INFINITY;
}

// static
const VDuration& VDuration::ZERO() {
    static const VDuration kZERO(CONST_S64(0));
    return kZERO;
}

// static
const VDuration& VDuration::MILLISECOND() {
    static const VDuration kMILLISECOND(CONST_S64(1));
    return kMILLISECOND;
}

// static
const VDuration& VDuration::SECOND() {
    static const VDuration kSECOND(kMillisecondsPerSecond);
    return kSECOND;
}

// static
const VDuration& VDuration::MINUTE() {
    static const VDuration kMINUTE(kMillisecondsPerMinute);
    return kMINUTE;
}

// static
const VDuration& VDuration::HOUR() {
    static const VDuration kHOUR(kMillisecondsPerHour);
    return kHOUR;
}

// static
const VDuration& VDuration::DAY() {
    static const VDuration kDAY(kMillisecondsPerDay);
    return kDAY;
}

// static
const VDuration& VDuration::POSITIVE_INFINITY() {
    static const VDuration kPOSITIVE_INFINITY(V_MAX_S64 - CONST_S64(1));
    return kPOSITIVE_INFINITY;
}

VDuration::VDuration(const VInstant& sinceWhen) :
    mDurationMilliseconds(VDuration(VInstant() - sinceWhen).getDurationMilliseconds()) {
}

// static
VDuration VDuration::createFromDurationString(const VString& s) {
    VDuration duration;
    duration.setDurationString(s);
    return duration;
}

VDuration& VDuration::operator+=(const VDuration& forwardOffset) {
    // For normal values:
    // Adding +/- infinity results in +/- infinity.
    // All other operations result in no change.
    // For infinite values:
    // Adding the opposite infinity results in zero.
    // All other operations result in no change.
    if (this->isSpecific()) {
        if (forwardOffset.isSpecific()) {
            mDurationMilliseconds += forwardOffset.getDurationMilliseconds();
        } else if (forwardOffset == VDuration::POSITIVE_INFINITY()) {
            *this = VDuration::POSITIVE_INFINITY();
        } else if (forwardOffset == VDuration::NEGATIVE_INFINITY()) {
            *this = VDuration::NEGATIVE_INFINITY();
        }
    } else if ((*this == VDuration::POSITIVE_INFINITY()) && (forwardOffset == VDuration::NEGATIVE_INFINITY())) {
        *this = VDuration::ZERO();
    } else if ((*this == VDuration::NEGATIVE_INFINITY()) && (forwardOffset == VDuration::POSITIVE_INFINITY())) {
        *this = VDuration::ZERO();
    }

    return *this;
}

VDuration& VDuration::operator-=(const VDuration& backwardOffset) {
    // For normal values:
    // Subtracting +/- infinity results in the opposite +/- infinity.
    // All other operations result in no change.
    // For infinite values:
    // Subtracting the the same infinity results in zero.
    // All other operations result in no change.
    if (this->isSpecific()) {
        if (backwardOffset.isSpecific()) {
            mDurationMilliseconds -= backwardOffset.getDurationMilliseconds();
        } else if (backwardOffset == VDuration::POSITIVE_INFINITY()) {
            *this = VDuration::NEGATIVE_INFINITY();
        } else if (backwardOffset == VDuration::NEGATIVE_INFINITY()) {
            *this = VDuration::POSITIVE_INFINITY();
        }
    } else if ((*this == VDuration::POSITIVE_INFINITY()) && (backwardOffset == VDuration::POSITIVE_INFINITY())) {
        *this = VDuration::ZERO();
    } else if ((*this == VDuration::NEGATIVE_INFINITY()) && (backwardOffset == VDuration::NEGATIVE_INFINITY())) {
        *this = VDuration::ZERO();
    }

    return *this;
}

VDuration& VDuration::operator*=(Vs64 multiplier) {
    // Normal values use simple multiplication.
    // Anything multiplied by zero is zero.
    // +/- infinity can be flipped to the opposite +/- infinity with a
    // negative multiplier, and set to zero with a zero multiplier.
    // All other operations result in no change.
    if (this->isSpecific()) {
        mDurationMilliseconds *= multiplier;
    } else if (multiplier == 0) {
        *this = VDuration::ZERO();
    } else if (multiplier < 0) {
        if (*this == VDuration::POSITIVE_INFINITY()) {
            *this = VDuration::NEGATIVE_INFINITY();
        } else if (*this == VDuration::NEGATIVE_INFINITY()) {
            *this = VDuration::POSITIVE_INFINITY();
        }
    }

    return *this;
}

VDuration& VDuration::operator/=(int divisor) {
    // Normal values use simple division, with divide-by-zero yielding
    // +/- infinity.
    // All other operations result in no change.
    if (this->isSpecific()) {
        if (divisor != 0) {
            mDurationMilliseconds /= divisor;
        } else {
            // Dividing a normal duration by zero.
            if (mDurationMilliseconds >= 0) {
                *this = VDuration::POSITIVE_INFINITY();
            } else {
                *this = VDuration::NEGATIVE_INFINITY();
            }
        }
    }

    return *this;
}

VDuration& VDuration::operator%=(const VDuration& divisor) {
    // Normal values use simple remainder division, with mod-by-zero yielding
    // no change.
    // All other operations result in no change.
    if (this->isSpecific()) {
        if (divisor.isSpecific() && (divisor != VDuration::ZERO())) {
            mDurationMilliseconds %= divisor.mDurationMilliseconds;
        }
    }

    return *this;
}

VDuration VDuration::operator-() const {
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

VDuration operator/(const VDuration& d, int divisor) {
    // Re-use in-place division code.
    VDuration result = d;
    result /= divisor;
    return result;
}

VDuration operator%(const VDuration& d, const VDuration& divisor) {
    // Re-use in-place mod code.
    VDuration result = d;
    result %= divisor;
    return result;
}

VString VDuration::getDurationString() const {
    // most common case first!
    if ((mDurationMilliseconds >= 0) && (mDurationMilliseconds < kMillisecondsPerSecond))
        return VSTRING_FORMAT(VSTRING_FORMATTER_S64 "ms", mDurationMilliseconds);
    else if (*this == VDuration::UNSPECIFIED())
        return "UNSPECIFIED";
    else if (*this == VDuration::POSITIVE_INFINITY())
        return "INFINITY";
    else if (*this == VDuration::NEGATIVE_INFINITY())
        return "-INFINITY";
    else if (mDurationMilliseconds % kMillisecondsPerDay == 0)
        return VSTRING_FORMAT(VSTRING_FORMATTER_INT "d", this->getDurationDays());
    else if (mDurationMilliseconds % kMillisecondsPerHour == 0)
        return VSTRING_FORMAT(VSTRING_FORMATTER_INT "h", this->getDurationHours());
    else if (mDurationMilliseconds % kMillisecondsPerMinute == 0)
        return VSTRING_FORMAT(VSTRING_FORMATTER_INT "m", this->getDurationMinutes());
    else if (mDurationMilliseconds % kMillisecondsPerSecond == 0)
        return VSTRING_FORMAT(VSTRING_FORMATTER_INT "s", this->getDurationSeconds());
    else
        return VSTRING_FORMAT(VSTRING_FORMATTER_S64 "ms", mDurationMilliseconds);
}

VString VDuration::getDurationStringFractionalSeconds() const {
    int wholeSeconds = this->getDurationSeconds();
    int thousandths = static_cast<int>(this->getDurationMilliseconds() % kMillisecondsPerSecond);

    return VSTRING_FORMAT("%d.%03d", wholeSeconds, thousandths);
}

void VDuration::setDurationString(const VString& s) {
    // Our API doc declares that we throw VRangeException on a malformed
    // input string. The VString parse functions are doing this for us.
    // Get the test order right: note that "ends with d" (days) is true
    // for "unspecified".
    VString value(s);
    if (s.endsWith("ms")) {
        value.truncateLength(value.length() - 2);
        mDurationMilliseconds = value.parseS64();
    } else if (s.endsWith('s')) {
        value.truncateLength(value.length() - 1);
        mDurationMilliseconds = kMillisecondsPerSecond * value.parseS64();
    } else if (s.endsWith('m')) {
        value.truncateLength(value.length() - 1);
        mDurationMilliseconds = kMillisecondsPerMinute * value.parseS64();
    } else if (s.endsWith('h')) {
        value.truncateLength(value.length() - 1);
        mDurationMilliseconds = kMillisecondsPerHour * value.parseS64();
    } else if (s.equalsIgnoreCase("UNSPECIFIED")) {
        *this = VDuration::UNSPECIFIED();
    } else if (s.equalsIgnoreCase("INFINITY")) {
        *this = VDuration::POSITIVE_INFINITY();
    } else if (s.equalsIgnoreCase("-INFINITY")) {
        *this = VDuration::NEGATIVE_INFINITY();
    } else if (s.endsWith('d')) {
        value.truncateLength(value.length() - 1);
        mDurationMilliseconds = kMillisecondsPerDay * value.parseS64();
    } else {
        VDouble seconds = value.parseDouble();
        VDouble milliseconds = seconds * 1000.0;
        mDurationMilliseconds = (Vs64) milliseconds;
    }
}

// static
VDuration VDuration::_complexAdd(const VDuration& d1, const VDuration& d2) {
    // Re-use in-place addition code.
    VDuration result = d1;
    result += d2;
    return result;
}

// static
VDuration VDuration::_complexSubtract(const VDuration& d1, const VDuration& d2) {
    // Re-use in-place subtraction code.
    VDuration result = d1;
    result -= d2;
    return result;
}

// static
VDuration VDuration::_complexMultiply(const VDuration& d, Vs64 multiplier) {
    // Re-use in-place multiplication code.
    VDuration result = d;
    result *= multiplier;
    return result;
}

// static
VDuration VDuration::_complexMin(const VDuration& d1, const VDuration& d2) {
    if (VDuration::areValuesSpecific(d1, d2))
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
VDuration VDuration::_complexMax(const VDuration& d1, const VDuration& d2) {
    if (VDuration::areValuesSpecific(d1, d2))
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
VDuration VDuration::_complexAbs(const VDuration& d) {
    if (d.isSpecific())
        return (d.mDurationMilliseconds < CONST_S64(0)) ? -d : d;

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
    mDayOfWeek(0) {
}

// VInstant ------------------------------------------------------------------

static const Vs64 kInfinitePastInternalValue = V_MIN_S64;
static const Vs64 kInfiniteFutureInternalValue = V_MAX_S64 - CONST_S64(1);
static const Vs64 kNeverOccurredInternalValue = V_MAX_S64;

// The static constants are returned with accessor methods because of the
// unpredictable order of static initialization. So we use local statics
// that are initialized on first use, and return them from these accessors.

// static
const VInstant& VInstant::INFINITE_PAST() {
    static const VInstant kINFINITE_PAST(kInfinitePastInternalValue);
    return kINFINITE_PAST;
}

const VInstant& VInstant::INFINITE_FUTURE() {
    static const VInstant kINFINITE_FUTURE(kInfiniteFutureInternalValue);
    return kINFINITE_FUTURE;
}

const VInstant& VInstant::NEVER_OCCURRED() {
    static const VInstant kNEVER_OCCURRED(kNeverOccurredInternalValue);
    return kNEVER_OCCURRED;
}

const VString& VInstant::UTC_TIME_ZONE_ID() {
    static const VString kUTC_TIME_ZONE_ID("UTC");
    return kUTC_TIME_ZONE_ID;
}

const VString& VInstant::LOCAL_TIME_ZONE_ID() {
    static const VString kLOCAL_TIME_ZONE_ID; // empty string means local time zone
    return kLOCAL_TIME_ZONE_ID;
}

Vs64 VInstant::gSimulatedClockOffset(0);
Vs64 VInstant::gFrozenClockValue(0);

MRemoteTimeZoneConverter* VInstant::gRemoteTimeZoneConverter = NULL;

// static
void VInstant::setRemoteTimeZoneConverter(MRemoteTimeZoneConverter* converter) {
    gRemoteTimeZoneConverter = converter;
}

// static
MRemoteTimeZoneConverter* VInstant::getRemoteTimeZoneConverter() {
    return gRemoteTimeZoneConverter;
}

VInstant::VInstant()
    : mValue(0)
    {
    VInstant::setNow();
}

VInstant& VInstant::operator=(const VInstant& i) {
    if (this != &i) {
        mValue = i.getValue();
    }

    return *this;
}

VInstant& VInstant::operator+=(const VDuration& forwardOffsetDuration) {
    if (this->isSpecific() && forwardOffsetDuration.isSpecific())
        mValue += forwardOffsetDuration.getDurationMilliseconds();
    else if (this->isSpecific() && (forwardOffsetDuration == VDuration::NEGATIVE_INFINITY()))
        *this = VInstant::INFINITE_PAST();
    else if (this->isSpecific() && (forwardOffsetDuration == VDuration::POSITIVE_INFINITY()))
        *this = VInstant::INFINITE_FUTURE();

    return *this;
}

VInstant& VInstant::operator-=(const VDuration& backwardOffsetDuration) {
    if (this->isSpecific() && backwardOffsetDuration.isSpecific())
        mValue -= backwardOffsetDuration.getDurationMilliseconds();
    else if (this->isSpecific() && (backwardOffsetDuration == VDuration::NEGATIVE_INFINITY()))
        *this = VInstant::INFINITE_FUTURE();
    else if (this->isSpecific() && (backwardOffsetDuration == VDuration::POSITIVE_INFINITY()))
        *this = VInstant::INFINITE_PAST();

    return *this;
}

void VInstant::setNow() {
    if (gFrozenClockValue == 0) {
        mValue = VInstant::_platform_now();
        mValue += gSimulatedClockOffset;
    } else {
        mValue = gFrozenClockValue;
    }
}

void VInstant::setTrueNow() {
    mValue = VInstant::_platform_now();
}

VInstantStruct VInstant::getUTCInstantFields() const {
    VInstantStruct when;
    VInstant::_platform_offsetToUTCStruct(mValue, when);
    return when;
}

VInstantStruct VInstant::getLocalInstantFields() const {
    VInstantStruct when;
    VInstant::_platform_offsetToLocalStruct(mValue, when);
    return when;
}

#define FORMAT_FILE_NAME_SAFE_WITH_MILLISECONDS     "%d%02d%02d%02d%02d%02d%03d"
#define FORMAT_FILE_NAME_SAFE_WITHOUT_MILLISECONDS  "%d%02d%02d%02d%02d%02d"
#define FORMAT_UTC_WITH_MILLISECONDS                "%d-%02d-%02d %02d:%02d:%02d.%03d UTC"
#define FORMAT_UTC_WITHOUT_MILLISECONDS             "%d-%02d-%02d %02d:%02d:%02d UTC"
#define FORMAT_LOCAL_WITH_MILLISECONDS              "%d-%02d-%02d %02d:%02d:%02d.%03d"
#define FORMAT_LOG_LOCAL_WITH_MILLISECONDS          "%d-%02d-%02d %02d:%02d:%02d,%03d"
#define FORMAT_LOCAL_WITHOUT_MILLISECONDS           "%d-%02d-%02d %02d:%02d:%02d"

static void _formatInstantString(const VInstantStruct& when, bool isUTC, VString& s, bool fileNameSafe, bool wantMilliseconds) {
    if (fileNameSafe) {
        if (wantMilliseconds) {
            s.format(FORMAT_FILE_NAME_SAFE_WITH_MILLISECONDS, when.mYear, when.mMonth, when.mDay, when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
        } else {
            s.format(FORMAT_FILE_NAME_SAFE_WITHOUT_MILLISECONDS, when.mYear, when.mMonth, when.mDay, when.mHour, when.mMinute, when.mSecond);
        }
    } else if (isUTC) {
        if (wantMilliseconds) {
            s.format(FORMAT_UTC_WITH_MILLISECONDS, when.mYear, when.mMonth, when.mDay, when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
        } else {
            s.format(FORMAT_UTC_WITHOUT_MILLISECONDS, when.mYear, when.mMonth, when.mDay, when.mHour, when.mMinute, when.mSecond);
        }
    } else {
        if (wantMilliseconds) {
            s.format(FORMAT_LOCAL_WITH_MILLISECONDS, when.mYear, when.mMonth, when.mDay, when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
        } else {
            s.format(FORMAT_LOCAL_WITHOUT_MILLISECONDS, when.mYear, when.mMonth, when.mDay, when.mHour, when.mMinute, when.mSecond);
        }
    }
}

void VInstant::getUTCString(VString& s, bool fileNameSafe, bool wantMilliseconds) const {
    if (this->isSpecific()) {
        VInstantStruct    when;
        VInstant::_platform_offsetToUTCStruct(mValue, when);
        _formatInstantString(when, true/*utc*/, s, fileNameSafe, wantMilliseconds);
    } else if (*this == VInstant::INFINITE_PAST()) {
        s = "PAST";
    } else if (*this == VInstant::INFINITE_FUTURE()) {
        s = "FUTURE";
    } else { /* NEVER_OCCURRED */
        s = "NEVER";
    }
}

VString VInstant::getUTCString(bool fileNameSafe, bool wantMilliseconds) const {
    VString s;
    this->getUTCString(s, fileNameSafe, wantMilliseconds);
    return s;
}

void VInstant::setUTCString(const VString& s) {
    if (s == "NOW") {
        this->setNow();
    } else if (s == "PAST") {
        mValue = kInfinitePastInternalValue;
    } else if (s == "FUTURE") {
        mValue = kInfiniteFutureInternalValue;
    } else if (s == "NEVER") {
        mValue = kNeverOccurredInternalValue;
    } else {
        VInstantStruct when;
        when.mDayOfWeek = 0;

        (void) ::sscanf(s, FORMAT_UTC_WITH_MILLISECONDS, &when.mYear, &when.mMonth, &when.mDay, &when.mHour, &when.mMinute, &when.mSecond, &when.mMillisecond);

        mValue = VInstant::_platform_offsetFromUTCStruct(when);
    }
}

void VInstant::getLocalLogString(VString& s) const {
    if (this->isSpecific()) {
        VInstantStruct    when;
        VInstant::_platform_offsetToLocalStruct(mValue, when);
        s.format(FORMAT_LOG_LOCAL_WITH_MILLISECONDS, when.mYear, when.mMonth, when.mDay, when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
    } else if (*this == VInstant::INFINITE_PAST()) {
        s = "PAST";
    } else if (*this == VInstant::INFINITE_FUTURE()) {
        s = "FUTURE";
    } else { /* NEVER_OCCURRED */
        s = "NEVER";
    }
}

void VInstant::getLocalString(VString& s, bool fileNameSafe, bool wantMilliseconds) const {
    if (this->isSpecific()) {
        VInstantStruct    when;
        VInstant::_platform_offsetToLocalStruct(mValue, when);
        _formatInstantString(when, false/*not utc*/, s, fileNameSafe, wantMilliseconds);
    } else if (*this == VInstant::INFINITE_PAST()) {
        s = "PAST";
    } else if (*this == VInstant::INFINITE_FUTURE()) {
        s = "FUTURE";
    } else { /* NEVER_OCCURRED */
        s = "NEVER";
    }
}

VString VInstant::getLocalString(bool fileNameSafe, bool wantMilliseconds) const {
    VString s;
    this->getLocalString(s, fileNameSafe, wantMilliseconds);
    return s;
}

void VInstant::setLocalString(const VString& s) {
    if (s == "NOW") {
        this->setNow();
    } else if (s == "PAST") {
        mValue = kInfinitePastInternalValue;
    } else if (s == "FUTURE") {
        mValue = kInfiniteFutureInternalValue;
    } else if (s == "NEVER") {
        mValue = kNeverOccurredInternalValue;
    } else {
        VInstantStruct when;
        when.mDayOfWeek = 0;

        (void) ::sscanf(s, FORMAT_LOCAL_WITH_MILLISECONDS, &when.mYear, &when.mMonth, &when.mDay, &when.mHour, &when.mMinute, &when.mSecond, &when.mMillisecond);

        mValue = VInstant::_platform_offsetFromLocalStruct(when);
    }
}

void VInstant::getValues(VDate& date, VTimeOfDay& timeOfDay, const VString& timeZoneID) const {
    VInstantStruct when;

    if (timeZoneID == VInstant::LOCAL_TIME_ZONE_ID())
        VInstant::_platform_offsetToLocalStruct(mValue, when);
    else if (timeZoneID == VInstant::UTC_TIME_ZONE_ID())
        VInstant::_platform_offsetToUTCStruct(mValue, when);
    else if (gRemoteTimeZoneConverter == NULL)
        throw VStackTraceException(VSTRING_FORMAT("Request for remote time zone conversion (%s) without a converter.", timeZoneID.chars()));
    else
        gRemoteTimeZoneConverter->offsetToRTZStruct(mValue, timeZoneID, when);

    date.set(when.mYear, when.mMonth, when.mDay);
    timeOfDay.set(when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
}

VDate VInstant::getLocalDate() const {
    return this->getDate(VInstant::LOCAL_TIME_ZONE_ID());
}

VDate VInstant::getDate(const VString& timeZoneID) const {
    VInstantStruct when;

    if (timeZoneID == VInstant::LOCAL_TIME_ZONE_ID())
        VInstant::_platform_offsetToLocalStruct(mValue, when);
    else if (timeZoneID == VInstant::UTC_TIME_ZONE_ID())
        VInstant::_platform_offsetToUTCStruct(mValue, when);
    else if (gRemoteTimeZoneConverter == NULL)
        throw VStackTraceException(VSTRING_FORMAT("Request for remote time zone conversion (%s) without a converter.", timeZoneID.chars()));
    else
        gRemoteTimeZoneConverter->offsetToRTZStruct(mValue, timeZoneID, when);

    return VDate(when.mYear, when.mMonth, when.mDay);
}

VTimeOfDay VInstant::getLocalTimeOfDay() const {
    return this->getTimeOfDay(VInstant::LOCAL_TIME_ZONE_ID());
}

VTimeOfDay VInstant::getTimeOfDay(const VString& timeZoneID) const {
    VInstantStruct when;

    if (timeZoneID == VInstant::LOCAL_TIME_ZONE_ID())
        VInstant::_platform_offsetToLocalStruct(mValue, when);
    else if (timeZoneID == VInstant::UTC_TIME_ZONE_ID())
        VInstant::_platform_offsetToUTCStruct(mValue, when);
    else if (gRemoteTimeZoneConverter == NULL)
        throw VStackTraceException(VSTRING_FORMAT("Request for remote time zone conversion (%s) without a converter.", timeZoneID.chars()));
    else
        gRemoteTimeZoneConverter->offsetToRTZStruct(mValue, timeZoneID, when);

    return VTimeOfDay(when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
}

VDateAndTime VInstant::getLocalDateAndTime() const {
    return this->getDateAndTime(VInstant::LOCAL_TIME_ZONE_ID());
}

VDateAndTime VInstant::getDateAndTime(const VString& timeZoneID) const {
    VInstantStruct when;

    if (*this == VInstant::INFINITE_PAST() ||
        *this == VInstant::INFINITE_FUTURE() ||
        *this == VInstant::NEVER_OCCURRED())
        throw VStackTraceException(VSTRING_FORMAT("Request for specific time values with non-specific time '%s'.", this->getLocalString().chars()));
    else if (timeZoneID == VInstant::LOCAL_TIME_ZONE_ID())
        VInstant::_platform_offsetToLocalStruct(mValue, when);
    else if (timeZoneID == VInstant::UTC_TIME_ZONE_ID())
        VInstant::_platform_offsetToUTCStruct(mValue, when);
    else if (gRemoteTimeZoneConverter == NULL)
        throw VStackTraceException(VSTRING_FORMAT("Request for remote time zone conversion (%s) without a converter.", timeZoneID.chars()));
    else
        gRemoteTimeZoneConverter->offsetToRTZStruct(mValue, timeZoneID, when);

    return VDateAndTime(when.mYear, when.mMonth, when.mDay, when.mHour, when.mMinute, when.mSecond, when.mMillisecond);
}

void VInstant::setLocalDateAndTime(const VDateAndTime& dt) {
    this->setDateAndTime(dt, VInstant::LOCAL_TIME_ZONE_ID());
}

void VInstant::setDateAndTime(const VDateAndTime& dt, const VString& timeZoneID) {
    VInstantStruct when(dt.getDate(), dt.getTimeOfDay());

    if (timeZoneID == VInstant::LOCAL_TIME_ZONE_ID())
        mValue = VInstant::_platform_offsetFromLocalStruct(when);
    else if (timeZoneID == VInstant::UTC_TIME_ZONE_ID())
        mValue = VInstant::_platform_offsetFromUTCStruct(when);
    else if (gRemoteTimeZoneConverter == NULL)
        throw VStackTraceException(VSTRING_FORMAT("Request for remote time zone conversion (%s) without a converter.", timeZoneID.chars()));
    else
        mValue = gRemoteTimeZoneConverter->offsetFromRTZStruct(timeZoneID, when);
}

void VInstant::setValues(const VDate& date, const VTimeOfDay& timeOfDay, const VString& timeZoneID) {
    VInstantStruct when(date, timeOfDay);

    if (timeZoneID == VInstant::LOCAL_TIME_ZONE_ID())
        mValue = VInstant::_platform_offsetFromLocalStruct(when);
    else if (timeZoneID == VInstant::UTC_TIME_ZONE_ID())
        mValue = VInstant::_platform_offsetFromUTCStruct(when);
    else if (gRemoteTimeZoneConverter == NULL)
        throw VStackTraceException(VSTRING_FORMAT("Request for remote time zone conversion (%s) without a converter.", timeZoneID.chars()));
    else
        mValue = gRemoteTimeZoneConverter->offsetFromRTZStruct(timeZoneID, when);
}

static const int kSecondsPerDay = 86400;

Vs64 VInstant::getLocalOffsetMilliseconds() const {
    /*
    Return the offset, in milliseconds, of the local time zone, at this
    indicated instant.
    */

    VInstantStruct localStruct;
    VInstant::_platform_offsetToLocalStruct(mValue, localStruct);

    VInstantStruct utcStruct;
    VInstant::_platform_offsetToUTCStruct(mValue, utcStruct);

    // Now we have two structs for the instant.
    // A little math will tell us the difference in h/m/s.
    // We know the delta cannot be more than 24 hours.
    // No need to worry about milliseconds; time zone offset resolutions are only in minutes.
    // (We could presumably even ignore the seconds for that matter.)
    int deltaSeconds;
    int localSecondsOfDay = (3600 * localStruct.mHour) + (60 * localStruct.mMinute) + localStruct.mSecond;
    int utcSecondsOfDay = (3600 * utcStruct.mHour) + (60 * utcStruct.mMinute) + utcStruct.mSecond;

    if (localStruct.mDay == utcStruct.mDay) {
        // Same date. Just a difference in hours/minutes.

        deltaSeconds = localSecondsOfDay - utcSecondsOfDay;
    } else if ((localStruct.mDay == utcStruct.mDay + 1) ||
               ((localStruct.mDay == 1) && (utcStruct.mDay > 27))) { // detect wrap-around day 1 and day 28-31
        // We are ahead of GMT and we are already on the "next" calendar date.
        // localSecondsOfDay is therefore smaller, so add a day's worth of seconds it to compensate.

        deltaSeconds = (localSecondsOfDay + kSecondsPerDay) - utcSecondsOfDay;
    } else {
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

    return (Vs64)(deltaSeconds * 1000);
}

// static
void VInstant::threadsafe_localtime(const time_t epochOffset, struct tm* resultStorage) {
    time_t      offset = epochOffset;
    struct tm*  result;

#ifdef V_HAVE_REENTRANT_TIME
    result = ::localtime_r(&offset, resultStorage);
#else
    result = ::localtime(&offset);
#endif

    if (result == NULL)
        throw VStackTraceException(VSTRING_FORMAT("VInstant::threadsafe_localtime: input time value " VSTRING_FORMATTER_INT " is out of range.", (int) offset));

// Only copy result if we're NOT using reentrant version that already wrote result.
#ifndef V_HAVE_REENTRANT_TIME
    *resultStorage = *result;
#endif
}

// static
void VInstant::threadsafe_gmtime(const time_t epochOffset, struct tm* resultStorage) {
    struct tm* result;

#ifdef V_HAVE_REENTRANT_TIME
    result = ::gmtime_r(&epochOffset, resultStorage);
#else
    result = ::gmtime(&epochOffset);
#endif

    if (result == NULL)
        throw VStackTraceException(VSTRING_FORMAT("VInstant::threadsafe_gmtime: input time value " VSTRING_FORMATTER_INT " is out of range.", (int) epochOffset));

// Only copy result if we're NOT using reentrant version that already wrote result.
#ifndef V_HAVE_REENTRANT_TIME
    *resultStorage = *result;
#endif
}

// static
bool VInstant::_complexGT(const VInstant& i1, const VInstant& i2) {
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
bool VInstant::_complexGTE(const VInstant& i1, const VInstant& i2) {
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
bool VInstant::_complexLT(const VInstant& i1, const VInstant& i2) {
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
bool VInstant::_complexLTE(const VInstant& i1, const VInstant& i2) {
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
Vs64 VInstant::snapshot() {
    if (gFrozenClockValue == 0)
        return VInstant::_platform_snapshot() + gSimulatedClockOffset;
    else
        return gFrozenClockValue;
}

// static
VDuration VInstant::snapshotDelta(Vs64 snapshotValue) {
    return VDuration::MILLISECOND() * (VInstant::snapshot() - snapshotValue);
}

// static
void VInstant::incrementSimulatedClockOffset(const VDuration& delta) {
    gSimulatedClockOffset += delta.getDurationMilliseconds();
}

// static
void VInstant::setSimulatedClockOffset(const VDuration& offset) {
    gSimulatedClockOffset = offset.getDurationMilliseconds();
}

// static
void VInstant::setSimulatedClockValue(const VInstant& simulatedCurrentTime) {
    gSimulatedClockOffset = 0; // so that "now" will be true current time, not existing simulated time
    VInstant now;
    VInstant::setSimulatedClockOffset(simulatedCurrentTime - now);
}

// static
VDuration VInstant::getSimulatedClockOffset() {
    return VDuration::MILLISECOND() * gSimulatedClockOffset;
}

// static
void VInstant::freezeTime(const VInstant& frozenTimeValue) {
    gFrozenClockValue = frozenTimeValue.getValue();
}

// static
void VInstant::shiftFrozenTime(const VDuration& delta) {
    gFrozenClockValue += delta.getDurationMilliseconds();
}

// static
void VInstant::unfreezeTime() {
    gFrozenClockValue = 0;
}

// static
bool VInstant::isTimeFrozen() {
    return gFrozenClockValue != 0;
}

// VDate ---------------------------------------------------------------------

// Is ASSERT_INVARIANT enabled/disabled specifically for VDate and VTimeOfDay?
#ifdef V_ASSERT_INVARIANT_VDATE_AND_TIME_ENABLED
    #undef ASSERT_INVARIANT
    #if V_ASSERT_INVARIANT_VDATE_AND_TIME_ENABLED == 1
        #define ASSERT_INVARIANT() this->_assertInvariant() ///< Macro to call this->_assertInvariant().
    #else
        #define ASSERT_INVARIANT() ((void) 0) ///< No-op.
    #endif
#endif

const VChar VDate::kLocalDateSeparator('/');

VDate::VDate()
    : mYear(0)
    , mMonth(1)
    , mDay(1)
    {
    ASSERT_INVARIANT();
}

VDate::VDate(const VString& timeZoneID)
    : mYear(0)
    , mMonth(1)
    , mDay(1)
    {
    VInstant    now;
    VDate       nowDate = now.getDate(timeZoneID);
    mYear = nowDate.getYear();
    mMonth = nowDate.getMonth();
    mDay = nowDate.getDay();

    ASSERT_INVARIANT();
}

VDate::VDate(int year, int month, int day)
    : mYear(year)
    , mMonth(month)
    , mDay(day)
    {
    if ((month < 1) || (month > 12) ||
            (day   < 1) || (day   > 32)) // 32 allowed when incrementing the date
        throw VRangeException(VSTRING_FORMAT("VDate: %d-%02d-%02d is an invalid value.", year, month, day));

    ASSERT_INVARIANT();
}

int VDate::getYear() const {
    ASSERT_INVARIANT();

    return mYear;
}

int VDate::getMonth() const {
    ASSERT_INVARIANT();

    return mMonth;
}

int VDate::getDay() const {
    ASSERT_INVARIANT();

    return mDay;
}

int VDate::getDayOfWeek() const {
    ASSERT_INVARIANT();

    VInstantStruct when;

    when.mYear = mYear;
    when.mMonth = mMonth;
    when.mDay = mDay;
    when.mHour = 12;    // noon, smack dab in middle of day
    when.mMinute = 0;
    when.mSecond = 0;
    when.mMillisecond = 0;

    // First get the UTC offset of that UTC date.
    Vs64 offset = VInstant::_platform_offsetFromUTCStruct(when);

    // Now reverse to get the mDayOfWeek filled out.
    VInstant::_platform_offsetToUTCStruct(offset, when);

    return when.mDayOfWeek;
}

void VDate::set(int year, int month, int day) {
    ASSERT_INVARIANT();

    if ((month < 1) || (month > 12) ||
            (day   < 1) || (day   > 32)) // 32 allowed when incrementing the date
        throw VRangeException(VSTRING_FORMAT("VDate::set: %d-%02d-%02d is an invalid value.", year, month, day));

    mYear = year;
    mMonth = month;
    mDay = day;

    ASSERT_INVARIANT();
}

void VDate::setYear(int year) {
    ASSERT_INVARIANT();

    mYear = year;

    ASSERT_INVARIANT();
}

void VDate::setMonth(int month) {
    ASSERT_INVARIANT();

    mMonth = month;

    ASSERT_INVARIANT();
}

void VDate::setDay(int day) {
    ASSERT_INVARIANT();

    mDay = day;

    ASSERT_INVARIANT();
}

void VDate::_assertInvariant() const {
    VASSERT_IN_RANGE(mMonth, 0, 12);
    VASSERT_IN_RANGE(mDay, 0, 32);
}

// VTimeOfDay ----------------------------------------------------------------

const VChar VTimeOfDay::kLocalTimeSeparator(':');

VTimeOfDay::VTimeOfDay()
    : mHour(0)
    , mMinute(0)
    , mSecond(0)
    , mMillisecond(0)
    {
    ASSERT_INVARIANT();
}

VTimeOfDay::VTimeOfDay(const VString& timeZoneID)
    : mHour(0)
    , mMinute(0)
    , mSecond(0)
    , mMillisecond(0)
    {
    VInstant    now;
    VTimeOfDay  nowTimeOfDay = now.getTimeOfDay(timeZoneID);
    mHour = nowTimeOfDay.getHour();
    mMinute = nowTimeOfDay.getMinute();
    mSecond = nowTimeOfDay.getSecond();
    mMillisecond = nowTimeOfDay.getMillisecond();

    ASSERT_INVARIANT();
}

VTimeOfDay::VTimeOfDay(int hour, int minute, int second, int millisecond)
    : mHour(hour)
    , mMinute(minute)
    , mSecond(second)
    , mMillisecond(millisecond)
    {
    if ((hour   < 0) || (hour   > 23) ||
            (minute < 0) || (minute > 59) ||
            (second < 0) || (second > 59) ||
            (millisecond < 0) || (millisecond > 999))
        throw VRangeException(VSTRING_FORMAT("VTimeOfDay: %02d:%02d:%02d.%03d is an invalid value.", hour, minute, second, millisecond));

    ASSERT_INVARIANT();
}

int VTimeOfDay::getHour() const {
    ASSERT_INVARIANT();

    return mHour;
}

int VTimeOfDay::getMinute() const {
    ASSERT_INVARIANT();

    return mMinute;
}

int VTimeOfDay::getSecond() const {
    ASSERT_INVARIANT();

    return mSecond;
}

int VTimeOfDay::getMillisecond() const {
    ASSERT_INVARIANT();

    return mMillisecond;
}

void VTimeOfDay::set(int hour, int minute, int second, int millisecond) {
    // No point in doing our own ASSERT_INVARIANT here, because each
    // setter we call will do it anyway.
    this->setHour(hour);
    this->setMinute(minute);
    this->setSecond(second);
    this->setMillisecond(millisecond);
}

void VTimeOfDay::setHour(int hour) {
    ASSERT_INVARIANT();

    if ((hour < 0) || (hour > 23))
        throw VRangeException(VSTRING_FORMAT("VTimeOfDay::set/setHour: " VSTRING_FORMATTER_INT " is an invalid value.", hour));

    mHour = hour;

    ASSERT_INVARIANT();
}

void VTimeOfDay::setMinute(int minute) {
    ASSERT_INVARIANT();

    if ((minute < 0) || (minute > 59))
        throw VRangeException(VSTRING_FORMAT("VTimeOfDay::set/setMinute: " VSTRING_FORMATTER_INT " is an invalid value.", minute));

    mMinute = minute;

    ASSERT_INVARIANT();
}

void VTimeOfDay::setSecond(int second) {
    ASSERT_INVARIANT();

    if ((second < 0) || (second > 59))
        throw VRangeException(VSTRING_FORMAT("VTimeOfDay::set/setSecond: " VSTRING_FORMATTER_INT " is an invalid value.", second));

    mSecond = second;

    ASSERT_INVARIANT();
}

void VTimeOfDay::setMillisecond(int millisecond) {
    ASSERT_INVARIANT();

    if ((millisecond < 0) || (millisecond > 999))
        throw VRangeException(VSTRING_FORMAT("VTimeOfDay::set/setMillisecond: " VSTRING_FORMATTER_INT " is an invalid value.", millisecond));

    mMillisecond = millisecond;

    ASSERT_INVARIANT();
}

void VTimeOfDay::setToStartOfDay() {
    // No need for ASSERT_INVARIANT, since set() will call it.
    this->set(0, 0, 0, 0);
}

void VTimeOfDay::_assertInvariant() const {
    VASSERT_IN_RANGE(mHour, 0, 23);
    VASSERT_IN_RANGE(mMinute, 0, 59);
    VASSERT_IN_RANGE(mSecond, 0, 59);
    VASSERT_IN_RANGE(mMillisecond, 0, 999);
}

// VDateAndTime --------------------------------------------------------------

VDateAndTime::VDateAndTime(const VString& timeZoneID)
    : mDate()
    , mTimeOfDay()
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

// VInstantFormatter ---------------------------------------------------------

// static
const VInstantFormatterLocaleInfo& VInstantFormatterLocaleInfo::getLocaleInfo(const VString& /*localeName*/) {
    // TODO: Allow info for other locales to be registered and returned.
    static const VInstantFormatterLocaleInfo EN_US_INFO;
    return EN_US_INFO;
}

VInstantFormatterLocaleInfo::VInstantFormatterLocaleInfo()
    : CE_MARKER("AD")
    , AM_MARKER("AM")
    , PM_MARKER("PM")
    {
    MONTH_NAMES_SHORT.push_back("Jan");
    MONTH_NAMES_SHORT.push_back("Feb");
    MONTH_NAMES_SHORT.push_back("Mar");
    MONTH_NAMES_SHORT.push_back("Apr");
    MONTH_NAMES_SHORT.push_back("May");
    MONTH_NAMES_SHORT.push_back("Jun");
    MONTH_NAMES_SHORT.push_back("Jul");
    MONTH_NAMES_SHORT.push_back("Aug");
    MONTH_NAMES_SHORT.push_back("Sep");
    MONTH_NAMES_SHORT.push_back("Oct");
    MONTH_NAMES_SHORT.push_back("Nov");
    MONTH_NAMES_SHORT.push_back("Dec");

    MONTH_NAMES_LONG.push_back("January");
    MONTH_NAMES_LONG.push_back("February");
    MONTH_NAMES_LONG.push_back("March");
    MONTH_NAMES_LONG.push_back("April");
    MONTH_NAMES_LONG.push_back("May");
    MONTH_NAMES_LONG.push_back("June");
    MONTH_NAMES_LONG.push_back("July");
    MONTH_NAMES_LONG.push_back("August");
    MONTH_NAMES_LONG.push_back("September");
    MONTH_NAMES_LONG.push_back("October");
    MONTH_NAMES_LONG.push_back("November");
    MONTH_NAMES_LONG.push_back("December");

    DAY_NAMES_SHORT.push_back("Sun");
    DAY_NAMES_SHORT.push_back("Mon");
    DAY_NAMES_SHORT.push_back("Tue");
    DAY_NAMES_SHORT.push_back("Wed");
    DAY_NAMES_SHORT.push_back("Thu");
    DAY_NAMES_SHORT.push_back("Fri");
    DAY_NAMES_SHORT.push_back("Sat");

    DAY_NAMES_LONG.push_back("Sunday");
    DAY_NAMES_LONG.push_back("Monday");
    DAY_NAMES_LONG.push_back("Tuesday");
    DAY_NAMES_LONG.push_back("Wednesday");
    DAY_NAMES_LONG.push_back("Thursday");
    DAY_NAMES_LONG.push_back("Friday");
    DAY_NAMES_LONG.push_back("Saturday");
}

#define DEFAULT_FORMAT_SPECIFIER "y-MM-dd HH:mm:ss.SSS"
#define V_DEFAULT_LOCALE "en-us"

VInstantFormatter::VInstantFormatter()
    : mFormatSpecifier(DEFAULT_FORMAT_SPECIFIER)
    , mLocaleInfo(VInstantFormatterLocaleInfo::getLocaleInfo(V_DEFAULT_LOCALE))
    {
}

VInstantFormatter::VInstantFormatter(const VInstantFormatterLocaleInfo& localeInfo)
    : mFormatSpecifier(DEFAULT_FORMAT_SPECIFIER)
    , mLocaleInfo(localeInfo)
    {
}

VInstantFormatter::VInstantFormatter(const VString& formatSpecifier)
    : mFormatSpecifier(formatSpecifier)
    , mLocaleInfo(VInstantFormatterLocaleInfo::getLocaleInfo(V_DEFAULT_LOCALE))
    {
}

VInstantFormatter::VInstantFormatter(const VString& formatSpecifier, const VInstantFormatterLocaleInfo& localeInfo)
    : mFormatSpecifier(formatSpecifier)
    , mLocaleInfo(localeInfo)
    {
}

VString VInstantFormatter::formatLocalString(const VInstant& when) const {
    return this->_format(when.getLocalInstantFields(), static_cast<int>(when.getLocalOffsetMilliseconds()));
}

VString VInstantFormatter::formatUTCString(const VInstant& when) const {
    return this->_format(when.getUTCInstantFields(), 0);
}

VString VInstantFormatter::_format(const VInstantStruct& when, int utcOffsetMilliseconds) const {
    // For efficiency, extract the date and time once rather than while looping as needed.
    // We'll almost inevitably need them.
    VString result;
    VString pendingFieldSpecifier;
    bool isEscaped = false; // true if we have encountered a single quote (') but not its match to close
    bool isUnescapePending = false; // true if we have encountered a second single quote (')
    bool gotEscapedChars = false;
    const int formatLength = mFormatSpecifier.length();
    
    for (int i = 0; i < formatLength; ++i) {
        VChar c = mFormatSpecifier[i];
        
        // If we're in escaped mode, handle all cases separately right here.
        if (isEscaped) {
        
            if (c == '\'') {
                // This is likely the end of the escape block, but could be the first of a two adjacent quotes which would mean to emit a single quote
                if (isUnescapePending) {
                    // Get back to normal escape mode and emit the single quote.
                    result += '\'';
                    isUnescapePending = false;
                } else {
                    isUnescapePending = true;
                }

                continue;

            } else if (isUnescapePending) {
                // The unescape is complete. Exit escape mode and proceed on to the big switch statement as normal.
                // If there was nothing between the escape quotes, we should emit a single quote.
                if (!gotEscapedChars) {
                    result += '\'';
                }

                isEscaped = false;
                isUnescapePending = false;
                gotEscapedChars = false;
                // lack of continue here, so we will proceed to the switch below and process this character
            } else {
                // This is some character inside the escape sequence. Just emit it.
                gotEscapedChars = true;
                result += c;
                continue;
            }
        
        }
        
        switch (c) {
        
            case '\'':
                // Enter escaped mode.
                this->_flushPendingFieldSpecifier(when, utcOffsetMilliseconds, pendingFieldSpecifier, result);
                isEscaped = true;
                isUnescapePending = false;
                break;
        
            // The following characters match those in Java 1.7 SimpleDateFormat:
            // <http://docs.oracle.com/javase/7/docs/api/java/text/SimpleDateFormat.html>
            case 'G':
            case 'y':
            case 'Y':
            case 'M':
            //case 'w': // week in year: NOT YET IMPLEMENTED
            //case 'W': // week in month: NOT YET IMPLEMENTED
            //case 'D':
            case 'd':
            //case 'F': // day of week in month: NOT YET IMPLEMENTED
            case 'E':
            case 'u':
            case 'a':
            case 'H':
            case 'k':
            case 'K':
            case 'h':
            case 'm':
            case 's':
            case 'S':
            case 'z':
            case 'Z':
            case 'X':
                if (pendingFieldSpecifier.isNotEmpty() && (c != pendingFieldSpecifier[0])) {
                    this->_flushPendingFieldSpecifier(when, utcOffsetMilliseconds, pendingFieldSpecifier, result);
                }

                pendingFieldSpecifier += c;
                break;

            // These are the ones we don't yet implement. Emit nothing, rather than unprocessed char like default does.
            case 'w': // week in year: NOT YET IMPLEMENTED
            case 'W': // week in month: NOT YET IMPLEMENTED
            case 'D':
            case 'F': // day of week in month: NOT YET IMPLEMENTED
                this->_flushPendingFieldSpecifier(when, utcOffsetMilliseconds, pendingFieldSpecifier, result);
                break;

            default:
                this->_flushPendingFieldSpecifier(when, utcOffsetMilliseconds, pendingFieldSpecifier, result);
                result += c;
                break;
        }
    }

    this->_flushPendingFieldSpecifier(when, utcOffsetMilliseconds, pendingFieldSpecifier, result);
    
    return result;
}

void VInstantFormatter::_flushPendingFieldSpecifier(const VInstantStruct& when, int utcOffsetMilliseconds, VString& fieldSpecifier, VString& resultToAppendTo) const {
    if (fieldSpecifier.isEmpty()) {
        return;
    }

    int fieldSpecifierLength = fieldSpecifier.length();
    if (fieldSpecifier.startsWith('G')) {
        this->_flushFixedLengthTextValue(mLocaleInfo.CE_MARKER, resultToAppendTo);
    }
    
    else if (fieldSpecifier.startsWithIgnoreCase("y")) {
        this->_flushYearValue(when.mYear, fieldSpecifierLength, resultToAppendTo);
    }

    else if (fieldSpecifier.startsWith('M')) { // month (long or short name, or number 1-12)
        this->_flushMonthValue(when.mMonth, fieldSpecifierLength, resultToAppendTo);
    }

    else if (fieldSpecifier.startsWith('d')) { // day in month 1-31
        this->_flushNumberValue(when.mDay, fieldSpecifierLength, resultToAppendTo);
    }

    else if (fieldSpecifier.startsWith('E')) { // day name in week (long or short name)
        this->_flushDayNameValue(when.mDayOfWeek, fieldSpecifierLength, resultToAppendTo);
    }

    else if (fieldSpecifier.startsWith('u')) { // day number in week (1=Monday, ..., 7=Sunday)
        this->_flushDayNumberValue(when.mDayOfWeek, fieldSpecifierLength, resultToAppendTo);
    }

    else if (fieldSpecifier.startsWith('a')) { // AM or PM
        this->_flushFixedLengthTextValue((when.mHour < 12) ? mLocaleInfo.AM_MARKER : mLocaleInfo.PM_MARKER, resultToAppendTo);
    }

    else if (fieldSpecifier.startsWith('H')) { // hour in day 0-23
        this->_flushNumberValue(when.mHour, fieldSpecifierLength, resultToAppendTo);
    }

    else if (fieldSpecifier.startsWith('k')) { // hour in day 1-24
        this->_flushNumberValue(when.mHour + 1, fieldSpecifierLength, resultToAppendTo);
    }

    else if (fieldSpecifier.startsWith('K')) { // hour in am/pm 0-11
        this->_flushNumberValue(when.mHour % 12, fieldSpecifierLength, resultToAppendTo);
    }

    else if (fieldSpecifier.startsWith('h')) { // hour in am/pm 1-12
        int hour = when.mHour % 12;
        if (hour == 0) {
            hour = 12;
        }
        this->_flushNumberValue(hour, fieldSpecifierLength, resultToAppendTo);
    }

    else if (fieldSpecifier.startsWith('m')) { // minute in hour 0-59
        this->_flushNumberValue(when.mMinute, fieldSpecifierLength, resultToAppendTo);
    }

    else if (fieldSpecifier.startsWith('s')) { // second in minute 0-59
        this->_flushNumberValue(when.mSecond, fieldSpecifierLength, resultToAppendTo);
    }

    else if (fieldSpecifier.startsWith('S')) { // millisecond 0-999
        this->_flushNumberValue(when.mMillisecond, fieldSpecifierLength, resultToAppendTo);
    }

    else if (fieldSpecifier.startsWith('z') || // time zone "general" format
            fieldSpecifier.startsWith('Z') || // time zone RFC 822 format
            fieldSpecifier.startsWith('X')) { // time zone ISO 8601 format
        this->_flushTimeZoneValue(utcOffsetMilliseconds, fieldSpecifier, resultToAppendTo);
    }

    fieldSpecifier = VString::EMPTY();
}

void VInstantFormatter::_flushFixedLengthTextValue(const VString& value, VString& resultToAppendTo) const {
    resultToAppendTo += value;
}

void VInstantFormatter::_flushVariableLengthTextValue(const VString& shortValue, const VString& longValue, int fieldLength, VString& resultToAppendTo) const {
    resultToAppendTo += (fieldLength < 4) ? shortValue : longValue;
}

void VInstantFormatter::_flushNumberValue(int value, int fieldLength, VString& resultToAppendTo) const {
    VString numberFormatter(VSTRING_FORMAT("%%0%dd", fieldLength)); // note double-percent to escape the first percent sign, and extra d: we want to end up with, say, "%05d" if the field length is 5.
    resultToAppendTo += VSTRING_FORMAT(numberFormatter, value);
}

void VInstantFormatter::_flushYearValue(int year, int fieldLength, VString& resultToAppendTo) const {
    // Rules say if if fieldLength is 2, truncate to 2 digits; otherwise treat as "number".
    if (fieldLength == 2) {
        resultToAppendTo += VSTRING_FORMAT("%02d", year % 100);
    } else {
        this->_flushNumberValue(year, fieldLength, resultToAppendTo);
    }
}

void VInstantFormatter::_flushMonthValue(int month, int fieldLength, VString& resultToAppendTo) const {
    VASSERT_IN_RANGE(month, 1, 12);

    // Rules say if if fieldLength is 3 or more, use name; otherwise treat as "number".
    if (fieldLength >= 3) {
        this->_flushVariableLengthTextValue(mLocaleInfo.MONTH_NAMES_SHORT[month-1], mLocaleInfo.MONTH_NAMES_LONG[month-1], fieldLength, resultToAppendTo);
    } else {
        this->_flushNumberValue(month, fieldLength, resultToAppendTo);
    }
}

void VInstantFormatter::_flushDayNameValue(int dayOfWeek/*0=sun ... 6=sat*/, int fieldLength, VString& resultToAppendTo) const {
    this->_flushVariableLengthTextValue(mLocaleInfo.DAY_NAMES_SHORT[dayOfWeek], mLocaleInfo.DAY_NAMES_LONG[dayOfWeek], fieldLength, resultToAppendTo);
}

void VInstantFormatter::_flushDayNumberValue(int dayOfWeek/*0=sun ... 6=sat*/, int fieldLength, VString& resultToAppendTo) const {
    // SimpleDateFormat day-of-week numbers are 1=Monday ... 7=Sunday, so compensate for Sunday:
    this->_flushNumberValue(dayOfWeek == 0 ? 7 : dayOfWeek, fieldLength, resultToAppendTo);
}

void VInstantFormatter::_flushTimeZoneValue(int utcOffsetMilliseconds, const VString& fieldSpecifier, VString& resultToAppendTo) const {
    int absOffsetHours = V_ABS(utcOffsetMilliseconds / (1000 * 60 * 60));
    int absOffsetMinutes = V_ABS((utcOffsetMilliseconds / (1000 * 60)) % 60);

    if (fieldSpecifier.startsWith('z')) { // general
        resultToAppendTo += VSTRING_FORMAT("GMT%c%02d:%02d", (utcOffsetMilliseconds < 0 ? '-':'+'), absOffsetHours, absOffsetMinutes);
    } else if (fieldSpecifier.startsWith('Z')) { // RFC 822
        resultToAppendTo += VSTRING_FORMAT("%c%02d%02d", (utcOffsetMilliseconds < 0 ? '-':'+'), absOffsetHours, absOffsetMinutes);
    } else if (fieldSpecifier.startsWith('X')) { // ISO 8601
        const int fieldSpecifierLength = fieldSpecifier.length();
        VASSERT_IN_RANGE(fieldSpecifierLength, 1, 4);

        if (utcOffsetMilliseconds == 0) {
            resultToAppendTo += 'Z';
        } else if (fieldSpecifier.length() == 1) { // rule says: sign followed by two-digit hours only
            resultToAppendTo += VSTRING_FORMAT("%c%02dZ", (utcOffsetMilliseconds < 0 ? '-':'+'), absOffsetHours);
        } else if (fieldSpecifier.length() == 2) { // rule says: sign followed by two-digit hours and minutes
            resultToAppendTo += VSTRING_FORMAT("%c%02d%02dZ", (utcOffsetMilliseconds < 0 ? '-':'+'), absOffsetHours, absOffsetMinutes);
        } else if (fieldSpecifier.length() == 3) { // rule says: sign followed by two-digit hours, colon, and minutes
            resultToAppendTo += VSTRING_FORMAT("%c%02d:%02dZ", (utcOffsetMilliseconds < 0 ? '-':'+'), absOffsetHours, absOffsetMinutes);
        }
    }
}
