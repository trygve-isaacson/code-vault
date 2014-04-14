/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

/** @file */

#include "vinstantunit.h"

#include "vinstant.h"
#include "vexception.h"
#include "vthread.h"
#include "vassert.h"

VInstantUnit::VInstantUnit(bool logOnSuccess, bool throwOnError) :
    VUnit("VInstantUnit", logOnSuccess, throwOnError) {
}

void VInstantUnit::run() {
    VInstant now;
    this->logStatus(VSTRING_FORMAT("VInstant current local time is %s. This must be visually confirmed to be correct.", now.getLocalString().chars()));
    this->logStatus(VSTRING_FORMAT("VInstant current UTC time is %s. This must be visually confirmed to be correct.", now.getUTCString().chars()));

    this->_runInstantOperatorTests();
    this->_runInstantComparatorTests();
    this->_runClockSimulationTests();
    this->_runTimeZoneConversionTests();
    this->_runDurationValueTests();
    this->_runExoticDurationValueTests();
    this->_runDurationStringTests();
    this->_runInstantFormatterTests();
}

void VInstantUnit::_runInstantOperatorTests() {

    VInstant    i1;
    VInstant    i2;
    Vs64        offset1;
    Vs64        offset2;
    Vs64        base = i1.getValue();    // may be useful in debugging below

    // Test symmetry of sets and gets.
    offset1 = i1.getValue();
    i2.setValue(offset1);
    offset2 = i2.getValue();
    VUNIT_ASSERT_EQUAL_LABELED(offset1, offset2, "symmetry test 1");
    VUNIT_ASSERT_EQUAL_LABELED(i1, i2, "symmetry test 2");

    // Test modification functions.
    // Set things up for this set of tests.
    offset1 = base;
    offset2 = base;
    i1.setValue(base);
    i2.setValue(base);

    // operator+=
    const VDuration kDeltaA = VDuration::MILLISECOND() * CONST_S64(12345);
    i1 += kDeltaA;                                      // i1 is base + kDeltaA
    offset2 += kDeltaA.getDurationMilliseconds();        // off2 is base + kDeltaA
    offset1 = i1.getValue();                            // off1 is base + kDeltaA
    VUNIT_ASSERT_EQUAL_LABELED(offset1, offset2, "modification test 1");
    i2 += kDeltaA;                                      // i2 is base + kDeltaA
    VUNIT_ASSERT_EQUAL_LABELED(i1, i2, "modification test 2");

    // operator-=
    const VDuration kDeltaC = VDuration::MILLISECOND() * CONST_S64(13579);
//    const Vs64 kDeltaABC = kDeltaAB - kDeltaC;            // Might be useful in debugging.
    i1 -= kDeltaC;                                      // i1 is base + kDeltaABC
    offset2 -= kDeltaC.getDurationMilliseconds();        // off2 is base + kDeltaABC
    offset1 = i1.getValue();                            // off1 is base + kDeltaABC
    VUNIT_ASSERT_EQUAL_LABELED(offset1, offset2, "modification test 5");
    i2 -= VDuration(kDeltaC);                            // i2 is base + kDeltaABC
    VUNIT_ASSERT_EQUAL_LABELED(i1, i2, "modification test 6");

    // Test comparison operators.
    // Set things up for this set of tests.
    offset1 = base;
    offset2 = base;
    i1.setValue(base);
    i2.setValue(base);

    const VDuration kDeltaD = VDuration::MILLISECOND() * CONST_S64(24680);
    i2 += kDeltaD;
    // Now i2 is kDeltaD milliseconds later than i1.
    VUNIT_ASSERT_FALSE_LABELED(i1 == i2, "comparison test 1a");
    VUNIT_ASSERT_TRUE_LABELED(i1 != i2, "comparison test 1b");
    VUNIT_ASSERT_TRUE_LABELED(i1 < i2, "comparison test 1c");
    VUNIT_ASSERT_TRUE_LABELED(i1 <= i2, "comparison test 1d");
    VUNIT_ASSERT_TRUE_LABELED(i2 > i1, "comparison test 1e");
    VUNIT_ASSERT_TRUE_LABELED(i2 >= i1, "comparison test 1f");
    VUNIT_ASSERT_TRUE_LABELED(i2 - i1 == kDeltaD, "comparison test 1g");
    i2 -= kDeltaD;
    // Now i1 and i2 are equal.
    VUNIT_ASSERT_TRUE_LABELED(i1 == i2, "comparison test 2a");
    VUNIT_ASSERT_FALSE_LABELED(i1 != i2, "comparison test 2b");
    VUNIT_ASSERT_FALSE_LABELED(i1 < i2, "comparison test 2c");
    VUNIT_ASSERT_TRUE_LABELED(i1 <= i2, "comparison test 2d");
    VUNIT_ASSERT_FALSE_LABELED(i2 > i1, "comparison test 2e");
    VUNIT_ASSERT_TRUE_LABELED(i2 >= i1, "comparison test 2f");
    VUNIT_ASSERT_TRUE_LABELED(i2 - i1 == VDuration::ZERO(), "comparison test 2g");

}

void VInstantUnit::_runInstantComparatorTests() {

    // Test comparison operators with "infinite" values.
    VInstant    now;
    VInstant    infinitePast = VInstant::INFINITE_PAST();
    VInstant    infiniteFuture = VInstant::INFINITE_FUTURE();
    VInstant    past = now; past -= VDuration::MINUTE(); // about a minute before "now"
    VInstant    future = now; future += VDuration::MINUTE(); // about a minute later than "now"

    VUNIT_ASSERT_TRUE_LABELED(infinitePast < now, "comparison test 3a");
    VUNIT_ASSERT_TRUE_LABELED(infinitePast <= now, "comparison test 3b");
    VUNIT_ASSERT_TRUE_LABELED(now > infinitePast, "comparison test 3c");
    VUNIT_ASSERT_TRUE_LABELED(now >= infinitePast, "comparison test 3d");
    VUNIT_ASSERT_FALSE_LABELED(infinitePast > now, "comparison test 3e");
    VUNIT_ASSERT_FALSE_LABELED(infinitePast >= now, "comparison test 3f");
    VUNIT_ASSERT_TRUE_LABELED(infinitePast != now, "comparison test 3g");
    VUNIT_ASSERT_FALSE_LABELED(infinitePast == now, "comparison test 3h");

    VUNIT_ASSERT_TRUE_LABELED(infiniteFuture > now, "comparison test 4a");
    VUNIT_ASSERT_TRUE_LABELED(infiniteFuture >= now, "comparison test 4b");
    VUNIT_ASSERT_TRUE_LABELED(now < infiniteFuture, "comparison test 4c");
    VUNIT_ASSERT_TRUE_LABELED(now <= infiniteFuture, "comparison test 4d");
    VUNIT_ASSERT_FALSE_LABELED(infiniteFuture < now, "comparison test 4e");
    VUNIT_ASSERT_FALSE_LABELED(infiniteFuture <= now, "comparison test 4f");
    VUNIT_ASSERT_TRUE_LABELED(infiniteFuture != now, "comparison test 4g");
    VUNIT_ASSERT_FALSE_LABELED(infiniteFuture == now, "comparison test 4h");

    VUNIT_ASSERT_TRUE_LABELED(infinitePast < infiniteFuture, "comparison test 5a");
    VUNIT_ASSERT_TRUE_LABELED(infinitePast <= infiniteFuture, "comparison test 5b");
    VUNIT_ASSERT_TRUE_LABELED(infiniteFuture > infinitePast, "comparison test 5c");
    VUNIT_ASSERT_TRUE_LABELED(infiniteFuture >= infinitePast, "comparison test 5d");
    VUNIT_ASSERT_FALSE_LABELED(infinitePast > infiniteFuture, "comparison test 5e");
    VUNIT_ASSERT_FALSE_LABELED(infinitePast >= infiniteFuture, "comparison test 5f");
    VUNIT_ASSERT_TRUE_LABELED(infinitePast != infiniteFuture, "comparison test 5g");
    VUNIT_ASSERT_FALSE_LABELED(infinitePast == infiniteFuture, "comparison test 5h");

    VUNIT_ASSERT_TRUE_LABELED(infinitePast == VInstant::INFINITE_PAST(), "comparison test 6a");
    VUNIT_ASSERT_TRUE_LABELED(infiniteFuture == VInstant::INFINITE_FUTURE(), "comparison test 6b");
    VUNIT_ASSERT_TRUE_LABELED(VInstant::min(infinitePast, now) == infinitePast, "comparison test 6c");
    VUNIT_ASSERT_TRUE_LABELED(VInstant::max(infinitePast, now) == now, "comparison test 6d");
    VUNIT_ASSERT_TRUE_LABELED(VInstant::min(infiniteFuture, now) == now, "comparison test 6e");
    VUNIT_ASSERT_TRUE_LABELED(VInstant::max(infiniteFuture, now) == infiniteFuture, "comparison test 6f");
    VUNIT_ASSERT_TRUE_LABELED(VInstant::min(past, now) == past, "comparison test 6g");
    VUNIT_ASSERT_TRUE_LABELED(VInstant::max(past, now) == now, "comparison test 6h");
    VUNIT_ASSERT_TRUE_LABELED(VInstant::min(future, now) == now, "comparison test 6i");
    VUNIT_ASSERT_TRUE_LABELED(VInstant::max(future, now) == future, "comparison test 6j");
    VUNIT_ASSERT_TRUE_LABELED(VInstant::min(past, future) == past, "comparison test 6k");
    VUNIT_ASSERT_TRUE_LABELED(VInstant::max(past, future) == future, "comparison test 6l");

}

