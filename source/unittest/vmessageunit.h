/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

#ifndef vmessageunit_h
#define vmessageunit_h

/** @file */

#include "vunit.h"
#include "vmessagepool.h"

/**
Unit test class for validating VMessage and related classes.
*/
class VMessageUnit : public VUnit
    {
    public:
    
        /**
        Constructs a unit test object.
        @param    logOnSuccess    true if you want successful tests to be logged
        @param    throwOnError    true if you want an exception thrown for failed tests
        */
        VMessageUnit(bool logOnSuccess, bool throwOnError);
        /**
        Destructor.
        */
        virtual ~VMessageUnit() {}
        
        /**
        Executes the unit test.
        */
        virtual void run();

    private:
    
        void _validateStats(const VMessagePool& pool, const VMessagePool& stats, int numInPool, const VString& label);
    };

#endif /* vmessageunit_h */
