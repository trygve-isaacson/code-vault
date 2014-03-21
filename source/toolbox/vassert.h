/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vassert_h
#define vassert_h

/** @file */

#include "vtypes.h"

#include "vstring.h"
#include "vcodepoint.h"
#include "vchar.h"
#include "vinstant.h"
#include "vcolor.h"

/*
These macros log an error and throw a VStackTraceException containing a stack crawl if the assertion fails.

Note: the "do { ... } while (false)" macro syntax is the preferred way to ensure proper expansion regardless
of caller's conditional branching syntax.

We separate out the testing of the assertion from the processing of the assertion failure so that the label
supplied to the macro does not have to be evaluated in the success case. For example, a complicated string
formatting operation to form the label will have no overhead until actual use in a failed assertion.

Usage Notes

The preprocessor symbol V_ASSERT_ENABLED determines whether the macros do anything at all.
By default it is on if V_DEBUG is on. Normally this means for debug builds.
However, if you #define V_ASSERT_ENABLED 1 then it will be on all the time. This is actually recommended.

The preprocessor symbol VAULT_THROW_ON_ASSERTION_FAILURE determines whether a failed assertion
throws a stack trace exception in addition to logging an error.

You can assert any boolean condition with the VASSERT() macro. However, the more specific macros will yield
more informative output when the assertion fails. For example, compare the output on failure of these two
similar tests:

VASSERT(nameOfOwner == nameOfParticipant);
  Assertion failed: nameOfOwner == nameOfParticipant

VASSERT_EQUAL(nameOfOwner, nameOfParticipant);
  Equal assertion failed. (nameOfOwner = Flavio) (nameOfParticipant = Fernando)

You can use VASSERT_VALUE if you have a complex boolean to evaluate and want to supply a value string or other
information to be included if the assertion fails, for example:
VASSERT_VALUE(x == 1 || x == 20 || x > 50, x, VSTRING_INT(x));
  Assertion failed: x == 1 || x == 20 || x > 50 (x = 42)
Arg 1 is the boolean expression to test.
Arg 2 gets stringified to form the label. (x)
Arg 3 is a string used for the value. (42)

Another example, with good/better/best output:

VASSERT(balance < 100);
  Assertion failed: balance < 100

VASSERT_LESS_THAN(balance, 100);
  Less Than assertion failed. (balance = 250) (100 = 100)

VASSERT_LESS_THAN(balance, MAX_BALANCE);
  Less Than assertion failed. (balance = 250) (MAX_BALANCE = 100)

*/

#ifdef V_ASSERT_ACTIVE
    #define VASSERT(expression)                             do { if (!(expression))                     VAssert::failedAssert(#expression, __FILE__, __LINE__); } while (false)
    #define VASSERT_VALUE(expression, valName, valString)   do { if (!(expression))                     VAssert::failedAssertValue(#expression, #valName, valString, __FILE__, __LINE__); } while (false)
    #define VASSERT_NULL(pointer)                           do { if (pointer != NULL)                   VAssert::failedAssertNull(pointer, #pointer, __FILE__, __LINE__); } while (false)
    #define VASSERT_NOT_NULL(pointer)                       do { if (pointer == NULL)                   VAssert::failedAssertNotNull(#pointer, __FILE__, __LINE__); } while (false)
    #define VASSERT_ZERO(i)                                 do { if (i != 0)                            VAssert::failedAssertZero((Vs64)i, #i, __FILE__, __LINE__); } while (false)
    #define VASSERT_NON_ZERO(i)                             do { if (i == 0)                            VAssert::failedAssertNonZero(#i, __FILE__, __LINE__); } while (false)
    #define VASSERT_EQUAL(a, b)                             do { if (a != b)                            VAssert::failedAssertEqual(a, b, #a, #b, __FILE__, __LINE__); } while (false)
    #define VASSERT_NOT_EQUAL(a, b)                         do { if (a == b)                            VAssert::failedAssertNotEqual(a, #a, #b, __FILE__, __LINE__); } while (false)
    #define VASSERT_LESS_THAN(a, b)                         do { if (!(a < b))                          VAssert::failedLessGreaterComparison(true, false, a, b, #a, #b, __FILE__, __LINE__); } while (false)
    #define VASSERT_LESS_THAN_OR_EQUAL(a, b)                do { if (!(a <= b))                         VAssert::failedLessGreaterComparison(true, true, a, b, #a, #b, __FILE__, __LINE__); } while (false)
    #define VASSERT_GREATER_THAN(a, b)                      do { if (!(a > b))                          VAssert::failedLessGreaterComparison(false, false, a, b, #a, #b, __FILE__, __LINE__); } while (false)
    #define VASSERT_GREATER_THAN_OR_EQUAL(a, b)             do { if (!(a >= b))                         VAssert::failedLessGreaterComparison(false, true, a, b, #a, #b, __FILE__, __LINE__); } while (false)
    #define VASSERT_IN_RANGE(i, minVal, maxVal)             do { if (!((i >= minVal) && (i <= maxVal))) VAssert::failedRangeCheck(i, minVal, maxVal, #i, #minVal, #maxVal, __FILE__, __LINE__); } while (false)
