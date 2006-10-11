/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

#ifndef vinstant_h
#define vinstant_h

/** @file */

#include "vtypes.h"

class VChar;
class VString;
class VDate;
class VTimeOfDay;
class VDateAndTime;

/**
A VDuration is a length of time. It is most useful in conjunction with VInstant
as the proper type to use when adding or subtracting an amount of time to a
VInstant, or when subtracting two VInstants to find out the length of time
between them.

Several constant values are defined, and by applying the multiplication
operator, you can create your own values describing particular lengths of
time. For example, to declare a constant for the duration of 1.5 hours, you
could do any of the following:
<code>
VDuration d1 = 90 * VDuration::MINUTE();
VDuration d2 = VDuration::HOUR() + (30 * VDuration::MINUTE());
assert(d1 == d2);
</code>

To get a VInstant describing a point in time 15 minutes in the future, you
could do the following:
<code>
VInstant x; // constructs to the current time
x += (15 * VDuration::MINUTE());
</code>

The only unusual behavior within VDuration is the existence of the UNSPECIFIED
duration constant. This is provided for cases where, due to backward compatibility,
you need to initialize a duration to a special value that a normal value will
never equal. It's a much better practice to let durations initialize to ZERO by
default. Behavior is undefined if you perform operations on an UNSPECIFIED duration.
You should only test them for equality.

Negative durations are perfectly OK. They behave just as you'd expect; for example,
if you subtract a later VInstant from an earlier VInstant, the resulting VDuration
will be negative.

The NEGATIVE_INFINITY and POSITIVE_INFINITY constants are provided primarily so
that the VInstant math operations can provide reasonable results when the instant
values are INFINITE_PAST or INFINITE_FUTURE. In addition, VDuration's own math
operations provide logical results when using these infinity values.
*/
class VDuration
    {
    public:

        static const VDuration& ZERO();        ///< Duration constant of zero milliseconds.
        static const VDuration& MILLISECOND(); ///< Duration constant of 1 millisecond.
        static const VDuration& SECOND();      ///< Duration constant of 1 second (1000 milliseconds).
        static const VDuration& MINUTE();      ///< Duration constant of 1 minute (60 seconds).
        static const VDuration& HOUR();        ///< Duration constant of 1 hour (60 minutes).
        static const VDuration& DAY();         ///< Duration constant of 1 day (24 hours).

        static const VDuration& UNSPECIFIED();         ///< Special duration constant that is unequal to any other duration value.
        static const VDuration& NEGATIVE_INFINITY();   ///< Special duration constant that is "less than" all specific durations.
        static const VDuration& POSITIVE_INFINITY();   ///< Special duration constant that is "greater than" all specific durations.

        /** Constructs a duration equal to ZERO, or zero milliseconds. */
        VDuration() : mDurationMilliseconds(0) {}
        /** Copy constructor. @param d a duration to copy. */
        VDuration(const VDuration& d) : mDurationMilliseconds(d.mDurationMilliseconds) {}
        /** Non-virtual destructor. This class is not intended to be subclassed. */
        ~VDuration() {}

        /** Assignment operator. @param d a duration to assign from */
        VDuration& operator=(const VDuration& d) { if (this != &d) mDurationMilliseconds = d.getDurationMilliseconds(); return *this; }
        /** Increment operator. @param forwardOffset a duration to add */
        VDuration& operator+=(const VDuration& forwardOffset);
        /** Decrement operator. @param backwardOffset a duration to subtract */
        VDuration& operator-=(const VDuration& backwardOffset);
        /** In-place multiplication operator. @param multiplier a number to multiply the duration by */
        VDuration& operator*=(Vs64 multiplier);
        /** In-place division operator. @param divisor a number to divide the duration by @throws does nothing if the divisor is zero */
        VDuration& operator/=(int divisor);
        /** In-place modulo operator. @param divisor a duration to modulo the duration by @throws does nothing if the divisor is zero or not specific */
        VDuration& operator%=(const VDuration& divisor);
        /** Unary negation operator. @return the negative of this duration */
        VDuration operator-() const;

        /** Returns the duration in milliseconds. @return obvious */
        Vs64 getDurationMilliseconds() const { return mDurationMilliseconds; }
        /** Returns the duration in whole seconds, using simple truncating division of milliseconds. @return obvious */
        int getDurationSeconds() const { return static_cast<int>(mDurationMilliseconds / kMillisecondsPerSecond); }
        /** Returns the duration in whole minutes, using simple truncating division of milliseconds. @return obvious */
        int getDurationMinutes() const { return static_cast<int>(mDurationMilliseconds / kMillisecondsPerMinute); }
        /** Returns the duration in whole hours, using simple truncating division of milliseconds. @return obvious */
        int getDurationHours() const { return static_cast<int>(mDurationMilliseconds / kMillisecondsPerHour); }
        /** Returns the duration in whole days, using simple truncating division of milliseconds. @return obvious */
        int getDurationDays() const { return static_cast<int>(mDurationMilliseconds / kMillisecondsPerDay); }

        /** Sets the duration in milliseconds. @param durationMilliseconds the duration to set, in milliseconds */
        void setDurationMilliseconds(Vs64 durationMilliseconds) { mDurationMilliseconds = durationMilliseconds; }
        /** Sets the duration in seconds. @param durationSeconds the duration to set, in seconds */
        void setDurationSeconds(int durationSeconds) { mDurationMilliseconds = kMillisecondsPerSecond * static_cast<Vs64>(durationSeconds); }
        /** Sets the duration in minutes. @param durationMinutes the duration to set, in minutes */
        void setDurationMinutes(int durationMinutes) { mDurationMilliseconds = kMillisecondsPerMinute * static_cast<Vs64>(durationMinutes); }
        /** Sets the duration in hours. @param durationHours the duration to set, in hours */
        void setDurationHours(int durationHours) { mDurationMilliseconds = kMillisecondsPerHour * static_cast<Vs64>(durationHours); }
        /** Sets the duration in 24-hour days (no DST calculations are done). @param durationDays the duration to set, in 24-hour days */
        void setDurationDays(int durationDays) { mDurationMilliseconds = kMillisecondsPerDay * static_cast<Vs64>(durationDays); }

        friend inline bool operator==(const VDuration& d1, const VDuration& d2);    ///< Compares d1 and d2 for equality in milliseconds using ==.
        friend inline bool operator!=(const VDuration& d1, const VDuration& d2);    ///< Compares d1 and d2 for equality in milliseconds using !=.
        friend inline bool operator>=(const VDuration& d1, const VDuration& d2);    ///< Compares d1 and d2 for equality in milliseconds using >=.
        friend inline bool operator<=(const VDuration& d1, const VDuration& d2);    ///< Compares d1 and d2 for equality in milliseconds using <=.
        friend inline bool operator>(const VDuration& d1, const VDuration& d2);     ///< Compares d1 and d2 for equality in milliseconds using >.
        friend inline bool operator<(const VDuration& d1, const VDuration& d2);     ///< Compares d1 and d2 for equality in milliseconds using <.
        friend inline VDuration operator+(const VDuration& d1, const VDuration& d2);///< Adds d1 + d2 and returns their total duration.
        friend inline VDuration operator-(const VDuration& d1, const VDuration& d2);///< Subtracts d2 from d1 and returns the difference in duration.
        friend inline VDuration operator*(Vs64 multiplier, const VDuration& d1);    ///< Multiplies d1 by a number and returns the resulting duration; uses Vs64 for VInstant delta compatibility.
        friend inline VDuration operator*(const VDuration& d1, Vs64 multiplier);    ///< Multiplies d1 by a number and returns the resulting duration; uses Vs64 for VInstant delta compatibility.
        friend VDuration operator/(const VDuration& d, int divisor);                ///< Divides d1 by a number and returns the resulting duration. @throws VException if the divisor is zero
        friend VDuration operator%(const VDuration& d, const VDuration& divisor);   ///< Returns d modulo the divisor. Does nothing if the divisor is zero or not specific.

        static inline VDuration min(const VDuration& d1, const VDuration& d2);      ///< Returns a duration that is the lesser of d1 and d2.
        static inline VDuration max(const VDuration& d1, const VDuration& d2);      ///< Returns a duration that is the greater of d1 and d2.
        static inline VDuration abs(const VDuration& d);                            ///< Returns a duration that is equal to d if d is > ZERO, and -d if d is < ZERO.

        /**
        Returns true if the duration can be legitimately compared using comparison
        operators such as <, <=, >, >=. This is the case for specific durations as
        well as the infinite durations. It is not the case for unspecified durations.
        */
        bool isComparable() const { return (*this != VDuration::UNSPECIFIED()); }
        /**
        Returns true if the duration has a specific value (not infinite) and therefore
        its internal duration value can be tested and used in simple math calculations.
        */
        bool isSpecific() const { return (*this != VDuration::UNSPECIFIED()) && (*this != VDuration::NEGATIVE_INFINITY()) && (*this != VDuration::POSITIVE_INFINITY()); }
        /**
        Returns true if d1 and d2 can be compared using simple value comparison.
        In other words, both d1 and d2 have specific values. It's false if either
        one is not specific.
        */
        static bool canCompareValues(const VDuration& d1, const VDuration& d2) { return d1.isSpecific() && d2.isSpecific(); }

    private:

        // These are called by the friend inline comparison operators for comparing non-simple durations.
        static bool _complexGT(const VDuration& d1, const VDuration& d2);
        static bool _complexGTE(const VDuration& d1, const VDuration& d2);
        static bool _complexLT(const VDuration& d1, const VDuration& d2);
        static bool _complexLTE(const VDuration& d1, const VDuration& d2);
        static VDuration _complexAdd(const VDuration& d1, const VDuration& d2);
        static VDuration _complexSubtract(const VDuration& d1, const VDuration& d2);
        static VDuration _complexMultiply(const VDuration& d, Vs64 multiplier);
        static VDuration _complexMin(const VDuration& d1, const VDuration& d2);
        static VDuration _complexMax(const VDuration& d1, const VDuration& d2);
        static VDuration _complexAbs(const VDuration& d);

        // This is the private constructor used internally.
        explicit VDuration(Vs64 durationMilliseconds) : mDurationMilliseconds(durationMilliseconds) {}

        Vs64 mDurationMilliseconds; ///< The duration in milliseconds. The values for UNSPECIFIED, NEGATIVE_INFINITY, and POSITIVE_INFINITY are special.

        // These are used internally for conversion.
        static const Vs64 kMillisecondsPerSecond = CONST_S64(1000);
        static const Vs64 kMillisecondsPerMinute = CONST_S64(60000);
        static const Vs64 kMillisecondsPerHour = CONST_S64(3600000);
        static const Vs64 kMillisecondsPerDay = CONST_S64(86400000);
    };

