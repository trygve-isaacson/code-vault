/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

/** @file */

#include "vunit.h"

#include "vlogger.h"
#include "vexception.h"
#include "vtextiostream.h"

// VUnit ---------------------------------------------------------------------

// static
void VUnit::runUnit(VUnit& unit, VUnitOutputWriterList* writers) {
    unit.setWriters(writers);

    unit.logStart();

    try {
        VAutoreleasePool pool;
        unit.run();
    } catch (const std::exception& ex) { // will include VException
        unit.logExceptionalEnd(ex.what());
        throw;
    } catch (...) {
        unit.logExceptionalEnd("(exception type unknown)");
        throw;
    }

    unit.logNormalEnd();
}

void VUnit::rerunUnit(VUnit& unit, VUnitOutputWriterList* writers) {
    unit.reset();
    VUnit::runUnit(unit, writers);
}

VUnit::VUnit(const VString& name, bool logOnSuccess, bool throwOnError) :
    mName(name),
    mLogOnSuccess(logOnSuccess),
    mThrowOnError(throwOnError),
    mWriters(NULL),
    mNumSuccessfulTests(0),
    mNumFailedTests(0),
    mResults(),
    mUnitStartTimeSnapshot(VInstant::snapshot()),
    mPreviousTestEndedSnapshot(VInstant::snapshot()),
    mLastTestDescription() {
}

VUnit::~VUnit() {
}

void VUnit::reset() {
    mNumSuccessfulTests = 0;
    mNumFailedTests = 0;
    mResults.clear();
    mUnitStartTimeSnapshot = VInstant::snapshot();
    mPreviousTestEndedSnapshot = VInstant::snapshot();
    mLastTestDescription = VString::EMPTY();
}

void VUnit::logStart() {
    if (mWriters != NULL)
        for (VUnitOutputWriterList::iterator i = mWriters->begin(); i != mWriters->end(); ++i)
            (*i)->testSuiteBegin(mName);
}

void VUnit::logNormalEnd() {
    if (mWriters != NULL)
        for (VUnitOutputWriterList::iterator i = mWriters->begin(); i != mWriters->end(); ++i)
            (*i)->testSuiteEnd();
}

void VUnit::logExceptionalEnd(const VString& exceptionMessage) {
    VTestInfo error(false, VSTRING_FORMAT("after %s, threw exception: %s", mLastTestDescription.chars(), exceptionMessage.chars()), VDuration::ZERO());
    mResults.push_back(error);

    ++mNumFailedTests;

    if (mWriters != NULL)
        for (VUnitOutputWriterList::iterator i = mWriters->begin(); i != mWriters->end(); ++i) {
            (*i)->testCaseBegin("exception thrown");
            (*i)->testCaseEnd(error);
        }
}

void VUnit::assertSuccess(const VString& labelSuffix, const VString& filePath, int lineNumber) {
    VString fileName;
    filePath.getSubstring(fileName, filePath.lastIndexOf(VFSNode::PATH_SEPARATOR_CHAR) + 1);
    VString testName(VSTRING_ARGS("%s:%d %s", fileName.chars(), lineNumber, labelSuffix.chars()));

    mLastTestDescription = testName;

    this->recordSuccess(testName);

    mPreviousTestEndedSnapshot = VInstant::snapshot();
}

void VUnit::assertFailure(const VString& labelSuffix, const VString& filePath, int lineNumber) {
    VString fileName;
    filePath.getSubstring(fileName, filePath.lastIndexOf(VFSNode::PATH_SEPARATOR_CHAR) + 1);
    VString testName(VSTRING_ARGS("%s:%d %s", fileName.chars(), lineNumber, labelSuffix.chars()));

    mLastTestDescription = testName;

    this->recordFailure(VSTRING_FORMAT("%s: %s", testName.chars()));

    mPreviousTestEndedSnapshot = VInstant::snapshot();
}

void VUnit::testAssertion(bool successful, const VString& filePath, int lineNumber, const VString& labelSuffix, const VString& expectedDescription) {
    VString fileName;
    filePath.getSubstring(fileName, filePath.lastIndexOf(VFSNode::PATH_SEPARATOR_CHAR) + 1);
    VString testName(VSTRING_ARGS("%s:%d %s", fileName.chars(), lineNumber, labelSuffix.chars()));

    mLastTestDescription = testName;

    if (successful)
        this->recordSuccess(testName);
    else
        this->recordFailure(VSTRING_FORMAT("%s: %s", testName.chars(), expectedDescription.chars()));

    mPreviousTestEndedSnapshot = VInstant::snapshot();
}

