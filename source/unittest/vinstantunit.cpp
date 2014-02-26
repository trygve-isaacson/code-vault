/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
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
    this->test(offset1 == offset2, "symmetry test 1");
    this->test(i1 == i2, "symmetry test 2");

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
    this->test(offset1 == offset2, "modification test 1");
    i2 += kDeltaA;                                      // i2 is base + kDeltaA
    this->test(i1 == i2, "modification test 2");

    // operator-=
    const VDuration kDeltaC = VDuration::MILLISECOND() * CONST_S64(13579);
//    const Vs64 kDeltaABC = kDeltaAB - kDeltaC;            // Might be useful in debugging.
    i1 -= kDeltaC;                                      // i1 is base + kDeltaABC
    offset2 -= kDeltaC.getDurationMilliseconds();        // off2 is base + kDeltaABC
    offset1 = i1.getValue();                            // off1 is base + kDeltaABC
    this->test(offset1 == offset2, "modification test 5");
    i2 -= VDuration(kDeltaC);                            // i2 is base + kDeltaABC
    this->test(i1 == i2, "modification test 6");

    // Test comparison operators.
    // Set things up for this set of tests.
    offset1 = base;
    offset2 = base;
    i1.setValue(base);
    i2.setValue(base);

    const VDuration kDeltaD = VDuration::MILLISECOND() * CONST_S64(24680);
    i2 += kDeltaD;
    // Now i2 is kDeltaD milliseconds later than i1.
    this->test(!(i1 == i2), "comparison test 1a");
    this->test(i1 != i2, "comparison test 1b");
    this->test(i1 < i2, "comparison test 1c");
    this->test(i1 <= i2, "comparison test 1d");
    this->test(i2 > i1, "comparison test 1e");
    this->test(i2 >= i1, "comparison test 1f");
    this->test(i2 - i1 == kDeltaD, "comparison test 1g");
    i2 -= kDeltaD;
    // Now i1 and i2 are equal.
    this->test(i1 == i2, "comparison test 2a");
    this->test(!(i1 != i2), "comparison test 2b");
    this->test(!(i1 < i2), "comparison test 2c");
    this->test(i1 <= i2, "comparison test 2d");
    this->test(!(i2 > i1), "comparison test 2e");
    this->test(i2 >= i1, "comparison test 2f");
    this->test(i2 - i1 == VDuration::ZERO(), "comparison test 2g");

}