// Inline implementations of some of the above VDuration operators.
// The purpose is to make simple operations quick, and divert the complex versions
// to the separate functions.
inline bool    operator==(const VDuration& d1, const VDuration& d2) { return d1.mDurationMilliseconds == d2.mDurationMilliseconds; }
inline bool    operator!=(const VDuration& d1, const VDuration& d2) { return d1.mDurationMilliseconds != d2.mDurationMilliseconds; }
inline bool    operator>=(const VDuration& d1, const VDuration& d2) { if (VDuration::canCompareValues(d1, d2)) return d1.mDurationMilliseconds >= d2.mDurationMilliseconds; else return VDuration::_complexGTE(d1, d2); }
inline bool    operator<=(const VDuration& d1, const VDuration& d2) { if (VDuration::canCompareValues(d1, d2)) return d1.mDurationMilliseconds <= d2.mDurationMilliseconds; else return VDuration::_complexLTE(d1, d2); }
inline bool    operator>(const VDuration& d1, const VDuration& d2) { if (VDuration::canCompareValues(d1, d2)) return d1.mDurationMilliseconds > d2.mDurationMilliseconds; else return VDuration::_complexGT(d1, d2); }
inline bool    operator<(const VDuration& d1, const VDuration& d2) { if (VDuration::canCompareValues(d1, d2)) return d1.mDurationMilliseconds < d2.mDurationMilliseconds; else return VDuration::_complexLT(d1, d2); }
inline VDuration operator+(const VDuration& d1, const VDuration& d2) { if (VDuration::canCompareValues(d1, d2)) return VDuration(d1.mDurationMilliseconds + d2.mDurationMilliseconds); else return VDuration::_complexAdd(d1, d2); }
inline VDuration operator-(const VDuration& d1, const VDuration& d2) { if (VDuration::canCompareValues(d1, d2)) return VDuration(d1.mDurationMilliseconds - d2.mDurationMilliseconds); else return VDuration::_complexSubtract(d1, d2); }
inline VDuration operator*(Vs64 multiplier, const VDuration& d) { if (d.isSpecific()) return VDuration(d.mDurationMilliseconds * multiplier); else return VDuration::_complexMultiply(d, multiplier); }
inline VDuration operator*(const VDuration& d, Vs64 multiplier) { if (d.isSpecific()) return VDuration(d.mDurationMilliseconds * multiplier); else return VDuration::_complexMultiply(d, multiplier); }
inline VDuration VDuration::min(const VDuration& d1, const VDuration& d2) { if (VDuration::canCompareValues(d1, d2)) return (d1 < d2) ? d1 : d2; else return VDuration::_complexMin(d1, d2); }
inline VDuration VDuration::max(const VDuration& d1, const VDuration& d2) { if (VDuration::canCompareValues(d1, d2)) return (d1 > d2) ? d1 : d2; else return VDuration::_complexMax(d1, d2); }
inline VDuration VDuration::abs(const VDuration& d) { if (d.isSpecific()) return (d.mDurationMilliseconds < CONST_S64(0)) ? d : VDuration(-d.mDurationMilliseconds); else return VDuration::_complexAbs(d); }