void VUnit::test(bool successful, const VString& description) {
    mLastTestDescription = description;

    if (successful)
        this->recordSuccess(description);
    else
        this->recordFailure(description);

    mPreviousTestEndedSnapshot = VInstant::snapshot();
}

void VUnit::test(const VString& a, const VString& b, const VString& description) {
    this->test(a == b, description);
}

void VUnit::logStatus(const VString& description) {
    if (mWriters != NULL)
        for (VUnitOutputWriterList::iterator i = mWriters->begin(); i != mWriters->end(); ++i)
            (*i)->testSuiteStatusMessage(description);
}

void VUnit::recordSuccess(const VString& description) {
    if (mWriters != NULL)
        for (VUnitOutputWriterList::iterator i = mWriters->begin(); i != mWriters->end(); ++i)
            (*i)->testCaseBegin(description);

    ++mNumSuccessfulTests;

    VTestInfo info(true, description, VInstant::snapshotDelta(mPreviousTestEndedSnapshot));
    mResults.push_back(info);

    if (mWriters != NULL)
        for (VUnitOutputWriterList::iterator i = mWriters->begin(); i != mWriters->end(); ++i)
            (*i)->testCaseEnd(info);
}

void VUnit::recordFailure(const VString& description) {
    if (mWriters != NULL)
        for (VUnitOutputWriterList::iterator i = mWriters->begin(); i != mWriters->end(); ++i)
            (*i)->testCaseBegin(description);

    ++mNumFailedTests;

    VTestInfo info(false, description, VInstant::snapshotDelta(mPreviousTestEndedSnapshot));
    mResults.push_back(info);

    if (mWriters != NULL)
        for (VUnitOutputWriterList::iterator i = mWriters->begin(); i != mWriters->end(); ++i)
            (*i)->testCaseEnd(info);
}

// VTestInfo -----------------------------------------------------------------

VTestInfo::VTestInfo(bool success, const VString& description, const VDuration& duration) :
    mSuccess(success),
    mDescription(description),
    mDuration(duration) {
    // Some tests that manipulate time simulation will yield bogus durations.
    if ((mDuration < VDuration::ZERO()) || (mDuration > VDuration::DAY()))
        mDuration = VDuration::ZERO();
}

// VFailureEmitter -----------------------------------------------------------

VFailureEmitter::VFailureEmitter(const VString& testName, bool logOnSuccess, bool throwOnError, const VString& errorMessage) :
    VUnit(testName, logOnSuccess, throwOnError), fErrorMessage(errorMessage) {
}

void VFailureEmitter::run() {
    this->logStatus(VSTRING_FORMAT("%s failed due to this error: %s", this->getName().chars(), fErrorMessage.chars()));
    this->test(false, fErrorMessage);
}

// VUnitOutputWriter ------------------------------------------------------

static const VString VUNIT_OUTPUT_DIRECTIVE("-vunit-out");

static const VString OUTPUT_TYPE_SIMPLE("text");
static const VString OUTPUT_TYPE_JUNIT("junit");
static const VString OUTPUT_TYPE_TEAMCITY("tc");
static const VString OUTPUT_TYPE_TEAMCITY_STATUS("tcstatus");

static const VString OUTPUT_FILEPATH_STDOUT("stdout");

// static
void VUnitOutputWriter::createOutputWriters(const VStringVector& args, VUnitOutputWriterList& writers, VUnitLogAppenderList& appenders) {
    for (VStringVector::const_iterator i = args.begin(); i != args.end(); ++i) {
        if ((*i) == VUNIT_OUTPUT_DIRECTIVE) {
            VString outputType = *(++i);
            VString filePath = *(++i);
            VUnitOutputWriter::_addNewOutputWriter(writers, appenders, outputType, filePath);
        }
    }

    // If no specific output was specified, log simple output to stdout.
    if (writers.size() == 0)
        VUnitOutputWriter::_addNewOutputWriter(writers, appenders, OUTPUT_TYPE_SIMPLE, OUTPUT_FILEPATH_STDOUT);
}

