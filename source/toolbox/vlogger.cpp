/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vlogger.h"

#include "vinstant.h"
#include "vmutexlocker.h"
#include "vmemorystream.h"
#include "vhex.h"
#include "vshutdownregistry.h"
#include "vbento.h"

V_STATIC_INIT_TRACE

// VLoggerRepetitionFilter ---------------------------------------------------

VLoggerRepetitionFilter::VLoggerRepetitionFilter() :
mEnabled(true),
mHasSavedMessage(false),
mNumSuppressedOccurrences(0),
mTimeOfLastOccurrence(VInstant::NEVER_OCCURRED()),
mLogLevel(0),
mFile(NULL),
mLine(0),
mMessage() // -> empty
    {
    }

void VLoggerRepetitionFilter::reset()
    {
    mHasSavedMessage = false;
    mNumSuppressedOccurrences = 0;
    mTimeOfLastOccurrence = VInstant::NEVER_OCCURRED();
    mLogLevel = 0;
    mFile = NULL;
    mLine = 0;
    mMessage = VString::EMPTY();
    }

bool VLoggerRepetitionFilter::checkMessage(VLogger* logger, int logLevel, const char* file, int line, const VString& message)
    {
    if (!mEnabled)
        return true;

    bool isRepeatMessage = mHasSavedMessage &&
        (logLevel == mLogLevel) &&
        (file == mFile) &&
        (line == mLine) &&
        (message == mMessage);

    if (isRepeatMessage)
        {
        // This is a repeat message. Update our info and return false to indicate that
        // this message should not yet be emitted.
        ++mNumSuppressedOccurrences;
        mTimeOfLastOccurrence.setNow();
        }
    else
        {
        // This is not a repeat message. Emit any pending saved recurring message,
        // then reset to store this message, and return true to indicate that
        // this message should be emitted (the first occurrence of a message is
        // always emitted).

        // Emit pending saved message.
        if (mHasSavedMessage && (mNumSuppressedOccurrences > 0))
            this->_emitSuppressedMessages(logger);

        // Reset and store this new message.
        mHasSavedMessage = true;
        mNumSuppressedOccurrences = 0;
        mTimeOfLastOccurrence.setNow();
        mLogLevel = logLevel;
        mFile = file;
        mLine = line;
        mMessage = message;
        }

    return ! isRepeatMessage;
    }

void VLoggerRepetitionFilter::checkTimeout(VLogger* logger)
    {
    if (!mEnabled)
        return;

    if (mHasSavedMessage && (mNumSuppressedOccurrences > 0))
        {
        VInstant now;
        if ((now - mTimeOfLastOccurrence) > VDuration::MINUTE())
            this->_emitSuppressedMessages(logger);
        }
    }

void VLoggerRepetitionFilter::_emitSuppressedMessages(VLogger* logger)
    {
    // If there was only 1 suppressed message, no need to mark it.
    if (mNumSuppressedOccurrences > 1)
        {
        VString tweakedMessage("[%dx] %s", mNumSuppressedOccurrences, mMessage.chars());
        logger->emitMessage(mLogLevel, mFile, mLine, tweakedMessage);
        }
    else
        {
        logger->emitMessage(mLogLevel, mFile, mLine, mMessage);
        }

    mHasSavedMessage = false;
    mNumSuppressedOccurrences = 0;
    }

// VLogger -------------------------------------------------------------------

// This style of static mutex declaration and access ensures correct
// initialization if accessed during the static initialization phase.
static VMutex* _mutexInstance()
    {
    static VMutex gMutex("VLogger _mutexInstance() gMutex", true/*suppress logging of the logger mutex*/);
    return &gMutex;
    }

static VLoggerList& _loggers()
    {
    static VLoggerList gLoggerList;
    return gLoggerList;
    }

const VString VLogger::kDefaultLoggerName; // empty string by definition

typedef VLoggerList::const_iterator const_VLoggerIterator;
typedef VLoggerList::iterator VLoggerIterator;

VLogger* VLogger::gDefaultLogger = NULL;