/**
This structure is passed to or returned by the core functions to
describe a calendar structured instant in some implied time zone.
*/
class VInstantStruct
    {
    public:
        VInstantStruct() : mYear(0), mMonth(1), mDay(1), mHour(0), mMinute(0), mSecond(0), mMillisecond(0), mDayOfWeek(0) {}
        VInstantStruct(const VDate& date, const VTimeOfDay& timeOfDay);
        int mYear;          ///< The year.
        int mMonth;         ///< The month (1 to 12).
        int mDay;           ///< The day of the month (1 to 31).
        int mHour;          ///< The hour of day (0 to 23).
        int mMinute;        ///< The minute of the hour (0 to 59).
        int mSecond;        ///< The second of the minute (0 to 59).
        int mMillisecond;   ///< The millisecond within the second (0 to 999).
        int mDayOfWeek;     ///< See enum in VInstant: kSunday=0..kSaturday=6.
    };

/**
You can provide a callback interface for VInstant to allow for converting
to and from "remote time zones" (RTZ). Currently the built-in code of VInstant
(using OS-provided functions) can only convert between UTC and the local
time zone.

To implement remote time zone conversion, implement this interface, and
call VInstant::setRemoteTimeZoneConverter(). Then you can pass RTZ specifiers
to the VInstant APIs that allow them, and VInstant will call back to
the installed RTZ converter interface.
*/
class MRemoteTimeZoneConverter
    {
    public:

        MRemoteTimeZoneConverter() {}
        virtual ~MRemoteTimeZoneConverter() {}

        /**
        Converts an offset (ms from 1970 UTC, same as VInstant "value") to
        a broken-down calendar time in the specified time zone. This function
        should throw a VException if the time zone ID is invalid.
        @param    offset        a number milliseconds since UTC 1970 00:00:00.000
        @param    timeZoneID    a time zone ID string (e.g., PST, EST, etc.) from
                            the set of strings supported by the converter
        @param    when        a structure that will be filled to contain the
                            y/m/d/h/m/s for the specified UTC offset as seen in
                            the specified time zone
        */
        virtual void offsetToRTZStruct(Vs64 offset, const VString& timeZoneID, VInstantStruct& when) = 0;
        /**
        Converts a broken-down calendar time, interpreted in the specified
        time zone, to an offset (ms from 1970 UTC, same as VInstant "value").
        This function should throw a VException if the time zone ID is invalid.
        @param    timeZoneID    a time zone ID string (e.g., PST, EST, etc.) from
                            the set of strings supported by the converter
        @param    when        a structure containing y/m/d/h/m/s values in the
                            specified time zone
        @return the number milliseconds since UTC 1970 00:00:00.000 that the
                            time represents as interpreted in the specified
                            time zone
        */
        virtual Vs64 offsetFromRTZStruct(const VString& timeZoneID, const VInstantStruct& when) = 0;
    };

/**
A VInstant is an object that represents an instant in time regardless of the
location (and time zone) of where code is running.

A key purpose of VInstant is to abstract the OS time-related APIs while
retaining the ability to easily compare two instants that originated in
code running in different time zones--internally, the value is stored in
UTC time.

You can create a VInstant to represent the current instant in time, and you
can convert to and from string representations in UTC or the current locale.
A VInstant can also be created to represent instants infinitely far in the
past or future, to indicate events that have not yet happened or should
never happen.
*/