#else
    #define VASSERT(expression)                             ((void) 0)
    #define VASSERT_VALUE(expression, valName, valString)   ((void) 0)
    #define VASSERT_NULL(pointer)                           ((void) 0)
    #define VASSERT_NOT_NULL(pointer)                       ((void) 0)
    #define VASSERT_ZERO(i)                                 ((void) 0)
    #define VASSERT_NON_ZERO(i)                             ((void) 0)
    #define VASSERT_EQUAL(a, b)                             ((void) 0)
    #define VASSERT_NOT_EQUAL(a, b)                         ((void) 0)
    #define VASSERT_LESS_THAN(a, b)                         ((void) 0)
    #define VASSERT_LESS_THAN_OR_EQUAL(a, b)                ((void) 0)
    #define VASSERT_GREATER_THAN(a, b)                      ((void) 0)
    #define VASSERT_GREATER_THAN_OR_EQUAL(a, b)             ((void) 0)
    #define VASSERT_IN_RANGE(i, minVal, maxVal)             ((void) 0)
#endif

/**
This static helper class provides the various overloaded assertion functions that we call from our VASSERT
macros.
*/
class VAssert {
    public:

        // Note: The reason that 'expressions' are passed as raw (const char*) rather than (const VString&) is that they are always formed
        // by the # stringifier from one of the macros defined above, are therefore compile-time constants, and constructing a VString to wrap
        // each of them as we pass them along to the string formatter would be needlessly wasteful.

        // For convenience, each numeric non-int version of failedAssertEqual(), failedLessGreaterComparison(), and failedRangeCheck() has
        // an alternate where the values other than a are declared as int. This allows you to assert a non-int variable to be some int value,
        // e.g. VASSERT_EQUAL(x, 5) where x is not an int. Otherwise, you'd have to carefully cast int constants to particular types.

        static void failedAssert(const char* expression, const char* file, int line);
        static void failedAssertValue(const char* expression, const char* valName, const VString& valString, const char* file, int line);
        static void failedAssertNull(const void* p, const char* expression, const char* file, int line);
        static void failedAssertNotNull(const char* expression, const char* file, int line);
        static void failedAssertZero(Vs64 i, const char* expression, const char* file, int line); // we always upcast to Vs64
        static void failedAssertNonZero(const char* expression, const char* file, int line);

