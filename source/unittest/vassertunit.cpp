/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vassertunit.h"

#include "vassert.h"
#include "vgeometry.h"
#include "vexception.h"

VAssertUnit::VAssertUnit(bool logOnSuccess, bool throwOnError) :
VUnit("VAssertUnit", logOnSuccess, throwOnError)
    {
    }

void VAssertUnit::run()
    {
    // The purpose of this unit test is to exercise assertion macros of all flavors for all data types,
    // to make sure they compile and behave correctly. Numeric type overloading of the function APIs is the touchy part.

#ifndef V_ASSERT_ACTIVE
    this->logStatus("Cannot test assertions because V_ASSERT_ACTIVE is not defined on, so assertions do nothing.");
#endif

#ifndef V_ASSERT_THROWS_EXCEPTION
    this->logStatus("Cannot test assertions because V_ASSERT_THROWS_EXCEPTION is not defined on, so failed assertions do not throw.");
#endif

#ifndef VASSERTUNIT_IS_NOT_USEFUL

        {
        /* scope for variable and its tests */
        const int zero = 0;
        VASSERT_ZERO(zero);
        }
        
    // We can use a template function for all the basic numeric types, since they have uniform APIs to test.
    // We need to pass in some value so that the compiler cannot outsmart us by noticing that all the asserted conditions are true,
    // and skipping past them!
    this->_positiveAssertionsForNumericType<int>("int", 100);
    this->_positiveAssertionsForNumericType<unsigned int>("unsigned int", 100);
    this->_positiveAssertionsForNumericType<Vs8>("Vs8", 100);
    this->_positiveAssertionsForNumericType<Vu8>("Vu8", 100);
    this->_positiveAssertionsForNumericType<Vs16>("Vs16", 100);
    this->_positiveAssertionsForNumericType<Vu16>("Vu16", 100);
    this->_positiveAssertionsForNumericType<Vs32>("Vs32", 100);
    this->_positiveAssertionsForNumericType<Vu32>("Vu32", 100);
    this->_positiveAssertionsForNumericType<Vs64>("Vs64", 100);
    this->_positiveAssertionsForNumericType<Vu64>("Vu64", 100);
    
    this->_positiveAssertionsForDouble(100.0);
    this->_positiveAssertionsForString();
    this->_positiveAssertionsForDuration();
    this->_positiveAssertionsForInstant();

    this->_negativeAssertionsForNumericType<int>("int (negative test)", 100);
    this->_negativeAssertionsForNumericType<unsigned int>("unsigned int (negative test)", 100);
    this->_negativeAssertionsForNumericType<Vs8>("Vs8 (negative test)", 100);
    this->_negativeAssertionsForNumericType<Vu8>("Vu8 (negative test)", 100);
    this->_negativeAssertionsForNumericType<Vs16>("Vs16 (negative test)", 100);
    this->_negativeAssertionsForNumericType<Vu16>("Vu16 (negative test)", 100);
    this->_negativeAssertionsForNumericType<Vs32>("Vs32 (negative test)", 100);
    this->_negativeAssertionsForNumericType<Vu32>("Vu32 (negative test)", 100);
    this->_negativeAssertionsForNumericType<Vs64>("Vs64 (negative test)", 100);
    this->_negativeAssertionsForNumericType<Vu64>("Vu64 (negative test)", 100);

    this->_negativeAssertionsForDouble(100.0);
    this->_negativeAssertionsForString();
    this->_negativeAssertionsForDuration();
    this->_negativeAssertionsForInstant();
    
#endif /* VASSERTUNIT_IS_NOT_USEFUL */
    }

#ifndef VASSERTUNIT_IS_NOT_USEFUL

