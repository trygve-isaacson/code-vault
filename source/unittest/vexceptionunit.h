/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

#ifndef vexceptionunit_h
#define vexceptionunit_h

/** @file */

#include "vunit.h"

/**
Unit test class for validating VException.
*/
class VExceptionUnit : public VUnit
    {
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

    };

#endif /* vexceptionunit_h */
