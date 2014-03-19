/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vunit_h
#define vunit_h

/** @file */

/*
Not trying to build an xUnit facility here, nor interface with one,
just defining a trivial unit test base class from which I can derive
unit tests and regression tests for various testable classes and APIs
in the Vault.
*/

#include "vtypes.h"
#include "vstring.h"    // not needed by this header, but needed by all subclasses
#include "vinstant.h"
#include "vchar.h"
#include "vcolor.h"

class VLogAppender;
class VLogger;
class VTextIOStream;

/**
VTestInfo is used internally by VUnit to hold a single test's info and
result.
*/
class VTestInfo {
    public:

        VTestInfo(bool success, const VString& description, const VDuration& duration);
        ~VTestInfo() {}

        bool        mSuccess;       ///< True if the test succeeded.
        VString     mDescription;   ///< The text description or name of the test.
        VDuration   mDuration;      ///< The number of milliseconds it took to run the test.
};

typedef std::vector<VTestInfo> TestInfoVector;

class VUnitOutputWriter;
typedef std::vector<VUnitOutputWriter*> VUnitOutputWriterList;

typedef std::vector<VLogAppender*> VUnitLogAppenderList;

/**
VUnitOutputWriter provides an abstract API for providing test results
in various formats. Provided concrete implementations are at the bottom of
this file.
Because most implementations will need to keep track of the current suite
name, current test name, and the current suite's accumulated test case
info, we store that here in the base class.
Output should be written by calling mLogger.rawLog(); using a
VLogger and rawLog() allows good flexibility because the logger may write
to standard out, or to a file, or whatever it wants to.
*/
class VUnitOutputWriter {
    public:

        /**
        This utility will set up writers and loggers according to a set of
        command line arguments:
          -vunit-out <type> <file>
          The <type> argument determines the type of VUnitOutputWriter instantiated.
            text | junit | tc | tcstatus
          The <file> argument is the file path it creates and writes to.
            "stdout" writes to std::cout instead of a file.
          Multiple sets of "-vunit-out <type> <file>" will cause multiple writers to
          be created and written to during unit tests.
        @param  args the command line arguments
        @param  writers a vector of VUnitOutputWriter* to which this function adds
            the caller is responsible for deleting these objects when done with unit tests
        @param  appenders a vector of VLogAppender* to which this function appends
            the caller is responsible for deleting these objects when done with unit tests
        */
        static void createOutputWriters(const VStringVector& args, VUnitOutputWriterList& writers, VUnitLogAppenderList& appenders);

        VUnitOutputWriter(VLogAppender& outputAppender);
        virtual ~VUnitOutputWriter() {}

        virtual void testSuitesBegin() = 0;
        virtual void testSuiteBegin(const VString& testSuiteName) = 0;
        virtual void testSuiteStatusMessage(const VString& message) = 0;
        virtual void testCaseBegin(const VString& testCaseName) = 0;
        virtual void testCaseEnd(const VTestInfo& testInfo) = 0;
        virtual void testSuiteEnd() = 0;
        virtual void testSuitesEnd() = 0;

    protected:

        // The concrete classes must call these helper functions
        // from their corresponding interface implementations, to manage the data.
        void _testSuitesBegin();
        void _testSuiteBegin(const VString& testSuiteName);
        void _testCaseBegin(const VString& testCaseName);
        void _testCaseEnd(const VTestInfo& testInfo);
        void _testSuiteEnd();

        VLogAppender& mLogAppender;
        VInstant mTestSuitesStartTime;
        int mTotalNumSuccesses;
        int mTotalNumErrors;
        VString mCurrentTestSuiteName;
        TestInfoVector mCurrentTestSuiteResults;
        int mCurrentTestSuiteNumSuccesses;
        int mCurrentTestSuiteNumErrors;
        VInstant mCurrentTestSuiteStartTime;
        VInstant mCurrentTestSuiteEndTime;
        VString mCurrentTestCaseName;
        VInstant mCurrentTestCaseStartTime;
        VInstant mCurrentTestCaseEndTime;
        VStringVector mFailedTestSuiteNames;