// These macros help to perform the assertion, and log a unit test success message containing that assertion as a string, without having to repeat it by hand.
// This does the (positive) assertion and logs success if we reach the following statement.
#define TEST_POSITIVE_ASSERTION_CALL(statement) \
    do { \
        statement; \
        VString statementString(#statement); \
        this->test(true, VSTRING_FORMAT("%s: %s", dataTypeName.chars(), statementString.chars())); \
    } while (false)

// This does the (negative) assertion in a try/catch block, and logs failure if we reach the next statement, or logs success if we land in the catch block as desired.
#define TEST_NEGATIVE_ASSERTION_CALL(statement) \
    do { \
        VString statementString(#statement); \
        try { \
            statement; \
            this->test(false, VSTRING_FORMAT("%s: %s", dataTypeName.chars(), statementString.chars())); \
        } catch (const VStackTraceException&) { \
            this->test(true, VSTRING_FORMAT("%s: %s", dataTypeName.chars(), statementString.chars())); \
        } \
    } while (false)

void VAssertUnit::_positiveAssertionsForDouble(VDouble testValue)
    {
    const VString dataTypeName("double");
    const VDouble i = testValue;
    
    TEST_POSITIVE_ASSERTION_CALL(VASSERT(VGeometry::equal(i, testValue)));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_VALUE(VGeometry::equal(i, testValue), i, VSTRING_DOUBLE(i)));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_EQUAL(i, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_NON_ZERO(i));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_NOT_EQUAL(i, testValue-10.0));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN(i, testValue+10.0));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(i, testValue+1.0));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(i, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN(i, testValue-10.0));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(i, testValue-1.0));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(i, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_IN_RANGE(i, testValue-5.0, testValue+5.0));
    }