/*
We define a shutdown method here, but we don't install it into the
VShutdownRegistry. This is because experience has shown that the
shutdown registry sequence often involves code that emits log output,
and if we delete the loggers before all log output is done, that log
output will just end up creating a new default cout logger. So
instead, if you want to delete loggers during shutdown, call this
function directly as late as possible--ideally, as the last line of
code in main().
*/
// static
void VLogger::shutdown()
    {
    VMutexLocker locker(_mutexInstance(), "VLogger::shutdown()");
    vault::vectorDeleteAll(_loggers());
    gDefaultLogger = NULL;
    }

// static
VLogger* VLogger::getDefaultLogger()
    {
    // Install one if there isn't one yet.

    if (gDefaultLogger == NULL)
        VLogger::installDefaultLogger();

    return gDefaultLogger;
    }

// static
VLogger* VLogger::getLogger(const VString& name)
    {
    // If user is asking for default logger, don't bother searching.
    if (! name.isEmpty())
        {
        VMutexLocker locker(_mutexInstance(), "VLogger::getLogger()");

        for (const_VLoggerIterator i = _loggers().begin(); i != _loggers().end(); ++i)
            {
            if ((*i)->mName == name)
                return *i;
            }
        }

    return VLogger::getDefaultLogger();
    }

// static
VLogger* VLogger::getLoggerConditional(const VString& name, int logLevel)
    {
    VLogger* logger = VLogger::getLogger(name);

    while ((logger != NULL) && (logger->getLevel() < logLevel))
        {
        logger = logger->_getNextLogger();
        }

    return logger;
    }

// static
VLogger* VLogger::findLogger(const VString& name)
    {
    VMutexLocker locker(_mutexInstance(), "VLogger::findLogger()");

    for (const_VLoggerIterator i = _loggers().begin(); i != _loggers().end(); ++i)
        {
        if ((*i)->mName == name)
            return *i;
        }

    return NULL;
    }

// static
void VLogger::installLogger(VLogger* logger)
    {
    VMutexLocker locker(_mutexInstance(), "VLogger::installLogger()");

    for (VLoggerIterator i = _loggers().begin(); i != _loggers().end(); ++i)
        {
        VLogger* existingLogger = (*i);

        // Protect agains caller accidentally installing the same logger object twice.
        // Otherwise, we'll end up deleting the object being installed.
        if (existingLogger == logger)
            return;

        if (existingLogger->mName == logger->mName)
            {
            if (gDefaultLogger == existingLogger)
                gDefaultLogger = logger;

            *i = logger;            // replace the vector entry
            delete existingLogger;    // delete the one we're replacing
            return;
            }
        }

    // No match found, so just add this one.
    _loggers().push_back(logger);

    if (gDefaultLogger == NULL)
        gDefaultLogger = logger;
    }

// static
void VLogger::deleteLogger(const VString& name)
    {
	// 2008.08.12 JHR ARGO-13876  Logging messages to N4 to script output
	VLogger* theLogger = VLogger::findLogger(name);
	if (theLogger == NULL)
		return;		// not found

	VMutexLocker locker(_mutexInstance(), "VLogger::deleteLogger()");

	// If the logger is "chained", remove the chain reference first.
    for (VLoggerIterator i = _loggers().begin(); i != _loggers().end(); ++i)
        {
        VLogger* existingLogger = (*i);

        if (existingLogger->mNextLoggerName == name)
			existingLogger->mNextLoggerName = theLogger->mNextLoggerName;
        }


 	// Finally, delete the actual logger
	for (VLoggerIterator i = _loggers().begin(); i != _loggers().end(); ++i)
        {
        VLogger* existingLogger = (*i);

        if (existingLogger->mName == name)
            {
            if (gDefaultLogger == existingLogger)
                gDefaultLogger = NULL;

            _loggers().erase(i);        // erase only removes the vector element (the pointer)
            delete existingLogger;    // so we also must delete the object
            return;
            }
        }

    }

// static
void VLogger::setDefaultLogger(VLogger* logger)
    {
    VMutexLocker locker(_mutexInstance(), "VLogger::setDefaultLogger()");

    gDefaultLogger = logger;
    }