    private:

        // Prevent copy construction and assignment since there is no provision for cloning this class.
        VUnitOutputWriter(const VUnitOutputWriter& other);
        VUnitOutputWriter& operator=(const VUnitOutputWriter& other);

        // These are the functions used by createOutputWriters().
        static VLogAppender* _newLogAppenderByType(const VString& outputType, const VString& filePath);
        static VUnitOutputWriter* _newOutputWriterByType(const VString& outputType, VLogAppender* logAppender);
        static void _addNewOutputWriter(VUnitOutputWriterList& outputters, VUnitLogAppenderList& outpuAppenders, const VString& outputType, const VString& filePath);
};

/**
VUnit is a simple abstract base class used to build unit tests
for the Vault classes. Each unit test class is derived from
VUnit and overrides the run() method. That method simply
executes a bunch of tests, calling one of the test() functions
to verify the results of each test.

The VUnit constructor lets you specify whether you want to
log successful tests (you might want to only log the errors)
and whether you want to throw an exception on failed tests
(you might just want them logged rather than to throw).

The VUnitRunAll source executes every Vault unit test class.
*/
class VUnit {
    public:

        /**
        Runs a single unit's tests.
        @param  unit    the unit to run
        @param  writers writers to which test output will be collected and written
        */
        static void runUnit(VUnit& unit, VUnitOutputWriterList* writers);
        /**
        Re-runs a single unit's tests. The unit is reset(), then we runUnit().
        @param  unit    the unit to re-run
        @param  writers writers to which test output will be collected and written
        */
        static void rerunUnit(VUnit& unit, VUnitOutputWriterList* writers);

        /**
        Constructs a unit test object.
        @param    logOnSuccess    true if you want successful tests to be logged
        @param    throwOnError    true if you want an exception thrown for failed tests
        */
        VUnit(const VString& name, bool logOnSuccess, bool throwOnError);
        /**
        Destructor. May log test result totals.
        */
        virtual ~VUnit();

        /**
        Before a unit test suite is run, its output writers are set to that
        its results are recorded via those writers. Had to do this with a setter
        rather than the constructor to avoid having to change existing code.
        @param  writers the output writers that will record all results
        */
        void setWriters(VUnitOutputWriterList* writers) { mWriters = writers; }

        /**
        Executes the unit test. Must be overridden by concrete class.
        */
        virtual void run() = 0;
        /**
        Resets all state before a re-run. You can override if you had
        state in the subclass, in order to reset and then call inherited.
        */
        virtual void reset();

        /**
        Returns the unit's name.
        @return c.f.
        */
        const VString& getName() const { return mName; }
        /**
        Returns true if all tests succeeded.
        @return    c.f.
        */
        bool success() { return mNumFailedTests == 0; }
        /**
        Returns the number of tests that succeeded.
        @return c.f.
        */
        int getNumSuccessfulTests() { return mNumSuccessfulTests; }
        /**
        Returns the number of tests that failed.
        @return c.f.
        */
        int getNumFailedTests() { return mNumFailedTests; }

        // These functions are used by the runUnit() static function.

        void logStart();
        void logNormalEnd();
        void logExceptionalEnd(const VString& exceptionMessage);

    protected:

        VString                 mName;          ///< Name for display in log file.
        bool                    mLogOnSuccess;  ///< True if we log successful tests.
        bool                    mThrowOnError;  ///< True if we throw a VException on failed tests.
        VUnitOutputWriterList*  mWriters;       ///< The output writers to which test results are recorded.

#define VUNIT_ASSERT_SUCCESS(suffix) this->assertSuccess(suffix, __FILE__, __LINE__)
#define VUNIT_ASSERT_FAILURE(suffix) this->assertFailure(suffix, __FILE__, __LINE__)
        void assertSuccess(const VString& labelSuffix, const VString& filePath, int lineNumber);
        void assertFailure(const VString& labelSuffix, const VString& filePath, int lineNumber);

