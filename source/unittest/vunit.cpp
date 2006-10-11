/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vunit.h"

#include "vlogger.h"
#include "vexception.h"
#include "vtextiostream.h"

// VUnit ---------------------------------------------------------------------

// static
void VUnit::runUnit(VUnit& unit, VTextIOStream* xmlOutputStream)
    {
    unit.logStart();
    
    try
        {
        unit.run();
        }
    catch (const std::exception& ex)    // will include VException
        {
        unit.logExceptionalEnd(xmlOutputStream, ex.what());
        throw;
        }
    catch (...)
        {
        unit.logExceptionalEnd(xmlOutputStream, VString::kEmptyString);
        throw;
        }

    unit.logNormalEnd(xmlOutputStream);
    }

VUnit::VUnit(const VString& name, bool logOnSuccess, bool throwOnError) :
mName(name),
mLogOnSuccess(logOnSuccess),
mThrowOnError(throwOnError),
mNumSuccessfulTests(0),
mNumFailedTests(0),
mUnitStartTimeSnapshot(VInstant::snapshot()),
mPreviousTestEndedSnapshot(VInstant::snapshot())
    {
    }

VUnit::~VUnit()
    {
    }

void VUnit::logStart()
    {
    this->logStatus("starting.");
    }

void VUnit::logNormalEnd(VTextIOStream* xmlOutputStream)
    {
    this->logStatus("ended.");
    this->logResults(xmlOutputStream);
    }

void VUnit::logExceptionalEnd(VTextIOStream* xmlOutputStream, const VString& exceptionMessage)
    {
    mResults.push_back(VTestInfo(false, VString("after %s, threw exception: %s", mLastTestDescription.chars(), exceptionMessage.chars()), VDuration::ZERO()));

    ++mNumFailedTests;
    this->logMessage(VString("[FAILURE] %s : ended with an exception after previous test '%s'. Exception: '%s'",
            mName.chars(), mLastTestDescription.chars(), exceptionMessage.chars()));
    this->logResults(xmlOutputStream);
    }

void VUnit::logResults(VTextIOStream* xmlOutputStream)
    {
    this->logMessage(VString("[results] %s : tests passed: %d", mName.chars(), mNumSuccessfulTests));
    this->logMessage(VString("[results] %s : tests failed: %d", mName.chars(), mNumFailedTests));
    this->logMessage(VString("[results] %s : summary: %s", mName.chars(), (this->success() ? "SUCCESS" : "FAILURE")));
    this->logMessage(VString::kEmptyString);    // Blank line to increase readability
    
    this->logXMLResults(xmlOutputStream);
    }

void VUnit::test(bool success, const VString& description)
    {
    mLastTestDescription = description;

    if (success)
        this->recordSuccess(description);
    else
        this->recordFailure(description);

    mPreviousTestEndedSnapshot = VInstant::snapshot();
    }

void VUnit::test(const VString& a, const VString& b, const VString& description)
    {
    this->test(a == b, description);
    }

void VUnit::logStatus(const VString& description)
    {
    this->logMessage(VString("[status ] %s : %s", mName.chars(), description.chars()));
    }

void VUnit::logMessage(const VString& message)
    {
    // Note that we don't use VLOGGER_xxx and its time stamping and
    // formatting, but use raw logging, so that:
    // - we don't time stamp the messages, so diff will work well
    // - the code undergoing testing can log w/o us interfering and vice versa
    
    VLogger::getLogger("VUnit")->rawLog(message);
    }

void VUnit::recordSuccess(const VString& description)
    {
    VInstant    now;
    ++mNumSuccessfulTests;
    mResults.push_back(VTestInfo(true, description, VInstant::snapshotDelta(mPreviousTestEndedSnapshot)));

    if (mLogOnSuccess)
        this->logMessage(VString("[success] %s : %s", mName.chars(), description.chars()));
    }

void VUnit::recordFailure(const VString& description)
    {
    VInstant    now;
    ++mNumFailedTests;
    mResults.push_back(VTestInfo(false, description, VInstant::snapshotDelta(mPreviousTestEndedSnapshot)));

    // Uppercase is easier for the eye to pickup in a lot of text
    VString    message("[FAILURE] %s : %s", mName.chars(), description.chars());

    this->logMessage(message);

    if (mThrowOnError)
        throw VException(message);
    }

static void _getDurationString(const VDuration& duration, VString& prettyString)
    {
    int    wholeSeconds = duration.getDurationSeconds();
    int    thousandths = static_cast<int>(duration.getDurationMilliseconds() % 1000);
    
    prettyString.format("%d.%03d", wholeSeconds, thousandths);
    }

void VUnit::logXMLResults(VTextIOStream* xmlOutputStream)
    {
    if (xmlOutputStream == NULL)
        return;

    VInstant    now;
    VString        durationString;
    
    _getDurationString(VInstant::snapshotDelta(mUnitStartTimeSnapshot), durationString);

    VString    suiteBeginTag("<testsuite errors=\"0\" failures=\"%d\" name=\"%s\" tests=\"%d\" time=\"%s\">",
                mNumFailedTests,    // these are "failures", not "errors", in JUnit parlance
                mName.chars(),
                mNumSuccessfulTests + mNumFailedTests,
                durationString.chars());

    xmlOutputStream->writeLine(suiteBeginTag);
    
    for (TestInfoVector::const_iterator i = mResults.begin(); i != mResults.end(); ++i)
        {
        VTestInfo    info = (*i);

        _getDurationString(info.mDuration, durationString);
        
        // We need to cleanse the description for XML.
        VString    xmlCleanDescription(info.mDescription);
        xmlCleanDescription.replace("&", "&amp;");
        xmlCleanDescription.replace("\"", "&quot;");
        xmlCleanDescription.replace("<", "&lt;");
        xmlCleanDescription.replace(">", "&gt;");

        VString    openCaseTag("<testcase class=\"%s\" name=\"%s\" time=\"%s\">",
                    mName.chars(),
                    xmlCleanDescription.chars(),
                    durationString.chars());
        VString    closeCaseTag("</testcase>");
        
        if (info.mSuccess)
            {
            xmlOutputStream->writeString("    ");
            xmlOutputStream->writeString(openCaseTag);
            xmlOutputStream->writeLine(closeCaseTag);
            }
        else
            {
            xmlOutputStream->writeString("    ");
            xmlOutputStream->writeLine(openCaseTag);

            xmlOutputStream->writeString("        ");
            VString failureTag("<failure message=\"Test case %s failed.\" />", xmlCleanDescription.chars());
            xmlOutputStream->writeLine(failureTag);

            xmlOutputStream->writeString("    ");
            xmlOutputStream->writeLine(closeCaseTag);
            }

        }

    xmlOutputStream->writeLine("</testsuite>");
    
    xmlOutputStream->flush();
    }

// VTestInfo -----------------------------------------------------------------

VTestInfo::VTestInfo(bool success, const VString& description, const VDuration& duration) :
mSuccess(success),
mDescription(description),
mDuration(duration)
    {
    }