// static
void VLogger::setLogLevels(const VString& name, int logLevel)
    {
    VMutexLocker locker(_mutexInstance(), "VLogger::setLogLevels()");

	for (VLoggerIterator i = _loggers().begin(); i != _loggers().end(); ++i)
        {
        VLogger* logger = (*i);

        if (name.isEmpty() || (logger->mName == name))
            logger->setLevel(logLevel);
        }
    }

// static
void VLogger::setPrintStackInfos(const VString& name, int printStackLevel, int maxNumOccurrences, const VDuration& timeLimit)
    {
    VMutexLocker locker(_mutexInstance(), "VLogger::setPrintStackLevels()");

	for (VLoggerIterator i = _loggers().begin(); i != _loggers().end(); ++i)
        {
        VLogger* logger = (*i);

        if (name.isEmpty() || (logger->mName == name))
            logger->setPrintStackInfo(printStackLevel, maxNumOccurrences, timeLimit);
        }
    }

// static
VBentoNode* VLogger::getLoggerInfo()
    {
    VBentoNode* node = new VBentoNode("loggerInfo");
    VMutexLocker locker(_mutexInstance(), "VLogger::getLoggerInfo()");


    for (const_VLoggerIterator i = _loggers().begin(); i != _loggers().end(); ++i)
        {
        const VLogger* logger = (*i);
        
        VBentoNode* child = node->addNewChildNode("logger");
        child->addString("name", logger->mName);
        child->addInt("logLevel", logger->mLogLevel);
        child->addString("nextLoggerName", logger->mNextLoggerName);
        child->addInt("printStackLevel", logger->mPrintStackLevel);
        child->addInt("printStackMaxCount", logger->mPrintStackMaxCount);
        child->addDuration("printStackDuration", logger->mPrintStackDuration);
        child->addInstant("printStackExpiration", logger->mPrintStackExpiration);
        }
    
    return node;
    }

// static
void VLogger::installDefaultLogger()
    {
    VMutexLocker locker(_mutexInstance(), "VLogger::installDefaultLogger()");

    if (gDefaultLogger == NULL)
        {
        VLogger* logger = new VCoutLogger(kWarn, "default", VString::EMPTY());

        _loggers().push_back(logger);
        gDefaultLogger = logger;
        }
    }

VLogger::VLogger(int logLevel, const VString& name, const VString& nextLoggerName) :
mMutex(VString("VLogger(%s)::mMutex", name.chars()), true/*this mutex itself must not log*/), // -> unlocked
mRepetitionFilter(),
mLogLevel(logLevel),
mName(name),
mNextLoggerName(nextLoggerName),
mPrintStackLevel(kOff),
mPrintStackMaxCount(-1),
mPrintStackDuration(VDuration::POSITIVE_INFINITY()),
mPrintStackCountdown(-1),
mPrintStackExpiration(VInstant::INFINITE_FUTURE())
    {
    }

VLogger::~VLogger()
    {
    }

void VLogger::_breakpointLocationForLog()
    {
    // Put a breakpoint here if you want to break on every message that
    // is potentially logged, regardless of log level.
    }

void VLogger::_breakpointLocationForEmit()
    {
    // Put a breakpoint here if you want to break on every message that
    // is actually logged after the logger checks the log level.
    }

void VLogger::log(int logLevel, const char* file, int line, const VString& message)
    {
    VLogger::_breakpointLocationForLog();

    if (mRepetitionFilter.isEnabled()) // avoid mutex if no need to check filter
        {
        VMutexLocker timeoutLocker(&mMutex, "VLogger::log() checkTimeout");
        mRepetitionFilter.checkTimeout(this);
        }

    if (this->isEnabledFor(logLevel))
        {
        VLogger::_breakpointLocationForEmit();

        VMutexLocker locker(&mMutex, "VLogger::log()");    // ensure multiple threads don't intertwine their log messages
        if (mRepetitionFilter.checkMessage(this, logLevel, file, line, message))
            {
            this->emitMessage(logLevel, file, line, message);
            this->_printStackCrawl(logLevel, file, line, message);
            }
        }

    if (this->_hasNextLogger())
        this->_forwardLog(logLevel, file, line, message);
    }