        // These are the methods that test equality of two values of the same type.
#define VUNIT_ASSERT_EQUAL(a, b) this->assertEqual(a, b, VString::EMPTY(), __FILE__, __LINE__)
#define VUNIT_ASSERT_EQUAL_LABELED(a, b, suffix) this->assertEqual(a, b, suffix, __FILE__, __LINE__)
#define VUNIT_ASSERT_TRUE(v) this->assertTrue(v, VString::EMPTY(), __FILE__, __LINE__)
#define VUNIT_ASSERT_TRUE_LABELED(v, suffix) this->assertTrue(v, suffix, __FILE__, __LINE__)
        void assertTrue(bool b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(b, filePath, lineNumber, labelSuffix, VString("failed assertion: value is false but should be true")); }
        void assertEqual(int a, int b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %d == %d", a, b)); }
        void assertEqual(unsigned int a, unsigned int b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %ud == %ud", a, b)); }
        void assertEqual(long a, long b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %ld == %ld", a, b)); }
        void assertEqual(bool a, bool b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %s == %s", (a ? "true" : "false"), (b ? "true" : "false"))); }
        void assertEqual(const VString& a, const VString& b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: '%s' == '%s'", a.chars(), b.chars())); }
        void assertEqual(const VString& a, const char* b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: '%s' == '%s'", a.chars(), b)); }
        void assertEqual(const VCodePoint& a, const VCodePoint& b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: '%s' == '%s'", a.toString().chars(), b.toString().chars())); }
        void assertEqual(const char* a, const VString& b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: '%s' == '%s'", a, b.chars())); }
        void assertEqual(const VChar& a, const VChar& b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: '%c' == '%c'", a.charValue(), b.charValue())); }
        void assertEqual(const VChar& a, char b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: '%c' == '%c'", a.charValue(), b)); }
        void assertEqual(VDouble a, VDouble b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %lf == %lf", a, b)); }
        void assertEqual(const VDuration& a, const VDuration& b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %lldms == %lldms", a.getDurationMilliseconds(), b.getDurationMilliseconds())); }
        void assertEqual(const VInstant& a, const VInstant& b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %lld == %lld", a.getValue(), b.getValue())); }
        void assertEqual(Vs8 a, Vs8 b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %hhd == %hhd", a, b)); }
        void assertEqual(Vu8 a, Vu8 b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %hhu == %hhu", a, b)); }
        void assertEqual(Vs16 a, Vs16 b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %hd == %hd", a, b)); }
        void assertEqual(Vu16 a, Vu16 b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %hu == %hu", a, b)); }

        void assertEqual(char a, char b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %d == %d", a, b)); }

#ifndef Vx32_IS_xINT /* don't redefine if types are same */
        void assertEqual(Vs32 a, Vs32 b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %ld == %ld", a, b)); }
        void assertEqual(Vu32 a, Vu32 b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %lu == %lu", a, b)); }
#endif /* not Vx32_IS_xINT */

#ifndef Vx64_IS_xINT /* don't redefine if types are same */
        void assertEqual(Vs64 a, Vs64 b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %lld == %lld", a, b)); }
        void assertEqual(Vu64 a, Vu64 b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %llu == %llu", a, b)); }
#endif /* not Vx64_IS_xINT */

#ifndef V_SIZE_T_IS_UNSIGNED_INT /* don't redefine if types are same */
        void assertEqual(size_t a, size_t b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: " VSTRING_FORMATTER_SIZE " == " VSTRING_FORMATTER_SIZE, a, b)); }
#endif /* not V_SIZE_T_IS_UNSIGNED_INT */

        void assertEqual(const VColor& a, const VColor& b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %s == %s", a.getCSSColor().chars(), b.getCSSColor().chars())); }
        void assertEqual(const VColorPair& a, const VColorPair& b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a == b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %s == %s", a.getCSSColor().chars(), b.getCSSColor().chars())); }
#define VUNIT_ASSERT_NOT_EQUAL(a, b) this->assertNotEqual(a, b, VString::EMPTY(), __FILE__, __LINE__)
#define VUNIT_ASSERT_NOT_EQUAL_LABELED(a, b, suffix) this->assertNotEqual(a, b, suffix, __FILE__, __LINE__)
#define VUNIT_ASSERT_FALSE(v) this->assertFalse(v, VString::EMPTY(), __FILE__, __LINE__)
#define VUNIT_ASSERT_FALSE_LABELED(v, suffix) this->assertFalse(v, suffix, __FILE__, __LINE__)
        void assertFalse(bool b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(!b, filePath, lineNumber, labelSuffix, VString("failed assertion: value is true but should be false")); }
        void assertNotEqual(int a, int b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: %d != %d", a, b)); }
        void assertNotEqual(unsigned int a, unsigned int b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: %ud != %ud", a, b)); }
        void assertNotEqual(long a, long b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: %ld != %ld", a, b)); }
        void assertNotEqual(bool a, bool b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: %s != %s", (a ? "true" : "false"), (b ? "true" : "false"))); }
        void assertNotEqual(const VString& a, const VString& b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: '%s' != '%s'", a.chars(), b.chars())); }
        void assertNotEqual(const VString& a, const char* b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: '%s' != '%s'", a.chars(), b)); }
        void assertNotEqual(const VCodePoint& a, const VCodePoint& b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: '%s' != '%s'", a.toString().chars(), b.toString().chars())); }
        void assertNotEqual(const char* a, const VString& b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: '%s' != '%s'", a, b.chars())); }
        void assertNotEqual(const VChar& a, const VChar& b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: '%c' != '%c'", a.charValue(), b.charValue())); }
        void assertNotEqual(const VChar& a, char b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: '%c' != '%c'", a.charValue(), b)); }
        void assertNotEqual(VDouble a, VDouble b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: %lf != %lf", a, b)); }
        void assertNotEqual(const VDuration& a, const VDuration& b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: %lldms != %lldms", a.getDurationMilliseconds(), b.getDurationMilliseconds())); }
        void assertNotEqual(const VInstant& a, const VInstant& b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: %lld != %lld", a.getValue(), b.getValue())); }
        void assertNotEqual(Vs8 a, Vs8 b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: %hhd != %hhd", a, b)); }
        void assertNotEqual(Vu8 a, Vu8 b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: %hhu != %hhu", a, b)); }
        void assertNotEqual(Vs16 a, Vs16 b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: %hd != %hd", a, b)); }
        void assertNotEqual(Vu16 a, Vu16 b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: %hu != %hu", a, b)); }

#ifndef Vx32_IS_xINT /* don't redefine if types are same */
        void assertNotEqual(Vs32 a, Vs32 b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: %ld != %ld", a, b)); }
        void assertNotEqual(Vu32 a, Vu32 b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: %lu != %lu", a, b)); }
#endif /* not Vx32_IS_xINT */

#ifndef Vx64_IS_xINT /* don't redefine if types are same */
        void assertNotEqual(Vs64 a, Vs64 b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: %lld != %lld", a, b)); }
        void assertNotEqual(Vu64 a, Vu64 b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: %llu != %llu", a, b)); }
#endif /* not Vx64_IS_xINT */

#ifndef V_SIZE_T_IS_UNSIGNED_INT /* don't redefine if types are same */
        void assertNotEqual(size_t a, size_t b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed inequality: " VSTRING_FORMATTER_SIZE " != " VSTRING_FORMATTER_SIZE, a, b)); }
#endif /* not V_SIZE_T_IS_UNSIGNED_INT */

        void assertNotEqual(const VColor& a, const VColor& b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %s != %s", a.getCSSColor().chars(), b.getCSSColor().chars())); }
        void assertNotEqual(const VColorPair& a, const VColorPair& b, const VString& labelSuffix, const VString& filePath, int lineNumber) { this->testAssertion(a != b, filePath, lineNumber, labelSuffix, VSTRING_FORMAT("failed equality: %s != %s", a.getCSSColor().chars(), b.getCSSColor().chars())); }

#define VUNIT_ASSERT_NULL(ptrValue) this->assertTrue(ptrValue == NULL, VString::EMPTY(), __FILE__, __LINE__)
#define VUNIT_ASSERT_NULL_LABELED(ptrValue, suffix) this->assertTrue(ptrValue == NULL, suffix, __FILE__, __LINE__)
#define VUNIT_ASSERT_NOT_NULL(ptrValue) this->assertFalse(ptrValue == NULL, VString::EMPTY(), __FILE__, __LINE__)
#define VUNIT_ASSERT_NOT_NULL_LABELED(ptrValue, suffix) this->assertFalse(ptrValue == NULL, suffix, __FILE__, __LINE__)

#define VUNIT_ASSERT_EQUAL_LABELED(a, b, suffix) this->assertEqual(a, b, suffix, __FILE__, __LINE__)

        /**
        Evaluates a boolean parameter that indicates test success,
        and logs and/or throws based on success/failure.
        @param    success        true if the test succeeded; false if not
        @param    description    the text to log that describes the test
        */
        virtual void testAssertion(bool success, const VString& filePath, int lineNumber, const VString& labelSuffix, const VString& expectedDescription);
        /**
        Evaluates a boolean parameter that indicates test success,
        and logs and/or throws based on success/failure.
        @param    success        true if the test succeeded; false if not
        @param    description    the text to log that describes the test
        */
        virtual void test(bool success, const VString& description);
        /**
        Compares two strings for equality as the test evaluation.
        @param    a            a string
        @param    b            another string
        @param    description    the text to log that describes the test
        */
        virtual void test(const VString& a, const VString& b, const VString& description);
        /**
        Logs an informational message to the unit test log, with a "[status ]"
        prefix. This does not affect the test counters.
        @param    description    the text to log that describes the status
        */
        void logStatus(const VString& description);

    private:

        // It does not make sense to copy/assign VUnit objects; the mWriters would have to be cloned or safely copied.
        VUnit(const VUnit&);
        void operator=(const VUnit&);

        void recordSuccess(const VString& description);
        void recordFailure(const VString& description);

        int             mNumSuccessfulTests;        ///< Running total of number of successful tests.
        int             mNumFailedTests;            ///< Running total of number of failed tests.
        TestInfoVector  mResults;                   ///< Detailed information about each test run.
        Vs64            mUnitStartTimeSnapshot;     ///< Identifies when the unit started.
        Vs64            mPreviousTestEndedSnapshot; ///< Identifies when the previous test ended, so we can determine approximate duration of each test.
        VString         mLastTestDescription;       ///< Description of last test invoked.
};

/**
VTestRunner is an abstract interface for running a single unit test class. This can
be useful when a unit test harness needs to allow something else to run tests for it,
and that thing does not have access to the caller's API. The caller can simply
implement VTestRunner and pass that interface.
*/
class VTestRunner {
    public:

        VTestRunner() {}
        virtual ~VTestRunner() {}

        virtual void runUnit(VUnit& unit, VUnitOutputWriterList* output) = 0;
};

/**
VTestSuitesRunner is a helper/wrapper class that handles proper setup and teardown
of an entire set of test suites. It will set up the output writers for you.
*/
class VTestSuitesWrapper {
    public:

        VTestSuitesWrapper(const VStringVector& args);
        virtual ~VTestSuitesWrapper();

        VUnitOutputWriterList   mWriters;
        VUnitLogAppenderList    mAppenders;
};

/**
There are some types of unit tests where you have to do some setup before actually
invoking the VUnit subclass, where the setup can fail. An example is setting up
loopback socket, so that a unit test can send and receive data on the socket.
If the setup fails, you want to emit a unit test failure to record the failure.
This class lets you emit the failure information easily. Just declare this and
give it a test name describing the failed setup operation, along with the error
message you want shown, and "run" this unit test.
*/
class VFailureEmitter : public VUnit {
    public:

        VFailureEmitter(const VString& testName, bool logOnSuccess, bool throwOnError, const VString& errorMessage);
        virtual ~VFailureEmitter() {}

        virtual void run();

    protected:

        VString fErrorMessage;
};

/**
This outputter writes test results in a JUnit-compatible XML format.
This consists of a tag wrapping all suites, within which is a tag
for each suite (with summary information), within which is a tag for
each test (with its result).
*/
class VUnitJUnitXMLOutput : public VUnitOutputWriter {
    public:

        VUnitJUnitXMLOutput(VLogAppender& outputAppender);
        virtual ~VUnitJUnitXMLOutput() {}

        virtual void testSuitesBegin();
        virtual void testSuiteBegin(const VString& testSuiteName);
        virtual void testSuiteStatusMessage(const VString& message);
        virtual void testCaseBegin(const VString& testCaseName);
        virtual void testCaseEnd(const VTestInfo& testInfo);
        virtual void testSuiteEnd();
        virtual void testSuitesEnd();
};

/**
This outputter writes test results in a simple human-readable text format.
This consists of the result of each test, summarized for each suite, and
a summary at the end for everything.
*/
class VUnitSimpleTextOutput : public VUnitOutputWriter {
    public:

        VUnitSimpleTextOutput(VLogAppender& outputAppender);
        virtual ~VUnitSimpleTextOutput() {}

        virtual void testSuitesBegin();
        virtual void testSuiteBegin(const VString& testSuiteName);
        virtual void testSuiteStatusMessage(const VString& message);
        virtual void testCaseBegin(const VString& testCaseName);
        virtual void testCaseEnd(const VTestInfo& testInfo);
        virtual void testSuiteEnd();
        virtual void testSuitesEnd();
};

/**
This outputter writes test results in a TeamCity stdout reporting format.
This consists of Team City "##teamcity" reports for the begin and end of
each suite and each test.
*/
class VUnitTeamCityOutput : public VUnitOutputWriter {
    public:

        VUnitTeamCityOutput(VLogAppender& outputAppender);
        virtual ~VUnitTeamCityOutput() {}

        virtual void testSuitesBegin();
        virtual void testSuiteBegin(const VString& testSuiteName);
        virtual void testSuiteStatusMessage(const VString& message);
        virtual void testCaseBegin(const VString& testCaseName);
        virtual void testCaseEnd(const VTestInfo& testInfo);
        virtual void testSuiteEnd();
        virtual void testSuitesEnd();
};

/**
This outputter writes test results in a TeamCity build status XML file format.
This consists of a few lines of XML summarizing the whole test run.
*/
class VUnitTeamCityBuildStatusOutput : public VUnitOutputWriter {
    public:

        VUnitTeamCityBuildStatusOutput(VLogAppender& outputAppender);
        virtual ~VUnitTeamCityBuildStatusOutput() {}

        virtual void testSuitesBegin();
        virtual void testSuiteBegin(const VString& testSuiteName);
        virtual void testSuiteStatusMessage(const VString& message);
        virtual void testCaseBegin(const VString& testCaseName);
        virtual void testCaseEnd(const VTestInfo& testInfo);
        virtual void testSuiteEnd();
        virtual void testSuitesEnd();
};

#endif /* vunit_h */
