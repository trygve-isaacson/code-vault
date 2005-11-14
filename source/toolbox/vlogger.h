/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

#ifndef vlogplus_h
#define vlogplus_h

/** @file */

#include "vmutex.h"
#include "vbinaryiostream.h"
#include "vbufferedfilestream.h"
#include "vsocketstream.h"
#include "vtextiostream.h"

#include <vector>

class VString;

/**
    @ingroup toolbox
*/

// These macros make it easier to emit log messages without as much verbose typing.
// Each macro has two versions: one using the default logger, one using a named logger.

/** Emits a message at kFatal level to the default logger. */
#define VLOGGER_FATAL(message) VLogger::getDefaultLogger()->log(VLogger::kFatal, NULL, 0, message)
/** Emits a message at kError level to the default logger. */
#define VLOGGER_ERROR(message) VLogger::getDefaultLogger()->log(VLogger::kError, NULL, 0, message)
/** Emits a message at kWarn level to the default logger. */
#define VLOGGER_WARN(message) VLogger::getDefaultLogger()->log(VLogger::kWarn, NULL, 0, message)
/** Emits a message at kInfo level to the default logger. */
#define VLOGGER_INFO(message) VLogger::getDefaultLogger()->log(VLogger::kInfo, NULL, 0, message)
/** Emits a message at kDebug level to the default logger. */
#define VLOGGER_DEBUG(message) VLogger::getDefaultLogger()->log(VLogger::kDebug, NULL, 0, message)
/** Emits a message at kTrace level to the default logger. */
#define VLOGGER_TRACE(message) VLogger::getDefaultLogger()->log(VLogger::kTrace, NULL, 0, message)
/** Emits a message at a specified level to the default logger. */
#define VLOGGER_LEVEL(level, message) VLogger::getDefaultLogger()->log(level, NULL, 0, message)
/** Emits a message at a specified level, including file and line number, to the default logger. */
#define VLOGGER_LINE(level, message) VLogger::getDefaultLogger()->log(level, __FILE__, __LINE__, message)
/** Emits a hex dump at a specified level to the default logger. */
#define VLOGGER_HEXDUMP(level, message, buffer, length) VLogger::getDefaultLogger()->logHexDump(level, message, buffer, length)

/** Emits a message at kFatal level to the default logger. */
#define VLOGGER_NAMED_FATAL(loggername, message) VLogger::getLogger(loggername)->log(VLogger::kFatal, NULL, 0, message)
/** Emits a message at kError level to the default logger. */
#define VLOGGER_NAMED_ERROR(loggername, message) VLogger::getLogger(loggername)->log(VLogger::kError, NULL, 0, message)
/** Emits a message at kWarn level to the default logger. */
#define VLOGGER_NAMED_WARN(loggername, message) VLogger::getLogger(loggername)->log(VLogger::kWarn, NULL, 0, message)
/** Emits a message at kInfo level to the default logger. */
#define VLOGGER_NAMED_INFO(loggername, message) VLogger::getLogger(loggername)->log(VLogger::kInfo, NULL, 0, message)
/** Emits a message at kDebug level to the default logger. */
#define VLOGGER_NAMED_DEBUG(loggername, message) VLogger::getLogger(loggername)->log(VLogger::kDebug, NULL, 0, message)
/** Emits a message at kTrace level to the default logger. */
#define VLOGGER_NAMED_TRACE(loggername, message) VLogger::getLogger(loggername)->log(VLogger::kTrace, NULL, 0, message)
/** Emits a message at a specified level to the default logger. */
#define VLOGGER_NAMED_LEVEL(loggername, level, message) VLogger::getLogger(loggername)->log(level, NULL, 0, message)
/** Emits a message at a specified level, including file and line number, to the default logger. */
#define VLOGGER_NAMED_LINE(loggername, level, message) VLogger::getLogger(loggername)->log(level, __FILE__, __LINE__, message)
/** Emits a hex dump at a specified level to the default logger. */
#define VLOGGER_NAMED_HEXDUMP(loggername, level, message, buffer, length) VLogger::getLogger(loggername)->logHexDump(level, message, buffer, length)