void VInstantUnit::_runInstantComparatorTests() {

    // Test comparison operators with "infinite" values.
    VInstant    now;
    VInstant    infinitePast = VInstant::INFINITE_PAST();
    VInstant    infiniteFuture = VInstant::INFINITE_FUTURE();
    VInstant    past = now; past -= VDuration::MINUTE(); // about a minute before "now"
    VInstant    future = now; future += VDuration::MINUTE(); // about a minute later than "now"

    this->test(infinitePast < now, "comparison test 3a");
    this->test(infinitePast <= now, "comparison test 3b");
    this->test(now > infinitePast, "comparison test 3c");
    this->test(now >= infinitePast, "comparison test 3d");
    this->test(!(infinitePast > now), "comparison test 3e");
    this->test(!(infinitePast >= now), "comparison test 3f");
    this->test(infinitePast != now, "comparison test 3g");
    this->test(!(infinitePast == now), "comparison test 3h");

    this->test(infiniteFuture > now, "comparison test 4a");
    this->test(infiniteFuture >= now, "comparison test 4b");
    this->test(now < infiniteFuture, "comparison test 4c");
    this->test(now <= infiniteFuture, "comparison test 4d");
    this->test(!(infiniteFuture < now), "comparison test 4e");
    this->test(!(infiniteFuture <= now), "comparison test 4f");
    this->test(infiniteFuture != now, "comparison test 4g");
    this->test(!(infiniteFuture == now), "comparison test 4h");

    this->test(infinitePast < infiniteFuture, "comparison test 5a");
    this->test(infinitePast <= infiniteFuture, "comparison test 5b");
    this->test(infiniteFuture > infinitePast, "comparison test 5c");
    this->test(infiniteFuture >= infinitePast, "comparison test 5d");
    this->test(!(infinitePast > infiniteFuture), "comparison test 5e");
    this->test(!(infinitePast >= infiniteFuture), "comparison test 5f");
    this->test(infinitePast != infiniteFuture, "comparison test 5g");
    this->test(!(infinitePast == infiniteFuture), "comparison test 5h");

    this->test(infinitePast == VInstant::INFINITE_PAST(), "comparison test 6a");
    this->test(infiniteFuture == VInstant::INFINITE_FUTURE(), "comparison test 6b");
    this->test(VInstant::min(infinitePast, now) == infinitePast, "comparison test 6c");
    this->test(VInstant::max(infinitePast, now) == now, "comparison test 6d");
    this->test(VInstant::min(infiniteFuture, now) == now, "comparison test 6e");
    this->test(VInstant::max(infiniteFuture, now) == infiniteFuture, "comparison test 6f");
    this->test(VInstant::min(past, now) == past, "comparison test 6g");
    this->test(VInstant::max(past, now) == now, "comparison test 6h");
    this->test(VInstant::min(future, now) == now, "comparison test 6i");
    this->test(VInstant::max(future, now) == future, "comparison test 6j");
    this->test(VInstant::min(past, future) == past, "comparison test 6k");
    this->test(VInstant::max(past, future) == future, "comparison test 6l");

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
        this->test(fakeFutureNow > basePlus1Minute, "advance simulated clock offset");
        VInstant::setSimulatedClockOffset(VDuration::ZERO()); // restore the time continuum to normal
        VInstant normalNow;
        this->test(normalNow >= base0, "restore simulated clock offset part 1");
        this->test(normalNow < basePlus1Minute, "restore simulated clock offset part 2"); // can only fail if it takes > 1 real minute to execute the last 5 lines of code
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
        this->test(fakePastNow - fakePastInstant < VDuration::SECOND(), "set clock to past instant");
        // note that we do NOT zero the offset before the next test; we want to verify it can be set directly
        VDateAndTime fakeFutureDT(2034, 1, 6, 14, 35, 0, 0);
        VInstant fakeFutureInstant; fakeFutureInstant.setLocalDateAndTime(fakeFutureDT);
        VInstant::setSimulatedClockValue(fakeFutureInstant);
        VInstant fakeFutureNow;
        this->test(fakeFutureNow - fakeFutureInstant < VDuration::SECOND(), "set clock to future instant");

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
        this->test(frozenNow1 == fakePastInstant, "freeze time 1");

        Vs64 frozenSnapshot = VInstant::snapshot();

        // Shift frozen time forward by 10 seconds and validate.
        VDuration shiftAmount = 10 * VDuration::SECOND();
        VInstant::shiftFrozenTime(shiftAmount);
        VInstant frozenNow2;
        this->test(frozenNow2 == frozenNow1 + shiftAmount, "shift frozen time");

        VDuration frozenSnapshotDelta = VInstant::snapshotDelta(frozenSnapshot);
        this->test(frozenSnapshotDelta == shiftAmount, "shift frozen time snapshot");

        this->test(VInstant::isTimeFrozen(), "time is frozen");

        // Sleep for 2 seconds and verify that no time seemed to actually pass.
        VThread::sleep(2 * VDuration::SECOND());
        VInstant frozenNow3;
        this->test(frozenNow3 == frozenNow2, "freeze time 2");

        // Unfreeze time and make sure it now rolls forward in true real time.
        // First we verify that the current time is equal to or later than the
        // real time when we started this test block.
        // Then we sleep a little bit and verify that a later time is reported.
        // We need to sleep long enough to exceed the time resolution on all
        // platforms. On Windows this can be > 100ms. Otherwise, it may look like
        // time did not roll forward while we slept.
        VInstant::unfreezeTime();
        VInstant realNow1;
        this->test(realNow1 >= realNow, "normal time resumed");
        VThread::sleep(200 * VDuration::MILLISECOND());
        VInstant realNow2;
        this->test(realNow2 > realNow1, "unfrozen time proceeds");

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
    this->test(utc0Instant.getValue() == CONST_S64(0), "utc epoch base");

    // A little debugging code here:
    // Out of curiosity, do all platforms agree on what values we get exactly
    // 24 hours after that? (Let's avoid pre-1970 values for Windows compatibility.)
    VInstant utc0Plus1Instant = utc0Instant;
    utc0Plus1Instant += VDuration::DAY(); // one day later
    utc0Plus1Instant.getValues(utc0Date, utc0Time, VInstant::UTC_TIME_ZONE_ID()); // see what that is in Greenwich (should be 1970 Jan 2 00:00:00)
    utc0Plus1Instant.getValues(utc0Date, utc0Time, VInstant::LOCAL_TIME_ZONE_ID()); // see what that is in local time (should be 1970 Jan 2 00:00:00 minus local time zone delta)
    VDate        utc1Date(1970, 1, 2);
    VTimeOfDay    utc1Time(0, 0, 0, 0);
    utc0Plus1Instant.setValues(utc1Date, utc1Time, VInstant::UTC_TIME_ZONE_ID()); // see if setting Jan 2 UTC works out to 86400000

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
    Vs64            nowOffset = VInstant::_platform_now();
    VInstantStruct    nowUTCStruct;
    VInstantStruct    nowLocalStruct;

    VInstant::_platform_offsetToUTCStruct(nowOffset, nowUTCStruct);
    VInstant::_platform_offsetToLocalStruct(nowOffset, nowLocalStruct);

    Vs64    utcCheckOffset = VInstant::_platform_offsetFromUTCStruct(nowUTCStruct);
    Vs64    localCheckOffset = VInstant::_platform_offsetFromLocalStruct(nowLocalStruct);

    this->test(nowOffset == utcCheckOffset, "platform UTC conversion cycle");
    this->test(nowOffset == localCheckOffset, "platform local conversion cycle");

    // We know exactly what the correct value for July 14 2004 noon UTC is:
    this->test(july_14_2004_noon_utc.getValue() == CONST_S64(1089806400000), "utc epoch known offset");
    // (The value for July 14 2004 local time depends on our local time zone.)

    // Those two times must not have the same underlying "value", because
    // they were specified in different time zones -- (We'll assume that
    // this machine is not running with local time zone = UTC. This test
    // will appear to fail if UTC is local, if things are OK. But in that
    // case, the other zone conversion tests are not really being exercised
    // anyway, so testing in UTC is perhaps a bogus test environment anyway.)
    this->test(july_14_2004_noon_local != july_14_2004_noon_utc, "local != gm time");

    // Reverse each VInstant back into VDate and VTimeOfDay, and verify.

    VDate        dateLocalFromLocal;
    VTimeOfDay    timeLocalFromLocal;
    july_14_2004_noon_local.getValues(dateLocalFromLocal, timeLocalFromLocal, VInstant::LOCAL_TIME_ZONE_ID());
    this->test((dateLocalFromLocal == july_14_2004) && (timeLocalFromLocal == noon), "local conversion cycle");

    VDate        dateUTCFromUTC;
    VTimeOfDay    timeUTCFromUTC;
    july_14_2004_noon_utc.getValues(dateUTCFromUTC, timeUTCFromUTC, VInstant::UTC_TIME_ZONE_ID());
    this->test((dateUTCFromUTC == july_14_2004) && (timeUTCFromUTC == noon), "utc conversion cycle 1");

    dateUTCFromUTC = july_14_2004_noon_utc.getDate(VInstant::UTC_TIME_ZONE_ID());
    timeUTCFromUTC = july_14_2004_noon_utc.getTimeOfDay(VInstant::UTC_TIME_ZONE_ID());
    this->test((dateUTCFromUTC == july_14_2004) && (timeUTCFromUTC == noon), "utc conversion cycle 2");

    this->test((dateUTCFromUTC.getYear() == 2004) &&
               (dateUTCFromUTC.getMonth() == 7) &&
               (dateUTCFromUTC.getDay() == 14) &&
               (dateUTCFromUTC.getDayOfWeek() == VDate::kWednesday), "date values");
    this->test((timeUTCFromUTC.getHour() == 12) &&
               (timeUTCFromUTC.getMinute() == 0) &&
               (timeUTCFromUTC.getSecond() == 0), "time of day values");

}

