/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
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

/**
This structure is passed to or returned by the core functions to
describe a calendar structured instant in some implied time zone.
*/
class VInstantStruct
    {
    public:
        VInstantStruct() {}
        VInstantStruct(const VDate& date, const VTimeOfDay& timeOfDay);
        int    mYear;        ///< The year.
        int    mMonth;        ///< The month (1 to 12).
        int    mDay;        ///< The day of the month (1 to 31).
        int    mHour;        ///< The hour of day (0 to 23).
        int    mMinute;    ///< The minute of the hour (0 to 59).
        int    mSecond;    ///< The second of the minute (0 to 59).
        int mMillisecond;///< The millisecond within the second (0 to 999).
        int mDayOfWeek;    ///< See enum in VInstant: kSunday=0..kSaturday=6.
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
    
        enum {
            kActualValue,        ///< An instant whose value has normal meaning.
            kInfinitePast,        ///< An instant representing a time infinitely far in the past.
            kInfiniteFuture,    ///< An instant representing a time infinitely far in the future.
            kNeverOccurred,        ///< An instant representing an event that never happened.

            kNumKinds            ///< (Used internally.)
            };

        /** Constant for use with functions that take a timeZoneID parameter,
        indicating the time conversion is for UTC. */
        static const VString kUTCTimeZoneID;
        /** Constant for use with functions that take a timeZoneID parameter,
        indicating the time conversion is for the local time zone. */
        static const VString kLocalTimeZoneID;
        
        /**
        Installs a Remote Time Zone Converter, that VInstant will use for functions
        that take a timeZoneID, if that parameter is not UTC or the local time zone.
        These functions include getValues(), setValues(), getDate(), getTimeOfDay(),
        as well as the constructors for the Date and TimeOfDay classes. The caller
        still owns the converter; VInstant will not delete it. You can pass NULL to
        disable use of RTZ conversion (by default, no converter is installed).
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
        Creates an instant to represent the current time.
        */
        VInstant();
        /**
        Creates an instant to represent a particular time value.
        @param    kind    the instant kind
        @see    VInstant::kActualValue
        @see    VInstant::kInfinitePast
        @see    VInstant::kInfiniteFuture
        @see    VInstant::kNeverOccurred
        @param    value    for a kind of kActualValue, the time value in ms since UTC 1970 00:00:00.000 (negative values
                represent earlier instants in time); otherwise, pass zero for
                consistency's sake.
        */
        VInstant(int kind, Vs64 value);
        /**
        Creates an instant to represent a given Unix time_t.
        @param    time    the Unix time_t value to use
        */
        explicit VInstant(time_t inTime);
        /**
        Destructor.
        */
        virtual ~VInstant() {}

        /**
        Copy constructor.
        @param    i    the instant to copy
        */
        VInstant& operator=(const VInstant& i);
        /**
        Increments (or decrements) the instant in time from its current value
        by the specified number of milliseconds if the kind is kActualValue.
        If the kind is not kActualValue, this function has no effect.
        @param    forwardOffsetMilliseconds    the offset in milliseconds to apply; positive values
                        move the instant forward in time; negative values
                        move it backward
        */
        VInstant& operator+=(Vs64 forwardOffsetMilliseconds);
        /**
        Decrements (or increments) the instant in time from its current value
        by the specified number of milliseconds if the kind is kActualValue.
        If the kind is not kActualValue, this function has no effect.
        @param    backwardOffsetMilliseconds    the offset in milliseconds to apply; positive values
                        move the instant backward in time; negative values
                        move it forward
        */
        VInstant& operator-=(Vs64 backwardOffsetMilliseconds);
        /**
        Sets the instant to the current time.
        */
        void    setNow();
        /**
        Increments (or decrements) the instant in time from its current value
        by the specified number of milliseconds if the kind is kActualValue.
        If the kind is not kActualValue, this function has no effect.
        @param    offsetMilliseconds    the offset in milliseconds to apply; positive values
                        move the instant forward in time; negative values
                        move it backward
        */
        void    deltaOffset(Vs64 offsetMilliseconds);
        /**
        Returns a UTC string representation of the instant.
        @param    s                the string to be formatted
        @param    fileNameSafe    if true, the returned string is stripped of the
                                punctuation formatting chars, yielding YYYYMMDDHHMMSS;
                                if false (the default), the returned string is
                                formatted using standard time notation.
        */
        void    getUTCString(VString& s, bool fileNameSafe=false) const;
        /**
        Sets the instant from a UTC string representation.
        You must use the same string format as returned by getUTCString.
        @param    s    the UTC string representation of the instant
        @see    VInstant::getUTCString()
        */
        void    setUTCString(const VString& s);
        /**
        Returns a local string representation of the instant.
        @param    s                the string to be formatted
        @param    fileNameSafe    if true, the returned string is stripped of the
                                punctuation formatting chars, yielding YYYYMMDDHHMMSS.
                                if false (the default), the returned string is
                                formatted using standard time notation.
        */
        void    getLocalString(VString& s, bool fileNameSafe=false) const;
        /**
        Sets the instant from a local string representation.
        You must use the same string format as returned by getLocalString.
        @param    s    the local string representation of the instant
        @see    VInstant::getLocalString()
        */
        void    setLocalString(const VString& s);
        /**
        Returns the kind of instant, namely one of the enum values defined by
        this class.
        You only need this if your instant can take on one of the special
        meanings defined by the enum.
        @return    the instant kind
        @see    VInstant::kActualValue
        @see    VInstant::kInfinitePast
        @see    VInstant::kInfiniteFuture
        @see    VInstant::kNeverOccurred
        */
        int        getKind() const;
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
        Vs64    getValue() const;
        /**
        Returns the raw time value of the instant, defined as seconds since
        UTC 1970 00:00:00.000. Note that because this is a 32-bit value, it can
        only represent about +/- 66 years of time on either side of 1970.
        You should generally avoid accessing this value since it is not abstract;
        you can use other functions and operators to shift forward and backward
        in time, to get streamable values, etc. You can use the value returned
        by this function to set into another VInstant using setValueSeconds(),
        but you must also synchronize the "kind" as well (see getKind(), setKind()) if
        you are using any kinds other than kActualValue.
        @return    the time value in sec since UTC 1970 00:00:00.000 (negative values
                represent earlier instants in time)
        */
        Vs32    getValueSeconds() const;
        /**
        Sets the kind of instant, namely one of the enum values defined by
        this class.
        You only need this if your instant can take on one of the special
        meanings defined by the enum.
        If you set the kind to kActualValue, then you should presumably also
        call VInstant::setValues() to specify what that actual value is.
        @param    kind    the instant kind
        @see    VInstant::kActualValue
        @see    VInstant::kInfinitePast
        @see    VInstant::kInfiniteFuture
        @see    VInstant::kNeverOccurred
        @see    VInstant::setValues()
        */
        void    setKind(int kind);
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
        void    setValue(Vs64 value);
        /**
        Sets the raw time value of the instant, defined as seconds since
        UTC 1970 00:00:00.000. Note that because this is a 32-bit value, it can
        only represent about +/- 66 years of time on either side of 1970.
        You should generally avoid accessing this value since it is not abstract;
        you can use other functions and operators to shift forward and backward
        in time, to set from streamed values, etc. You can use the value returned
        by getValueSeconds() to supply to this function, but you must also synchronize
        the "kind" as well (see getKind(), setKind()) if
        you are using any kinds other than kActualValue.
        @param    value    the time value in ms since UTC 1970 00:00:00.000 (negative values
                represent earlier instants in time)
        */
        void    setValueSeconds(Vs32 value);
        /**
        Returns the fields needed to store this instant in a stream. We don't
        provide stream i/o methods directly here because we would rather make
        stream undestand instant than make instant understand stream. If you
        only deal in kActualValue kinds, then you don't need to store the kind
        in a stream, and can just use getValue().
        @param    kind    reference to variable to return kind field in
        @param    value    reference to variable to return value field in
        @see VInstant::setStreamValues
        */
        void    getStreamValues(Vs32& kind, Vs64& value) const;
        /**
        Sets the instant from values stored in a stream. We don't
        provide stream i/o methods directly here because we would rather make
        stream undestand instant than make instant understand stream. If you
        only deal in kActualValue kinds, then you don't need to store the kind
        in a stream, and can just use setValue().
        @param    kind    what to set the kind field to
        @param    value    what to set the value field to
        @see VInstant::getStreamValues
        */
        void    setStreamValues(Vs32 kind, Vs64 value);
        /**
        Returns a VDate and VTimeOfDay to represent the instant, either in
        UTC or local time, or in a remote time zone (if an RTZ converter has
        been installed). Throws an exception if you specify RTZ conversion
        and there is no converter installed.
        @param    date        the VDate to set
        @param    timeOfDay    the VTimeOfDay to set
        @param    timeZoneID    specifies which time zone the strings should be given in
                            (default is kUTCTimeZoneID; kLocalTimeZoneID is the usual
                            other choice, but other time zone IDs may be passed if an
                            RTZ converter has been installed)
        */
        void    getValues(VDate& date, VTimeOfDay& timeOfDay, const VString& timeZoneID=kUTCTimeZoneID) const;
        /**
        Sets the instant from a VDate and VTimeOfDay, either in UTC or local time,
        or in a remote time zone (if an RTZ converter has been installed). Throws an
        exception if you specify RTZ conversion and there is no converter installed.
        @param    date        the date to use
        @param    timeOfDay    the time of day to use
        @param    timeZoneID    specifies which time zone in which the supplied date and
                            time are presumed to be
                            (default is kUTCTimeZoneID; kLocalTimeZoneID is the usual
                            other choice, but other time zone IDs may be passed if an
                            RTZ converter has been installed)
        */
        void    setValues(const VDate& date, const VTimeOfDay& timeOfDay, const VString& timeZoneID=kUTCTimeZoneID);
        /**
        Returns the VDate for the instant, either in UTC or local time, or in a remote
        time zone (if an RTZ converter has been installed). Throws an exception if you
        specify RTZ conversion and there is no converter installed.
        @param    date        the VDate to set
        @param    timeZoneID    specifies which time zone the strings should be given in
                            (default is kUTCTimeZoneID; kLocalTimeZoneID is the usual
                            other choice, but other time zone IDs may be passed if an
                            RTZ converter has been installed)
        */
        void    getDate(VDate& date, const VString& timeZoneID=kUTCTimeZoneID) const;
        /**
        Returns the VTimeOfDay for represent the instant, either in UTC or local time,
        or in a remote time zone (if an RTZ converter has been installed). Throws an
        exception if you specify RTZ conversion and there is no converter installed.
        @param    timeOfDay    the VTimeOfDay to set
        @param    timeZoneID    specifies which time zone the strings should be given in
                            (default is kUTCTimeZoneID; kLocalTimeZoneID is the usual
                            other choice, but other time zone IDs may be passed if an
                            RTZ converter has been installed)
        */
        void    getTimeOfDay(VTimeOfDay& timeOfDay, const VString& timeZoneID=kUTCTimeZoneID) const;
        /**
        Returns the local offset in milliseconds, of the local time zone, at the instant
        in time represented by this VInstant. For locales west of GMT, this value is
        negative (e.g., -8 hours or -7 hours in California, depending on daylight time),
        and for locales east of GMT, this value is positive.
        */
        Vs64    getLocalOffsetMilliseconds() const;

        friend inline bool operator==(const VInstant& i1, const VInstant& i2);    ///< Compares i1 and i2 for equality. @param i1    an instant @param i2 an instant @return true if i1 == i2
        friend inline bool operator!=(const VInstant& i1, const VInstant& i2);    ///< Compares i1 and i2 for equality. @param i1    an instant @param i2 an instant @return true if i1 != i2
        friend inline bool operator>=(const VInstant& i1, const VInstant& i2);    ///< Compares i1 and i2 for equality. @param i1    an instant @param i2 an instant @return true if i1 >= i2
        friend inline bool operator<=(const VInstant& i1, const VInstant& i2);    ///< Compares i1 and i2 for equality. @param i1    an instant @param i2 an instant @return true if i1 <= i2
        friend inline bool operator>(const VInstant& i1, const VInstant& i2);    ///< Compares i1 and i2 for equality. @param i1    an instant @param i2 an instant @return true if i1 > i2
        friend inline bool operator<(const VInstant& i1, const VInstant& i2);    ///< Compares i1 and i2 for equality. @param i1    an instant @param i2 an instant @return true if i1 < i2
        friend inline Vs64 operator-(const VInstant& i1, const VInstant& i2);    ///< Returns the difference in milliseconds between i1 and i2. @param i1 an instant @param i2 an instant @return i1 - i2
    
        // Let VDate call the getTimeValue() bottleneck for getting the day of
        // week, but don't make it a public API since it's exposing the tm
        // structure which is dependent on time.h functionality.
        friend class VDate;
        
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
        Returns the delta in milliseconds between the current time and the supplied
        origin snapshot value. As noted in the VInstant::snapshot() docs, the
        delta may not work across reboots.
        @param    snapshotValue    a snapshot value from VInstant::snapshot()
        @return the delta in milliseconds from the snapshot to now
        */
        static Vs64 snapshotDelta(Vs64 snapshotValue);

    private:
    
        /** Asserts if any invariant is broken. */
        void assertInvariant() const;
        
        int        mKind;    ///< Enumeration to allow special values to be expressed.
        Vs64    mValue; ///< Milliseconds since: UTC 1970 00:00:00.000. Actual resolution may only be seconds, depending on OS. Using {64-bit,ms} means our definition does not have range limits like {32-bit,sec}.

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
        static bool canCompareValues(const VInstant& i1, const VInstant& i2) { return (i1.mKind == kActualValue) && (i2.mKind == kActualValue); }
        /**
        Returns true if i1 > i2, considering the complexity of mKind != kActualValue.
        */
        static bool complexGT(const VInstant& i1, const VInstant& i2);
        /**
        Returns true if i1 >= i2, considering the complexity of mKind != kActualValue.
        */
        static bool complexGTE(const VInstant& i1, const VInstant& i2);
        /**
        Returns true if i1 < i2, considering the complexity of mKind != kActualValue.
        */
        static bool complexLT(const VInstant& i1, const VInstant& i2);
        /**
        Returns true if i1 <= i2, considering the complexity of mKind != kActualValue.
        */
        static bool complexLTE(const VInstant& i1, const VInstant& i2);

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
        
        There is one additional function, platform_snapshot(), which is not about
        time conversion, but just about measuring short time durations with
        millisecond resolution.
        */

        /**
        Returns the current UTC Epoch Offset in milliseconds. Actual resolution may
        only be seconds, depending on OS.
        @return the current UTC Epoch Offset in milliseconds
        */
        static Vs64 platform_now();
        /**
        Returns the UTC Epoch Offset of the specified Time Struct, treating the Time
        Struct as an instant in the machine's local time zone. The mDayOfWeek field
        is ignored.
        @return the UTC Epoch Offset of the specified local time struct
        */
        static Vs64 platform_offsetFromLocalStruct(const VInstantStruct& when);
        /**
        Returns the UTC Epoch Offset of the specified Time Struct, treating the Time
        Struct as an instant in the UTC time zone. The mDayOfWeek field is ignored.
        @return the UTC Epoch Offset of the specified UTC time struct
        */
        static Vs64 platform_offsetFromUTCStruct(const VInstantStruct& when);
        /**
        Fills in a Time Struct to represent the specified UTC Epoch Offset in the
        machine's local time zone.
        @param    offset    a UTC Epoch Offset
        @param    when    the Time Struct to fill in with the local time zone
                        representation of the offset
        */
        static void platform_offsetToLocalStruct(Vs64 offset, VInstantStruct& when);
        /**
        Fills in a Time Struct to represent the specified UTC Epoch Offset in the
        UTC time zone.
        @param    offset    a UTC Epoch Offset
        @param    when    the Time Struct to fill in with the UTC time zone
                        representation of the offset
        */
        static void platform_offsetToUTCStruct(Vs64 offset, VInstantStruct& when);
        /**
        Returns a current time value with millisecond resolution; the base value
        is unspecified, and this is only to be used to count time deltas between
        two snapshot values.
        @return a clock value representing "now" in milliseconds, base undefined
        */
        static Vs64 platform_snapshot();
        
        static MRemoteTimeZoneConverter* gRemoteTimeZoneConverter; ///< The converter for RTZ conversion, or NULL.

        friend class VInstantUnit;    // Let unit test validate our internal APIs.

    };