class VLogger;
typedef std::vector<VLogger*> VLoggerList;

/**
VLogger provides a logging interface that looks very similar to log4j in C++.
You can instantiate loggers and log to them directly via the pointer to the
logger object (usually via the VLOGGER macros that are defined), but the
drawback there is that you have to have access to the logger object pointer
whenver you want to log. So the better way is to "install" the logger object,
and then log to it by name. You can leave the name empty if you want to log
to the default logger; if you don't install any loggers, a default logger is
created as soon as it is needed, and it will log to std::cout.
*/
class VLogger
    {
    public:
    
        /**
        Returns the default logger, creating it if necessary.
        Because this function always returns a pointer
        to a valid logger object, you don't need to check for NULL.
        @return the specified logger, or the default logger if not found
        */
        static VLogger* getDefaultLogger();
        /**
        Returns the named logger, or the default logger if there is no logger
        with the specified name. Because this function always returns a pointer
        to a valid logger object, you don't need to check for NULL. (It will
        create and install a default logger if necessary as a failsafe.)
        @param    name    the name of the logger to access
        @return the specified logger, or the default logger if not found
        */
        static VLogger* getLogger(const VString& name);
        /**
        Installs a logger in the list of named loggers you can look up. If this
        is the first logger installed, it is set to be the default logger. If
        there is already a logger with this logger's name, it is deleted. See
        the cautionary note on deletion in the comments for deleteLogger().
        @param    logger    the logger to install
        */
        static void installLogger(VLogger* logger);
        /**
        Deletes a logger, specified by name. If no such logger is found, nothing
        happens. It is important that no one (in another thread, for example)
        has retained a pointer to the deleted logger, because it will be deleted
        from under them; you need to know whether this is a problem for your
        application usage. If you delete the default logger and don't set another
        one as default, then a new default logger will get set or created when it
        is needed.
        @param    name    the name of the logger to delete
        */
        static void deleteLogger(const VString& name);
        /**
        Sets the default logger. This can be called after some other logger has
        been installed as the default, to change which logger is the default.
        */
        static void setDefaultLogger(VLogger* logger);
        /**
        Sets one or all loggers' levels. If you specify a logger name and
        it's not found, then nothing happens.
        @param    name        empty for all loggers; or the name of a logger
        @param    logLevel    the level to set the logger(s) to
        */
        static void setLogLevels(const VString& name, int logLevel);
        /**
        Returns display info about the installed loggers. The result is
        two strings pushed onto the supplied vector for each logger.
        The first string is the logger name. The second string is its
        log level as a string. So if there are 4 loggers, there will
        be 8 strings pushed.
        @param    resultStrings    the string vector that will be modified
        */
        static void getLoggerInfo(VStringVector& resultStrings);
    
        /**
        Constructs a new logger object.
        @param    logLevel    the level of detail for messages to emit
        @param    name        the name of the logger, used for finding by name
        */
        VLogger(int logLevel, const VString& name);
        /**
        Destructor.
        */
        virtual ~VLogger();

        /**
        The core logging interface, usually called via one of the VLOGGER macros.
        Normally the message is formatted with time stamp and log level before it
        is written to the log, and is entirely suppressed if the specified log
        level is lower than the logger's level.
        @param    logLevel    the level of detail of the message; if lower than
                            the logger's level, will be suppressed
        @param    file        the file name that is emitting the log message;
                            NULL will suppress file and line in output
        @param    line        the line number that is emitting the log message
        @param    inFormat        the formatting string
        @param    ...            variable arguments for the formatting string
        */
        void log(int logLevel, const char* file, int line, const char* inFormat, ...);
        void log(int logLevel, const char* inFormat, ...);

        /**
        Logs the specified message as if separately logged, followed by a hex
        dump of a buffer, if the log level is appropriate.
        @param    logLevel    the level of detail of the message; if lower than
                            the logger's level, will be suppressed
        @param    message        the log message
        @param    buffer        the buffer of data to be hex dumped
        @param    length        the number of bytes of data to be hex dumped
        */
        void logHexDump(int logLevel, const VString& message, const Vu8* buffer, Vs64 length);

        /**
        The raw logging interface, not usually called but available if you need to
        log something without any timestamp or other information added to the text
        (for example in unit test output that you want to be diff'able) or need to
        log it regardless of the logger's log level. The message will be logged as-is
        and will not be suppressed based on log level.
        @param    inFormat        the formatting string
        @param    ...            variable arguments for the formatting string
        */
        void rawLog(const char* inFormat, ...);
        
        /**
        Returns the current level of detail, below which messages are suppressed.
        @return the current level of detail
        */
        int getLevel() const { return mLogLevel; }
        /**
        Sets the logger's detail level; messages with higher detail levels will be
        suppressed when logged.
        @param    logLevel    the level of detail for messages to emit
        */
        void setLevel(int logLevel) { mLogLevel = logLevel; }
        /**
        Returns the logger's name.
        @return the logger name
        */
        const VString& getName() const { return mName; }

        /**
        Returns true if the specified level of detail would be emitted given the
        current detail level setting.
        @param logLevel    the level of detail to check
        @return true if messages with the specified level of detail would be emitted
        */
        bool isEnabledFor(int logLevel) { return logLevel <= mLogLevel; }

        CLASS_CONST(int, kOff, 0);        ///< Level of detail to suppress all output.
        CLASS_CONST(int, kFatal, 1);    ///< Level of detail to only show failures likely to be fatal.
        CLASS_CONST(int, kError, 20);    ///< Level of detail to add error messages.
        CLASS_CONST(int, kWarn, 40);    ///< Level of detail to add basic warning messages.
        CLASS_CONST(int, kInfo, 60);    ///< Level of detail to add coarse-grained status messages.
        CLASS_CONST(int, kDebug, 80);    ///< Level of detail to add fine-grained status messages.
        CLASS_CONST(int, kTrace, 100);    ///< Level of detail to add trace-level messages.
        CLASS_CONST(int, kAll, 100);    ///< Level of detail to output all messages.
        
        static const VString kDefaultLoggerName;
        
    protected:

        /**
        Emits a message to the logger's output. The base class implementation calls
        the format() function to build the final message string, and then calls
        emitRawLine() to write the message. A logger class that does not want to have the
        standard output format applied directly to the message should override this
        message; for example a database logger might want the parameters to be
        placed in columns in a database table rather than formatted into the string
        itself. But logger classes that just log text should probably just override
        emitRawLine(), not this function. When this function is called, the log level
        has already been checked, so this function MUST emit the message; the log level
        is provided so that it can be included in the output, not so that this function
        can decided whether the level is high enough to emit the output.
        @param    logLevel    the level of detail of the message
        @param    file        the file name that is emitting the log message;
                            NULL will suppress file and line in output
        @param    line        the line number that is emitting the log message
        @param    inFormat    the formatting string
        @param    args        the argument list
        */
        virtual void emit(int logLevel, const char* file, int line, const char* inFormat, va_list args);
        
        /**
        Emits a raw line to the logger's output. Normally this is the function that
        concrete logger classes override, because the base class formats the message
        and then calls this function to emit the message. However, as mentioned in
        the comments for emit(), a database logger might just override emit() and
        do the work there. The override implementation should just do its thing without
        calling inherited.
        @param    line    the string to be emitted
        */
        virtual void emitRawLine(const VString& /*line*/) {}
        
        /**
        Builds a string using the standard default logging output format.
        This is declared protected so that subclasses can format the message
        in the standard way if they need to. Most subclasses will just
        override emitRawLine() and receive a message that has already been
        formatted.
        @param    stringToFormat    the string to be formatted
        @param    logLevel        the level of detail of the message
        @param    file            the file name that is emitting the log message
        @param    line            the line number that is emitting the log message
        @param    inFormat        the formatting string
        @param    ...                variable arguments for the formatting string
        */
        static void format(VString& stringToFormat, int logLevel, const char* file, int line, const char* inFormat, ...);

        /**
        Builds a string using the standard default logging output format.
        This is declared protected so that subclasses can format the message
        in the standard way if they need to. Most subclasses will just
        override emitRawLine() and receive a message that has already been
        formatted.
        @param    stringToFormat    the string to be formatted
        @param    logLevel        the level of detail of the message
        @param    file            the file name that is emitting the log message
        @param    line            the line number that is emitting the log message
        @param    inFormat        the formatting string
        @param    args            the argument list
        */
        static void vaFormat(VString& stringToFormat, int logLevel, const char* file, int line, const char* inFormat, va_list args);

        /**
        Returns the log level as a short descriptive string ("warn", "error", etc.)
        for use in formatted message output.
        @param    logLevel    the level of detail
        @param    name        the string to format
        */
        static void getLevelName(int logLevel, VString& name);
    
    private:

        /**
        Creates and sets a default logger. This is called if needed by getLogger()
        if there are no loggers installed and a default one is needed.
        */
        static void installDefaultLogger();

        VMutex    mMutex;        ///< A mutex to protect against multiple threads' messages from being intertwined
        int        mLogLevel;    ///< Only messages with a detail level <= this will be emitted.
        VString    mName;        ///< This logger's unique name so it can be looked up.
        
        static VLoggerList    smLoggers;            ///< The list of installed loggers.
        static VLogger*        smDefaultLogger;    ///< The default logger returned if get by name fails.
        static VMutex        smLoggersMutex;        ///< Protects default logger in housekeeping code.
    };