        static void failedAssertEqual(int a, int b,                             const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(unsigned int a, unsigned int b,           const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(unsigned int a, int b,                    const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(const void* a, const void* b,             const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(bool a, bool b,                           const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(Vs8 a, Vs8 b,                             const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(Vs8 a, int b,                             const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(Vu8 a, Vu8 b,                             const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(Vu8 a, int b,                             const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(Vs16 a, Vs16 b,                           const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(Vs16 a, int b,                            const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(Vu16 a, Vu16 b,                           const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(Vu16 a, int b,                            const char* expressionA, const char* expressionB, const char* file, int line);

#ifndef Vx32_IS_xINT /* don't redefine if types are same */
        static void failedAssertEqual(Vs32 a, Vs32 b,                           const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(Vs32 a, int b,                            const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(Vu32 a, Vu32 b,                           const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(Vu32 a, int b,                            const char* expressionA, const char* expressionB, const char* file, int line);
#endif /* not Vx32_IS_xINT */

#ifndef Vx64_IS_xINT /* don't redefine if types are same */
        static void failedAssertEqual(Vs64 a, Vs64 b,                           const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(Vs64 a, int b,                            const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(Vu64 a, Vu64 b,                           const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(Vu64 a, int b,                            const char* expressionA, const char* expressionB, const char* file, int line);
#endif /* not Vx64_IS_xINT */

        static void failedAssertEqual(VDouble a, VDouble b,                     const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(const VString& a, const VString& b,       const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(const VString& a, const char* b,          const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(const char* a, const VString& b,          const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(const VCodePoint& a, const VCodePoint& b, const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(const VChar& a, const VChar& b,           const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(const VChar& a, char b,                   const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(const VDuration& a, const VDuration& b,   const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(const VInstant& a, const VInstant& b,     const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(const VColor& a, const VColor& b,         const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertEqual(const VColorPair& a, const VColorPair& b, const char* expressionA, const char* expressionB, const char* file, int line);

        static void failedAssertNotEqual(int val,               const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertNotEqual(unsigned int val,      const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertNotEqual(const void* val,       const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertNotEqual(bool val,              const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertNotEqual(Vs8 val,               const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertNotEqual(Vu8 val,               const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertNotEqual(Vs16 val,              const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertNotEqual(Vu16 val,              const char* expressionA, const char* expressionB, const char* file, int line);

#ifndef Vx32_IS_xINT /* don't redefine if types are same */
        static void failedAssertNotEqual(Vs32 val,              const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertNotEqual(Vu32 val,              const char* expressionA, const char* expressionB, const char* file, int line);
#endif /* not Vx32_IS_xINT */

#ifndef Vx64_IS_xINT /* don't redefine if types are same */
        static void failedAssertNotEqual(Vs64 val,              const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertNotEqual(Vu64 val,              const char* expressionA, const char* expressionB, const char* file, int line);
#endif /* not Vx64_IS_xINT */

        static void failedAssertNotEqual(VDouble val,           const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertNotEqual(const VString& val,    const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertNotEqual(const VCodePoint& val, const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertNotEqual(const VChar& val,      const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertNotEqual(const VDuration& val,  const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertNotEqual(const VInstant& val,   const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertNotEqual(const VColor& val,     const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedAssertNotEqual(const VColorPair& val, const char* expressionA, const char* expressionB, const char* file, int line);

        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, int a, int b,                              const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, unsigned int a, unsigned int b,            const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, unsigned int a, int b,                     const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vs8 a, Vs8 b,                              const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vs8 a, int b,                              const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vu8 a, Vu8 b,                              const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vu8 a, int b,                              const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vs16 a, Vs16 b,                            const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vs16 a, int b,                             const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vu16 a, Vu16 b,                            const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vu16 a, int b,                             const char* expressionA, const char* expressionB, const char* file, int line);

#ifndef Vx32_IS_xINT /* don't redefine if types are same */
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vs32 a, Vs32 b,                            const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vs32 a, int b,                             const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vu32 a, Vu32 b,                            const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vu32 a, int b,                             const char* expressionA, const char* expressionB, const char* file, int line);
#endif /* not Vx32_IS_xINT */

#ifndef Vx64_IS_xINT /* don't redefine if types are same */
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vs64 a, Vs64 b,                            const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vs64 a, int b,                             const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vu64 a, Vu64 b,                            const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vu64 a, int b,                             const char* expressionA, const char* expressionB, const char* file, int line);
#endif /* not Vx64_IS_xINT */

        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, VDouble a, VDouble b,                      const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, const VString& a, const VString& b,        const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, const VString& a, const char* b,           const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, const char* a, const VString& b,           const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, const VCodePoint& a, const VCodePoint& b,  const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, const VChar& a, const VChar& b,            const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, const VChar& a, char b,                    const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, const VDuration& a, const VDuration& b,    const char* expressionA, const char* expressionB, const char* file, int line);
        static void failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, const VInstant& a, const VInstant& b,      const char* expressionA, const char* expressionB, const char* file, int line);

        static void failedRangeCheck(int val, int minVal, int maxVal,                                           const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line);
        static void failedRangeCheck(unsigned int val, unsigned int minVal, unsigned int maxVal,                const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line);
        static void failedRangeCheck(unsigned int val, int minVal, int maxVal,                                  const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line);
        // Note: 8- and 16-bit overloads seem to be unnecessary due to up-conversion.

#ifndef Vx32_IS_xINT /* don't redefine if types are same */
        static void failedRangeCheck(Vs32 val, Vs32 minVal, Vs32 maxVal,                                        const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line);
        static void failedRangeCheck(Vs32 val, int minVal, int maxVal,                                          const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line);
        static void failedRangeCheck(Vu32 val, Vu32 minVal, Vu32 maxVal,                                        const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line);
        static void failedRangeCheck(Vu32 val, int minVal, int maxVal,                                          const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line);
#endif /* not Vx32_IS_xINT */

#ifndef Vx64_IS_xINT /* don't redefine if types are same */
        static void failedRangeCheck(Vs64 val, Vs64 minVal, Vs64 maxVal,                                        const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line);
        static void failedRangeCheck(Vs64 val, int minVal, int maxVal,                                          const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line);
        static void failedRangeCheck(Vu64 val, Vu64 minVal, Vu64 maxVal,                                        const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line);
        static void failedRangeCheck(Vu64 val, int minVal, int maxVal,                                          const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line);
#endif /* not Vx64_IS_xINT */

        static void failedRangeCheck(VDouble val, VDouble minVal, VDouble maxVal,                               const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line);
        static void failedRangeCheck(const VString& val, const VString& minVal, const VString& maxVal,          const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line);
        static void failedRangeCheck(const VCodePoint& val, const VCodePoint& minVal, const VCodePoint& maxVal, const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line);
        static void failedRangeCheck(const VChar& val, const VChar& minVal, const VChar& maxVal,                const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line);
        static void failedRangeCheck(const VDuration& val, const VDuration& minVal, const VDuration& maxVal,    const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line);
        static void failedRangeCheck(const VInstant& val, const VInstant& minVal, const VInstant& maxVal,       const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line);

    private:

        VAssert() {}
        ~VAssert() {}
};

#endif /* vassert_h */