/**
VDate represents a calendar date.
*/
class VDate
    {
    public:
        
        /**
        Constructs a date from the current instant.
        By default the date will use the local time zone, but you
        can specify that you want the date of the current instant
        in UTC or a remote time zone. Throws an exception if you
        specify RTZ conversion and there is no converter installed.
        @param    timeZoneID    specifies which time zone the y/m/d should be given in
                            (default is kLocalTimeZoneID; kUTCTimeZoneID is the usual
                            other choice, but other time zone IDs may be passed if an
                            RTZ converter has been installed for VInstant)
        */
        VDate(const VString& timeZoneID=VInstant::kLocalTimeZoneID);
        /**
        Constructs a date from specified values.
        @param    year    the year
        @param    month    the month (1 to 12)
        @param    day        the day of the month (1 to 31)
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
        int        year() const;
        /**
        Returns the month of the date.
        @return    the month (1 to 12)
        */
        int        month() const;
        /**
        Returns the day of the month of the date.
        @return    the day of the month (1 to 31)
        */
        int        day() const;
        /**
        Returns the day of the week of the date.
        @return the day of the week (0=kSunday to 6=kSaturday, as in time.h
                and defined in the enum below)
        */
        int        dayOfWeek() const;
        /**
        Sets the date from specified values.
        @param    year    the year
        @param    month    the month (1 to 12)
        @param    day        the day of the month (1 to 31)
        */
        void    set(int inYear, int inMonth, int inDay);
        
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
        int    mDay;    ///< The day of the month (1 to 31).
        
        static const VChar kLocalDateSeparator;    ///< The character to separate M/D/Y
    };

