/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
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

class VLogger;
class VTextIOStream;

/**
VTestInfo is used internally by VUnit to hold a single test's info and
result.
*/
class VTestInfo
    {
    public:
    
        VTestInfo(bool success, const VString& description, Vs64 durationMilliseconds);
        ~VTestInfo() {}

        bool    mSuccess;                ///< True if the test succeeded.
        VString    mDescription;            ///< The text description or name of the test.
        Vs64    mDurationMilliseconds;    ///< The number of milliseconds it took to run the test.
    };

typedef std::vector<VTestInfo> TestInfoVector;

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
class VUnit
    {
    public:
    
        /**
        Runs a single unit's tests. The 
        @param    unit            the unit to run
        @param    xmlOutputStream    if not NULL, the results are appended to this stream
        */
        static void runUnit(VUnit& unit, VTextIOStream* xmlOutputStream);
    
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
        Executes the unit test. Must be overridden by concrete class.
        */
        virtual void run() = 0;
        
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
        void logNormalEnd(VTextIOStream* xmlOutputStream);
        void logExceptionalEnd(VTextIOStream* xmlOutputStream, const VString& exceptionMessage);
        void logResults(VTextIOStream* xmlOutputStream);

    protected:
    
        VString    mName;            ///< Name for display in log file.
        bool    mLogOnSuccess;    ///< True if we log successful tests.
        bool    mThrowOnError;    ///< True if we throw a VException on failed tests.

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
        /**
        Logs an informational message verbatim; the unit test code should not
        call this directly, but instead either call test() to check a condition
        and generate a message containing the result, or call logStatus() to
        generate an informational ("[status ]") message.
        @param    message    the message to log
        */
        void logMessage(const VString& message);
    
    private:
    
        void recordSuccess(const VString& description);
        void recordFailure(const VString& description);
        void logXMLResults(VTextIOStream* xmlOutputStream);
    
        int    mNumSuccessfulTests;    ///< Running total of number of successful tests.
        int    mNumFailedTests;        ///< Running total of number of failed tests.
        TestInfoVector mResults;    ///< Detailed information about each test run.
        Vs64 mUnitStartTimeSnapshot;        ///< Identifies when the unit started.
        Vs64 mPreviousTestEndedSnapshot;    ///< Identifies when the previous test ended, so we can determine approximate duration of each test.
        VString    mLastTestDescription;    ///< Description of last test invoked.
    };

#endif /* vunit_h */
