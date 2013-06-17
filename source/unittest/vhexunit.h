/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vhexunit_h
#define vhexunit_h

/** @file */

#include "vunit.h"

/**
Unit test class for validating VHex.
*/
class VHexUnit : public VUnit {
    public:

        /**
        Constructs a unit test object.
        @param    logOnSuccess    true if you want successful tests to be logged
        @param    throwOnError    true if you want an exception thrown for failed tests
        */
        VHexUnit(bool logOnSuccess, bool throwOnError);
        /**
        Destructor.
        */
        virtual ~VHexUnit() {}

        /**
        Executes the unit test.
        */
        virtual void run();

};

#endif /* vhexunit_h */