void VLogger::log(int logLevel, const VString& message)
    {
    this->log(logLevel, NULL, 0, message);
    }

void VLogger::logHexDump(int logLevel, const VString& message, const Vu8* buffer, Vs64 length)
    {
    // Short circuit immediately if the detail level is too high.
    if (this->isEnabledFor(logLevel))
        {
        VMutexLocker locker(&mMutex, "VLogger::logHexDump()");    // ensure multiple threads don't intertwine their log messages

        this->emitMessage(logLevel, NULL, 0, message);

        // If the length is zero, we don't output any lines at all.
        if (length > 0)
            {
            VMemoryStream   tempBuffer;
            VTextIOStream   stream(tempBuffer);
            VHex            hex(&stream);
            hex.printHex(buffer, length);
            Vu8             nullByte = 0;
            (void) tempBuffer.write(&nullByte, 1);    // null terminate so it is a valid C string

            this->emitRawLine((char*) tempBuffer.getBuffer());
            }
        }

    if (this->_hasNextLogger())
        this->_forwardLogHexDump(logLevel, message, buffer, length);
    }

void VLogger::rawLog(const VString& message)
    {
    VMutexLocker locker(&mMutex, "VLogger::rawLog()");    // ensure multiple threads don't intertwine their log messages
    this->emitRawLine(message);
    }

void VLogger::setPrintStackInfo(int printStackLevel, int maxNumOccurrences, const VDuration& timeLimit)
    {
    VMutexLocker locker(&mMutex, "VLogger::setPrintStackInfo()");

    mPrintStackLevel = printStackLevel;
    mPrintStackMaxCount = (maxNumOccurrences > 0) ? maxNumOccurrences : -1; // 0 really means off (-1)
    mPrintStackDuration = timeLimit;

    mPrintStackCountdown = mPrintStackMaxCount;
    if (timeLimit.isSpecific())
        mPrintStackExpiration = VInstant(/*now*/) + timeLimit;
    else
        mPrintStackExpiration = VInstant::INFINITE_FUTURE();
    }

void VLogger::emitStackCrawlLine(const VString& line)
    {
    // We assume we're being called with mMutex locked, from VLogger::log()
    // calling VStackCrawl_emitStackCrawl(this).
    this->emitRawLine(line);
    }

void VLogger::emitMessage(int logLevel, const char* file, int line, const VString& message)
    {
    VString rawMessage;
    VLogger::format(rawMessage, logLevel, file, line, message);

    this->emitRawLine(rawMessage);
    }

void VLogger::_forwardLog(int logLevel, const char* file, int line, const VString& message)
    {
    VLogger* next = this->_getNextLogger();
    if (next != NULL)
        next->log(logLevel, file, line, message);
    }

void VLogger::_forwardLogHexDump(int logLevel, const VString& message, const Vu8* buffer, Vs64 length)
    {
    VLogger* next = this->_getNextLogger();
    if (next != NULL)
        next->logHexDump(logLevel, message, buffer, length);
    }

VLogger* VLogger::_getNextLogger() const
    {
    VLogger* result = NULL;

    if (mNextLoggerName.isNotEmpty())
        {
        VLogger* next = VLogger::getLogger(mNextLoggerName);
        if ((next != NULL) && (next != this))
            result = next;
        }

    return result;
    }