void VInstantUnit::_runClockSimulationTests() {

    /* scope for test subset local variables */ {
        // Test the operation of the simulated clock offset. Restore it right away,
        // because while we do this, we are messing with the time continuum! (Other
        // threads that get the current time from VInstant will see weirdness.)
        
        VInstant base0;
        VInstant basePlus1Minute = base0; basePlus1Minute += VDuration::MINUTE();
        VInstant::incrementSimulatedClockOffset(2 * VDuration::MINUTE()); // should put us forward about 2 additional minutes
        VInstant fakeFutureNow;
        VUNIT_ASSERT_TRUE_LABELED(fakeFutureNow > basePlus1Minute, "advance simulated clock offset");
        VInstant::setSimulatedClockOffset(VDuration::ZERO()); // restore the time continuum to normal
        VInstant normalNow;
        VUNIT_ASSERT_TRUE_LABELED(normalNow >= base0, "restore simulated clock offset part 1");
        VUNIT_ASSERT_TRUE_LABELED(normalNow < basePlus1Minute, "restore simulated clock offset part 2"); // can only fail if it takes > 1 real minute to execute the last 5 lines of code
    }

    /* scope for test subset local variables */ {

        // Here we test that setSimulatedClockValue() sets the time correctly;
        // we set it and the obtain the current time, which should differ by
        // only by the amount of time it takes to execute the set and get, so
        // we'll allow 1 second to be on the safe side. Should be 1ms or less in reality.
        VDateAndTime fakePastDT(1984, 1, 23, 9, 15, 0, 0);
        VInstant fakePastInstant; fakePastInstant.setLocalDateAndTime(fakePastDT);
        VInstant::setSimulatedClockValue(fakePastInstant);
        VInstant fakePastNow;
        VUNIT_ASSERT_TRUE_LABELED(fakePastNow - fakePastInstant < VDuration::SECOND(), "set clock to past instant");
        // note that we do NOT zero the offset before the next test; we want to verify it can be set directly
        VDateAndTime fakeFutureDT(2034, 1, 6, 14, 35, 0, 0);
        VInstant fakeFutureInstant; fakeFutureInstant.setLocalDateAndTime(fakeFutureDT);
        VInstant::setSimulatedClockValue(fakeFutureInstant);
        VInstant fakeFutureNow;
        VUNIT_ASSERT_TRUE_LABELED(fakeFutureNow - fakeFutureInstant < VDuration::SECOND(), "set clock to future instant");

        VInstant::setSimulatedClockOffset(VDuration::ZERO()); // restore the time continuum to normal
    }

    /* scope for test subset local variables */ {

        VInstant realNow;

        VDateAndTime fakePastDT(1990, 3, 17, 10, 11, 0, 0);
        VInstant fakePastInstant; fakePastInstant.setLocalDateAndTime(fakePastDT);

        // Freeze time at the specified past time.
        VInstant::freezeTime(fakePastInstant);

        // Sleep for 2 seconds and verify that no time seemed to actually pass.
        VThread::sleep(2 * VDuration::SECOND());
        VInstant frozenNow1;
        VUNIT_ASSERT_TRUE_LABELED(frozenNow1 == fakePastInstant, "freeze time 1");

        Vs64 frozenSnapshot = VInstant::snapshot();

        // Shift frozen time forward by 10 seconds and validate.
        VDuration shiftAmount = 10 * VDuration::SECOND();
        VInstant::shiftFrozenTime(shiftAmount);
        VInstant frozenNow2;
        VUNIT_ASSERT_TRUE_LABELED(frozenNow2 == frozenNow1 + shiftAmount, "shift frozen time");

        VDuration frozenSnapshotDelta = VInstant::snapshotDelta(frozenSnapshot);
        VUNIT_ASSERT_TRUE_LABELED(frozenSnapshotDelta == shiftAmount, "shift frozen time snapshot");

        VUNIT_ASSERT_TRUE_LABELED(VInstant::isTimeFrozen(), "time is frozen");

        // Sleep for 2 seconds and verify that no time seemed to actually pass.
        VThread::sleep(2 * VDuration::SECOND());
        VInstant frozenNow3;
        VUNIT_ASSERT_TRUE_LABELED(frozenNow3 == frozenNow2, "freeze time 2");

        // Unfreeze time and make sure it now rolls forward in true real time.
        // First we verify that the current time is equal to or later than the
        // real time when we started this test block.
        // Then we sleep a little bit and verify that a later time is reported.
        // We need to sleep long enough to exceed the time resolution on all
        // platforms. On Windows this can be > 100ms. Otherwise, it may look like
        // time did not roll forward while we slept.
        VInstant::unfreezeTime();
        VInstant realNow1;
        VUNIT_ASSERT_TRUE_LABELED(realNow1 >= realNow, "normal time resumed");
        VThread::sleep(200 * VDuration::MILLISECOND());
        VInstant realNow2;
        VUNIT_ASSERT_TRUE_LABELED(realNow2 > realNow1, "unfrozen time proceeds");

        VInstant::setSimulatedClockOffset(VDuration::ZERO()); // restore the time continuum to normal
    }

}