void VInstantUnit::_runDurationValueTests() {
    // VDuration tests.

    this->test(VDuration::ZERO().getDurationMilliseconds() == CONST_S64(0), "VDuration ZERO");
    this->test(VDuration::SECOND().getDurationMilliseconds() == CONST_S64(1000), "VDuration SECOND");
    this->test(VDuration::MINUTE().getDurationMilliseconds() == CONST_S64(60000), "VDuration MINUTE");
    this->test(VDuration::HOUR().getDurationMilliseconds() == CONST_S64(3600000), "VDuration HOUR");
    this->test(VDuration::DAY().getDurationMilliseconds() == CONST_S64(86400000), "VDuration DAY");

    VString zeroDurationString = VDuration::ZERO().getDurationString();
    this->test(VDuration::ZERO().getDurationString(), "0ms", "VDuration zero string");
    this->test(VDuration::ZERO().getDurationStringFractionalSeconds(), "0.000", "VDuration zero fractional string");
    this->test(VDuration::UNSPECIFIED().getDurationString(), "UNSPECIFIED", "VDuration UNSPECIFIED string");
    this->test(VDuration::NEGATIVE_INFINITY().getDurationString(), "-INFINITY", "VDuration NEGATIVE_INFINITY string");
    this->test(VDuration::POSITIVE_INFINITY().getDurationString(), "INFINITY", "VDuration POSITIVE_INFINITY string");

    VDuration durationStringTest;
    durationStringTest = CONST_S64(987) * VDuration::MILLISECOND();
    this->test(durationStringTest.getDurationString(), "987ms", "VDuration 0.987 string");
    this->test(durationStringTest.getDurationStringFractionalSeconds(), "0.987", "VDuration 0.987 fractional string");
    durationStringTest = CONST_S64(1001) * VDuration::MILLISECOND();
    this->test(durationStringTest.getDurationString(), "1001ms", "VDuration 1.001 string");
    this->test(durationStringTest.getDurationStringFractionalSeconds(), "1.001", "VDuration 1.001 fractional string");
    durationStringTest = VDuration::MINUTE();
    this->test(durationStringTest.getDurationString(), "1m", "VDuration MINUTE string");
    this->test(durationStringTest.getDurationStringFractionalSeconds(), "60.000", "VDuration MINUTE fractional string");
    durationStringTest = VDuration::HOUR();
    this->test(durationStringTest.getDurationString(), "1h", "VDuration HOUR string");
    this->test(durationStringTest.getDurationStringFractionalSeconds(), "3600.000", "VDuration HOUR0 fractional string");
    durationStringTest = VDuration::DAY();
    this->test(durationStringTest.getDurationString(), "1d", "VDuration DAY string");
    this->test(durationStringTest.getDurationStringFractionalSeconds(), "86400.000", "VDuration DAY fractional string");

    VDuration duration; // zero
    this->test(duration == VDuration::ZERO(), "VDuration default equals ZERO");
    duration += VDuration::SECOND();
    this->test(duration == VDuration::SECOND(), "VDuration ZERO plus SECOND equals SECOND");
    duration = 2 * VDuration::MINUTE();
    this->test(duration.getDurationMilliseconds() == 120000, "VDuration 2 * MINUTE equals 120000ms");
    duration = VDuration::MINUTE() * 2; // cover the transitive version of operator*
    this->test(duration.getDurationMilliseconds() == 120000, "VDuration MINUTE * 2 equals 120000ms");
    this->test(duration == (120 * VDuration::SECOND()), "VDuration 2 * MINUTE equals 120 * SECOND");
    duration = VDuration::DAY() - (10 * VDuration::HOUR());
    this->test(duration == (14 * VDuration::HOUR()), "VDuration DAY minus 10 * HOUR equals 14 * HOUR");
    duration = VDuration::DAY() - VDuration::MINUTE();
    this->test(duration == ((23 * VDuration::HOUR()) + (59 * VDuration::MINUTE())), "VDuration DAY minus MINUTE equals 23h59m");
    duration = VDuration::MINUTE();
    duration -= VDuration::SECOND();
    this->test(duration == (59 * VDuration::SECOND()), "VDuration operator-=");
    duration += VDuration::SECOND();
    this->test(duration == VDuration::MINUTE(), "VDuration operator+=");
    duration *= 60;
    this->test(duration == VDuration::HOUR(), "VDuration operator*=");
    duration /= 60;
    this->test(duration == VDuration::MINUTE(), "VDuration operator/= test 1");
    duration /= 2;
    this->test(duration == (30 * VDuration::SECOND()), "VDuration operator/= test 2");
    duration = VDuration::MINUTE() + VDuration::MINUTE();
    this->test(duration == (2 * VDuration::MINUTE()), "VDuration operator+"); // operator- already tested implicitly
    this->test(VDuration::MINUTE() > VDuration::SECOND(), "VDuration operator>");
    this->test(VDuration::MINUTE() >= VDuration::SECOND(), "VDuration operator>=");
    this->test(VDuration::MINUTE() < VDuration::HOUR(), "VDuration operator<");
    this->test(VDuration::MINUTE() <= VDuration::HOUR(), "VDuration operator<=");
    this->test(VDuration::MINUTE() == VDuration::MINUTE(), "VDuration operator==");
    this->test(VDuration::MINUTE() != VDuration::DAY(), "VDuration operator!=");
    this->test(VDuration::min(VDuration::MINUTE(), VDuration::HOUR()) == VDuration::MINUTE(), "VDuration min test 1");
    this->test(VDuration::min(VDuration::HOUR(), VDuration::MINUTE()) == VDuration::MINUTE(), "VDuration min test 2");
    this->test(VDuration::min(VDuration::MINUTE(), VDuration::MINUTE()) == VDuration::MINUTE(), "VDuration min test 3");
    this->test(VDuration::max(VDuration::MINUTE(), VDuration::HOUR()) == VDuration::HOUR(), "VDuration max test 1");
    this->test(VDuration::max(VDuration::HOUR(), VDuration::MINUTE()) == VDuration::HOUR(), "VDuration max test 2");
    this->test(VDuration::max(VDuration::HOUR(), VDuration::HOUR()) == VDuration::HOUR(), "VDuration max test 3");
    this->test(-duration == duration * -1, "VDuration negation test 1");
    this->test(-duration != duration, "VDuration negation test 2");
    this->test(-duration == duration - duration - duration, "VDuration negation test 3");
    this->test(duration / 0 == VDuration::POSITIVE_INFINITY(), "VDuration positive divide by zero test");
    this->test((-duration) / 0 == VDuration::NEGATIVE_INFINITY(), "VDuration positive divide by zero test");
    this->test(VDuration::abs(VDuration::MINUTE()) == VDuration::MINUTE(), "VDuration positive abs test");
    this->test(VDuration::abs(VDuration::MINUTE() * -1) == VDuration::MINUTE(), "VDuration negative abs test");
    this->test(VDuration::abs(VDuration::NEGATIVE_INFINITY()) == VDuration::POSITIVE_INFINITY(), "VDuration negative infinity abs test");
    this->test(VDuration::abs(VDuration::POSITIVE_INFINITY()) == VDuration::POSITIVE_INFINITY(), "VDuration positive infinity abs test");
    this->test((- VDuration::MINUTE()) == VDuration::MINUTE() * -1, "VDuration unary minus test");
    this->test((- VDuration::MINUTE()) < VDuration::ZERO(), "VDuration unary minus less than zero test");
    this->test((- VDuration::NEGATIVE_INFINITY()) == VDuration::POSITIVE_INFINITY(), "VDuration unary minus of negative infinity test");
    this->test((- VDuration::POSITIVE_INFINITY()) == VDuration::NEGATIVE_INFINITY(), "VDuration unary minus of positive infinity test");

}

