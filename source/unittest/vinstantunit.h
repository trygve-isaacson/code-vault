/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
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

};

#endif /* vinstantunit_h */
