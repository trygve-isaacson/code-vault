/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

#ifndef vgeometryunit_h
#define vgeometryunit_h

/** @file */

#include "vunit.h"

/**
Unit test class for validating VGeometry classes.
*/
class VGeometryUnit : public VUnit {
    public:

        /**
        Constructs a unit test object.
        @param    logOnSuccess    true if you want successful tests to be logged
        @param    throwOnError    true if you want an exception thrown for failed tests
        */
        VGeometryUnit(bool logOnSuccess, bool throwOnError);
        /**
        Destructor.
        */
        virtual ~VGeometryUnit() {}

        /**
        Executes the unit test.
        */
        virtual void run();

    private:

        void _testVSize();
        void _testVPoint();
        void _testVPoint3D();
        void _testVRect();
        void _testVLine();
        void _testVPolygon();
};

#endif /* vgeometryunit_h */
