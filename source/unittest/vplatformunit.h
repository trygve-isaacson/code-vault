/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
*/

#ifndef vplatformunit_h
#define vplatformunit_h

/** @file */

#include "vunit.h"

/**
Unit test class for validating the platform configuration.
*/
class VPlatformUnit : public VUnit {
    public:

        /**
        Constructs a unit test object.
        @param    logOnSuccess    true if you want successful tests to be logged
        @param    throwOnError    true if you want an exception thrown for failed tests
        */
        VPlatformUnit(bool logOnSuccess, bool throwOnError);
        /**
        Destructor.
        */
        virtual ~VPlatformUnit() {}

        /**
        Executes the unit test.
        */
        virtual void run();

    private:

        void _reportEnvironment();
        void _runEfficientSprintfCheck();
        void _runByteswapCheck();
        void _runMinMaxAbsCheck();
        void _runTimeCheck();
        void _runUtilitiesTest();
        void _runSocketTests();

        void _runResolveAndConnectHostNameTest(const VString& hostName);
        void _assertStringIsNumericIPAddressString(const VString& label, const VString& hostName, const VString& value);

};

#endif /* vplatformunit_h */
