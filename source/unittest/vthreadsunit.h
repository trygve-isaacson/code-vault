/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

#ifndef vthreadsunit_h
#define vthreadsunit_h

/** @file */

#include "vunit.h"

/**
Unit test class for validating VThread, VMutex, VMutexLocker, VSemaphore.
*/
class VThreadsUnit : public VUnit
    {
    public:
    
        /**
        Constructs a unit test object.
        @param    logOnSuccess    true if you want successful tests to be logged
        @param    throwOnError    true if you want an exception thrown for failed tests
        */
        VThreadsUnit(bool logOnSuccess, bool throwOnError);
        /**
        Destructor.
        */
        virtual ~VThreadsUnit() {}
        
        /**
        Executes the unit test.
        */
        virtual void run();

        friend class TestThreadClass; // Our test uses this thread class and it needs to log status to unit test output.
    };

#endif /* vthreadsunit_h */
