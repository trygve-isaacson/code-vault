/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

#ifndef vbentounit_h
#define vbentounit_h

/** @file */

#include "vunit.h"

/**
Unit test class for validating VBento.
*/
class VBentoUnit : public VUnit
    {
    public:
    
        /**
        Constructs a unit test object.
        @param    logOnSuccess    true if you want successful tests to be logged
        @param    throwOnError    true if you want an exception thrown for failed tests
        */
        VBentoUnit(bool logOnSuccess, bool throwOnError);
        /**
        Destructor.
        */
        virtual ~VBentoUnit() {}
        
        /**
        Executes the unit test.
        */
        virtual void run();

    };

#endif /* vbentounit_h */