/**
VSuppressionLogger is a logger that eats everything. You could install
it as the default logger to make sure nothing is emitted to cout since
normally a VCoutLogger is installed as the default, if that's what you
want.
*/
class VSuppressionLogger : public VLogger
    {
    public:
    
        /**
        Constructs a new suppression logger object.
        @param    name    the name of the logger, used for finding by name
        */
        VSuppressionLogger(const VString& name);
        /**
        Destructor.
        */
        virtual ~VSuppressionLogger() {}
    };

/**
VFileLogger is a logger that writes each message as a line in a text file.
*/
class VFileLogger : public VLogger
    {
    public:
    
        /**
        Constructs a new file logger object.
        @param    logLevel    the level of detail for messages to emit
        @param    name        the name of the logger, used for finding by name
        @param    filePath    the path of the file to create and write to
        */
        VFileLogger(int logLevel, const VString& name, const VString& filePath);
        /**
        Destructor.
        */
        virtual ~VFileLogger() {}

    protected:

        /**
        Override of base class function, emits the line to the file.
        @param    line    the string to be emitted
        */
        virtual void emitRawLine(const VString& line);
    
    private:

        VBufferedFileStream    mFileStream;    ///< The underlying files stream we open and write to.
        VTextIOStream        mOutputStream;    ///< The high-level text stream we write to.
    };