void VInstantUnit::_runTimeZoneConversionTests() {

    // Test local-gm time conversion consistency.

    // First let's validate that we get the expected value for UTC zero time.
    VDate        utc0Date(1970, 1, 1);
    VTimeOfDay    utc0Time(0, 0, 0, 0);
    VInstant    utc0Instant;

    utc0Instant.setValues(utc0Date, utc0Time, VInstant::UTC_TIME_ZONE_ID());
    VUNIT_ASSERT_TRUE_LABELED(utc0Instant.getValue() == CONST_S64(0), "utc epoch base");

    // A little debugging code here:
    // Out of curiosity, do all platforms agree on what values we get exactly
    // 24 hours after that? (Let's avoid pre-1970 values for Windows compatibility.)
    VInstant utc0Plus1Instant = utc0Instant;
    utc0Plus1Instant += VDuration::DAY(); // one day later
    utc0Plus1Instant.getValues(utc0Date, utc0Time, VInstant::UTC_TIME_ZONE_ID()); // see what that is in Greenwich (should be 1970 Jan 2 00:00:00)
    utc0Plus1Instant.getValues(utc0Date, utc0Time, VInstant::LOCAL_TIME_ZONE_ID()); // see what that is in local time (should be 1970 Jan 2 00:00:00 minus local time zone delta)
    VDate       utc1Date(1970, 1, 2);
    VTimeOfDay  utc1Time(0, 0, 0, 0);
    VInstant    utc1Instant;
    utc1Instant.setValues(utc1Date, utc1Time, VInstant::UTC_TIME_ZONE_ID()); // see if setting Jan 2 UTC works out to 86400000
    VUNIT_ASSERT_EQUAL_LABELED(utc0Plus1Instant, utc1Instant, "utc day 0 plus 1 is utc day 1");

    // Create a date and time, specified in both local and gm.
    VDate        july_14_2004(2004, 7, 14);
    VTimeOfDay    noon(12, 0, 0, 0);
    VInstant    july_14_2004_noon_local;
    VInstant    july_14_2004_noon_utc;

    july_14_2004_noon_local.setValues(july_14_2004, noon, VInstant::LOCAL_TIME_ZONE_ID());
    july_14_2004_noon_utc.setValues(july_14_2004, noon, VInstant::UTC_TIME_ZONE_ID());

    VDate        dateLocal;
    VTimeOfDay    noonLocal;
    VDate        dateUTC;
    VTimeOfDay    noonUTC;

    july_14_2004_noon_utc.getValues(dateLocal, noonLocal, VInstant::LOCAL_TIME_ZONE_ID());
    july_14_2004_noon_utc.getValues(dateUTC, noonUTC, VInstant::UTC_TIME_ZONE_ID());
    // If you're testing this in Pacific time, dateLocal/noonLocal vs. dateUTC/noonUTC
    // should differ by 8 hours in the winter (standard), 7 hours in the summer (daylight).

    // Verify symmetry of UTC<->Local conversion in the core platform-specific code.
    this->_testInstantRangeRoundTripConversion("Testing current time", VInstant(), VDuration::HOUR(), 24);
    
    VInstant daylightSwitchTestInstant;
    daylightSwitchTestInstant.setValues(VDate(2014, 3, 9), VTimeOfDay(0, 0, 0, 0), VInstant::UTC_TIME_ZONE_ID());
    daylightSwitchTestInstant -= VDuration::DAY();
    this->_testInstantRangeRoundTripConversion("Testing near daylight switch", daylightSwitchTestInstant, 30 * VDuration::MINUTE(), 96);
    
    // Test some instants in time ranges that may have platform issues.
    VInstant platformTest;

/*
Notes:
- No platform seems to support times before 1904, regardless of 64-bit, etc.
- I'm surprised that Unix times don't start at 1900, what with the "subtract 1900" time base junk in struct tm.
- Windows does not support negative offsets, so times before 1970 are not supported.
- 32-bit offsets end in 2038, so 64-bit values/APIs are needed to reach beyond.
- 64-bit offsets can reach past the year 5000, but Windows 64-bit APIs only support through the year 3000.
- The UTC time offset for 1 second before 1970 is the value -1 (or -1000 milliseconds). But -1 is the value
  returned for failure in the timegm() and mktime(), so we can't tell success from failure.
  On Windows this is probably OK because it doesn't support pre-1970 times anyway.
  But on Unix we may be able to special case UTC time 1969-12-31 23:59:59.000 and return -1000ms.
  (Our timegm() will do this, so just call it always?)
  Not sure if this workaround is possible for mktime() because it is time zone dependent.
*/
#ifdef VCOMPILER_64BIT
    #define VCOMPILER_IS_64BIT true
#else
    #define VCOMPILER_IS_64BIT false
#endif

#ifdef VPLATFORM_MAC
    #define TIME_APIS_SUPPORT_1776 false
    #define TIME_APIS_SUPPORT_1900 false
    #define TIME_APIS_SUPPORT_1906 true
    #define TIME_APIS_SUPPORT_1969 true
    #define TIME_APIS_SUPPORT_2039 VCOMPILER_IS_64BIT
    #define TIME_APIS_SUPPORT_3001 VCOMPILER_IS_64BIT
    #define TIME_APIS_SUPPORT_5001 VCOMPILER_IS_64BIT
#endif

#ifdef VPLATFORM_UNIX
    #define TIME_APIS_SUPPORT_1776 false
    #define TIME_APIS_SUPPORT_1900 false
    #define TIME_APIS_SUPPORT_1906 true
    #define TIME_APIS_SUPPORT_1969 true
    #define TIME_APIS_SUPPORT_2039 VCOMPILER_IS_64BIT
    #define TIME_APIS_SUPPORT_3001 VCOMPILER_IS_64BIT
    #define TIME_APIS_SUPPORT_5001 VCOMPILER_IS_64BIT
#endif

#ifdef VPLATFORM_WIN

    #ifdef VCOMPILER_MSVC
        #define HAS_MKTIME64 true
    #else
        #define HAS_MKTIME64 false
    #endif

    #define TIME_APIS_SUPPORT_1776 false
    #define TIME_APIS_SUPPORT_1900 false
    #define TIME_APIS_SUPPORT_1906 false
    #define TIME_APIS_SUPPORT_1969 false
    #define TIME_APIS_SUPPORT_2039 HAS_MKTIME64  // With MSVC, we use ::_mktime64() which reaches through year 3000
    #define TIME_APIS_SUPPORT_3001 false
    #define TIME_APIS_SUPPORT_5001 false
#endif
    
    // Using UTC times here so our values are identical no matter where test is run from.
    // TODO: vary the booleans by platform; initial values are expected result for Mac OS X
    this->_test1InstantRangeRoundTripConversion("Testing earliest",         VDateAndTime(1776,  7,  4, 12,  0,  0, 0), TIME_APIS_SUPPORT_1776); // way before various epoch boundaries
    this->_test1InstantRangeRoundTripConversion("Testing pre-Mac",          VDateAndTime(1900,  7,  4, 12,  0,  0, 0), TIME_APIS_SUPPORT_1900); // before Mac 1904 boundary
    this->_test1InstantRangeRoundTripConversion("Testing post-Mac",         VDateAndTime(1906,  7,  4, 12,  0,  0, 0), TIME_APIS_SUPPORT_1906); // after Mac 1904 boundary
    this->_test1InstantRangeRoundTripConversion("Testing pre-Unix",         VDateAndTime(1966,  7,  4, 12,  0,  0, 0), TIME_APIS_SUPPORT_1969); // well before 1970 Unix epoch, so still before Windows API support
    this->_test1InstantRangeRoundTripConversion("Testing 2s pre-Unix",      VDateAndTime(1969, 12, 31, 23, 59, 58, 0), TIME_APIS_SUPPORT_1969); // 2 second before 1970 Unix epoch, so still before Windows API support
    this->_test1InstantRangeRoundTripConversion("Testing 1s pre-Unix",      VDateAndTime(1969, 12, 31, 23, 59, 59, 0), false); // 1 second before 1970 Unix epoch is indistinguishible from -1 return code from OS time APIs
    this->_test1InstantRangeRoundTripConversion("Testing @ Unix",           VDateAndTime(1970,  1,  1,  0,  0,  0, 0), true); // start of 1970 Unix epoch, first Windows API support (give or take local time zone)
    this->_test1InstantRangeRoundTripConversion("Testing 1ms into Unix",    VDateAndTime(1970,  1,  1,  0,  0,  0, 1), true); // 1ms after tart of 1970 Unix epoch
    this->_test1InstantRangeRoundTripConversion("Testing well-supported",   VDateAndTime(2001,  1,  1,  0,  0,  0, 0), true); // well into fully supported date range
    this->_test1InstantRangeRoundTripConversion("Testing after Unix",       VDateAndTime(2039,  1,  1,  0,  0,  0, 0), TIME_APIS_SUPPORT_2039); // 32-bit Unix offsets end in 2038
    this->_test1InstantRangeRoundTripConversion("Testing after Win64",      VDateAndTime(3001,  1,  1,  0,  0,  0, 0), TIME_APIS_SUPPORT_3001); // 64-bit Windows support ends in 3000
    this->_test1InstantRangeRoundTripConversion("Testing far future",       VDateAndTime(5001,  1,  1,  0,  0,  0, 0), TIME_APIS_SUPPORT_5001); // 64-bit milliseconds go way beyond this
    
    // We know exactly what the correct value for July 14 2004 noon UTC is:
    VUNIT_ASSERT_TRUE_LABELED(july_14_2004_noon_utc.getValue() == CONST_S64(1089806400000), "utc epoch known offset");
    // (The value for July 14 2004 local time depends on our local time zone.)

    // Those two times must not have the same underlying "value", because
    // they were specified in different time zones -- (We'll assume that
    // this machine is not running with local time zone = UTC. This test
    // will appear to fail if UTC is local, if things are OK. But in that
    // case, the other zone conversion tests are not really being exercised
    // anyway, so testing in UTC is perhaps a bogus test environment anyway.)
    VUNIT_ASSERT_TRUE_LABELED(july_14_2004_noon_local != july_14_2004_noon_utc, "local != gm time");

    // Reverse each VInstant back into VDate and VTimeOfDay, and verify.

    VDate        dateLocalFromLocal;
    VTimeOfDay    timeLocalFromLocal;
    july_14_2004_noon_local.getValues(dateLocalFromLocal, timeLocalFromLocal, VInstant::LOCAL_TIME_ZONE_ID());
    VUNIT_ASSERT_TRUE_LABELED((dateLocalFromLocal == july_14_2004) && (timeLocalFromLocal == noon), "local conversion cycle");

    VDate        dateUTCFromUTC;
    VTimeOfDay    timeUTCFromUTC;
    july_14_2004_noon_utc.getValues(dateUTCFromUTC, timeUTCFromUTC, VInstant::UTC_TIME_ZONE_ID());
    VUNIT_ASSERT_TRUE_LABELED((dateUTCFromUTC == july_14_2004) && (timeUTCFromUTC == noon), "utc conversion cycle 1");

    dateUTCFromUTC = july_14_2004_noon_utc.getDate(VInstant::UTC_TIME_ZONE_ID());
    timeUTCFromUTC = july_14_2004_noon_utc.getTimeOfDay(VInstant::UTC_TIME_ZONE_ID());
    VUNIT_ASSERT_TRUE_LABELED((dateUTCFromUTC == july_14_2004) && (timeUTCFromUTC == noon), "utc conversion cycle 2");

    VUNIT_ASSERT_TRUE_LABELED((dateUTCFromUTC.getYear() == 2004) &&
               (dateUTCFromUTC.getMonth() == 7) &&
               (dateUTCFromUTC.getDay() == 14) &&
               (dateUTCFromUTC.getDayOfWeek() == VDate::kWednesday), "date values");
    VUNIT_ASSERT_TRUE_LABELED((timeUTCFromUTC.getHour() == 12) &&
               (timeUTCFromUTC.getMinute() == 0) &&
               (timeUTCFromUTC.getSecond() == 0), "time of day values");

}

