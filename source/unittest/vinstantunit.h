/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

#ifndef vinstantunit_h
#define vinstantunit_h

/** @file */

#include "vunit.h"

/**
Unit test class for validating VInstant.
*/
class VInstantUnit : public VUnit {
    public:

        /**
        Constructs a unit test object.
        @param    logOnSuccess    true if you want successful tests to be logged
        @param    throwOnError    true if you want an exception thrown for failed tests
        */
        VInstantUnit(bool logOnSuccess, bool throwOnError);
        /**
        Destructor.
        */
        virtual ~VInstantUnit() {}

        /**
        Executes the unit test.
        */
        virtual void run();

    private:
    
        void _runInstantOperatorTests();
        void _runInstantComparatorTests();
        void _runClockSimulationTests();
        void _runTimeZoneConversionTests();
        void _runDurationValueTests();
        void _runExoticDurationValueTests();
        void _runDurationStringTests();
        void _runInstantFormatterTests();

        void _testInstantFormatter(const VString& label, const VInstant& instant, const VString& format, const VString& expectedUTCOutput, const VString& expectedLocalOutput);
        void _testInstantRangeRoundTripConversion(const VString& label, const VInstant& startInstant, const VDuration& increment, int numIncrements);
        void _test1InstantRangeRoundTripConversion(const VString& label, const VDateAndTime& when, bool expectSuccess);
};

#endif /* vinstantunit_h */