VUnitOutputWriter::VUnitOutputWriter(VLogAppender& outputAppender) :
    mLogAppender(outputAppender),
    mTestSuitesStartTime(VInstant::NEVER_OCCURRED()),
    mTotalNumSuccesses(0),
    mTotalNumErrors(0),
    mCurrentTestSuiteName(),
    mCurrentTestSuiteResults(),
    mCurrentTestSuiteNumSuccesses(0),
    mCurrentTestSuiteNumErrors(0),
    mCurrentTestSuiteStartTime(VInstant::NEVER_OCCURRED()),
    mCurrentTestSuiteEndTime(VInstant::NEVER_OCCURRED()),
    mCurrentTestCaseName(),
    mCurrentTestCaseStartTime(VInstant::NEVER_OCCURRED()),
    mCurrentTestCaseEndTime(VInstant::NEVER_OCCURRED()),
    mFailedTestSuiteNames() {
}

void VUnitOutputWriter::_testSuitesBegin() {
    mTestSuitesStartTime.setNow();
}

void VUnitOutputWriter::_testSuiteBegin(const VString& testSuiteName) {
    mCurrentTestSuiteName = testSuiteName;
    mCurrentTestSuiteResults.clear();
    mCurrentTestSuiteNumSuccesses = 0;
    mCurrentTestSuiteNumErrors = 0;
    mCurrentTestSuiteStartTime.setNow();
    mCurrentTestSuiteEndTime = VInstant::NEVER_OCCURRED();
    mCurrentTestCaseStartTime = VInstant::NEVER_OCCURRED();
    mCurrentTestCaseEndTime = VInstant::NEVER_OCCURRED();
}

void VUnitOutputWriter::_testCaseBegin(const VString& testCaseName) {
    mCurrentTestCaseName = testCaseName;
    mCurrentTestCaseStartTime.setNow();
}

void VUnitOutputWriter::_testCaseEnd(const VTestInfo& testInfo) {
    mCurrentTestCaseEndTime.setNow();
    mCurrentTestSuiteResults.push_back(testInfo);

    if (testInfo.mSuccess) {
        ++mTotalNumSuccesses;
        ++mCurrentTestSuiteNumSuccesses;
    } else {
        ++mTotalNumErrors;
        ++mCurrentTestSuiteNumErrors;
    }
}

void VUnitOutputWriter::_testSuiteEnd() {
    mCurrentTestSuiteEndTime.setNow();

    if (mCurrentTestSuiteNumErrors != 0)
        mFailedTestSuiteNames.push_back(mCurrentTestSuiteName);
}

// static
VLogAppender* VUnitOutputWriter::_newLogAppenderByType(const VString& outputType, const VString& filePath) {
    // We allow either cout logging, or file logging.
    VLogAppender* appender = NULL;
    if (filePath == OUTPUT_FILEPATH_STDOUT) {
        appender = new VCoutLogAppender(VSTRING_FORMAT("vunit-%s-cout", outputType.chars()), false, VString::EMPTY(), VString::EMPTY());
    } else {
        VFSNode logFile(filePath);
        logFile.rm();
        appender = new VFileLogAppender(VSTRING_FORMAT("vunit-%s-%s", outputType.chars(), filePath.chars()), false, VString::EMPTY(), VString::EMPTY(), filePath);
    }

    return appender;
}

// static
VUnitOutputWriter* VUnitOutputWriter::_newOutputWriterByType(const VString& outputType, VLogAppender* appender) {
    VUnitOutputWriter* writer = NULL;

    if (outputType == OUTPUT_TYPE_SIMPLE) {
        writer = new VUnitSimpleTextOutput(*appender);
    } else if (outputType == OUTPUT_TYPE_JUNIT) {
        writer = new VUnitJUnitXMLOutput(*appender);
    } else if (outputType == OUTPUT_TYPE_TEAMCITY) {
        writer = new VUnitTeamCityOutput(*appender);
    } else if (outputType == OUTPUT_TYPE_TEAMCITY_STATUS) {
        writer = new VUnitTeamCityBuildStatusOutput(*appender);
    } else {
        VLOGGER_ERROR(VSTRING_FORMAT("Invalid unit test output type '%s' will be ignored.", outputType.chars()));
    }

    return writer;
}

