/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
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

// VLogger -------------------------------------------------------------------

const VString VLogger::kDefaultLoggerName; // empty string by definition

typedef VLoggerList::const_iterator const_VLoggerIterator;
typedef VLoggerList::iterator VLoggerIterator;

VLoggerList VLogger::gLoggers;
VLogger* VLogger::gDefaultLogger = NULL;
VMutex VLogger::gLoggersMutex;

// static
void VLogger::shutdown()
    {
    VMutexLocker locker(&gLoggersMutex);
    vault::vectorDeleteAll(gLoggers);
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
        VMutexLocker    locker(&gLoggersMutex);

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
    VMutexLocker    locker(&gLoggersMutex);

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
    VMutexLocker    locker(&gLoggersMutex);
    
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
    VMutexLocker    locker(&gLoggersMutex);
    
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
    VMutexLocker    locker(&gLoggersMutex);

    gDefaultLogger = logger;
    }

// static
void VLogger::setLogLevels(const VString& name, int logLevel)
    {
    VMutexLocker    locker(&gLoggersMutex);

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
    VMutexLocker    locker(&gLoggersMutex);

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
    VMutexLocker    locker(&gLoggersMutex);

    if (gDefaultLogger == NULL)
        {
        VLogger*        logger = new VCoutLogger(kWarn, "default");

        gLoggers.push_back(logger);
        gDefaultLogger = logger;
        }
    }

static void _installShutdownCallback()
    {
    // One-time only initialization to install shutdown cleanup hook.
    static bool loggerShutdownInstalled = false;
    if (! loggerShutdownInstalled)
        {
        loggerShutdownInstalled = true;
        VShutdownRegistry::instance()->registerFunction(VLogger::shutdown);
        }
    }

VLogger::VLogger(int logLevel, const VString& name) :
mLogLevel(logLevel),
mName(name)
    {
    _installShutdownCallback(); // one-time initialization
    }

VLogger::~VLogger()
    {
    }

void VLogger::log(int logLevel, const char* file, int line, const char* inFormat, ...)
    {
    if (this->isEnabledFor(logLevel))
        {
        VMutexLocker    locker(&mMutex);    // ensure multiple threads don't intertwine their log messages
        va_list    args;
        va_start(args, inFormat);

        this->emit(logLevel, file, line, inFormat, args);

        va_end(args);
        }
    }

void VLogger::log(int logLevel, const char* inFormat, ...)
{
    if (this->isEnabledFor(logLevel))
    {
        va_list    args;
        va_start(args, inFormat);

        VMutexLocker    locker(&mMutex);    // ensure multiple threads don't intertwine their log messages
        this->emit(logLevel, NULL, 0, inFormat, args);

        va_end(args);
    }
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

void VLogger::emit(int logLevel, const char* file, int line, const char* inFormat, va_list args)
    {
    VString    message;
    VLogger::vaFormat(message, logLevel, file, line, inFormat, args);
    
    this->emitRawLine(message);
    }

// static
void VLogger::format(VString& stringToFormat, int logLevel, const char* file, int line, const char* inFormat, ...)
    {
     va_list    args;
    va_start(args, inFormat);

    VLogger::vaFormat(stringToFormat, logLevel, file, line, inFormat, args);

    va_end(args);
    }

// static
void VLogger::vaFormat(VString& stringToFormat, int logLevel, const char* file, int line, const char* inFormat, va_list args)
    {
    VString    message;
    message.vaFormat(inFormat, args);
    
    VInstant    now;
    VString        timeStampString;
    now.getLocalString(timeStampString);
    
    VString    levelName;
    VLogger::getLevelName(logLevel, levelName);

    // If there's file/line number info, then always show it.
    if (file == NULL)
        stringToFormat.format("%s [%s] %s", timeStampString.chars(), levelName.chars(), message.chars());
    else
        stringToFormat.format("%s [%s] @ %s line %d: %s", timeStampString.chars(), levelName.chars(), file, line, message.chars());
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

    mOutputStream.writeLine(VString::kEmptyString);
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


void VInterceptLogger::emit(int /*logLevel*/, const char* /*file*/, int /*line*/, const char* inFormat, va_list args)
    {
    VString    message;
    message.vaFormat(inFormat, args);
    this->emitRawLine(message);
    }

void VInterceptLogger::emitRawLine(const VString& line)
    {
    mLastLoggedMessage = line;
    }