class VCoutLogger : public VLogger
    {
    public:
    
        /**
        Constructs a new standard output logger object.
        @param    logLevel    the level of detail for messages to emit
        @param    name        the name of the logger, used for finding by name
        */
        VCoutLogger(int logLevel, const VString& name);
        /**
        Destructor.
        */
        virtual ~VCoutLogger() {}

    protected:

        /**
        Override of base class function, emits the line to cout.
        @param    line    the string to be emitted
        */
        virtual void emitRawLine(const VString& line);
    };

/**
VInterceptLogger is a logger that remembers the last message logged and allows
examination for use in classes that communicate failure via logger output instead
of via a throw.
*/
class VInterceptLogger : public VLogger
    {
    public:
    
        /**
        Constructs a new suppression logger object.
        */
        VInterceptLogger(int logLevel, const VString& name);
        /**
        Destructor.
        */
        virtual ~VInterceptLogger() {}

        bool sawExpectedMessage(const VString& inMessage);
        const VString& getLastMessage() const { return mLastLoggedMessage; }
        
    protected:
        virtual void emit(int logLevel, const char* file, int line, const char* inFormat, va_list args);
        virtual void emitRawLine(const VString& line);
        
    private:
        VString mLastLoggedMessage;
    };


#endif /* vlogplus_h */