void VLogger::_printStackCrawl(int logLevel, const char* file, int line, const VString& message)
    {
    if (logLevel <= mPrintStackLevel)
        {
        // mPrintStackCountdown of -1 means no count limit, just a timeout limit, then turn off
        // mPrintStackExpiration of infinite means no time limit, just a count limit, then turn off
        // if both are set, it means count down to zero, then suppress until timeout is reached
        // if neither is set, we always print the stack crawl for this level
        
        bool resetCountdown = false;
        bool turnOff = false;
        bool printStack = false;
        VInstant now;
        
        if (mPrintStackCountdown == -1)
            {
            if (mPrintStackDuration == VDuration::POSITIVE_INFINITY())
                {
                // no count limit, no time limit: always print
                printStack = true;
                }
            else
                {
                // no count limit, but time limit defined: print if not expired, turn off if expired
                printStack = (now < mPrintStackExpiration);
                turnOff = !printStack;
                }
            }
        else if (mPrintStackCountdown == 0)
            {
            if (mPrintStackDuration == VDuration::POSITIVE_INFINITY())
                {
                // count limit reached, no time limit: turn off printing completely
                turnOff = true;
                }
            else
                {
                // count limit reached, time limit defined: check time limit and reset if expired
                resetCountdown = (now >= mPrintStackExpiration);
                printStack = resetCountdown;
                }
            }
        else // mPrintStackCountdown > 0
            {
            // count limit exists but not yet reached, so print
            printStack = true;
            
            if (mPrintStackDuration == VDuration::POSITIVE_INFINITY())
                {
                // no time limit: nothing to do
                }
            else
                {
                // time limit defined: reset if expired
                resetCountdown = (now >= mPrintStackExpiration);
                }
            }
            
        if (resetCountdown)
            {
            mPrintStackExpiration = (now + mPrintStackDuration); // easier than looping increment from previous to future; good enough
            
            if (mPrintStackMaxCount > 0)
                mPrintStackCountdown = mPrintStackMaxCount;
            }
            
        if (printStack)
            {
            if (mPrintStackCountdown > 0)
                --mPrintStackCountdown;
                
            VString rawMessage;
            VLogger::format(rawMessage, logLevel, file, line, message);
            VThread::logStackCrawl(VString("Stack for log entry: %s", rawMessage.chars()), this, false);
            }
        
        if (turnOff)
            {
            mPrintStackLevel = kOff;
            mPrintStackMaxCount = -1;
            mPrintStackDuration = VDuration::POSITIVE_INFINITY();
            mPrintStackCountdown = -1;
            mPrintStackExpiration = VInstant::INFINITE_FUTURE();
            this->emitMessage(kInfo, NULL, 0, "Print stack crawl for this logger is auto-disabling now.");
            }
        
        }
    }

// static
void VLogger::format(VString& stringToFormat, int logLevel, const char* file, int line, const VString& message)
    {
    VInstant    now;
    VString     timeStampString;
    now.getLocalString(timeStampString);

    // ARGO-18646 log both current and simulated time.
    if ((VInstant::getSimulatedClockOffset() != VDuration::ZERO()) || VInstant::isTimeFrozen()) 
        {
        now.setTrueNow();
        VString     trueTimeStampString;
        now.getLocalString(trueTimeStampString);

        timeStampString = trueTimeStampString + " " + timeStampString;
        }

    VString levelName;
    VLogger::getLevelName(logLevel, levelName);

    // If there's file/line number info, then always show it.
    if (file == NULL)
        stringToFormat.format("%s [%s] %s", timeStampString.chars(), levelName.chars(), message.chars());
    else
        stringToFormat.format("%s [%s] @ %s:%d: %s", timeStampString.chars(), levelName.chars(), file, line, message.chars());
    }

// static
void VLogger::getLevelName(int logLevel, VString& name)
    {
    // We make all strings 5 characters long for clean layout in the log output.

    if (logLevel == kFatal)
        name = "fatal";
    else if (logLevel == kError)
        name = "error";
    else if (logLevel == kWarn)
        name = "warn ";
    else if (logLevel == kInfo)
        name = "info ";
    else if (logLevel == kDebug)
        name = "debug";
    else if (logLevel == kTrace)
        name = "trace";
    else if (logLevel > kDebug)
        name.format("dbg%2d", logLevel);
    else if (logLevel > kInfo)
        name.format("inf%2d", logLevel);
    else if (logLevel > kWarn)
        name.format("wrn%2d", logLevel);
    else if (logLevel > kError)
        name.format("err%2d", logLevel);
    else
        name.format("%5d", logLevel);
    }

// 2008.08.12 JHR ARGO-13876 Logging messages to N4 to script output 
void VLogger::setNextLogger(const VString& name)
{
	mNextLoggerName = name;
}

// 2008.08.12 JHR ARGO-13876 Logging messages to N4 to script output 
void VLogger::removeNextLogger()
{
	this->setNextLogger(VString::EMPTY());
}

