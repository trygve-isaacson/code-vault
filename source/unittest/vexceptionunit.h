/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

#ifndef vexceptionunit_h
#define vexceptionunit_h

/** @file */

#include "vunit.h"

/**
Unit test class for validating VException.
*/
class VExceptionUnit : public VUnit {
    public:

        /**
        Constructs a unit test object.
        @param    logOnSuccess    true if you want successful tests to be logged
        @param    throwOnError    true if you want an exception thrown for failed tests
        */
        VExceptionUnit(bool logOnSuccess, bool throwOnError);
        /**
        Destructor.
        */
        virtual ~VExceptionUnit() {}

        /**
        Executes the unit test.
        */
        virtual void run();

    private:

        void _testConstructors();
        void _testCatchHierarchy();
        void _testCheckedDynamicCast();

};

#endif /* vexceptionunit_h */
