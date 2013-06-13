/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

/** @file */

#include "vassert.h"

static void _failedAssert(const VString& failMessage, const char* file, int line) {
    VLOGGER_LEVEL_FILELINE(VLoggerLevel::ERROR, failMessage, file, line);
#ifdef V_ASSERT_THROWS_EXCEPTION
    throw VStackTraceException(failMessage);
#endif /* V_ASSERT_THROWS_EXCEPTION */
}

static void _failedAssertEqual(const VString& valueA, const VString& valueB, const char* expressionA, const char* expressionB, const char* file, int line) {
    const VString failMessage(VSTRING_ARGS("Equal assertion failed. (%s = %s) (%s = %s)", expressionA, valueA.chars(), expressionB, valueB.chars()), file, line);
    _failedAssert(failMessage, file, line);
}

static void _failedAssertNotEqual(const VString& commonValue, const char* expressionA, const char* expressionB, const char* file, int line) {
    const VString failMessage(VSTRING_ARGS("Not Equal assertion failed. (%s = %s = %s)", expressionA, expressionB, commonValue.chars()), file, line);
    _failedAssert(failMessage, file, line);
}

static void _failedLessOrGreaterThan(bool comparingLessThan, bool comparingOrEqualTo, const VString& valueA, const VString& valueB, const char* expressionA, const char* expressionB, const char* file, int line) {
    const VString failMessage(VSTRING_ARGS("%s%s assertion failed. (%s = %s) (%s = %s)", (comparingLessThan ? "Less Than" : "Greater Than"), (comparingOrEqualTo ? " Or Equal To" : ""), expressionA, valueA.chars(), expressionB, valueB.chars()), file, line);
    _failedAssert(failMessage, file, line);
}

static void _failedAssertRangeCheck(const VString& val, const VString& minVal, const VString& maxVal, const char* expressionVal, const char* expressionMinVal, const char* expressionMaxVal, const char* file, int line) {
    const VString failMessage(VSTRING_ARGS("Range assertion failed. (%s = %s) (MIN: %s = %s) (MAX: %s = %s)", expressionVal, val.chars(), expressionMinVal, minVal.chars(), expressionMaxVal, maxVal.chars()), file, line);
    _failedAssert(failMessage, file, line);
}

// static
void VAssert::failedAssert(const char* expression, const char* file, int line) {
    _failedAssert(VSTRING_FORMAT("Assertion failed: %s.", expression), file, line);
}

// static
void VAssert::failedAssertValue(const char* expression, const char* valName, const VString& valString, const char* file, int line) {
    _failedAssert(VSTRING_FORMAT("Assertion failed: %s (%s = %s)", expression, valName, valString.chars()), file, line);
}

// static
void VAssert::failedAssertNull(const void* p, const char* expression, const char* file, int line) {
    _failedAssert(VSTRING_FORMAT("Null assertion failed. (%s = " VSTRING_FORMATTER_PTR ")", expression, p), file, line);
}

// static
void VAssert::failedAssertNotNull(const char* expression, const char* file, int line) {
    _failedAssert(VSTRING_FORMAT("Not Null assertion failed. (%s = NULL)", expression), file, line);
}

// static
void VAssert::failedAssertZero(Vs64 i, const char* expression, const char* file, int line) {
    _failedAssert(VSTRING_FORMAT("Zero assertion failed. (%s = " VSTRING_FORMATTER_S64 ")", expression, i), file, line);
}

// static
void VAssert::failedAssertNonZero(const char* expression, const char* file, int line) {
    _failedAssert(VSTRING_FORMAT("Non-zero assertion failed. (%s = 0)", expression), file, line);
}