// static
void VUnitOutputWriter::_addNewOutputWriter(VUnitOutputWriterList& outputters, VUnitLogAppenderList& outputAppenders, const VString& outputType, const VString& filePath) {
    VLogAppender* appender = VUnitOutputWriter::_newLogAppenderByType(outputType, filePath);
    VUnitOutputWriter* outputInterface = VUnitOutputWriter::_newOutputWriterByType(outputType, appender);

    if (outputInterface == NULL) {
        delete appender;
    } else {
        outputAppenders.push_back(appender);
        outputters.push_back(outputInterface);
    }
}

// VTestSuitesWrapper --------------------------------------------------------

VTestSuitesWrapper::VTestSuitesWrapper(const VStringVector& args) :
    mWriters(),
    mAppenders() {
    VUnitOutputWriter::createOutputWriters(args, mWriters, mAppenders);

    for (VUnitOutputWriterList::iterator i = mWriters.begin(); i != mWriters.end(); ++i)
        (*i)->testSuitesBegin();
}

VTestSuitesWrapper::~VTestSuitesWrapper() {
    for (VUnitOutputWriterList::iterator i = mWriters.begin(); i != mWriters.end(); ++i)
        (*i)->testSuitesEnd();

    vault::vectorDeleteAll(mWriters);
    vault::vectorDeleteAll(mAppenders);
}

// VUnitJUnitXMLOutput -------------------------------------------------------

static VString _escapeXMLString(const VString& original) {
    VString result(original);

    result.replace("&", "&amp;");
    result.replace("\"", "&quot;");
    result.replace("<", "&lt;");
    result.replace(">", "&gt;");

    return result;
}

VUnitJUnitXMLOutput::VUnitJUnitXMLOutput(VLogAppender& outputAppender) :
    VUnitOutputWriter(outputAppender) {
}

void VUnitJUnitXMLOutput::testSuitesBegin() {
    this->_testSuitesBegin();

    mLogAppender.emitRaw("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>");
    mLogAppender.emitRaw("<testsuites>");
}

void VUnitJUnitXMLOutput::testSuiteBegin(const VString& testSuiteName) {
    this->_testSuiteBegin(testSuiteName);
}

void VUnitJUnitXMLOutput::testSuiteStatusMessage(const VString& /*message*/) {
}

void VUnitJUnitXMLOutput::testCaseBegin(const VString& testCaseName) {
    this->_testCaseBegin(testCaseName);
}

void VUnitJUnitXMLOutput::testCaseEnd(const VTestInfo& testInfo) {
    this->_testCaseEnd(testInfo);
}

void VUnitJUnitXMLOutput::testSuiteEnd() {
    this->_testSuiteEnd();

    VDuration testSuiteDuration = mCurrentTestSuiteEndTime - mCurrentTestSuiteStartTime;

    mLogAppender.emitRaw(VSTRING_FORMAT(" <testsuite errors=\"%d\" failures=\"0\" name=\"%s\" tests=\"%d\" time=\"%s\">",
                                        mCurrentTestSuiteNumErrors, mCurrentTestSuiteName.chars(), (int)mCurrentTestSuiteResults.size(), testSuiteDuration.getDurationString().chars()));

    for (TestInfoVector::const_iterator i = mCurrentTestSuiteResults.begin(); i != mCurrentTestSuiteResults.end(); ++i) {
        mLogAppender.emitRaw(VSTRING_FORMAT("  <testcase class=\"%s\" name=\"%s\" time=\"%s\"></testcase>",
                                            mCurrentTestSuiteName.chars(), _escapeXMLString((*i).mDescription).chars(), (*i).mDuration.getDurationString().chars()));
    }

    mLogAppender.emitRaw(" </testsuite>");
}

void VUnitJUnitXMLOutput::testSuitesEnd() {
    mLogAppender.emitRaw("</testsuites>");
}

// VUnitSimpleTextOutput -----------------------------------------------------

VUnitSimpleTextOutput::VUnitSimpleTextOutput(VLogAppender& outputAppender) :
    VUnitOutputWriter(outputAppender) {
}

void VUnitSimpleTextOutput::testSuitesBegin() {
    this->_testSuitesBegin();

    mLogAppender.emitRaw(VSTRING_FORMAT("[status ] Test run starting at %s.", mTestSuitesStartTime.getLocalString().chars()));
    mLogAppender.emitRaw(VString::EMPTY());
}