// VForwardingLogger ---------------------------------------------------------------

VForwardingLogger::VForwardingLogger(const VString& name, const VString& nextLoggerName) :
VLogger(kOff, name, nextLoggerName)
    {
    }

// VSuppressionLogger ---------------------------------------------------------------

VSuppressionLogger::VSuppressionLogger(const VString& name) :
VLogger(kOff, name, VString::EMPTY())
    {
    }

// VFileLogger ---------------------------------------------------------------

// It turns out that on Mac OS X it is much more practical to use Unix text
// files for logging. So if running Mac, use Unix rather than "native" line endings.
// Otherwise things like "tail -f" won't be useful.

VFileLogger::VFileLogger(int logLevel, const VString& name, const VString& nextLoggerName, const VString& filePath) :
VLogger(logLevel, name, nextLoggerName),
mFileStream(VFSNode(filePath)),
mOutputStream(mFileStream)
    {
    mFileStream.openReadWrite();
    mFileStream.seek(CONST_S64(0), SEEK_END);

    mOutputStream.writeLine(VString::EMPTY());
    }

void VFileLogger::emitRawLine(const VString& line)
    {
    mOutputStream.writeLine(line);
    mOutputStream.flush();
    }

// VCoutLogger ---------------------------------------------------------------

VCoutLogger::VCoutLogger(int logLevel, const VString& name, const VString& nextLoggerName) :
VLogger(logLevel, name, nextLoggerName)
    {
    std::cout << std::endl;
    }

void VCoutLogger::emitRawLine(const VString& line)
    {
    std::cout << line.chars() << std::endl;
    (void) fflush(stdout);
    }

// VInterceptLogger ---------------------------------------------------------------

VInterceptLogger::VInterceptLogger(int logLevel, const VString& name, const VString& nextLoggerName) :
VLogger(logLevel, name, nextLoggerName),
mExpectedMessage(),
mSawExpectedMessage(false)
    {
    }

void VInterceptLogger::setExpectedMessage(const VString& message)
    {
    mExpectedMessage = message;
    mSawExpectedMessage = false;
    mRepetitionFilter.reset();
    }
    
void VInterceptLogger::reset()
    {
    mExpectedMessage = VString::EMPTY();
    mSawExpectedMessage = false;
    mRepetitionFilter.reset();
    }

void VInterceptLogger::emitMessage(int /*logLevel*/, const char* /*file*/, int /*line*/, const VString& message)
    {
    // The intercept logger does not want the supplied message to be
    // time-stamped or have file/line information included, because a
    // comparison will be done against just the logged message itself.
    this->emitRawLine(message);
    }

void VInterceptLogger::emitRawLine(const VString& line)
    {
    if (!mSawExpectedMessage)
        mSawExpectedMessage = (mExpectedMessage == line);
    }

// VRawFileLogger ---------------------------------------------------------------

VRawFileLogger::VRawFileLogger(int logLevel, const VString& name, const VString& nextLoggerName, const VString& filePath) :
VFileLogger(logLevel, name, nextLoggerName, filePath)
    {
    }

void VRawFileLogger::emitMessage(int /*logLevel*/, const char* /*file*/, int /*line*/, const VString& message)
    {
    // The raw file logger does not want the supplied message to be
    // time-stamped or have file/line information included, because a
    // comparison will be done against just the logged message itself.
    this->emitRawLine(message);
    }

// VStringLogger ---------------------------------------------------------------

VStringLogger::VStringLogger(int logLevel, const VString& name, const VString& nextLoggerName) :
VLogger(logLevel, name, nextLoggerName),
mLines()
    {
    }

void VStringLogger::emitRawLine(const VString& line)
    {
    mLines += line;
    mLines += VString::NATIVE_LINE_ENDING();
    }

// VStringVectorLogger ---------------------------------------------------------------

VStringVectorLogger::VStringVectorLogger(int logLevel, const VString& name, const VString& nextLoggerName) :
VLogger(logLevel, name, nextLoggerName),
mLines()
    {
    }

void VStringVectorLogger::emitRawLine(const VString& line)
    {
    mLines.push_back(line);
    }