class VInstant
    {
    public:

        /** Constant VInstant representing the kInfinitePast value. */
        static const VInstant& INFINITE_PAST();
        /** Constant VInstant representing the kInfiniteFuture value. */
        static const VInstant& INFINITE_FUTURE();
        /** Constant VInstant representing the kNeverOccurred value. */
        static const VInstant& NEVER_OCCURRED();

        /** Constant for use with functions that take a timeZoneID parameter,
        indicating the time conversion is for UTC. */
        static const VString& UTC_TIME_ZONE_ID();
        /** Constant for use with functions that take a timeZoneID parameter,
        indicating the time conversion is for the local time zone. */
        static const VString& LOCAL_TIME_ZONE_ID();

        /**
        Returns an instant created from a raw 64-bit millisecond UTC epoch offset,
        of the same form as is returned by getValue() and can be passed to setValue().
        @param    value    the time value in ms since UTC 1970 00:00:00.000 (negative values
                represent earlier instants in time)
        @return an instant representing the specified time
        */
        static VInstant instantFromRawValue(Vs64 value) { return VInstant(value); }
        /**
        Returns an instant created from a POSIX time_t value, which is defined as the
        number of seconds since UTC 1970 00:00:00.000.
        @param    value    the POSIX time value (seconds since UTC 1970 00:00:00.000) (negative values
                represent earlier instants in time)
        @return an instant representing the specified time
        */
        static VInstant instantFromPosixTime(time_t value) { return VInstant(CONST_S64(1000) * static_cast<Vs64>(value)); }

        /**
        Creates an instant to represent the current time.
        */
        VInstant();
        /**
        Creates an instant by copying an existing instant.
        */
        VInstant(const VInstant& i) : mValue(i.mValue) {}
        /**
        Destructor.
        */
        ~VInstant() {}

        /**
        Copy constructor.
        @param    i    the instant to copy
        */
        VInstant& operator=(const VInstant& i);
        /**
        Increments (or decrements) the instant in time from its current value
        by the specified VDuration if the kind is kActualValue.
        If the kind is not kActualValue, this function has no effect.
        @param    forwardOffsetDuration    the duration offset in milliseconds to apply;
                        positive values move the instant forward in time;
                        negative values move it backward
        */
        VInstant& operator+=(const VDuration& forwardOffsetDuration);
        /**
        Decrements (or increments) the instant in time from its current value
        by the specified VDuration if the kind is kActualValue.
        If the kind is not kActualValue, this function has no effect.
        @param    backwardOffsetDuration    the offset in milliseconds to apply;
                        positive values move the instant backward in time;
                        negative values move it forward
        */
        VInstant& operator-=(const VDuration& backwardOffsetDuration);
        /**
        Sets the instant to the current time.
        */
        void setNow();
        /**
        Returns a UTC string representation of the instant.
        @param    s                the string to be formatted
        @param    fileNameSafe    if true, the returned string is stripped of the
                                punctuation formatting chars, yielding YYYYMMDDHHMMSS;
                                if false (the default), the returned string is
                                formatted using standard time notation.
        */
        void getUTCString(VString& s, bool fileNameSafe=false) const;
        /**
        Sets the instant from a UTC string representation.
        You must use the same string format as returned by getUTCString.
        @param    s    the UTC string representation of the instant
        @see    VInstant::getUTCString()
        */
        void setUTCString(const VString& s);
        /**
        Returns a local string representation of the instant.
        @param    s                the string to be formatted
        @param    fileNameSafe    if true, the returned string is stripped of the
                                punctuation formatting chars, yielding YYYYMMDDHHMMSS.
                                if false (the default), the returned string is
                                formatted using standard time notation.
        */
        void getLocalString(VString& s, bool fileNameSafe=false) const;
        /**
        Sets the instant from a local string representation.
        You must use the same string format as returned by getLocalString.
        @param    s    the local string representation of the instant
        @see    VInstant::getLocalString()
        */
        void setLocalString(const VString& s);
        /**
        Returns true if the instance has a specific time value, indicating that it
        is not one of the special time constants NEVER_OCCURRED, INFINITE_PAST, or
        INFINITE_FUTURE.
        */
        bool isSpecific() const { return (*this != VInstant::NEVER_OCCURRED()) && (*this != VInstant::INFINITE_PAST()) && (*this != VInstant::INFINITE_FUTURE()); }
        /**
        Returns true if the instance kind is comparable, indicating that it
        can be compared against another time along the timeline; in particular,
        instants with kind kNeverOccurred are not comparable.
        */
        bool isComparable() const { return (*this != VInstant::NEVER_OCCURRED()); }
        /**
        Returns the raw time value of the instant, defined as milliseconds since
        UTC 1970 00:00:00.000. Actual resolution may only be seconds, depending on OS.
        You should generally avoid accessing this value since it is not abstract;
        you can use other functions and operators to shift forward and backward
        in time, to get streamable values, etc. You can use the value returned
        by this function to set into another VInstant using setValue(), but you
        must also synchronize the "kind" as well (see getKind(), setKind()) if
        you are using any kinds other than kActualValue.
        @return    the time value in ms since UTC 1970 00:00:00.000 (negative values
                represent earlier instants in time)
        */
        Vs64 getValue() const { return mValue; }
        /**
        Sets the raw time value of the instant, defined as milliseconds since
        UTC 1970 00:00:00.000. Actual resolution may only be seconds, depending on OS.
        You should generally avoid accessing this value since it is not abstract;
        you can use other functions and operators to shift forward and backward
        in time, to set from streamed values, etc. You can use the value returned
        by getValue() to supply to this function, but you must also synchronize
        the "kind" as well (see getKind(), setKind()) if
        you are using any kinds other than kActualValue.
        @param    value    the time value in ms since UTC 1970 00:00:00.000 (negative values
                represent earlier instants in time)
        */
        void setValue(Vs64 value) { mValue = value; }
        /**
        Returns a VDate to represent the instant's date, just as would be returned
        in the date value returned by getValues() if you specified the local time zone.
        @return the date of the instant in the local time zone
        */
        VDate getLocalDate() const;
        /**
        Returns a VDate to represent the instant's date, just as would be returned
        in the date value returned by getValues(). The difference is that this function
        only returns the date, as a convenience. If you need both date and time, it is
        more efficient to call getValues() than getDate() followed by getTimeOfDay(),
        because the underlying conversion must happen on each of those calls. Throws an
        exception if you specify RTZ conversion and there is no converter installed.
        @param    timeZoneID    specifies which time zone the result should be given in
        @return the date of the instant in the specified time zone
        */
        VDate getDate(const VString& timeZoneID) const;
        /**
        Returns a VTimeOfDay to represent the instant's time, just as would be returned
        in the timeOfDay value returned by getValues() if you specified the local time
        zone.
        @return the time of day of the instant in the local time zone
        */
        VTimeOfDay getLocalTimeOfDay() const;
        /**
        Returns a VTimeOfDay to represent the instant's time, just as would be returned
        in the timeOfDay value returned by getValues(). The difference is that this function
        only returns the time of day, as a convenience. If you need both date and time, it is
        more efficient to call getValues() than getDate() followed by getTimeOfDay(),
        because the underlying conversion must happen on each of those calls. Throws an
        exception if you specify RTZ conversion and there is no converter installed.
        @param    timeZoneID    specifies which time zone the result should be given in
        @return the time of day of the instant in the specified time zone
        */
        VTimeOfDay    getTimeOfDay(const VString& timeZoneID) const;
        /**
        Returns a VDateAndTime to represent the instant's time, just as would be returned
        in the values returned by getValues(), using the local time zone.
        @return the date and time of day of the instant in the local time zone
        */
        VDateAndTime getLocalDateAndTime() const;
        /**
        Returns a VDateAndTime to represent the instant's time, just as would be returned
        in the values returned by getValues(). Throws an
        exception if you specify RTZ conversion and there is no converter installed.
        @param    timeZoneID    specifies which time zone the result should be given in
        @return the date and time of day of the instant in the specified time zone
        */
        VDateAndTime getDateAndTime(const VString& timeZoneID) const;
        /**
        Sets the instant to represent the specified time, just as would be done
        by setValues(), using the local time zone.
        @param    dt    the date and time of day to use
        */
        void setLocalDateAndTime(const VDateAndTime& dt);
        /**
        Sets the instant to represent the specified time, just as would be done
        by setValues(), using the specified time zone. Throws an
        exception if you specify RTZ conversion and there is no converter installed.
        @param    dt    the date and time of day to use
        @param    timeZoneID    specifies which time zone the date and time is given in
        */
        void setDateAndTime(const VDateAndTime& dt, const VString& timeZoneID);
        /**
        Returns a VDate and VTimeOfDay to represent the instant, either in
        UTC or local time, or in a remote time zone (if an RTZ converter has
        been installed). Throws an exception if you specify RTZ conversion
        and there is no converter installed.
        @param    date        the VDate to set
        @param    timeOfDay    the VTimeOfDay to set
        @param    timeZoneID    specifies which time zone the result should be given in
        */
        void getValues(VDate& date, VTimeOfDay& timeOfDay, const VString& timeZoneID) const;
        /**
        Sets the instant from a VDate and VTimeOfDay, either in UTC or local time,
        or in a remote time zone (if an RTZ converter has been installed). Throws an
        exception if you specify RTZ conversion and there is no converter installed.
        @param    date        the date to use
        @param    timeOfDay    the time of day to use
        @param    timeZoneID    specifies which time zone in which the supplied date and
                            time are given in
        */
        void setValues(const VDate& date, const VTimeOfDay& timeOfDay, const VString& timeZoneID);
        /**
        Returns the local offset in milliseconds, of the local time zone, at the instant
        in time represented by this VInstant. For locales west of GMT, this value is
        negative (e.g., -8 hours or -7 hours in California, depending on daylight time),
        and for locales east of GMT, this value is positive.
        */
        Vs64 getLocalOffsetMilliseconds() const;

        friend inline bool operator==(const VInstant& i1, const VInstant& i2);    ///< Compares i1 and i2 for equality. @param i1    an instant @param i2 an instant @return true if i1 == i2
        friend inline bool operator!=(const VInstant& i1, const VInstant& i2);    ///< Compares i1 and i2 for equality. @param i1    an instant @param i2 an instant @return true if i1 != i2
        friend inline bool operator>=(const VInstant& i1, const VInstant& i2);    ///< Compares i1 and i2 for equality. @param i1    an instant @param i2 an instant @return true if i1 >= i2
        friend inline bool operator<=(const VInstant& i1, const VInstant& i2);    ///< Compares i1 and i2 for equality. @param i1    an instant @param i2 an instant @return true if i1 <= i2
        friend inline bool operator>(const VInstant& i1, const VInstant& i2);    ///< Compares i1 and i2 for equality. @param i1    an instant @param i2 an instant @return true if i1 > i2
        friend inline bool operator<(const VInstant& i1, const VInstant& i2);    ///< Compares i1 and i2 for equality. @param i1    an instant @param i2 an instant @return true if i1 < i2
        friend inline VDuration operator-(const VInstant& i1, const VInstant& i2);    ///< Returns the time duration between i1 and i2. @param i1 an instant @param i2 an instant @return i1 - i2 as a VDuration
        friend inline VInstant operator+(const VInstant& i1, const VDuration& forwardDuration);    ///< Returns an instant incremented by a delta. @param i1 an instant @param forwardDuration the duration to add @return the result of i1+forwardDuration
        friend inline VInstant operator-(const VInstant& i1, const VDuration& backwardDuration);///< Returns an instant decremented by a delta. @param i1 an instant @param backwardDuration the duration to subtract to add @return the result of i1-backwardDuration

        static inline VInstant min(const VInstant& i1, const VInstant& i2); ///< Returns the earlier of i1 and i2. @param i1    an instant @param i2 an instant @return i1 if i1 < i2
        static inline VInstant max(const VInstant& i1, const VInstant& i2); ///< Returns the later of i1 and i2. @param i1    an instant @param i2 an instant @return i1 if i1 > i2

        /**
        Returns a relatively high-resolution snapshot of the current time, for
        use with a subsequent call to VInstant::snapshotDelta(). You should
        not assume that the snapshot value remains valid across system starts;
        if you need to get the difference in times across reboots, use normal
        VInstant operator minus.
        @return the current system clock snapshot
        */
        static Vs64 snapshot();
        /**
        Returns the delta between the current time and the supplied
        origin snapshot value. As noted in the VInstant::snapshot() docs, the
        delta may not work across reboots.
        @param    snapshotValue    a snapshot value from VInstant::snapshot()
        @return the time delta from the snapshot to now
        */
        static VDuration snapshotDelta(Vs64 snapshotValue);

        /**
        Installs a Remote Time Zone Converter, that VInstant will use for functions
        that take a timeZoneID, if that parameter is not UTC or the local time zone.
        The caller still owns the converter; VInstant will not delete it. You can
        pass NULL to disable use of RTZ conversion (by default, no converter is
        installed).
        */
        static void setRemoteTimeZoneConverter(MRemoteTimeZoneConverter* converter);
        /**
        Returns the currently installed Remote Time Zone Converter, which may be
        NULL. You might use this to delete the old converter if you are installing
        a new one. VInstant uses whatever converter is installed (if it's not NULL)
        at the time it needs it.
        */
        static MRemoteTimeZoneConverter* getRemoteTimeZoneConverter();

        /**
        Adjusts the simulated clock offset. The simulated clock offset is applied
        by _platform_now() and _platform_snapshot() to the values they return. This
        can be used to simulate a faster passing of time, by adjusting the clock
        forward. It may be impractical to adjust the clock backwards, because some
        code constructs may behave badly if time flows backwards. However, you may
        be able to apply an initial backwards offset if you wish to start your
        program running in a simulated time in the past.
        */
        static void incrementSimulatedClockOffset(Vs64 delta);
        /**
        Sets the simulated clock offset. The simulated clock offset is applied
        by _platform_now() and _platform_snapshot() to the values they return. This
        can be used to simulate a faster passing of time, by adjusting the clock
        forward. It may be impractical to adjust the clock backwards, because some
        code constructs may behave badly if time flows backwards. However, you may
        be able to apply an initial backwards offset if you wish to start your
        program running in a simulated time in the past.
        */
        static void setSimulatedClockOffset(Vs64 offsetValue);

    private:

        // This is the private constructor used internally.
        explicit VInstant(Vs64 utcOffsetMilliseconds) : mValue(utcOffsetMilliseconds) {}

        Vs64 mValue; ///< Milliseconds since: UTC 1970 00:00:00.000. Actual resolution may only be seconds, depending on OS. Using {64-bit,ms} means our definition does not have painful range limits like {32-bit,sec}.

        /**
        Provides a robust interface to localtime without the thread safety pitfalls,
        using the re-entrant version if available. Throws an exception if the date
        specified is out of range for the platform's time implementation.
        (Specifically, Win32 does not support dates prior to 1970!)
        The result structure contains the broken-down time corresponding to the
        supplied offset, in local time.
        @param    epochOffset        number of seconds since 1/1/1970 UTC
        @param    resultStorage    pointer to tm struct to be filled out in local time
        */
        static void threadsafe_localtime(const time_t epochOffset, struct tm* resultStorage);
        /**
        Provides a robust interface to gmtime without the thread safety pitfalls,
        using the re-entrant version if available. Throws an exception if the date
        specified is out of range for the platform's time implementation.
        (Specifically, Win32 does not support dates prior to 1970!)
        The result structure contains the broken-down time corresponding to the
        supplied offset, in GMT time.
        @param    epochOffset        number of seconds since 1/1/1970 UTC
        @param    resultStorage    pointer to tm struct to be filled out in GMT time
        */
        static void threadsafe_gmtime(const time_t epochOffset, struct tm* resultStorage);

        /**
        Returns true if i1 and i2 can be compared using simple value comparison.
        (It's false if either one has mKind != kActualValue.)
        */
        static bool canCompareValues(const VInstant& i1, const VInstant& i2) { return i1.isSpecific() && i2.isSpecific(); }
        /**
        Returns true if i1 > i2, considering the complexity of mKind != kActualValue.
        */
        static bool _complexGT(const VInstant& i1, const VInstant& i2);
        /**
        Returns true if i1 >= i2, considering the complexity of mKind != kActualValue.
        */
        static bool _complexGTE(const VInstant& i1, const VInstant& i2);
        /**
        Returns true if i1 < i2, considering the complexity of mKind != kActualValue.
        */
        static bool _complexLT(const VInstant& i1, const VInstant& i2);
        /**
        Returns true if i1 <= i2, considering the complexity of mKind != kActualValue.
        */
        static bool _complexLTE(const VInstant& i1, const VInstant& i2);

        /*
        These are the core function interfaces that must be implemented on a
        per-platform basis. That is, they are platform-specific and are
        implemented in the vinstant_platform.* files. All other time
        conversion is performed via these calls. This allows us to isolate
        the platform and library quirks in those files rather than conditionally
        compile code in vinstant.cpp.

        There are only two types used here.
        - The 64-bit millisecond "offset" from UTC 1970. This is also what VInstant
          uses an offset. We'll just refer to this as a UTC Epoch Offset.
        - The y/m/d/h/m/s "struct" in some time zone. We'll just refer to this
          as a "Time Struct".

        It's all about converting between them.

        There is one additional function, _platform_snapshot(), which is not about
        time conversion, but just about measuring short time durations with
        millisecond resolution.
        */

        /**
        Returns the current UTC Epoch Offset in milliseconds. Actual resolution may
        only be seconds, depending on OS.
        @return the current UTC Epoch Offset in milliseconds
        */
        static Vs64 _platform_now();
        /**
        Returns the UTC Epoch Offset of the specified Time Struct, treating the Time
        Struct as an instant in the machine's local time zone. The mDayOfWeek field
        is ignored.
        @return the UTC Epoch Offset of the specified local time struct
        */
        static Vs64 _platform_offsetFromLocalStruct(const VInstantStruct& when);
        /**
        Returns the UTC Epoch Offset of the specified Time Struct, treating the Time
        Struct as an instant in the UTC time zone. The mDayOfWeek field is ignored.
        @return the UTC Epoch Offset of the specified UTC time struct
        */
        static Vs64 _platform_offsetFromUTCStruct(const VInstantStruct& when);
        /**
        Fills in a Time Struct to represent the specified UTC Epoch Offset in the
        machine's local time zone.
        @param    offset    a UTC Epoch Offset
        @param    when    the Time Struct to fill in with the local time zone
                        representation of the offset
        */
        static void _platform_offsetToLocalStruct(Vs64 offset, VInstantStruct& when);
        /**
        Fills in a Time Struct to represent the specified UTC Epoch Offset in the
        UTC time zone.
        @param    offset    a UTC Epoch Offset
        @param    when    the Time Struct to fill in with the UTC time zone
                        representation of the offset
        */
        static void _platform_offsetToUTCStruct(Vs64 offset, VInstantStruct& when);
        /**
        Returns a current time value with millisecond resolution; the base value
        is unspecified, and this is only to be used to count time deltas between
        two snapshot values.
        @return a clock value representing "now" in milliseconds, base undefined
        */
        static Vs64 _platform_snapshot();

        static Vs64 gSimulatedClockOffset; ///< Value applied by _platform_now() and _platform_snapshot() to simulate non-real-time flow.
        static MRemoteTimeZoneConverter* gRemoteTimeZoneConverter; ///< The converter for RTZ conversion, or NULL.

        // Let VDate call the getTimeValue() bottleneck for getting the day of
        // week, but don't make it a public API since it's exposing the tm
        // structure which is dependent on time.h functionality.
        friend class VDate;
        friend class VInstantUnit;    // Let unit test validate our internal APIs.
    };