void VUnitSimpleTextOutput::testSuiteBegin(const VString& testSuiteName) {
    this->_testSuiteBegin(testSuiteName);

    mLogAppender.emitRaw(VSTRING_FORMAT("[status ] %s : starting.", testSuiteName.chars()));
}

void VUnitSimpleTextOutput::testSuiteStatusMessage(const VString& message) {
    mLogAppender.emitRaw(VSTRING_FORMAT("[status ] %s : %s", mCurrentTestSuiteName.chars(), message.chars()));
}

void VUnitSimpleTextOutput::testCaseBegin(const VString& testCaseName) {
    this->_testCaseBegin(testCaseName);
}

void VUnitSimpleTextOutput::testCaseEnd(const VTestInfo& testInfo) {
    this->_testCaseEnd(testInfo);

    mLogAppender.emitRaw(VSTRING_FORMAT("[%s] %s : %s.", (testInfo.mSuccess ? "success" : "FAILURE"), mCurrentTestSuiteName.chars(), testInfo.mDescription.chars()));
}

void VUnitSimpleTextOutput::testSuiteEnd() {
    this->_testSuiteEnd();

    mLogAppender.emitRaw(VSTRING_FORMAT("[status ] %s : ended.", mCurrentTestSuiteName.chars()));
    mLogAppender.emitRaw(VSTRING_FORMAT("[results] %s : tests passed: %d", mCurrentTestSuiteName.chars(), mCurrentTestSuiteNumSuccesses));
    mLogAppender.emitRaw(VSTRING_FORMAT("[results] %s : tests failed: %d", mCurrentTestSuiteName.chars(), mCurrentTestSuiteNumErrors));
    mLogAppender.emitRaw(VSTRING_FORMAT("[results] %s : summary: %s.", mCurrentTestSuiteName.chars(), ((mCurrentTestSuiteNumErrors == 0) ? "success" : "FAILURE")));
    mLogAppender.emitRaw(VString::EMPTY());
}

void VUnitSimpleTextOutput::testSuitesEnd() {
    mLogAppender.emitRaw(VSTRING_FORMAT("[results] TOTAL tests passed: %d", mTotalNumSuccesses));
    mLogAppender.emitRaw(VSTRING_FORMAT("[results] TOTAL tests failed: %d", mTotalNumErrors));
    mLogAppender.emitRaw(VSTRING_FORMAT("[results] TOTAL summary: %s.", ((mTotalNumErrors == 0) ? "success" : "FAILURE")));

    if (mFailedTestSuiteNames.size() != 0) {
        VString names;
        for (VStringVector::const_iterator i = mFailedTestSuiteNames.begin(); i != mFailedTestSuiteNames.end(); ++i) {
            names += ' ';
            names += *i;
        }
        mLogAppender.emitRaw(VSTRING_FORMAT("[results] Names of suites with failures:%s", names.chars()));
    }

    VInstant now;
    VDuration totalTestTime = now - mTestSuitesStartTime;
    mLogAppender.emitRaw(VString::EMPTY());
    mLogAppender.emitRaw(VSTRING_FORMAT("[status ] Test run ending at %s. Total time %s.", now.getLocalString().chars(), totalTestTime.getDurationString().chars()));
}

// VUnitTeamCityOutput -------------------------------------------------------

static VString _escapeTeamCityString(const VString& original) {
    VString result = original;

    result.replace("|", "||");
    result.replace("'", "|'");
    result.replace("\n", "|\n");
    result.replace("\r", "|\r");
    result.replace("]", "|]");

    return result;
}

VUnitTeamCityOutput::VUnitTeamCityOutput(VLogAppender& outputAppender) :
    VUnitOutputWriter(outputAppender) {
}

void VUnitTeamCityOutput::testSuitesBegin() {
    this->_testSuitesBegin();
}

void VUnitTeamCityOutput::testSuiteBegin(const VString& testSuiteName) {
    this->_testSuiteBegin(testSuiteName);

    mLogAppender.emitRaw(VSTRING_FORMAT("##teamcity[testSuiteStarted name='%s']", _escapeTeamCityString(testSuiteName).chars()));
}

