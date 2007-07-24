/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

/** @file */

#include "vlogger.h"

#include "vinstant.h"
#include "vmutexlocker.h"
#include "vmemorystream.h"
#include "vhex.h"
#include "vshutdownregistry.h"

V_STATIC_INIT_TRACE

// VLoggerRepetitionFilter ---------------------------------------------------

VLoggerRepetitionFilter::VLoggerRepetitionFilter() :
mEnabled(true),
mHasSavedMessage(false),
mNumSuppressedOccurrences(0),
mTimeOfLastOccurrence(VInstant::NEVER_OCCURRED()),
mLogLevel(0),
mFile(NULL),
mLine(0)
// mMessage -> empty
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
        logger->emit(mLogLevel, mFile, mLine, tweakedMessage);
        }
    else
        {
        logger->emit(mLogLevel, mFile, mLine, mMessage);
        }

    mHasSavedMessage = false;
    mNumSuppressedOccurrences = 0;
    }

// VLogger -------------------------------------------------------------------

// This style of static mutex declaration and access ensures correct
// initialization if accessed during the static initialization phase.
static VMutex* _mutexInstance()
    {
    static VMutex gMutex;
    return &gMutex;
    }

const VString VLogger::kDefaultLoggerName; // empty string by definition

typedef VLoggerList::const_iterator const_VLoggerIterator;
typedef VLoggerList::iterator VLoggerIterator;

VLoggerList VLogger::gLoggers;
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
    VMutexLocker locker(_mutexInstance());
    vault::vectorDeleteAll(gLoggers);
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
        VMutexLocker    locker(_mutexInstance());

        const_VLoggerIterator    i = gLoggers.begin();

        while (i != gLoggers.end())
            {
            VLogger*    logger = (*i);

            if (logger->mName == name)
                return logger;

            ++i;
            }
        }

    return VLogger::getDefaultLogger();
    }

// static
VLogger* VLogger::findLogger(const VString& name)
    {
    VMutexLocker    locker(_mutexInstance());

    const_VLoggerIterator    i = gLoggers.begin();

    while (i != gLoggers.end())
        {
        VLogger*    logger = (*i);

        if (logger->mName == name)
            return logger;

        ++i;
        }

    return NULL;
    }

// static
void VLogger::installLogger(VLogger* logger)
    {
    VMutexLocker    locker(_mutexInstance());

    for (VLoggerIterator i = gLoggers.begin(); i != gLoggers.end(); ++i)
        {
        VLogger*    existingLogger = (*i);

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
    gLoggers.push_back(logger);
    }

// static
void VLogger::deleteLogger(const VString& name)
    {
    VMutexLocker    locker(_mutexInstance());

    for (VLoggerIterator i = gLoggers.begin(); i != gLoggers.end(); ++i)
        {
        VLogger*    existingLogger = (*i);

        if (existingLogger->mName == name)
            {
            if (gDefaultLogger == existingLogger)
                gDefaultLogger = NULL;

            gLoggers.erase(i);        // erase only removes the vector element (the pointer)
            delete existingLogger;    // so we also must delete the object
            return;
            }
        }

    }

// static
void VLogger::setDefaultLogger(VLogger* logger)
    {
    VMutexLocker    locker(_mutexInstance());

    gDefaultLogger = logger;
    }

// static
void VLogger::setLogLevels(const VString& name, int logLevel)
    {
    VMutexLocker    locker(_mutexInstance());

    VLoggerIterator    i = gLoggers.begin();

    while (i != gLoggers.end())
        {
        VLogger*    logger = (*i);

        if (name.isEmpty() || (logger->mName == name))
            logger->setLevel(logLevel);

        ++i;
        }
    }

// static
void VLogger::getLoggerInfo(VStringVector& resultStrings)
    {
    VMutexLocker    locker(_mutexInstance());

    const_VLoggerIterator    i = gLoggers.begin();

    while (i != gLoggers.end())
        {
        VLogger*    logger = (*i);

        resultStrings.push_back(logger->mName);
        resultStrings.push_back(VString("%d", logger->mLogLevel));

        ++i;
        }
    }

// static
void VLogger::installDefaultLogger()
    {
    VMutexLocker    locker(_mutexInstance());

    if (gDefaultLogger == NULL)
        {
        VLogger*        logger = new VCoutLogger(kWarn, "default");

        gLoggers.push_back(logger);
        gDefaultLogger = logger;
        }
    }

VLogger::VLogger(int logLevel, const VString& name) :
mLogLevel(logLevel),
mName(name)
// mRepetitionFilter -> initialized
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
        VMutexLocker timeoutLocker(&mMutex);
        mRepetitionFilter.checkTimeout(this);
        }

	if (this->isEnabledFor(logLevel))
        {
        VLogger::_breakpointLocationForEmit();

        VMutexLocker locker(&mMutex);    // ensure multiple threads don't intertwine their log messages
        if (mRepetitionFilter.checkMessage(this, logLevel, file, line, message))
            this->emit(logLevel, file, line, message);
        }
    }