#define TEST_STRING_LITERAL_VALUE "hello"
void VAssertUnit::_positiveAssertionsForString()
    {
    const VString dataTypeName("string");
    const VString testValue(TEST_STRING_LITERAL_VALUE);
    
    TEST_POSITIVE_ASSERTION_CALL(VASSERT(testValue == TEST_STRING_LITERAL_VALUE));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_VALUE(testValue == TEST_STRING_LITERAL_VALUE, testValue, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_EQUAL(testValue, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_EQUAL(TEST_STRING_LITERAL_VALUE, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_EQUAL(testValue, TEST_STRING_LITERAL_VALUE));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_NOT_EQUAL(testValue, "wrong value"));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_NOT_EQUAL("wrong value", testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN("aaa", testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN(testValue, "zzz"));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL("aaa", testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(testValue, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(testValue, "zzz"));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN(testValue, "aaa"));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN("zzz", testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(testValue, "aaa"));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(testValue, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL("zzz", testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_IN_RANGE(testValue, "aaa", "zzz"));
    }

void VAssertUnit::_positiveAssertionsForDuration()
    {
    const VString dataTypeName("duration");
    const VDuration testValue(VDuration::HOUR());
    
    TEST_POSITIVE_ASSERTION_CALL(VASSERT(testValue == VDuration::HOUR()));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT(testValue == 60 * VDuration::MINUTE()));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT(testValue == 60 * 60 * VDuration::SECOND()));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_EQUAL(testValue, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_EQUAL(VDuration::HOUR(), testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_EQUAL(testValue, VDuration::HOUR()));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_NOT_EQUAL(testValue, VDuration::MINUTE()));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_NOT_EQUAL(VDuration::MINUTE(), testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN(VDuration::MINUTE(), testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN(testValue, VDuration::DAY()));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(VDuration::MINUTE(), testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(VDuration::HOUR(), testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(testValue, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(testValue, VDuration::HOUR()));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(testValue, VDuration::DAY()));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN(testValue, VDuration::MINUTE()));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN(VDuration::DAY(), testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(testValue, VDuration::MINUTE()));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(testValue, VDuration::HOUR()));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(testValue, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(VDuration::HOUR(), testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(VDuration::DAY(), testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_IN_RANGE(testValue, VDuration::MINUTE(), VDuration::DAY()));
    }

void VAssertUnit::_positiveAssertionsForInstant()
    {
    const VString dataTypeName("instant");
    const VInstant now;
    const VInstant testValue = now;
    const VInstant pastValue = testValue - VDuration::HOUR();
    const VInstant futureValue = testValue + VDuration::HOUR();
    
    TEST_POSITIVE_ASSERTION_CALL(VASSERT(testValue == now));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_EQUAL(testValue, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_EQUAL(now, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_EQUAL(testValue, now));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_NOT_EQUAL(testValue, pastValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_NOT_EQUAL(pastValue, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN(pastValue, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN(testValue, futureValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(pastValue, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(now, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(testValue, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(testValue, now));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(testValue, futureValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN(testValue, pastValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN(futureValue, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(testValue, pastValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(testValue, now));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(testValue, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(now, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(futureValue, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_IN_RANGE(testValue, pastValue, futureValue));
    }

void VAssertUnit::_negativeAssertionsForDouble(VDouble testValue)
    {
    const VString dataTypeName("double (negative test)");
    const VDouble i = testValue + 100.0;
    
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT(VGeometry::equal(i, testValue)));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_VALUE(VGeometry::equal(i, testValue), i, VSTRING_DOUBLE(i)));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_EQUAL(i, testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_NON_ZERO(0.0));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_NOT_EQUAL(i, testValue+100.0));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_LESS_THAN(i, testValue-200.0));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(i, testValue-200.0));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN(i, testValue+200.0));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(i, testValue+200.0));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_IN_RANGE(i, testValue, testValue+5.0));
    }

#define SAME_STRING_LITERAL_VALUE "hello"
#define DIFFERENT_STRING_LITERAL_VALUE "other"
void VAssertUnit::_negativeAssertionsForString()
    {
    const VString dataTypeName("string (negative test)");
    const VString testValue(SAME_STRING_LITERAL_VALUE);
    const VString differentValue(DIFFERENT_STRING_LITERAL_VALUE);
    
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT(testValue == DIFFERENT_STRING_LITERAL_VALUE));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_VALUE(testValue == DIFFERENT_STRING_LITERAL_VALUE, testValue, testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_EQUAL(testValue, differentValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_EQUAL(DIFFERENT_STRING_LITERAL_VALUE, testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_EQUAL(testValue, DIFFERENT_STRING_LITERAL_VALUE));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_NOT_EQUAL(testValue, SAME_STRING_LITERAL_VALUE));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_NOT_EQUAL(SAME_STRING_LITERAL_VALUE, testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_LESS_THAN("zzz", testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_LESS_THAN(testValue, "aaa"));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL("zzz", testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(testValue, "aaa"));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN(testValue, "zzz"));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN("aaa", testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(testValue, "zzz"));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL("aaa", testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_IN_RANGE(testValue, "aaa", "bbb"));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_IN_RANGE(testValue, "yyy", "zzz"));
    }

void VAssertUnit::_negativeAssertionsForDuration()
    {
    const VString dataTypeName("duration (negative test)");
    const VDuration testValue(VDuration::HOUR());
    
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT(testValue == VDuration::MINUTE()));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_EQUAL(testValue, VDuration::MINUTE()));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_EQUAL(VDuration::MINUTE(), testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_NOT_EQUAL(testValue, VDuration::HOUR()));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_NOT_EQUAL(VDuration::HOUR(), testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_LESS_THAN(VDuration::HOUR(), testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_LESS_THAN(testValue, VDuration::HOUR()));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(VDuration::DAY(), testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(testValue, VDuration::MINUTE()));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN(testValue, VDuration::DAY()));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN(VDuration::MINUTE(), testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(testValue, VDuration::DAY()));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(VDuration::MINUTE(), testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_IN_RANGE(testValue, 2 * VDuration::HOUR(), VDuration::DAY()));
    }

void VAssertUnit::_negativeAssertionsForInstant()
    {
    const VString dataTypeName("instant (negative test)");
    const VInstant now;
    const VInstant testValue = now;
    const VInstant pastValue = testValue - VDuration::HOUR();
    const VInstant futureValue = testValue + VDuration::HOUR();
    
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT(testValue == futureValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_EQUAL(testValue, futureValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_EQUAL(now, futureValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_EQUAL(futureValue, now));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_NOT_EQUAL(testValue, now));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_NOT_EQUAL(now, testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_LESS_THAN(futureValue, testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_LESS_THAN(testValue, pastValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(futureValue, testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(testValue, pastValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN(testValue, futureValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN(pastValue, testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(testValue, futureValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(pastValue, testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_IN_RANGE(testValue, futureValue, futureValue + VDuration::MINUTE()));
    }

// Compilers will legitimately warn about signed/unsigned comparisons here, but we are intentionally
// performing them as part of the test suite. For VC++ we can disable the warnings locally with a
// VC++-specific pragma. GCC has no such feature, so I have separated the relevant tests (those that
// use an (int) cast) and left them out of the Unix build. User code that tries to perform such
// comparisons will warn legitimately and should be corrected.
#ifdef VCOMPILER_MSVC
#pragma warning(disable: 4018)
#endif

template <class T>
void VAssertUnit::_positiveAssertionsForNumericType(const VString& dataTypeName, T testValue)
    {
    const T i = testValue;
    
    TEST_POSITIVE_ASSERTION_CALL(VASSERT(i == testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT(i == (T)testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_VALUE(i == testValue, i, VSTRING_INT((int)i)));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_EQUAL(i, testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_EQUAL(i, (T)testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_NON_ZERO(i));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_NOT_EQUAL(i, (T)(testValue-10)));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN(i, (T)(testValue+10)));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(i, (T)(testValue+1)));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(i, (T)testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN(i, (T)(testValue-10)));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(i, (T)(testValue-1)));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(i, (T)testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_IN_RANGE(i, (T)(testValue-5), (T)(testValue+5)));

#ifndef VPLATFORM_UNIX // GCC Linux correctly warns on these when T is an unsigned type.
    TEST_POSITIVE_ASSERTION_CALL(VASSERT(i == (int)testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_EQUAL(i, (int)testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_NOT_EQUAL(i, (int)(testValue-10)));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN(i, (int)(testValue+10)));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(i, (int)(testValue+1)));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(i, (int)testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN(i, (int)(testValue-10)));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(i, (int)(testValue-1)));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(i, (int)testValue));
    TEST_POSITIVE_ASSERTION_CALL(VASSERT_IN_RANGE(i, (int)(testValue-5), (int)(testValue+5)));
#endif /* if not unix */
    }

template <class T>
void VAssertUnit::_negativeAssertionsForNumericType(const VString& dataTypeName, T testValue)
    {
    const T i = testValue;
    const T x = i + 5; // a value that should fail to test equal
    
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT(x == testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT(x == (T)testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_VALUE(x == testValue, x, VSTRING_INT((int)x)));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_EQUAL(x, testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_EQUAL(x, (T)testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_NON_ZERO(0));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_NOT_EQUAL(i, (T)i));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_LESS_THAN(i, (T)(testValue-10)));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(i, (T)(testValue-1)));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN(i, (T)(testValue+10)));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(i, (T)(testValue+1)));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(i, (T)testValue+10));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_IN_RANGE(i, (T)(testValue+5), (T)(testValue+10)));

#ifndef VPLATFORM_UNIX // GCC Linux correctly warns on these when T is an unsigned type.
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT(x  == (int)testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_EQUAL(x, (int)testValue));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_NOT_EQUAL(i, (int)i));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_LESS_THAN(i, (int)(testValue-10)));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_LESS_THAN_OR_EQUAL(i, (int)(testValue-1)));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN(i, (int)(testValue+10)));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(i, (int)(testValue+1)));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_GREATER_THAN_OR_EQUAL(i, (int)testValue+10));
    TEST_NEGATIVE_ASSERTION_CALL(VASSERT_IN_RANGE(i, (int)(testValue+5), (int)(testValue+10)));
#endif /* if not unix */
    }

#ifdef VCOMPILER_MSVC
#pragma warning(default: 4018)
#endif

#endif /* VASSERTUNIT_IS_NOT_USEFUL */