void VUnitTeamCityOutput::testSuiteStatusMessage(const VString& /*message*/) {
}

void VUnitTeamCityOutput::testCaseBegin(const VString& testCaseName) {
    this->_testCaseBegin(testCaseName);

    mLogAppender.emitRaw(VSTRING_FORMAT("##teamcity[testStarted name='%s']", _escapeTeamCityString(testCaseName).chars()));
}

void VUnitTeamCityOutput::testCaseEnd(const VTestInfo& testInfo) {
    this->_testCaseEnd(testInfo);

    if (!testInfo.mSuccess)
        mLogAppender.emitRaw(VSTRING_FORMAT("##teamcity[testFailed name='%s' message='%s']", _escapeTeamCityString(mCurrentTestCaseName).chars(), _escapeTeamCityString(testInfo.mDescription).chars()));

    mLogAppender.emitRaw(VSTRING_FORMAT("##teamcity[testFinished name='%s']", _escapeTeamCityString(mCurrentTestCaseName).chars()));
}

void VUnitTeamCityOutput::testSuiteEnd() {
    this->_testSuiteEnd();

    mLogAppender.emitRaw(VSTRING_FORMAT("##teamcity[testSuiteFinished name='%s']", _escapeTeamCityString(mCurrentTestSuiteName).chars()));
}

void VUnitTeamCityOutput::testSuitesEnd() {
}

// VUnitTeamCityBuildStatusOutput --------------------------------------------

VUnitTeamCityBuildStatusOutput::VUnitTeamCityBuildStatusOutput(VLogAppender& outputAppender) :
    VUnitOutputWriter(outputAppender) {
}

void VUnitTeamCityBuildStatusOutput::testSuitesBegin() {
    this->_testSuitesBegin();
}

void VUnitTeamCityBuildStatusOutput::testSuiteBegin(const VString& testSuiteName) {
    this->_testSuiteBegin(testSuiteName);
}

void VUnitTeamCityBuildStatusOutput::testSuiteStatusMessage(const VString& /*message*/) {
}

void VUnitTeamCityBuildStatusOutput::testCaseBegin(const VString& testCaseName) {
    this->_testCaseBegin(testCaseName);
}

void VUnitTeamCityBuildStatusOutput::testCaseEnd(const VTestInfo& testInfo) {
    this->_testCaseEnd(testInfo);
}

void VUnitTeamCityBuildStatusOutput::testSuiteEnd() {
    this->_testSuiteEnd();
}

void VUnitTeamCityBuildStatusOutput::testSuitesEnd() {
    try {
        mLogAppender.emitRaw(VString("<build number=\"{build.number}\">"));
        mLogAppender.emitRaw(VSTRING_FORMAT(" <statusInfo status=\"%s\">", (mTotalNumErrors == 0 ? "SUCCESS" : "FAILURE")));
        mLogAppender.emitRaw(VSTRING_FORMAT("  <text action=\"append\">Tests passed: %d</text>", mTotalNumSuccesses));
        mLogAppender.emitRaw(VSTRING_FORMAT("  <text action=\"append\">Tests failed: %d</text>", mTotalNumErrors));

        if (mFailedTestSuiteNames.size() != 0) {
            VString names;
            for (VStringVector::const_iterator i = mFailedTestSuiteNames.begin(); i != mFailedTestSuiteNames.end(); ++i) {
                names += ' ';
                names += *i;
            }

            mLogAppender.emitRaw(VSTRING_FORMAT("  <text action=\"append\">These are the names of the failed tests:%s</text>", names.chars()));
        }

        mLogAppender.emitRaw(VString(" </statusInfo>"));

        mLogAppender.emitRaw(VSTRING_FORMAT(" <statisticValue key=\"testCount\" value=\"%d\"/>", mTotalNumSuccesses + mTotalNumErrors));
        mLogAppender.emitRaw(VSTRING_FORMAT(" <statisticValue key=\"testsPassed\" value=\"%d\"/>", mTotalNumSuccesses));
        mLogAppender.emitRaw(VSTRING_FORMAT(" <statisticValue key=\"testsFailed\" value=\"%d\"/>", mTotalNumErrors));

        mLogAppender.emitRaw(VString("</build>"));
    } catch (...) {} // prevent exceptions from escaping destructor
}