// Inline implementations of some of the above VInstant operators.
inline bool    operator==(const VInstant& i1, const VInstant& i2) { return i1.mValue == i2.mValue; }
inline bool    operator!=(const VInstant& i1, const VInstant& i2) { return i1.mValue != i2.mValue; }
inline bool    operator>=(const VInstant& i1, const VInstant& i2) { if (VInstant::canCompareValues(i1, i2)) return i1.mValue >= i2.mValue; else return VInstant::_complexGTE(i1, i2); }
inline bool    operator<=(const VInstant& i1, const VInstant& i2) { if (VInstant::canCompareValues(i1, i2)) return i1.mValue <= i2.mValue; else return VInstant::_complexLTE(i1, i2); }
inline bool    operator>(const VInstant& i1, const VInstant& i2) { if (VInstant::canCompareValues(i1, i2)) return i1.mValue > i2.mValue; else return VInstant::_complexGT(i1, i2); }
inline bool    operator<(const VInstant& i1, const VInstant& i2) { if (VInstant::canCompareValues(i1, i2)) return i1.mValue < i2.mValue; else return VInstant::_complexLT(i1, i2); }
inline VDuration operator-(const VInstant& i1, const VInstant& i2) { if (VInstant::canCompareValues(i1, i2)) return VDuration::MILLISECOND() * (i1.mValue - i2.mValue); else return VDuration(); }
inline VInstant operator+(const VInstant& i1, const VDuration& forwardDuration) { VInstant result = i1; result += forwardDuration; return result; }
inline VInstant operator-(const VInstant& i1, const VDuration& backwardDuration) { VInstant result = i1; result -= backwardDuration; return result; }
inline VInstant VInstant::min(const VInstant& i1, const VInstant& i2) { return (i1 < i2) ? i1 : i2; }
inline VInstant VInstant::max(const VInstant& i1, const VInstant& i2) { return (i1 > i2) ? i1 : i2; }