// static
void VAssert::failedAssertEqual(int a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_INT(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(unsigned int a, unsigned int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_UINT(a), VSTRING_UINT(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(unsigned int a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_UINT(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(const void* a, const void* b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_PTR(a), VSTRING_PTR(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(bool a, bool b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_BOOL(a), VSTRING_BOOL(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(Vs8 a, Vs8 b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_S8(a), VSTRING_S8(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(Vs8 a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_S8(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(Vu8 a, Vu8 b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_U8(a), VSTRING_U8(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(Vu8 a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_U8(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(Vs16 a, Vs16 b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_S16(a), VSTRING_S16(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(Vs16 a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_S16(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(Vu16 a, Vu16 b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_U16(a), VSTRING_U16(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(Vu16 a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_U16(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}

#ifndef Vx32_IS_xINT /* don't redefine if types are same */
// static
void VAssert::failedAssertEqual(Vs32 a, Vs32 b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_S32(a), VSTRING_S32(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(Vs32 a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_S32(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(Vu32 a, Vu32 b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_U32(a), VSTRING_U32(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(Vu32 a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_U32(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}
#endif /* not Vx32_IS_xINT */

#ifndef Vx64_IS_xINT /* don't redefine if types are same */
// static
void VAssert::failedAssertEqual(Vs64 a, Vs64 b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_S64(a), VSTRING_S64(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(Vs64 a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_S64(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(Vu64 a, Vu64 b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_U64(a), VSTRING_U64(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(Vu64 a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_U64(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}
#endif /* not Vx64_IS_xINT */

// static
void VAssert::failedAssertEqual(VDouble a, VDouble b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(VSTRING_DOUBLE(a), VSTRING_DOUBLE(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(const VString& a, const VString& b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(a, b, expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(const VString& a, const char* b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(a, b, expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(const char* a, const VString& b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(a, b, expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(const VChar& a, const VChar& b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(a, b, expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(const VChar& a, char b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(a, VString(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(const VDuration& a, const VDuration& b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(a.getDurationString(), b.getDurationString(), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(const VInstant& a, const VInstant& b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(a.getLocalString(), b.getLocalString(), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(const VColor& a, const VColor& b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(a.getCSSColor(), b.getCSSColor(), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertEqual(const VColorPair& a, const VColorPair& b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertEqual(a.getCSSColor(), b.getCSSColor(), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertNotEqual(int val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(VSTRING_INT(val), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertNotEqual(unsigned int val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(VSTRING_UINT(val), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertNotEqual(const void* val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(VSTRING_PTR(val), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertNotEqual(bool val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(VSTRING_BOOL(val), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertNotEqual(Vs8 val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(VSTRING_S8(val), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertNotEqual(Vu8 val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(VSTRING_U8(val), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertNotEqual(Vs16 val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(VSTRING_S16(val), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertNotEqual(Vu16 val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(VSTRING_U16(val), expressionA, expressionB, file, line);
}

#ifndef Vx32_IS_xINT /* don't redefine if types are same */
// static
void VAssert::failedAssertNotEqual(Vs32 val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(VSTRING_S32(val), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertNotEqual(Vu32 val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(VSTRING_U32(val), expressionA, expressionB, file, line);
}
#endif /* not Vx32_IS_xINT */

#ifndef Vx64_IS_xINT /* don't redefine if types are same */
// static
void VAssert::failedAssertNotEqual(Vs64 val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(VSTRING_S64(val), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertNotEqual(Vu64 val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(VSTRING_U64(val), expressionA, expressionB, file, line);
}
#endif /* not Vx64_IS_xINT */

// static
void VAssert::failedAssertNotEqual(VDouble val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(VSTRING_DOUBLE(val), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertNotEqual(const VString& val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(val, expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertNotEqual(const VChar& val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(val, expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertNotEqual(const VDuration& val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(val.getDurationString(), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertNotEqual(const VInstant& val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(val.getLocalString(), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertNotEqual(const VColor& val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(val.getCSSColor(), expressionA, expressionB, file, line);
}

// static
void VAssert::failedAssertNotEqual(const VColorPair& val, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedAssertNotEqual(val.getCSSColor(), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, int a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_INT(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, unsigned int a, unsigned int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_UINT(a), VSTRING_UINT(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, unsigned int a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_UINT(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vs8 a, Vs8 b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_S8(a), VSTRING_S8(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vs8 a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_S8(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vu8 a, Vu8 b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_U8(a), VSTRING_U8(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vu8 a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_U8(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vs16 a, Vs16 b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_S16(a), VSTRING_S16(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vs16 a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_S16(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vu16 a, Vu16 b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_U16(a), VSTRING_U16(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vu16 a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_U16(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}

#ifndef Vx32_IS_xINT /* don't redefine if types are same */
// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vs32 a, Vs32 b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_S32(a), VSTRING_S32(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vs32 a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_S32(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vu32 a, Vu32 b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_U32(a), VSTRING_U32(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vu32 a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_U32(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}
#endif /* not Vx32_IS_xINT */

#ifndef Vx64_IS_xINT /* don't redefine if types are same */
// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vs64 a, Vs64 b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_S64(a), VSTRING_S64(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vs64 a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_S64(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vu64 a, Vu64 b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_U64(a), VSTRING_U64(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, Vu64 a, int b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_U64(a), VSTRING_INT(b), expressionA, expressionB, file, line);
}
#endif /* not Vx64_IS_xINT */

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, VDouble a, VDouble b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, VSTRING_DOUBLE(a), VSTRING_DOUBLE(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, const VString& a, const VString& b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, a, b, expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, const VString& a, const char* b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, a, b, expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, const char* a, const VString& b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, a, b, expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, const VChar& a, const VChar& b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, a, b, expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, const VChar& a, char b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, a, VString(b), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, const VDuration& a, const VDuration& b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, a.getDurationString(), b.getDurationString(), expressionA, expressionB, file, line);
}

// static
void VAssert::failedLessGreaterComparison(bool comparingLessThan, bool comparingOrEqualTo, const VInstant& a, const VInstant& b, const char* expressionA, const char* expressionB, const char* file, int line) {
    _failedLessOrGreaterThan(comparingLessThan, comparingOrEqualTo, a.getLocalString(), b.getLocalString(), expressionA, expressionB, file, line);
}

// static
void VAssert::failedRangeCheck(int val, int minVal, int maxVal, const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line) {
    _failedAssertRangeCheck(VSTRING_INT(val), VSTRING_INT(minVal), VSTRING_INT(maxVal), valExpression, minValExpression, maxValExpression, file, line);
}

// static
void VAssert::failedRangeCheck(unsigned int val, unsigned int minVal, unsigned int maxVal, const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line) {
    _failedAssertRangeCheck(VSTRING_UINT(val), VSTRING_UINT(minVal), VSTRING_UINT(maxVal), valExpression, minValExpression, maxValExpression, file, line);
}

// static
void VAssert::failedRangeCheck(unsigned int val, int minVal, int maxVal, const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line) {
    _failedAssertRangeCheck(VSTRING_UINT(val), VSTRING_INT(minVal), VSTRING_INT(maxVal), valExpression, minValExpression, maxValExpression, file, line);
}

#ifndef Vx32_IS_xINT /* don't redefine if types are same */
// static
void VAssert::failedRangeCheck(Vs32 val, Vs32 minVal, Vs32 maxVal, const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line) {
    _failedAssertRangeCheck(VSTRING_S32(val), VSTRING_S32(minVal), VSTRING_S32(maxVal), valExpression, minValExpression, maxValExpression, file, line);
}

// static
void VAssert::failedRangeCheck(Vs32 val, int minVal, int maxVal, const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line) {
    _failedAssertRangeCheck(VSTRING_S32(val), VSTRING_INT(minVal), VSTRING_INT(maxVal), valExpression, minValExpression, maxValExpression, file, line);
}

// static
void VAssert::failedRangeCheck(Vu32 val, Vu32 minVal, Vu32 maxVal, const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line) {
    _failedAssertRangeCheck(VSTRING_U32(val), VSTRING_U32(minVal), VSTRING_U32(maxVal), valExpression, minValExpression, maxValExpression, file, line);
}

// static
void VAssert::failedRangeCheck(Vu32 val, int minVal, int maxVal, const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line) {
    _failedAssertRangeCheck(VSTRING_U32(val), VSTRING_INT(minVal), VSTRING_INT(maxVal), valExpression, minValExpression, maxValExpression, file, line);
}
#endif /* not Vx32_IS_xINT */

#ifndef Vx64_IS_xINT /* don't redefine if types are same */
// static
void VAssert::failedRangeCheck(Vs64 val, Vs64 minVal, Vs64 maxVal, const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line) {
    _failedAssertRangeCheck(VSTRING_S64(val), VSTRING_S64(minVal), VSTRING_S64(maxVal), valExpression, minValExpression, maxValExpression, file, line);
}

// static
void VAssert::failedRangeCheck(Vs64 val, int minVal, int maxVal, const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line) {
    _failedAssertRangeCheck(VSTRING_S64(val), VSTRING_INT(minVal), VSTRING_INT(maxVal), valExpression, minValExpression, maxValExpression, file, line);
}

// static
void VAssert::failedRangeCheck(Vu64 val, Vu64 minVal, Vu64 maxVal, const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line) {
    _failedAssertRangeCheck(VSTRING_U64(val), VSTRING_U64(minVal), VSTRING_U64(maxVal), valExpression, minValExpression, maxValExpression, file, line);
}

// static
void VAssert::failedRangeCheck(Vu64 val, int minVal, int maxVal, const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line) {
    _failedAssertRangeCheck(VSTRING_U64(val), VSTRING_INT(minVal), VSTRING_INT(maxVal), valExpression, minValExpression, maxValExpression, file, line);
}
#endif /* not Vx64_IS_xINT */

// static
void VAssert::failedRangeCheck(VDouble val, VDouble minVal, VDouble maxVal, const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line) {
    _failedAssertRangeCheck(VSTRING_DOUBLE(val), VSTRING_DOUBLE(minVal), VSTRING_DOUBLE(maxVal), valExpression, minValExpression, maxValExpression, file, line);
}

// static
void VAssert::failedRangeCheck(const VString& val, const VString& minVal, const VString& maxVal, const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line) {
    _failedAssertRangeCheck(val, minVal, maxVal, valExpression, minValExpression, maxValExpression, file, line);
}

// static
void VAssert::failedRangeCheck(const VChar& val, const VChar& minVal, const VChar& maxVal, const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line) {
    _failedAssertRangeCheck(val, minVal, maxVal, valExpression, minValExpression, maxValExpression, file, line);
}

// static
void VAssert::failedRangeCheck(const VDuration& val, const VDuration& minVal, const VDuration& maxVal, const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line) {
    _failedAssertRangeCheck(val.getDurationString(), minVal.getDurationString(), maxVal.getDurationString(), valExpression, minValExpression, maxValExpression, file, line);
}

// static
void VAssert::failedRangeCheck(const VInstant& val, const VInstant& minVal, const VInstant& maxVal, const char* valExpression, const char* minValExpression, const char* maxValExpression, const char* file, int line) {
    _failedAssertRangeCheck(val.getLocalString(), minVal.getLocalString(), maxVal.getLocalString(), valExpression, minValExpression, maxValExpression, file, line);
}