void VInstantUnit::_runExoticDurationValueTests() {

    // Additional tests for exotic instant and duration properties such as
    // math operations on +/- infinity.

    VDuration negativeDay = -1 * VDuration::DAY();
    VDuration positiveDay = VDuration::DAY();
    VDuration someDuration = VDuration::HOUR();

    this->test(VDuration::NEGATIVE_INFINITY() != VDuration::POSITIVE_INFINITY(), "VDuration::NEGATIVE_INFINITY() != VDuration::POSITIVE_INFINITY()");
    this->test(VDuration::NEGATIVE_INFINITY() < VDuration::POSITIVE_INFINITY(), "VDuration::NEGATIVE_INFINITY() < VDuration::POSITIVE_INFINITY()");
    this->test(VDuration::NEGATIVE_INFINITY() <= VDuration::POSITIVE_INFINITY(), "VDuration::NEGATIVE_INFINITY() <= VDuration::POSITIVE_INFINITY()");
    this->test(VDuration::NEGATIVE_INFINITY() != VDuration::ZERO(), "VDuration::NEGATIVE_INFINITY() != VDuration::ZERO()");
    this->test(VDuration::NEGATIVE_INFINITY() < VDuration::ZERO(), "VDuration::NEGATIVE_INFINITY() < VDuration::ZERO()");
    this->test(VDuration::NEGATIVE_INFINITY() <= VDuration::ZERO(), "VDuration::NEGATIVE_INFINITY() <= VDuration::ZERO()");
    this->test(VDuration::ZERO() != VDuration::POSITIVE_INFINITY(), "VDuration::ZERO() != VDuration::POSITIVE_INFINITY()");
    this->test(VDuration::ZERO() < VDuration::POSITIVE_INFINITY(), "VDuration::ZERO() < VDuration::POSITIVE_INFINITY()");
    this->test(VDuration::ZERO() <= VDuration::POSITIVE_INFINITY(), "VDuration::ZERO() <= VDuration::POSITIVE_INFINITY()");
    this->test(VDuration::NEGATIVE_INFINITY() < negativeDay, "VDuration::NEGATIVE_INFINITY() < negativeDay");
    this->test(negativeDay < VDuration::ZERO(), "negativeDay < VDuration::ZERO()");
    this->test(VDuration::ZERO() < positiveDay, "VDuration::ZERO() < positiveDay");
    this->test(positiveDay < VDuration::POSITIVE_INFINITY(), "positiveDay < VDuration::POSITIVE_INFINITY()");
    this->test(VDuration::POSITIVE_INFINITY() - someDuration == VDuration::POSITIVE_INFINITY(), "VDuration::POSITIVE_INFINITY() - someDuration == VDuration::POSITIVE_INFINITY()");
    this->test(VDuration::POSITIVE_INFINITY() + someDuration == VDuration::POSITIVE_INFINITY(), "VDuration::POSITIVE_INFINITY() + someDuration == VDuration::POSITIVE_INFINITY()");
    this->test(VDuration::POSITIVE_INFINITY() * 5 == VDuration::POSITIVE_INFINITY(), "VDuration::POSITIVE_INFINITY() * 5 == VDuration::POSITIVE_INFINITY()");
    this->test(VDuration::POSITIVE_INFINITY() * -5 == VDuration::NEGATIVE_INFINITY(), "VDuration::POSITIVE_INFINITY() * -5 == VDuration::NEGATIVE_INFINITY()");
    this->test(VDuration::POSITIVE_INFINITY() * 0 == VDuration::ZERO(), "VDuration::POSITIVE_INFINITY() * 0 == VDuration::ZERO()");
    this->test(VDuration::POSITIVE_INFINITY() / 5 == VDuration::POSITIVE_INFINITY(), "VDuration::POSITIVE_INFINITY() / 5 == VDuration::POSITIVE_INFINITY()");
    this->test(negativeDay / 0 == VDuration::NEGATIVE_INFINITY(), "negativeDay / 0 == VDuration::NEGATIVE_INFINITY()");
    this->test(positiveDay / 0 == VDuration::POSITIVE_INFINITY(), "positiveDay / 0 == VDuration::POSITIVE_INFINITY()");
    this->test(VDuration::POSITIVE_INFINITY() % (VDuration::MILLISECOND() * 5) == VDuration::POSITIVE_INFINITY(), "VDuration::POSITIVE_INFINITY() % 5ms == VDuration::POSITIVE_INFINITY()");
    this->test(VDuration::NEGATIVE_INFINITY() - someDuration == VDuration::NEGATIVE_INFINITY(), "VDuration::NEGATIVE_INFINITY() - someDuration == VDuration::NEGATIVE_INFINITY()");
    this->test(VDuration::NEGATIVE_INFINITY() + someDuration == VDuration::NEGATIVE_INFINITY(), "VDuration::NEGATIVE_INFINITY() + someDuration == VDuration::NEGATIVE_INFINITY()");
    this->test(VDuration::NEGATIVE_INFINITY() * 5 == VDuration::NEGATIVE_INFINITY(), "VDuration::NEGATIVE_INFINITY() * 5 == VDuration::NEGATIVE_INFINITY()");
    this->test(VDuration::NEGATIVE_INFINITY() * -5 == VDuration::POSITIVE_INFINITY(), "VDuration::NEGATIVE_INFINITY() * -5 == VDuration::POSITIVE_INFINITY()");
    this->test(VDuration::NEGATIVE_INFINITY() * 0 == VDuration::ZERO(), "VDuration::NEGATIVE_INFINITY() * 0 == VDuration::ZERO()");
    this->test(VDuration::NEGATIVE_INFINITY() / 5 == VDuration::NEGATIVE_INFINITY(), "VDuration::NEGATIVE_INFINITY() / 5 == VDuration::NEGATIVE_INFINITY()");
    this->test(VDuration::NEGATIVE_INFINITY() % (VDuration::MILLISECOND() * 5) == VDuration::NEGATIVE_INFINITY(), "VDuration::NEGATIVE_INFINITY() % 5ms == VDuration::NEGATIVE_INFINITY()");
    this->test(VDuration::min(VDuration::NEGATIVE_INFINITY(), VDuration::NEGATIVE_INFINITY()) == VDuration::NEGATIVE_INFINITY(), "VDuration::min(VDuration::NEGATIVE_INFINITY(), VDuration::NEGATIVE_INFINITY()) == VDuration::NEGATIVE_INFINITY()");
    this->test(VDuration::min(VDuration::NEGATIVE_INFINITY(), someDuration) == VDuration::NEGATIVE_INFINITY(), "VDuration::min(VDuration::NEGATIVE_INFINITY(), someDuration) == VDuration::NEGATIVE_INFINITY()");
    this->test(VDuration::min(VDuration::NEGATIVE_INFINITY(), VDuration::POSITIVE_INFINITY()) == VDuration::NEGATIVE_INFINITY(), "VDuration::min(VDuration::NEGATIVE_INFINITY(), VDuration::POSITIVE_INFINITY()) == VDuration::NEGATIVE_INFINITY()");
    this->test(VDuration::max(VDuration::NEGATIVE_INFINITY(), VDuration::NEGATIVE_INFINITY()) == VDuration::NEGATIVE_INFINITY(), "VDuration::max(VDuration::NEGATIVE_INFINITY(), VDuration::NEGATIVE_INFINITY()) == VDuration::NEGATIVE_INFINITY()");
    this->test(VDuration::max(VDuration::NEGATIVE_INFINITY(), someDuration) == someDuration, "VDuration::max(VDuration::NEGATIVE_INFINITY(), someDuration) == someDuration");
    this->test(VDuration::max(VDuration::NEGATIVE_INFINITY(), VDuration::POSITIVE_INFINITY()) == VDuration::POSITIVE_INFINITY(), "VDuration::max(VDuration::NEGATIVE_INFINITY(), VDuration::POSITIVE_INFINITY()) == VDuration::POSITIVE_INFINITY()");
    this->test(VDuration::max(VDuration::POSITIVE_INFINITY(), VDuration::POSITIVE_INFINITY()) == VDuration::POSITIVE_INFINITY(), "VDuration::max(VDuration::POSITIVE_INFINITY(), VDuration::POSITIVE_INFINITY()) == VDuration::POSITIVE_INFINITY()");
    this->test(VDuration::max(VDuration::POSITIVE_INFINITY(), someDuration) == VDuration::POSITIVE_INFINITY(), "VDuration::max(VDuration::POSITIVE_INFINITY(), someDuration) == VDuration::POSITIVE_INFINITY()");
    this->test(VDuration::max(VDuration::POSITIVE_INFINITY(), VDuration::NEGATIVE_INFINITY()) == VDuration::POSITIVE_INFINITY(), "VDuration::max(VDuration::POSITIVE_INFINITY(), VDuration::NEGATIVE_INFINITY()) == VDuration::POSITIVE_INFINITY()");
    this->test(VDuration::min(VDuration::POSITIVE_INFINITY(), VDuration::NEGATIVE_INFINITY()) == VDuration::NEGATIVE_INFINITY(), "VDuration::min(VDuration::POSITIVE_INFINITY(), VDuration::NEGATIVE_INFINITY()) == VDuration::NEGATIVE_INFINITY()");
    this->test(VDuration::min(VDuration::POSITIVE_INFINITY(), someDuration) == someDuration, "VDuration::min(VDuration::POSITIVE_INFINITY(), someDuration) == someDuration");
    this->test(VDuration::min(VDuration::POSITIVE_INFINITY(), VDuration::POSITIVE_INFINITY()) == VDuration::POSITIVE_INFINITY(), "VDuration::min(VDuration::POSITIVE_INFINITY(), VDuration::POSITIVE_INFINITY()) == VDuration::POSITIVE_INFINITY()");
    // Now we modify the exotics locally and test.
    VDuration negativeInfinity = VDuration::NEGATIVE_INFINITY();
    negativeInfinity += someDuration;
    this->test(negativeInfinity == VDuration::NEGATIVE_INFINITY(), "negativeInfinity += someDuration == VDuration::NEGATIVE_INFINITY()");
    VDuration positiveInfinity = VDuration::POSITIVE_INFINITY();
    positiveInfinity -= someDuration;
    this->test(positiveInfinity == VDuration::POSITIVE_INFINITY(), "positiveInfinity -= someDuration == VDuration::POSITIVE_INFINITY()");
    positiveInfinity /= 5;
    this->test(positiveInfinity == VDuration::POSITIVE_INFINITY(), "positiveInfinity /= 5 == VDuration::POSITIVE_INFINITY()");
    negativeInfinity /= 5;
    this->test(negativeInfinity == VDuration::NEGATIVE_INFINITY(), "negativeInfinity /= 5 == VDuration::POSITIVE_INFINITY()");
    this->test(-VDuration::POSITIVE_INFINITY() == VDuration::NEGATIVE_INFINITY(), "-VDuration::POSITIVE_INFINITY() == VDuration::NEGATIVE_INFINITY()");
    this->test(-VDuration::NEGATIVE_INFINITY() == VDuration::POSITIVE_INFINITY(), "-VDuration::NEGATIVE_INFINITY() == VDuration::POSITIVE_INFINITY()");
    this->test(VDuration::NEGATIVE_INFINITY() + VDuration::POSITIVE_INFINITY() == VDuration::ZERO(), "VDuration::NEGATIVE_INFINITY() + VDuration::POSITIVE_INFINITY() == VDuration::ZERO()");
    this->test(VDuration::POSITIVE_INFINITY() - VDuration::POSITIVE_INFINITY() == VDuration::ZERO(), "VDuration::POSITIVE_INFINITY() - VDuration::POSITIVE_INFINITY() == VDuration::ZERO()");
    this->test(VDuration::NEGATIVE_INFINITY() - VDuration::NEGATIVE_INFINITY() == VDuration::ZERO(), "VDuration::NEGATIVE_INFINITY() - VDuration::NEGATIVE_INFINITY() == VDuration::ZERO()");

    VInstant currentTime;
    VInstant currentMinus1d = currentTime - VDuration::DAY();
    VInstant currentMinus24h = currentTime - (24 * VDuration::HOUR());
    VInstant currentPlus1d = currentTime + VDuration::DAY();
    VInstant currentPlus24h = currentTime + (24 * VDuration::HOUR());
    VInstant infinitePast = VInstant::INFINITE_PAST();
    VInstant infiniteFuture = VInstant::INFINITE_FUTURE();
    VInstant never = VInstant::NEVER_OCCURRED();

    this->test(infinitePast < currentTime, "infinitePast < currentTime");
    this->test(infinitePast <= currentTime, "infinitePast <= currentTime");
    this->test(!(infinitePast >= currentTime), "! (infinitePast >= currentTime)");
    this->test(!(infinitePast > currentTime), "! (infinitePast > currentTime)");

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