/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

#ifndef vloggerunit_h
#define vloggerunit_h

/** @file */

#include "vunit.h"

/**
Unit test class for validating VLogger.
*/
class VLoggerUnit : public VUnit
    {
    public:
    
        /**
        Constructs a unit test object.
        @param    logOnSuccess    true if you want successful tests to be logged
        @param    throwOnError    true if you want an exception thrown for failed tests
        */
        VLoggerUnit(bool logOnSuccess, bool throwOnError);
        /**
        Destructor.
        */
        virtual ~VLoggerUnit() {}
        
        /**
        Executes the unit test.
        */
        virtual void run();

    private:
    
        void _testNewInfrastructure();
        void _testMacros();
        void _testStringLoggers();
        void _testMaxActiveLogLevel();
        void _testLoggerPathNames();
        void _testOptimizationPerformance();

    };

#endif /* vloggerunit_h */
