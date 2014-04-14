/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
*/

#ifndef vassertunit_h
#define vassertunit_h

/** @file */

#include "vunit.h"

#ifndef V_ASSERT_ACTIVE
#define VASSERTUNIT_IS_NOT_USEFUL
#else
#ifndef V_ASSERT_THROWS_EXCEPTION
#define VASSERTUNIT_IS_NOT_USEFUL
#endif
#endif

/**
Unit test class for validating VHex.
*/
class VAssertUnit : public VUnit {
    public:

        /**
        Constructs a unit test object.
        @param    logOnSuccess    true if you want successful tests to be logged
        @param    throwOnError    true if you want an exception thrown for failed tests
        */
        VAssertUnit(bool logOnSuccess, bool throwOnError);
        /**
        Destructor.
        */
        virtual ~VAssertUnit() {}

        /**
        Executes the unit test.
        */
        virtual void run();

    private:

#ifndef VASSERTUNIT_IS_NOT_USEFUL

        void _positiveAssertionsForDouble(VDouble testValue);
        void _positiveAssertionsForString();
        void _positiveAssertionsForDuration();
        void _positiveAssertionsForInstant();

        void _negativeAssertionsForDouble(VDouble testValue);
        void _negativeAssertionsForString();
        void _negativeAssertionsForDuration();
        void _negativeAssertionsForInstant();

        template <class T>
        void _positiveAssertionsForNumericType(const VString& dataTypeName, T testValue);
        template <class T>
        void _negativeAssertionsForNumericType(const VString& dataTypeName, T testValue);

#endif /* VASSERTUNIT_IS_NOT_USEFUL */

};

#endif /* vassertunit_h */