void VInstantUnit::_runDurationValueTests() {
    // VDuration tests.

    VUNIT_ASSERT_TRUE_LABELED(VDuration::ZERO().getDurationMilliseconds() == CONST_S64(0), "VDuration ZERO");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::SECOND().getDurationMilliseconds() == CONST_S64(1000), "VDuration SECOND");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::MINUTE().getDurationMilliseconds() == CONST_S64(60000), "VDuration MINUTE");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::HOUR().getDurationMilliseconds() == CONST_S64(3600000), "VDuration HOUR");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::DAY().getDurationMilliseconds() == CONST_S64(86400000), "VDuration DAY");

    VString zeroDurationString = VDuration::ZERO().getDurationString();
    VUNIT_ASSERT_EQUAL_LABELED(VDuration::ZERO().getDurationString(), "0ms", "VDuration zero string");
    VUNIT_ASSERT_EQUAL_LABELED(VDuration::ZERO().getDurationStringFractionalSeconds(), "0.000", "VDuration zero fractional string");
    VUNIT_ASSERT_EQUAL_LABELED(VDuration::UNSPECIFIED().getDurationString(), "UNSPECIFIED", "VDuration UNSPECIFIED string");
    VUNIT_ASSERT_EQUAL_LABELED(VDuration::NEGATIVE_INFINITY().getDurationString(), "-INFINITY", "VDuration NEGATIVE_INFINITY string");
    VUNIT_ASSERT_EQUAL_LABELED(VDuration::POSITIVE_INFINITY().getDurationString(), "INFINITY", "VDuration POSITIVE_INFINITY string");

    VDuration durationStringTest;
    durationStringTest = CONST_S64(987) * VDuration::MILLISECOND();
    VUNIT_ASSERT_EQUAL_LABELED(durationStringTest.getDurationString(), "987ms", "VDuration 0.987 string");
    VUNIT_ASSERT_EQUAL_LABELED(durationStringTest.getDurationStringFractionalSeconds(), "0.987", "VDuration 0.987 fractional string");
    durationStringTest = CONST_S64(1001) * VDuration::MILLISECOND();
    VUNIT_ASSERT_EQUAL_LABELED(durationStringTest.getDurationString(), "1001ms", "VDuration 1.001 string");
    VUNIT_ASSERT_EQUAL_LABELED(durationStringTest.getDurationStringFractionalSeconds(), "1.001", "VDuration 1.001 fractional string");
    durationStringTest = VDuration::MINUTE();
    VUNIT_ASSERT_EQUAL_LABELED(durationStringTest.getDurationString(), "1m", "VDuration MINUTE string");
    VUNIT_ASSERT_EQUAL_LABELED(durationStringTest.getDurationStringFractionalSeconds(), "60.000", "VDuration MINUTE fractional string");
    durationStringTest = VDuration::HOUR();
    VUNIT_ASSERT_EQUAL_LABELED(durationStringTest.getDurationString(), "1h", "VDuration HOUR string");
    VUNIT_ASSERT_EQUAL_LABELED(durationStringTest.getDurationStringFractionalSeconds(), "3600.000", "VDuration HOUR0 fractional string");
    durationStringTest = VDuration::DAY();
    VUNIT_ASSERT_EQUAL_LABELED(durationStringTest.getDurationString(), "1d", "VDuration DAY string");
    VUNIT_ASSERT_EQUAL_LABELED(durationStringTest.getDurationStringFractionalSeconds(), "86400.000", "VDuration DAY fractional string");

    VDuration duration; // zero
    VUNIT_ASSERT_TRUE_LABELED(duration == VDuration::ZERO(), "VDuration default equals ZERO");
    duration += VDuration::SECOND();
    VUNIT_ASSERT_TRUE_LABELED(duration == VDuration::SECOND(), "VDuration ZERO plus SECOND equals SECOND");
    duration = 2 * VDuration::MINUTE();
    VUNIT_ASSERT_TRUE_LABELED(duration.getDurationMilliseconds() == 120000, "VDuration 2 * MINUTE equals 120000ms");
    duration = VDuration::MINUTE() * 2; // cover the transitive version of operator*
    VUNIT_ASSERT_TRUE_LABELED(duration.getDurationMilliseconds() == 120000, "VDuration MINUTE * 2 equals 120000ms");
    VUNIT_ASSERT_TRUE_LABELED(duration == (120 * VDuration::SECOND()), "VDuration 2 * MINUTE equals 120 * SECOND");
    duration = VDuration::DAY() - (10 * VDuration::HOUR());
    VUNIT_ASSERT_TRUE_LABELED(duration == (14 * VDuration::HOUR()), "VDuration DAY minus 10 * HOUR equals 14 * HOUR");
    duration = VDuration::DAY() - VDuration::MINUTE();
    VUNIT_ASSERT_TRUE_LABELED(duration == ((23 * VDuration::HOUR()) + (59 * VDuration::MINUTE())), "VDuration DAY minus MINUTE equals 23h59m");
    duration = VDuration::MINUTE();
    duration -= VDuration::SECOND();
    VUNIT_ASSERT_TRUE_LABELED(duration == (59 * VDuration::SECOND()), "VDuration operator-=");
    duration += VDuration::SECOND();
    VUNIT_ASSERT_TRUE_LABELED(duration == VDuration::MINUTE(), "VDuration operator+=");
    duration *= 60;
    VUNIT_ASSERT_TRUE_LABELED(duration == VDuration::HOUR(), "VDuration operator*=");
    duration /= 60;
    VUNIT_ASSERT_TRUE_LABELED(duration == VDuration::MINUTE(), "VDuration operator/= test 1");
    duration /= 2;
    VUNIT_ASSERT_TRUE_LABELED(duration == (30 * VDuration::SECOND()), "VDuration operator/= test 2");
    duration = VDuration::MINUTE() + VDuration::MINUTE();
    VUNIT_ASSERT_TRUE_LABELED(duration == (2 * VDuration::MINUTE()), "VDuration operator+"); // operator- already tested implicitly
    VUNIT_ASSERT_TRUE_LABELED(VDuration::MINUTE() > VDuration::SECOND(), "VDuration operator>");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::MINUTE() >= VDuration::SECOND(), "VDuration operator>=");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::MINUTE() < VDuration::HOUR(), "VDuration operator<");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::MINUTE() <= VDuration::HOUR(), "VDuration operator<=");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::MINUTE() == VDuration::MINUTE(), "VDuration operator==");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::MINUTE() != VDuration::DAY(), "VDuration operator!=");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::min(VDuration::MINUTE(), VDuration::HOUR()) == VDuration::MINUTE(), "VDuration min test 1");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::min(VDuration::HOUR(), VDuration::MINUTE()) == VDuration::MINUTE(), "VDuration min test 2");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::min(VDuration::MINUTE(), VDuration::MINUTE()) == VDuration::MINUTE(), "VDuration min test 3");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::max(VDuration::MINUTE(), VDuration::HOUR()) == VDuration::HOUR(), "VDuration max test 1");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::max(VDuration::HOUR(), VDuration::MINUTE()) == VDuration::HOUR(), "VDuration max test 2");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::max(VDuration::HOUR(), VDuration::HOUR()) == VDuration::HOUR(), "VDuration max test 3");
    VUNIT_ASSERT_TRUE_LABELED(-duration == duration * -1, "VDuration negation test 1");
    VUNIT_ASSERT_TRUE_LABELED(-duration != duration, "VDuration negation test 2");
    VUNIT_ASSERT_TRUE_LABELED(-duration == duration - duration - duration, "VDuration negation test 3");
    VUNIT_ASSERT_TRUE_LABELED(duration / 0 == VDuration::POSITIVE_INFINITY(), "VDuration positive divide by zero test");
    VUNIT_ASSERT_TRUE_LABELED((-duration) / 0 == VDuration::NEGATIVE_INFINITY(), "VDuration positive divide by zero test");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::abs(VDuration::MINUTE()) == VDuration::MINUTE(), "VDuration positive abs test");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::abs(VDuration::MINUTE() * -1) == VDuration::MINUTE(), "VDuration negative abs test");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::abs(VDuration::NEGATIVE_INFINITY()) == VDuration::POSITIVE_INFINITY(), "VDuration negative infinity abs test");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::abs(VDuration::POSITIVE_INFINITY()) == VDuration::POSITIVE_INFINITY(), "VDuration positive infinity abs test");
    VUNIT_ASSERT_TRUE_LABELED((- VDuration::MINUTE()) == VDuration::MINUTE() * -1, "VDuration unary minus test");
    VUNIT_ASSERT_TRUE_LABELED((- VDuration::MINUTE()) < VDuration::ZERO(), "VDuration unary minus less than zero test");
    VUNIT_ASSERT_TRUE_LABELED((- VDuration::NEGATIVE_INFINITY()) == VDuration::POSITIVE_INFINITY(), "VDuration unary minus of negative infinity test");
    VUNIT_ASSERT_TRUE_LABELED((- VDuration::POSITIVE_INFINITY()) == VDuration::NEGATIVE_INFINITY(), "VDuration unary minus of positive infinity test");

}