/**
VDate represents a calendar date: a year/month/day.
Per (*) below, the "day" field is actually allowed to be in the range 1 to 32,
for purposes of allowing the caller to increment the day and then convert to
a VInstant.
*/
class VDate
    {
    public:

        /**
        Constructs a date with values set to 0000-01-01 (useless but legal state
        for a VDate object). Your first use of such a date object should be to
        assign it a useful value.
        */
        VDate();
        /**
        Constructs a date from the current instant.
        Throws an exception if you
        specify RTZ conversion and there is no converter installed.
        @param    timeZoneID    specifies which time zone the y/m/d should be given in
        */
        VDate(const VString& timeZoneID);
        /**
        Constructs a date from specified values.
        @param    year    the year
        @param    month    the month (1 to 12)
        @param    day        the day of the month (1 to 31*)
        */
        VDate(int inYear, int inMonth, int inDay);
        /**
        Destructor.
        */
        virtual ~VDate() {}

        /**
        Returns the year of the date.
        @return    the year
        */
        int getYear() const;
        /**
        Returns the month of the date.
        @return    the month (1 to 12)
        */
        int getMonth() const;
        /**
        Returns the day of the month of the date.
        @return    the day of the month (1 to 31*)
        */
        int getDay() const;
        /**
        Returns the day of the week of the date.
        @return the day of the week (0=kSunday to 6=kSaturday, as in time.h
                and defined in the enum below)
        */
        int getDayOfWeek() const;
        /**
        Sets the date from specified values.
        @param    year    the year
        @param    month    the month of the year (1 to 12)
        @param    day        the day of the month (1 to 31*)
        */
        void set(int inYear, int inMonth, int inDay);
        /**
        Sets the year of the date.
        @param    year    the year
        */
        void setYear(int year);
        /**
        Sets the month of the date.
        @param    month    the month of the year (1 to 12)
        */
        void setMonth(int month);
        /**
        Sets the day of the month.
        @param    day    the day of the month (1 to 31*)
        */
        void setDay(int day);

        // These static functions can be extended in the future to
        // return values based on the locale.

        /**
        Returns the date separator for the locale.
        (Currently just returns a slash.)
        @return the date separator character constant
        */
        static const VChar& getLocalDateSeparator() { return kLocalDateSeparator; }

        enum {
            kYMD,        ///< Year/Month/Day
            kYDM,        ///< Year/Day/Month
            kMYD,        ///< Month/Year/Day
            kMDY,        ///< Year/Day/Month
            kDYM,        ///< Day/Year/Month
            kDMY,        ///< Day/Month/Year
            };

        /**
        Returns the date string element order for the locale.
        (Currently just returns a kMDY.)
        @return the date string format order
        */
        static int getLocalDateOrder() { return kMDY; }

        enum {
            kSunday = 0,
            kMonday,
            kTuesday,
            kWednesday,
            kThursday,
            kFriday,
            kSaturday
            };

        friend inline bool operator==(const VDate& d1, const VDate& d2);

    private:

        /** Asserts if any invariant is broken. */
        void assertInvariant() const;

        int    mYear;    ///< The year.
        int    mMonth;    ///< The month (1 to 12).
        int    mDay;    ///< The day of the month (1 to 31*).

        static const VChar kLocalDateSeparator;    ///< The character to separate M/D/Y
    };

