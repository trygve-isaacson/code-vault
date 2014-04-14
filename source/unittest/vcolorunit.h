/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
*/

#ifndef vcolorunit_h
#define vcolorunit_h

/** @file */

#include "vunit.h"

/**
Unit test class for validating VColor-related classes.
*/
class VColorUnit : public VUnit {
    public:

        /**
        Constructs a unit test object.
        @param    logOnSuccess    true if you want successful tests to be logged
        @param    throwOnError    true if you want an exception thrown for failed tests
        */
        VColorUnit(bool logOnSuccess, bool throwOnError);
        /**
        Destructor.
        */
        virtual ~VColorUnit() {}

        /**
        Executes the unit test.
        */
        virtual void run();

    private:

        void _testVColor();
        void _testVStringColorMapper();
        void _testVIntegerColorMapper();
        void _testVDoubleColorMapper();
        void _testVStringRangeColorMapper();
        void _testVIntegerRangeColorMapper();
        void _testVDoubleRangeColorMapper();
        void _testVColorPalette();
};

#endif /* vcolorunit_h */