void VLogger::log(int logLevel, const VString& message)
    {
    this->log(logLevel, NULL, 0, message);
    }

void VLogger::logHexDump(int logLevel, const VString& message, const Vu8* buffer, Vs64 length)
    {
    // Short circuit immediately if the detail level is too high.
    if (! this->isEnabledFor(logLevel))
        return;

    this->log(logLevel, NULL, 0, message);

    // We use an optimization here if the supplied length is zero: rather than
    // go through the hex dump object to get nothing, we just stick a zero
    // length indicator message in our "preface" string and skip the hex part.

    VMutexLocker    locker(&mMutex);    // ensure multiple threads don't intertwine their log messages

    if (length > 0)
        {
        VMemoryStream    tempBuffer;
        VTextIOStream    stream(tempBuffer);
        VHex            hex(&stream);
        hex.printHex(buffer, length);
        Vu8                nullByte = 0;
        (void) tempBuffer.write(&nullByte, 1);    // null terminate so it is a valid C string

        this->emitRawLine((char*) tempBuffer.getBuffer());
        }
    }

void VLogger::rawLog(const VString& message)
    {
    VMutexLocker    locker(&mMutex);    // ensure multiple threads don't intertwine their log messages
    this->emitRawLine(message);
    }

void VLogger::emit(int logLevel, const char* file, int line, const VString& message)
    {
    VString    rawMessage;
    VLogger::format(rawMessage, logLevel, file, line, message);

    this->emitRawLine(rawMessage);
    }

// static
void VLogger::format(VString& stringToFormat, int logLevel, const char* file, int line, const VString& message)
    {
    VInstant    now;
    VString     timeStampString;
    now.getLocalString(timeStampString);

    VString    levelName;
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

// VSuppressionLogger ---------------------------------------------------------------

VSuppressionLogger::VSuppressionLogger(const VString& name) :
VLogger(kOff, name)
    {
    }

// VFileLogger ---------------------------------------------------------------

// It turns out that on Mac OS X it is much more practical to use Unix text
// files for logging. So if running Mac, use Unix rather than "native" line endings.
// Otherwise things like "tail -f" won't be useful.

VFileLogger::VFileLogger(int logLevel, const VString& name, const VString& filePath) :
VLogger(logLevel, name),
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

VCoutLogger::VCoutLogger(int logLevel, const VString& name) :
VLogger(logLevel, name)
    {
    std::cout << std::endl;
    }

void VCoutLogger::emitRawLine(const VString& line)
    {
    std::cout << line.chars() << std::endl;
    (void) fflush(stdout);
    }

// VInterceptLogger ---------------------------------------------------------------

VInterceptLogger::VInterceptLogger(int logLevel, const VString& name) :
VLogger(logLevel, name)
// mLastLoggedMessage constructs to empty string
    {
    }

bool VInterceptLogger::sawExpectedMessage(const VString& inMessage)
    {
    return (mLastLoggedMessage == inMessage);
    }

void VInterceptLogger::reset()
    {
    mLastLoggedMessage = VString::EMPTY();
    mRepetitionFilter.reset();
    }

void VInterceptLogger::emit(int /*logLevel*/, const char* /*file*/, int /*line*/, const VString& message)
    {
    // The intercept logger does not want the supplied message to be
    // time-stamped or have file/line information included, because a
    // comparison will be done against just the logged message itself.
    this->emitRawLine(message);
    }

void VInterceptLogger::emitRawLine(const VString& line)
    {
    mLastLoggedMessage = line;
    }

// VRawFileLogger ---------------------------------------------------------------

VRawFileLogger::VRawFileLogger(int logLevel, const VString& name, const VString& filePath) :
VFileLogger(logLevel, name, filePath)
    {
    }

void VRawFileLogger::emit(int /*logLevel*/, const char* /*file*/, int /*line*/, const VString& message)
    {
    // The raw file logger does not want the supplied message to be
    // time-stamped or have file/line information included, because a
    // comparison will be done against just the logged message itself.
    this->emitRawLine(message);
    }

