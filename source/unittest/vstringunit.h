/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vstringunit_h
#define vstringunit_h

/** @file */

#include "vunit.h"

/**
Unit test class for validating VString.
*/
class VStringUnit : public VUnit {
    public:

        /**
        Constructs a unit test object.
        @param    logOnSuccess    true if you want successful tests to be logged
        @param    throwOnError    true if you want an exception thrown for failed tests
        */
        VStringUnit(bool logOnSuccess, bool throwOnError);
        /**
        Destructor.
        */
        virtual ~VStringUnit() {}

        /**
        Executes the unit test.
        */
        virtual void run();

};

#endif /* vstringunit_h */