void VInstantUnit::_runExoticDurationValueTests() {

    // Additional tests for exotic instant and duration properties such as
    // math operations on +/- infinity.

    VDuration negativeDay = -1 * VDuration::DAY();
    VDuration positiveDay = VDuration::DAY();
    VDuration someDuration = VDuration::HOUR();

    VUNIT_ASSERT_TRUE_LABELED(VDuration::NEGATIVE_INFINITY() != VDuration::POSITIVE_INFINITY(), "VDuration::NEGATIVE_INFINITY() != VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::NEGATIVE_INFINITY() < VDuration::POSITIVE_INFINITY(), "VDuration::NEGATIVE_INFINITY() < VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::NEGATIVE_INFINITY() <= VDuration::POSITIVE_INFINITY(), "VDuration::NEGATIVE_INFINITY() <= VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::NEGATIVE_INFINITY() != VDuration::ZERO(), "VDuration::NEGATIVE_INFINITY() != VDuration::ZERO()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::NEGATIVE_INFINITY() < VDuration::ZERO(), "VDuration::NEGATIVE_INFINITY() < VDuration::ZERO()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::NEGATIVE_INFINITY() <= VDuration::ZERO(), "VDuration::NEGATIVE_INFINITY() <= VDuration::ZERO()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::ZERO() != VDuration::POSITIVE_INFINITY(), "VDuration::ZERO() != VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::ZERO() < VDuration::POSITIVE_INFINITY(), "VDuration::ZERO() < VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::ZERO() <= VDuration::POSITIVE_INFINITY(), "VDuration::ZERO() <= VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::NEGATIVE_INFINITY() < negativeDay, "VDuration::NEGATIVE_INFINITY() < negativeDay");
    VUNIT_ASSERT_TRUE_LABELED(negativeDay < VDuration::ZERO(), "negativeDay < VDuration::ZERO()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::ZERO() < positiveDay, "VDuration::ZERO() < positiveDay");
    VUNIT_ASSERT_TRUE_LABELED(positiveDay < VDuration::POSITIVE_INFINITY(), "positiveDay < VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::POSITIVE_INFINITY() - someDuration == VDuration::POSITIVE_INFINITY(), "VDuration::POSITIVE_INFINITY() - someDuration == VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::POSITIVE_INFINITY() + someDuration == VDuration::POSITIVE_INFINITY(), "VDuration::POSITIVE_INFINITY() + someDuration == VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::POSITIVE_INFINITY() * 5 == VDuration::POSITIVE_INFINITY(), "VDuration::POSITIVE_INFINITY() * 5 == VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::POSITIVE_INFINITY() * -5 == VDuration::NEGATIVE_INFINITY(), "VDuration::POSITIVE_INFINITY() * -5 == VDuration::NEGATIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::POSITIVE_INFINITY() * 0 == VDuration::ZERO(), "VDuration::POSITIVE_INFINITY() * 0 == VDuration::ZERO()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::POSITIVE_INFINITY() / 5 == VDuration::POSITIVE_INFINITY(), "VDuration::POSITIVE_INFINITY() / 5 == VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(negativeDay / 0 == VDuration::NEGATIVE_INFINITY(), "negativeDay / 0 == VDuration::NEGATIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(positiveDay / 0 == VDuration::POSITIVE_INFINITY(), "positiveDay / 0 == VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::POSITIVE_INFINITY() % (VDuration::MILLISECOND() * 5) == VDuration::POSITIVE_INFINITY(), "VDuration::POSITIVE_INFINITY() % 5ms == VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::NEGATIVE_INFINITY() - someDuration == VDuration::NEGATIVE_INFINITY(), "VDuration::NEGATIVE_INFINITY() - someDuration == VDuration::NEGATIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::NEGATIVE_INFINITY() + someDuration == VDuration::NEGATIVE_INFINITY(), "VDuration::NEGATIVE_INFINITY() + someDuration == VDuration::NEGATIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::NEGATIVE_INFINITY() * 5 == VDuration::NEGATIVE_INFINITY(), "VDuration::NEGATIVE_INFINITY() * 5 == VDuration::NEGATIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::NEGATIVE_INFINITY() * -5 == VDuration::POSITIVE_INFINITY(), "VDuration::NEGATIVE_INFINITY() * -5 == VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::NEGATIVE_INFINITY() * 0 == VDuration::ZERO(), "VDuration::NEGATIVE_INFINITY() * 0 == VDuration::ZERO()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::NEGATIVE_INFINITY() / 5 == VDuration::NEGATIVE_INFINITY(), "VDuration::NEGATIVE_INFINITY() / 5 == VDuration::NEGATIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::NEGATIVE_INFINITY() % (VDuration::MILLISECOND() * 5) == VDuration::NEGATIVE_INFINITY(), "VDuration::NEGATIVE_INFINITY() % 5ms == VDuration::NEGATIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::min(VDuration::NEGATIVE_INFINITY(), VDuration::NEGATIVE_INFINITY()) == VDuration::NEGATIVE_INFINITY(), "VDuration::min(VDuration::NEGATIVE_INFINITY(), VDuration::NEGATIVE_INFINITY()) == VDuration::NEGATIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::min(VDuration::NEGATIVE_INFINITY(), someDuration) == VDuration::NEGATIVE_INFINITY(), "VDuration::min(VDuration::NEGATIVE_INFINITY(), someDuration) == VDuration::NEGATIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::min(VDuration::NEGATIVE_INFINITY(), VDuration::POSITIVE_INFINITY()) == VDuration::NEGATIVE_INFINITY(), "VDuration::min(VDuration::NEGATIVE_INFINITY(), VDuration::POSITIVE_INFINITY()) == VDuration::NEGATIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::max(VDuration::NEGATIVE_INFINITY(), VDuration::NEGATIVE_INFINITY()) == VDuration::NEGATIVE_INFINITY(), "VDuration::max(VDuration::NEGATIVE_INFINITY(), VDuration::NEGATIVE_INFINITY()) == VDuration::NEGATIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::max(VDuration::NEGATIVE_INFINITY(), someDuration) == someDuration, "VDuration::max(VDuration::NEGATIVE_INFINITY(), someDuration) == someDuration");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::max(VDuration::NEGATIVE_INFINITY(), VDuration::POSITIVE_INFINITY()) == VDuration::POSITIVE_INFINITY(), "VDuration::max(VDuration::NEGATIVE_INFINITY(), VDuration::POSITIVE_INFINITY()) == VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::max(VDuration::POSITIVE_INFINITY(), VDuration::POSITIVE_INFINITY()) == VDuration::POSITIVE_INFINITY(), "VDuration::max(VDuration::POSITIVE_INFINITY(), VDuration::POSITIVE_INFINITY()) == VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::max(VDuration::POSITIVE_INFINITY(), someDuration) == VDuration::POSITIVE_INFINITY(), "VDuration::max(VDuration::POSITIVE_INFINITY(), someDuration) == VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::max(VDuration::POSITIVE_INFINITY(), VDuration::NEGATIVE_INFINITY()) == VDuration::POSITIVE_INFINITY(), "VDuration::max(VDuration::POSITIVE_INFINITY(), VDuration::NEGATIVE_INFINITY()) == VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::min(VDuration::POSITIVE_INFINITY(), VDuration::NEGATIVE_INFINITY()) == VDuration::NEGATIVE_INFINITY(), "VDuration::min(VDuration::POSITIVE_INFINITY(), VDuration::NEGATIVE_INFINITY()) == VDuration::NEGATIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::min(VDuration::POSITIVE_INFINITY(), someDuration) == someDuration, "VDuration::min(VDuration::POSITIVE_INFINITY(), someDuration) == someDuration");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::min(VDuration::POSITIVE_INFINITY(), VDuration::POSITIVE_INFINITY()) == VDuration::POSITIVE_INFINITY(), "VDuration::min(VDuration::POSITIVE_INFINITY(), VDuration::POSITIVE_INFINITY()) == VDuration::POSITIVE_INFINITY()");
    // Now we modify the exotics locally and test.
    VDuration negativeInfinity = VDuration::NEGATIVE_INFINITY();
    negativeInfinity += someDuration;
    VUNIT_ASSERT_TRUE_LABELED(negativeInfinity == VDuration::NEGATIVE_INFINITY(), "negativeInfinity += someDuration == VDuration::NEGATIVE_INFINITY()");
    VDuration positiveInfinity = VDuration::POSITIVE_INFINITY();
    positiveInfinity -= someDuration;
    VUNIT_ASSERT_TRUE_LABELED(positiveInfinity == VDuration::POSITIVE_INFINITY(), "positiveInfinity -= someDuration == VDuration::POSITIVE_INFINITY()");
    positiveInfinity /= 5;
    VUNIT_ASSERT_TRUE_LABELED(positiveInfinity == VDuration::POSITIVE_INFINITY(), "positiveInfinity /= 5 == VDuration::POSITIVE_INFINITY()");
    negativeInfinity /= 5;
    VUNIT_ASSERT_TRUE_LABELED(negativeInfinity == VDuration::NEGATIVE_INFINITY(), "negativeInfinity /= 5 == VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(-VDuration::POSITIVE_INFINITY() == VDuration::NEGATIVE_INFINITY(), "-VDuration::POSITIVE_INFINITY() == VDuration::NEGATIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(-VDuration::NEGATIVE_INFINITY() == VDuration::POSITIVE_INFINITY(), "-VDuration::NEGATIVE_INFINITY() == VDuration::POSITIVE_INFINITY()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::NEGATIVE_INFINITY() + VDuration::POSITIVE_INFINITY() == VDuration::ZERO(), "VDuration::NEGATIVE_INFINITY() + VDuration::POSITIVE_INFINITY() == VDuration::ZERO()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::POSITIVE_INFINITY() - VDuration::POSITIVE_INFINITY() == VDuration::ZERO(), "VDuration::POSITIVE_INFINITY() - VDuration::POSITIVE_INFINITY() == VDuration::ZERO()");
    VUNIT_ASSERT_TRUE_LABELED(VDuration::NEGATIVE_INFINITY() - VDuration::NEGATIVE_INFINITY() == VDuration::ZERO(), "VDuration::NEGATIVE_INFINITY() - VDuration::NEGATIVE_INFINITY() == VDuration::ZERO()");

    VInstant currentTime;
    VInstant currentMinus1d = currentTime - VDuration::DAY();
    VInstant currentMinus24h = currentTime - (24 * VDuration::HOUR());
    VInstant currentPlus1d = currentTime + VDuration::DAY();
    VInstant currentPlus24h = currentTime + (24 * VDuration::HOUR());
    VInstant infinitePast = VInstant::INFINITE_PAST();
    VInstant infiniteFuture = VInstant::INFINITE_FUTURE();
    VInstant never = VInstant::NEVER_OCCURRED();

    VUNIT_ASSERT_TRUE_LABELED(infinitePast < currentTime, "infinitePast < currentTime");
    VUNIT_ASSERT_TRUE_LABELED(infinitePast <= currentTime, "infinitePast <= currentTime");
    VUNIT_ASSERT_FALSE_LABELED(infinitePast >= currentTime, "! (infinitePast >= currentTime)");
    VUNIT_ASSERT_FALSE_LABELED(infinitePast > currentTime, "! (infinitePast > currentTime)");

}

void VInstantUnit::_runDurationStringTests() {

    VDuration stringTestDuration;

    stringTestDuration.setDurationString("42ms");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), CONST_S64(42), "setDurationString ms suffix");
    stringTestDuration.setDurationString("2742ms");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), CONST_S64(2742), "setDurationString ms suffix gt 1s");
    stringTestDuration.setDurationString("-87ms");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), CONST_S64(-87), "setDurationString ms suffix negative value");

    stringTestDuration.setDurationString("19s");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), CONST_S64(19000), "setDurationString s suffix");
    stringTestDuration.setDurationString("194s");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), CONST_S64(194000), "setDurationString s suffix gt 1m");
    stringTestDuration.setDurationString("-130s");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), CONST_S64(-130000), "setDurationString s suffix negative value");

    stringTestDuration.setDurationString("5m");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), CONST_S64(300000), "setDurationString m suffix");
    stringTestDuration.setDurationString("78m");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), CONST_S64(4680000), "setDurationString m suffix gt 1h");
    stringTestDuration.setDurationString("-12m");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), CONST_S64(-720000), "setDurationString m suffix negative value");

    stringTestDuration.setDurationString("2h");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), CONST_S64(7200000), "setDurationString h suffix");
    stringTestDuration.setDurationString("48h");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), CONST_S64(172800000), "setDurationString h suffix gt 1d");
    stringTestDuration.setDurationString("-6h");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), CONST_S64(-21600000), "setDurationString h suffix negative value");

    stringTestDuration.setDurationString("0.123");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), CONST_S64(123), "setDurationString no suffix");
    stringTestDuration.setDurationString("5.678");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), CONST_S64(5678), "setDurationString no suffix gt 1s");
    stringTestDuration.setDurationString("-2.723");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), CONST_S64(-2723), "setDurationString no suffix negative value");

    stringTestDuration.setDurationString("unspecified");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), VDuration::UNSPECIFIED().getDurationMilliseconds(), "setDurationString unspecified");
    stringTestDuration.setDurationString("UNSPECIFIED");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), VDuration::UNSPECIFIED().getDurationMilliseconds(), "setDurationString UNSPECIFIED");

    stringTestDuration.setDurationString("-infinity");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), VDuration::NEGATIVE_INFINITY().getDurationMilliseconds(), "setDurationString -infinity");
    stringTestDuration.setDurationString("-INFINITY");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), VDuration::NEGATIVE_INFINITY().getDurationMilliseconds(), "setDurationString -INFINITY");

    stringTestDuration.setDurationString("infinity");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), VDuration::POSITIVE_INFINITY().getDurationMilliseconds(), "setDurationString infinity");
    stringTestDuration.setDurationString("INFINITY");
    VUNIT_ASSERT_EQUAL_LABELED(stringTestDuration.getDurationMilliseconds(), VDuration::POSITIVE_INFINITY().getDurationMilliseconds(), "setDurationString INFINITY");
    
}

