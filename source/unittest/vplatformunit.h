/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

#ifndef vplatformunit_h
#define vplatformunit_h

/** @file */

#include "vunit.h"

/**
Unit test class for validating the platform configuration.
*/
class VPlatformUnit : public VUnit
    {
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

    };

#endif /* vplatformunit_h */