inline bool operator==(const VDate& d1, const VDate& d2) { return (d1.mYear == d2.mYear) && (d1.mMonth == d2.mMonth) && (d1.mDay == d2.mDay); }

/**
VTimeOfDay represents a time of day without understanding about
calendars or time zones; it is simply an hour/minute/second container.
*/
class VTimeOfDay
    {
    public:

        /**
        Constructs a time of day with all values set to zero, which is valid
        and means midnight (start of a day).
        */
        VTimeOfDay();
        /**
        Constructs a time of day from the current instant.
        Throws an exception if you
        specify RTZ conversion and there is no converter installed.
        @param    timeZoneID    specifies which time zone the h/m/s should be given in
        */
        VTimeOfDay(const VString& timeZoneID);
        /**
        Constructs a time of day from specified values.
        @param    hour    the hour of day (0 to 23)
        @param    minute    the minute of the hour (0 to 59)
        @param    second    the second of the minute (0 to 59)
        @param    millisecond    the millisecond of the second (0 to 999)
        */
        VTimeOfDay(int inHour, int inMinute, int inSecond, int inMillisecond);
        /**
        Destructor.
        */
        virtual ~VTimeOfDay() {}

        /**
        Returns the hour of day.
        @return the hour (0 to 23)
        */
        int getHour() const;
        /**
        Returns the minute of the hour.
        @return the minute (0 to 59)
        */
        int getMinute() const;
        /**
        Returns the second of the minute.
        @return the second (0 to 59)
        */
        int getSecond() const;
        /**
        Returns the millisecond of the second.
        @return the second (0 to 999)
        */
        int getMillisecond() const;
        /**
        Sets the time of day from specified values.
        @param    hour    the hour of day (0 to 23)
        @param    minute    the minute of the hour (0 to 59)
        @param    second    the second of the minute (1 to 59)
        @param    millisecond    the millisecond of the second (0 to 999)
        */
        void set(int hour, int minute, int second, int millisecond);
        /**
        Sets the hour of the day.
        @param    hour    the hour of day (0 to 23)
        */
        void setHour(int hour);
        /**
        Sets the minute of the hour.
        @param    minute    the minute of the hour (0 to 59)
        */
        void setMinute(int minute);
        /**
        Sets the second of the minute.
        @param    second    the second of the minute (1 to 59)
        */
        void setSecond(int second);
        /**
        Sets the millisecond of the second.
        @param    millisecond    the millisecond of the second (0 to 999)
        */
        void setMillisecond(int millisecond);
        /**
        Sets the hour, minute, second, and millisecond to zero.
        */
        void setToStartOfDay();

        // These static functions can be extended in the future to
        // return values based on the locale.

        /**
        Returns the time separator for the locale.
        (Currently just returns a colon.)
        @return the time separator character constant
        */
        static const VChar& getLocalTimeSeparator() { return kLocalTimeSeparator; }

        friend inline bool operator==(const VTimeOfDay& t1, const VTimeOfDay& t2);

    private:

        /** Asserts if any invariant is broken. */
        void assertInvariant() const;

        int    mHour;        ///< The hour of day (0 to 23).
        int    mMinute;    ///< The minute of the hour (0 to 59).
        int    mSecond;    ///< The second of the minute (0 to 59).
        int    mMillisecond;///< The millisecond of the second (0 to 999).

        static const VChar kLocalTimeSeparator;    ///< The character to separate HH:MM:SS
    };