/**
VTimeOfDay represents a time of day without understanding about
calendars or time zones; it is simply an hour/minute/second container.
*/
class VTimeOfDay
    {
    public:
        
        /**
        Constructs a time of day from the current instant.
        By default the date will use the local time zone, but you
        can specify that you want the value of the current instant
        in UTC or a remote time zone. Throws an exception if you
        specify RTZ conversion and there is no converter installed.
        @param    timeZoneID    specifies which time zone the h/m/s should be given in
                            (default is kLocalTimeZoneID; kUTCTimeZoneID is the usual
                            other choice, but other time zone IDs may be passed if an
                            RTZ converter has been installed for VInstant)
        */
        VTimeOfDay(const VString& timeZoneID=VInstant::kLocalTimeZoneID);
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
        int        hour() const;
        /**
        Returns the minute of the hour.
        @return the minute (0 to 59)
        */
        int     minute() const;
        /**
        Returns the second of the minute.
        @return the second (0 to 59)
        */
        int     second() const;
        /**
        Returns the millisecond of the second.
        @return the second (0 to 999)
        */
        int     millisecond() const;
        /**
        Sets the time of day from specified values.
        @param    hour    the hour of day (0 to 23)
        @param    minute    the minute of the hour (0 to 59)
        @param    second    the second of the minute (1 to 59)
        @param    millisecond    the millisecond of the second (0 to 999)
        */
        void    set(int inHour, int inMinute, int inSecond, int inMillisecond);

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

inline bool    operator==(const VInstant& i1, const VInstant& i2) { return (i1.mKind == i2.mKind) && (i1.mValue == i2.mValue); }
inline bool    operator!=(const VInstant& i1, const VInstant& i2) { return (i1.mKind != i2.mKind) || (i1.mValue != i2.mValue); }
inline bool    operator>=(const VInstant& i1, const VInstant& i2) { if (VInstant::canCompareValues(i1, i2)) return i1.mValue >= i2.mValue; else return VInstant::complexGTE(i1, i2); }
inline bool    operator<=(const VInstant& i1, const VInstant& i2) { if (VInstant::canCompareValues(i1, i2)) return i1.mValue <= i2.mValue; else return VInstant::complexLTE(i1, i2); }
inline bool    operator>(const VInstant& i1, const VInstant& i2) { if (VInstant::canCompareValues(i1, i2)) return i1.mValue > i2.mValue; else return VInstant::complexGT(i1, i2); }
inline bool    operator<(const VInstant& i1, const VInstant& i2) { if (VInstant::canCompareValues(i1, i2)) return i1.mValue < i2.mValue; else return VInstant::complexLT(i1, i2); }
inline Vs64 operator-(const VInstant& i1, const VInstant& i2) { return i1.mValue - i2.mValue; }

inline bool operator==(const VDate& d1, const VDate& d2) { return (d1.mYear == d2.mYear) && (d1.mMonth == d2.mMonth) && (d1.mDay == d2.mDay); }
inline bool operator==(const VTimeOfDay& t1, const VTimeOfDay& t2) { return (t1.mHour == t2.mHour) && (t1.mMinute == t2.mMinute) && (t1.mSecond == t2.mSecond) && (t1.mMillisecond == t2.mMillisecond); }

#endif /* vinstant_h */