void VInstantUnit::_runInstantFormatterTests() {

    /* scope for formatter test */ {
        VInstant now;
        VInstantFormatter formatter;
        VString s;
        
        s = formatter.formatLocalString(now);
        this->logStatus(VSTRING_FORMAT("VInstant old API local string output for local time (offset=%lld) is '%s'", now.getValue(), now.getLocalString().chars()));
        this->logStatus(VSTRING_FORMAT("VInstantFormatter default output for local time is '%s'", s.chars()));
    }
    
    /* scope for UTC and local string test */ {
        // Verify the VInstant built-in string getters directly. We are examining the formatting variants.
        VString s;

        VInstant whenUTC;
        whenUTC.setValues(VDate(1998, 6, 3), VTimeOfDay(15, 56, 37, 444), VInstant::UTC_TIME_ZONE_ID());
        VUNIT_ASSERT_EQUAL_LABELED(whenUTC.getUTCString(), "1998-06-03 15:56:37.444 UTC", "getUTCString()");
        whenUTC.getUTCString(s);
        VUNIT_ASSERT_EQUAL_LABELED(s, "1998-06-03 15:56:37.444 UTC", "getUTCString(s)");
        whenUTC.getUTCString(s, false);
        VUNIT_ASSERT_EQUAL_LABELED(s, "1998-06-03 15:56:37.444 UTC", "getUTCString(s, false)");
        whenUTC.getUTCString(s, true);
        VUNIT_ASSERT_EQUAL_LABELED(s, "19980603155637444", "getUTCString(s, true)");
        whenUTC.getUTCString(s, false, true);
        VUNIT_ASSERT_EQUAL_LABELED(s, "1998-06-03 15:56:37.444 UTC", "getUTCString(s, false, true)");
        whenUTC.getUTCString(s, true, true);
        VUNIT_ASSERT_EQUAL_LABELED(s, "19980603155637444", "getUTCString(s, true, true)");
        whenUTC.getUTCString(s, false, false);
        VUNIT_ASSERT_EQUAL_LABELED(s, "1998-06-03 15:56:37 UTC", "getUTCString(s, false, false)");
        whenUTC.getUTCString(s, true, false);
        VUNIT_ASSERT_EQUAL_LABELED(s, "19980603155637", "getUTCString(s, true, false)");
    
        VInstant whenLocal;
        whenLocal.setValues(VDate(1998, 6, 3), VTimeOfDay(15, 56, 37, 444), VInstant::LOCAL_TIME_ZONE_ID());
        VUNIT_ASSERT_EQUAL_LABELED(whenLocal.getLocalString(), "1998-06-03 15:56:37.444", "getLocalString()");
        whenLocal.getLocalString(s);
        VUNIT_ASSERT_EQUAL_LABELED(s, "1998-06-03 15:56:37.444", "getLocalString(s)");
        whenLocal.getLocalString(s, false);
        VUNIT_ASSERT_EQUAL_LABELED(s, "1998-06-03 15:56:37.444", "getLocalString(s, false)");
        whenLocal.getLocalString(s, true);
        VUNIT_ASSERT_EQUAL_LABELED(s, "19980603155637444", "getLocalString(s, true)");
        whenLocal.getLocalString(s, false, true);
        VUNIT_ASSERT_EQUAL_LABELED(s, "1998-06-03 15:56:37.444", "getLocalString(s, false, true)");
        whenLocal.getLocalString(s, true, true);
        VUNIT_ASSERT_EQUAL_LABELED(s, "19980603155637444", "getLocalString(s, true, true)");
        whenLocal.getLocalString(s, false, false);
        VUNIT_ASSERT_EQUAL_LABELED(s, "1998-06-03 15:56:37", "getLocalString(s, false, false)");
        whenLocal.getLocalString(s, true, false);
        VUNIT_ASSERT_EQUAL_LABELED(s, "19980603155637", "getLocalString(s, true, false)");
    
    }
    
    // Now let's test some specific formatting directives.
    VInstant    when;
    when.setValues(VDate(1998, 6, 3), VTimeOfDay(15, 56, 37, 444), VInstant::LOCAL_TIME_ZONE_ID());
    
    // Note: some of these tests only work when run on a machine set to Pacific time, because TZ conversions are being
    // performed and then tested against. So we may need to conditionalize running these tests.
    
    this->_testInstantFormatter("E (short day of week name)", when,
        "E, y-MMM-dd HH:mm:ss.SSS",
        "Wed, 1998-Jun-03 22:56:37.444",
        "Wed, 1998-Jun-03 15:56:37.444");
    
    this->_testInstantFormatter("G (era)", when,
        "E, y-MMM-dd HH:mm:ss.SSS G",
        "Wed, 1998-Jun-03 22:56:37.444 AD",
        "Wed, 1998-Jun-03 15:56:37.444 AD");
    
    this->_testInstantFormatter("EEEE (long day of week name) / yy-d (2-digit year, 1-digit day)", when,
        "EEEE, yy-MMMM-d HH:mm:ss.SSS",
        "Wednesday, 98-June-3 22:56:37.444",
        "Wednesday, 98-June-3 15:56:37.444");
    
    this->_testInstantFormatter("z (general time zone)", when,
        "y-MM-dd HH:mm:ss.SSS z",
        "1998-06-03 22:56:37.444 GMT+00:00",
        "1998-06-03 15:56:37.444 GMT-07:00");
    
    this->_testInstantFormatter("u (day of week number)", when,
        "u, y-MMM-dd HH:mm:ss.SSS",
        "3, 1998-Jun-03 22:56:37.444",
        "3, 1998-Jun-03 15:56:37.444");
    
    this->_testInstantFormatter("Z (RFC 822 time zone)", when,
        "y-MM-dd HH:mm:ss.SSS Z",
        "1998-06-03 22:56:37.444 +0000",
        "1998-06-03 15:56:37.444 -0700");
    
    this->_testInstantFormatter("XXX (ISO 8601 3-letter)", when,
        "y-MM-dd HH:mm:ss.SSS XXX",
        "1998-06-03 22:56:37.444 Z",
        "1998-06-03 15:56:37.444 -07:00Z");
    
    this->_testInstantFormatter("XX (ISO 8601 2-letter)", when,
        "y-MM-dd HH:mm:ss.SSS XX",
        "1998-06-03 22:56:37.444 Z",
        "1998-06-03 15:56:37.444 -0700Z");
    
    this->_testInstantFormatter("X (ISO 8601 1-letter)", when,
        "y-MM-dd HH:mm:ss.SSS X",
        "1998-06-03 22:56:37.444 Z",
        "1998-06-03 15:56:37.444 -07Z");
    
    this->_testInstantFormatter("simplest form", when,
        "y-MM-dd HH:mm:ss",
        "1998-06-03 22:56:37",
        "1998-06-03 15:56:37");

    this->_testInstantFormatter("KK + a (12-hour with AM/PM suffix)", when,
        "y-MM-dd KK:mm:ss a",
        "1998-06-03 10:56:37 PM",
        "1998-06-03 03:56:37 PM");

    this->_testInstantFormatter("kitchen sink", when,
        "G|GG|y|yy|yyy|yyyy|Y|YY|YYY|YYYY|M|MM|MMM|MMMM|d|dd|E|EE|EEE|EEEE|u|uu|a|H|HH|k|kk|K|KK|h|hh|m|mm|s|ss|S|SS|SSS|z|Z|X|XX|XXX",
        "AD|AD|1998|98|1998|1998|1998|98|1998|1998|6|06|Jun|June|3|03|Wed|Wed|Wed|Wednesday|3|03|PM|22|22|23|23|10|10|10|10|56|56|37|37|444|444|444|GMT+00:00|+0000|Z|Z|Z",
        "AD|AD|1998|98|1998|1998|1998|98|1998|1998|6|06|Jun|June|3|03|Wed|Wed|Wed|Wednesday|3|03|PM|15|15|16|16|3|03|3|03|56|56|37|37|444|444|444|GMT-07:00|-0700|-07Z|-0700Z|-07:00Z");

    // Do tests on UTC values. Good because they don't depend on the TZ where we are running!
    when.setValues(VDate(1998, 6, 3), VTimeOfDay(15, 56, 37, 444), VInstant::UTC_TIME_ZONE_ID());
    this->_testInstantFormatter("kitchen sink UTC", when,
        "G|GG|y|yy|yyy|yyyy|Y|YY|YYY|YYYY|M|MM|MMM|MMMM|d|dd|E|EE|EEE|EEEE|u|uu|a|H|HH|k|kk|K|KK|h|hh|m|mm|s|ss|S|SS|SSS|z|Z|X|XX|XXX",
        "AD|AD|1998|98|1998|1998|1998|98|1998|1998|6|06|Jun|June|3|03|Wed|Wed|Wed|Wednesday|3|03|PM|15|15|16|16|3|03|3|03|56|56|37|37|444|444|444|GMT+00:00|+0000|Z|Z|Z",
        "AD|AD|1998|98|1998|1998|1998|98|1998|1998|6|06|Jun|June|3|03|Wed|Wed|Wed|Wednesday|3|03|AM|8|08|9|09|8|08|8|08|56|56|37|37|444|444|444|GMT-07:00|-0700|-07Z|-0700Z|-07:00Z");
    
    // Do the examples in the Javadoc, but with UTC for TZ independence of unit tests.
    // Note that I have left the commented out PDT version for comparison; if we pass true rather than false, and run in PDT and support TZ names, those would be the ones to use.
    when.setValues(VDate(2001, 7, 4), VTimeOfDay(12, 8, 56, 235), VInstant::UTC_TIME_ZONE_ID());

    this->_testInstantFormatter("SDF ex 1", when,
        "yyyy.MM.dd G 'at' HH:mm:ss z",
        "2001.07.04 AD at 12:08:56 GMT+00:00",
        //"2001.07.04 AD at 12:08:56 PDT", <-- localized time zone strings not yet supported, so we use offset form as above
        "2001.07.04 AD at 05:08:56 GMT-07:00");

    this->_testInstantFormatter("SDF ex 2", when,
        "EEE, MMM d, ''yy",
        "Wed, Jul 4, '01",
        "Wed, Jul 4, '01");

    this->_testInstantFormatter("SDF ex 3", when,
        "h:mm a",
        "12:08 PM",
        "5:08 AM");

    this->_testInstantFormatter("SDF ex 4", when,
        "hh 'o''clock' a, zzzz",
        "12 o'clock PM, GMT+00:00",
        //"12 o'clock PM, Pacific Daylight Time", <-- localized time zone strings not yet supported, so we use offset form as above
        "05 o'clock AM, GMT-07:00");

    this->_testInstantFormatter("SDF ex 5", when,
        "K:mm a, z",
        "0:08 PM, GMT+00:00",
        //"0:08 PM, PDT", <-- localized time zone strings not yet supported, so we use offset form as above
        "5:08 AM, GMT-07:00");

    this->_testInstantFormatter("SDF ex 6", when,
        "yyyyy.MMMMM.dd GGG hh:mm aaa",
        "02001.July.04 AD 12:08 PM",
        "02001.July.04 AD 05:08 AM");

    this->_testInstantFormatter("SDF ex 7", when,
        "EEE, d MMM yyyy HH:mm:ss Z",
        "Wed, 4 Jul 2001 12:08:56 +0000",
        "Wed, 4 Jul 2001 05:08:56 -0700");

    this->_testInstantFormatter("SDF ex 8", when,
        "yyMMddHHmmssZ",
        "010704120856+0000",
        "010704050856-0700");

    this->_testInstantFormatter("SDF ex 9", when,
        "yyyy-MM-dd'T'HH:mm:ss.SSSZ",
        "2001-07-04T12:08:56.235+0000",
        "2001-07-04T05:08:56.235-0700");

    this->_testInstantFormatter("SDF ex 10", when,
        "yyyy-MM-dd'T'HH:mm:ss.SSSXXX",
        "2001-07-04T12:08:56.235Z",
        "2001-07-04T05:08:56.235-07:00Z");

    this->_testInstantFormatter("SDF ex 11", when,
        "YYYY-'W'ww-u",
        "2001-W-3",
        //"2001-W27-3", <-- we don't yet support W and w at all; they currently emit blanks
        "2001-W-3");
}