inline bool operator==(const VTimeOfDay& t1, const VTimeOfDay& t2) { return (t1.mHour == t2.mHour) && (t1.mMinute == t2.mMinute) && (t1.mSecond == t2.mSecond) && (t1.mMillisecond == t2.mMillisecond); }

/**
VDateAndTime simply aggregates a VDate and a VTimeOfDay into one convenient object.
*/
class VDateAndTime
    {
    public:

        VDateAndTime() : mDate(), mTimeOfDay() {}
        VDateAndTime(const VString& timeZoneID);
        VDateAndTime(int inYear, int inMonth, int inDay, int inHour, int inMinute, int inSecond, int inMillisecond) :
            mDate(inYear, inMonth, inDay), mTimeOfDay(inHour, inMinute, inSecond, inMillisecond) {}
        ~VDateAndTime() {}

        // Use these accessors to call the date and time of day sub-objects directly.
        const VDate& getDate() const { return mDate; }
        const VTimeOfDay& getTimeOfDay() const { return mTimeOfDay; }

        int getYear() const { return mDate.getYear(); }
        int getMonth() const { return mDate.getMonth(); }
        int getDay() const { return mDate.getDay(); }
        int getDayOfWeek() const { return mDate.getDayOfWeek(); }
        int getHour() const { return mTimeOfDay.getHour(); }
        int getMinute() const { return mTimeOfDay.getMinute(); }
        int getSecond() const { return mTimeOfDay.getSecond(); }
        int getMillisecond() const { return mTimeOfDay.getMillisecond(); }

        void set(int inYear, int inMonth, int inDay, int inHour, int inMinute, int inSecond, int inMillisecond)
            { mDate.set(inYear, inMonth, inDay); mTimeOfDay.set(inHour, inMinute, inSecond, inMillisecond); }

        void setYear(int year) { mDate.setYear(year); }
        void setMonth(int month) { mDate.setMonth(month); }
        void setDay(int day) { mDate.setDay(day); }
        void setHour(int hour) { mTimeOfDay.setHour(hour); }
        void setMinute(int minute) { mTimeOfDay.setMinute(minute); }
        void setSecond(int second) { mTimeOfDay.setSecond(second); }
        void setMillisecond(int millisecond) { mTimeOfDay.setMillisecond(millisecond); }
        void setToStartOfDay() { mTimeOfDay.setToStartOfDay(); }

        friend inline bool operator==(const VDateAndTime& dt1, const VDateAndTime& dt2);

    private:

        VDate       mDate;
        VTimeOfDay  mTimeOfDay;
    };

inline bool operator==(const VDateAndTime& dt1, const VDateAndTime& dt2) { return (dt1.mDate == dt2.mDate) && (dt1.mTimeOfDay == dt2.mTimeOfDay); }

#endif /* vinstant_h */
