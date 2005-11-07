/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vinstantunit.h"

#include "vinstant.h"

VInstantUnit::VInstantUnit(bool logOnSuccess, bool throwOnError)
: VUnit("VInstantUnit", logOnSuccess, throwOnError)
    {
    }

void VInstantUnit::run()
    {
    
    {
    VInstant    now;
    VString        nowLocalString;
    VString        nowUTCString;
    now.getLocalString(nowLocalString);
    now.getUTCString(nowUTCString);
    
    std::cout << "VInstant thinks the current local time is " << nowLocalString << " (" << nowUTCString << ")." << std::endl;
    }
    
    VInstant    i1;
    VInstant    i2;
    Vs64        offset1;
    Vs64        offset2;
    Vs32        offset1Secs;
    Vs32        offset2Secs;
    Vs64        base = i1.getValue();    // may be useful in debugging below

    // Test symmetry of sets and gets.
    offset1 = i1.getValue();
    i2.setValue(offset1);
    offset2 = i2.getValue();
    this->test(offset1 == offset2, "symmetry test 1");
    this->test(i1 == i2, "symmetry test 2");
    
    offset1Secs = i1.getValueSeconds();
    offset2Secs = i2.getValueSeconds();
    this->test(offset1Secs == offset2Secs, "symmetry test 3");
    offset1Secs += 100L;
    i1.setValueSeconds(offset1Secs);
    i2 += CONST_S64(100000);
    // Note that i1 == i2 is probably false because of the resolution difference.
    // So we need to re-get the value in seconds and compare at that resolution.
    offset2Secs = i2.getValueSeconds();
    this->test(offset1Secs == offset2Secs, "symmetry test 4");
    
    // Test modification functions.
    // Set things up for this set of tests.
    offset1 = base;
    offset2 = base;
    i1.setValue(base);
    i2.setValue(base);

    // operator+=
    const Vs64 kDeltaA = CONST_S64(12345);
    i1 += kDeltaA;                                        // i1 is base + kDeltaA
    offset2 += kDeltaA;                                    // off2 is base + kDeltaA
    offset1 = i1.getValue();                            // off1 is base + kDeltaA
    this->test(offset1 == offset2, "modification test 1");
    i2 += kDeltaA;                                        // i2 is base + kDeltaA
    this->test(i1 == i2, "modification test 2");
    
    // deltaOffset()
    const Vs64 kDeltaB = CONST_S64(45678);
//    const Vs64 kDeltaAB = kDeltaA + kDeltaB;            // Might be useful in debugging.
    i1.deltaOffset(kDeltaB);                            // i1 is base + kDeltaAB
    offset2 += kDeltaB;                                    // off2 is base + kDeltaAB
    offset1 = i1.getValue();                            // off1 is base + kDeltaAB
    this->test(offset1 == offset2, "modification test 3");
    i2 += kDeltaB;                                        // i2 is base + kDeltaAB
    this->test(i1 == i2, "modification test 4");
    
    // operator-=
    const Vs64 kDeltaC = CONST_S64(13579);
//    const Vs64 kDeltaABC = kDeltaAB - kDeltaC;            // Might be useful in debugging.
    i1 -= kDeltaC;                                        // i1 is base + kDeltaABC
    offset2 -= kDeltaC;                                    // off2 is base + kDeltaABC
    offset1 = i1.getValue();                            // off1 is base + kDeltaABC
    this->test(offset1 == offset2, "modification test 5");
    i2 -= kDeltaC;                                        // i2 is base + kDeltaABC
    this->test(i1 == i2, "modification test 6");
    
    // Test comparison operators.
    // Set things up for this set of tests.
    offset1 = base;
    offset2 = base;
    i1.setValue(base);
    i2.setValue(base);

    const Vs64 kDeltaD = CONST_S64(24680);
    i2 += kDeltaD;
    // Now i2 is kDeltaD milliseconds later than i1.
    this->test(! (i1 == i2), "comparison test 1a");
    this->test(i1 != i2, "comparison test 1b");
    this->test(i1 < i2, "comparison test 1c");
    this->test(i1 <= i2, "comparison test 1d");
    this->test(i2 > i1, "comparison test 1e");
    this->test(i2 >= i1, "comparison test 1f");
    this->test(i2 - i1 == kDeltaD, "comparison test 1g");
    i2 -= kDeltaD;
    // Now i1 and i2 are equal.
    this->test(i1 == i2, "comparison test 2a");
    this->test(! (i1 != i2), "comparison test 2b");
    this->test(! (i1 < i2), "comparison test 2c");
    this->test(i1 <= i2, "comparison test 2d");
    this->test(! (i2 > i1), "comparison test 2e");
    this->test(i2 >= i1, "comparison test 2f");
    this->test(i2 - i1 == CONST_S64(0), "comparison test 2g");
    
    // Test comparison operators with "infinite" values.
    VInstant    now;
    VInstant    infinitePast;
    VInstant    infiniteFuture;
    infinitePast.setKind(VInstant::kInfinitePast);
    infiniteFuture.setKind(VInstant::kInfiniteFuture);

    this->test(infinitePast < now, "comparison test 3a");
    this->test(infinitePast <= now, "comparison test 3b");
    this->test(now > infinitePast, "comparison test 3c");
    this->test(now >= infinitePast, "comparison test 3d");
    this->test(! (infinitePast > now), "comparison test 3e");
    this->test(! (infinitePast >= now), "comparison test 3f");
    this->test(infinitePast != now, "comparison test 3g");
    this->test(! (infinitePast == now), "comparison test 3h");

    this->test(infiniteFuture > now, "comparison test 4a");
    this->test(infiniteFuture >= now, "comparison test 4b");
    this->test(now < infiniteFuture, "comparison test 4c");
    this->test(now <= infiniteFuture, "comparison test 4d");
    this->test(! (infiniteFuture < now), "comparison test 4e");
    this->test(! (infiniteFuture <= now), "comparison test 4f");
    this->test(infiniteFuture != now, "comparison test 4g");
    this->test(! (infiniteFuture == now), "comparison test 4h");

    this->test(infinitePast < infiniteFuture, "comparison test 5a");
    this->test(infinitePast <= infiniteFuture, "comparison test 5b");
    this->test(infiniteFuture > infinitePast, "comparison test 5c");
    this->test(infiniteFuture >= infinitePast, "comparison test 5d");
    this->test(! (infinitePast > infiniteFuture), "comparison test 5e");
    this->test(! (infinitePast >= infiniteFuture), "comparison test 5f");
    this->test(infinitePast != infiniteFuture, "comparison test 5g");
    this->test(! (infinitePast == infiniteFuture), "comparison test 5h");
    
    // Test local-gm time conversion consistency.
    
    // First let's validate that we get the expected value for UTC zero time.
    VDate        utc0Date(1970, 1, 1);
    VTimeOfDay    utc0Time(0, 0, 0);
    VInstant    utc0Instant;
    
    utc0Instant.setValues(utc0Date, utc0Time, false);
    this->test(utc0Instant.getValue() == CONST_S64(0), "utc epoch base");
    
    // A little debugging code here:
    // Out of curiosity, do all platforms agree on what values we get exactly
    // 24 hours after that? (Let's avoid pre-1970 values for Windows compatibility.)
    VInstant utc0Plus1Instant = utc0Instant;
    utc0Plus1Instant += CONST_S64(86400000); // one day of milliseconds
    utc0Plus1Instant.getValues(utc0Date, utc0Time, false); // see what that is in Greenwich (should be 1970 Jan 2 00:00:00)
    utc0Plus1Instant.getValues(utc0Date, utc0Time, true); // see what that is in local time (should be 1970 Jan 2 00:00:00 minus local time zone delta)
    VDate        utc1Date(1970, 1, 2);
    VTimeOfDay    utc1Time(0, 0, 0);
    utc0Plus1Instant.setValues(utc1Date, utc1Time, false); // see if setting Jan 2 UTC works out to 86400000 
    
    // Create a date and time, specified in both local and gm.
    VDate        july_14_2004(2004, 7, 14);
    VTimeOfDay    noon(12, 0, 0);
    VInstant    july_14_2004_noon_local;
    VInstant    july_14_2004_noon_utc;
    
    july_14_2004_noon_local.setValues(july_14_2004, noon, true);
    july_14_2004_noon_utc.setValues(july_14_2004, noon, false);
    
    VDate        dateLocal;
    VTimeOfDay    noonLocal;
    VDate        dateUTC;
    VTimeOfDay    noonUTC;
    
    july_14_2004_noon_utc.getValues(dateLocal, noonLocal, true);
    july_14_2004_noon_utc.getValues(dateUTC, noonUTC, false);
    // If you're testing this in Pacific time, dateLocal/noonLocal vs. dateUTC/noonUTC
    // should differ by 8 hours in the winter (standard), 7 hours in the summer (daylight).
    
    // Verify symmetry of UTC<->Local conversion in the core platform-specific code.
    Vs64            nowOffset = VInstant::platform_now();
    VInstantStruct    nowUTCStruct;
    VInstantStruct    nowLocalStruct;
    
    VInstant::platform_offsetToUTCStruct(nowOffset, nowUTCStruct);
    VInstant::platform_offsetToLocalStruct(nowOffset, nowLocalStruct);

    Vs64    utcCheckOffset = VInstant::platform_offsetFromUTCStruct(nowUTCStruct);
    Vs64    localCheckOffset = VInstant::platform_offsetFromLocalStruct(nowLocalStruct);

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
    july_14_2004_noon_local.getValues(dateLocalFromLocal, timeLocalFromLocal, true);
    this->test((dateLocalFromLocal == july_14_2004) && (timeLocalFromLocal == noon), "local conversion cycle");

    VDate        dateUTCFromUTC;
    VTimeOfDay    timeUTCFromUTC;
    july_14_2004_noon_utc.getValues(dateUTCFromUTC, timeUTCFromUTC, false);
    this->test((dateUTCFromUTC == july_14_2004) && (timeUTCFromUTC == noon), "utc conversion cycle 1");

    july_14_2004_noon_utc.getDate(dateUTCFromUTC, false);
    july_14_2004_noon_utc.getTimeOfDay(timeUTCFromUTC, false);
    this->test((dateUTCFromUTC == july_14_2004) && (timeUTCFromUTC == noon), "utc conversion cycle 2");

    this->test((dateUTCFromUTC.year() == 2004) &&
                (dateUTCFromUTC.month() == 7) &&
                (dateUTCFromUTC.day() == 14) &&
                (dateUTCFromUTC.dayOfWeek() == VDate::kWednesday), "date values");
    this->test((timeUTCFromUTC.hour() == 12) &&
                (timeUTCFromUTC.minute() == 0) &&
                (timeUTCFromUTC.second() == 0), "time of day values");

    }