void VInstantUnit::_testInstantFormatter(const VString& label, const VInstant& instant, const VString& format, const VString& expectedUTCOutput, const VString& expectedLocalOutput) {
    VInstantFormatter formatter(format);
    
    bool performUTCTests = true;
    bool performLocalTests = true;
    
    /* UTC TZ check */ if (performUTCTests) {
        VString s = formatter.formatUTCString(instant);
        VString assertLabel(VSTRING_ARGS("Exercising '%s' (UTC) '%s' -> '%s'", label.chars(), format.chars(), s.chars()));
        VUNIT_ASSERT_EQUAL_LABELED(s, expectedUTCOutput, assertLabel);
    }

    /* local TZ check */ if (performLocalTests) {
        VString s = formatter.formatLocalString(instant);
        VString assertLabel(VSTRING_ARGS("Exercising '%s' (local) '%s' -> '%s'", label.chars(), format.chars(), s.chars()));
        VUNIT_ASSERT_EQUAL_LABELED(s, expectedLocalOutput, assertLabel);
    }
}

void VInstantUnit::_testInstantRangeRoundTripConversion(const VString& label, const VInstant& startInstant, const VDuration& increment, int numIncrements) {
    
    for (int i = 0; i < numIncrements; ++i) {
        VInstant when = startInstant + (i * increment);
        Vs64 whenOffset = when.getValue();

        VInstantStruct whenUTCStruct;
        whenUTCStruct.setUTCStructFromOffset(whenOffset);
        Vs64 whenUTCCheckOffset = whenUTCStruct.getOffsetFromUTCStruct();
        VUNIT_ASSERT_EQUAL_LABELED(whenOffset, whenUTCCheckOffset, VSTRING_FORMAT("%s: %d utc   '%s'", label.chars(), i, when.getUTCString().chars()));
        
        VInstantStruct whenLocalStruct;
        whenLocalStruct.setLocalStructFromOffset(whenOffset);
        Vs64 whenLocalCheckOffset = whenLocalStruct.getOffsetFromLocalStruct();

        // Calculate the local vs. UTC hour delta for examination if there is a problem. We expect that during a daylight transition,
        // the delta will change during our iteration. This simple display calculation will not work across a month boundary but will
        // be useful when debugging across a daylight boundary.
        VString deltaDisplay;
        if (whenUTCStruct.mMonth == whenLocalStruct.mMonth) {
            int utcDaysHoursMinutes = (whenUTCStruct.mDay * 24 * 60) + (whenUTCStruct.mHour * 60) + whenUTCStruct.mMinute;
            int localDaysHoursMinutes = (whenLocalStruct.mDay * 24 * 60) + (whenLocalStruct.mHour * 60) + whenLocalStruct.mMinute;
            int deltaHours = (localDaysHoursMinutes - utcDaysHoursMinutes) / 60;
            int deltaMinutes = V_ABS((localDaysHoursMinutes - utcDaysHoursMinutes) % 60);
            deltaDisplay.format("%02d%02d", deltaHours, deltaMinutes);
        }

        VUNIT_ASSERT_EQUAL_LABELED(whenOffset, whenLocalCheckOffset, VSTRING_FORMAT("%s: %d local '%s' %s", label.chars(), i, when.getLocalString().chars(), deltaDisplay.chars()));
    }
}

#define INSTANT_STRUCT_FORMAT "%d-%02d-%02d %02d:%02d:%02d.%03d"

void VInstantUnit::_test1InstantRangeRoundTripConversion(const VString& label, const VDateAndTime& when, bool expectSuccess) {

    try {
        VInstant i;
        i.setDateAndTime(when, VInstant::UTC_TIME_ZONE_ID());
        this->_testInstantRangeRoundTripConversion(label, i, VDuration(), 1);
        VUNIT_ASSERT_TRUE_LABELED(expectSuccess, label);
    } catch (...) {
        if (!expectSuccess) {
            this->logStatus(VSTRING_FORMAT("As expected, time value '" INSTANT_STRUCT_FORMAT "' is outside of supported range on this platform.",
                when.getYear(), when.getMonth(), when.getDay(), when.getHour(), when.getMinute(), when.getSecond(), when.getMillisecond()));
        } else {
            VUNIT_ASSERT_TRUE_LABELED(!expectSuccess, label);
        }
    }
}
