/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
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
#include "vinstant.h"

#include <vector>

class VString;

/**
    @ingroup toolbox
*/

// These macros make it easier to emit log messages without as much verbose typing.
// Each macro has two versions: one using the default logger, one using a named logger.

/** Emits a message at kFatal level to the default logger. */
#define VLOGGER_FATAL(message) VLogger::getDefaultLogger()->log(VLogger::kFatal, __FILE__, __LINE__, message)
/** Emits a message at kError level to the default logger. */
#define VLOGGER_ERROR(message) VLogger::getDefaultLogger()->log(VLogger::kError, __FILE__, __LINE__, message)
/** Emits a message at kWarn level to the default logger. */
#define VLOGGER_WARN(message) VLogger::getDefaultLogger()->log(VLogger::kWarn, NULL, 0, message)
/** Emits a message at kInfo level to the default logger. */
#define VLOGGER_INFO(message) VLogger::getDefaultLogger()->log(VLogger::kInfo, NULL, 0, message)
/** Emits a message at kDebug level to the default logger. */
//lint -e717 do ... while(0);
#define VLOGGER_DEBUG(message) do { VLogger* vlcond = VLogger::getLoggerConditional(VString::EMPTY(), VLogger::kDebug); if (vlcond != NULL) vlcond->log(VLogger::kDebug, NULL, 0, message); } while (false)
/** Emits a message at kTrace level to the default logger. */
//lint -e717 do ... while(0);
#define VLOGGER_TRACE(message) do { VLogger* vlcond = VLogger::getLoggerConditional(VString::EMPTY(), VLogger::kTrace); if (vlcond != NULL) vlcond->log(VLogger::kTrace, NULL, 0, message); } while (false)
/** Emits a message at a specified level to the default logger. */
//lint -e717 do ... while(0);
#define VLOGGER_LEVEL(level, message) do { VLogger* vlcond = VLogger::getLoggerConditional(VString::EMPTY(), level); if (vlcond != NULL) vlcond->log(level, NULL, 0, message); } while (false)
/** Emits a message at a specified level, including file and line number, to the default logger. */
//lint -e717 do ... while(0);
#define VLOGGER_LINE(level, message) do { VLogger* vlcond = VLogger::getLoggerConditional(VString::EMPTY(), level); if (vlcond != NULL) vlcond->log(level, __FILE__, __LINE__, message); } while (false)
/** Emits a hex dump at a specified level to the default logger. */
#define VLOGGER_HEXDUMP(level, message, buffer, length) do { VLogger* vlcond = VLogger::getLoggerConditional(VString::EMPTY(), level); if (vlcond != NULL) vlcond->logHexDump(level, message, buffer, length); } while (false)
/** Returns true if the default logger would emit at the specified level. */
#define VLOGGER_WOULD_LOG(level) (VLogger::getDefaultLogger()->getLevel() >= level)

/** Emits a message at kFatal level to the specified logger. */
#define VLOGGER_NAMED_FATAL(loggername, message) VLogger::getLogger(loggername)->log(VLogger::kFatal, __FILE__, __LINE__, message)
/** Emits a message at kError level to the specified logger. */
#define VLOGGER_NAMED_ERROR(loggername, message) VLogger::getLogger(loggername)->log(VLogger::kError, __FILE__, __LINE__, message)
/** Emits a message at kWarn level to the specified logger. */
#define VLOGGER_NAMED_WARN(loggername, message) VLogger::getLogger(loggername)->log(VLogger::kWarn, NULL, 0, message)
/** Emits a message at kInfo level to the specified logger. */
#define VLOGGER_NAMED_INFO(loggername, message) VLogger::getLogger(loggername)->log(VLogger::kInfo, NULL, 0, message)
/** Emits a message at kDebug level to the specified logger. */
#define VLOGGER_NAMED_DEBUG(loggername, message) do { VLogger* vlcond = VLogger::getLoggerConditional(loggername, VLogger::kDebug); if (vlcond != NULL) vlcond->log(VLogger::kDebug, NULL, 0, message); } while (false)
/** Emits a message at kTrace level to the specified logger. */
#define VLOGGER_NAMED_TRACE(loggername, message) do { VLogger* vlcond = VLogger::getLoggerConditional(loggername, VLogger::kTrace); if (vlcond != NULL) vlcond->log(VLogger::kTrace, NULL, 0, message); } while (false)
/** Emits a message at a specified level to the specified logger. */
#define VLOGGER_NAMED_LEVEL(loggername, level, message) do { VLogger* vlcond = VLogger::getLoggerConditional(loggername, level); if (vlcond != NULL) vlcond->log(level, NULL, 0, message); } while (false)
/** Emits a message at a specified level, including file and line number, to the specified logger. */
#define VLOGGER_NAMED_LINE(loggername, level, message) do { VLogger* vlcond = VLogger::getLoggerConditional(loggername, level); if (vlcond != NULL) vlcond->log(level, __FILE__, __LINE__, message); } while (false)
/** Emits a hex dump at a specified level to the specified logger. */
#define VLOGGER_NAMED_HEXDUMP(loggername, level, message, buffer, length) do { VLogger* vlcond = VLogger::getLoggerConditional(loggername, level); if (vlcond != NULL) vlcond->logHexDump(level, message, buffer, length); } while (false)
/** Returns true if the specified logger would emit at the specified level. */
#define VLOGGER_NAMED_WOULD_LOG(loggername, level) do { VLogger* vlcond = VLogger::getLoggerConditional(loggername, level); return (vlcond != NULL); } while (false)

class VLogger;
typedef std::vector<VLogger*> VLoggerList;

/**
VLoggerRepetitionFilter provides VLogger a simple way of preventing runaway
repetitive log output. If the same text is emitted multiple times in succession,
only the first and last occurrence is emitted (the last one is adorned with an
indication of how many occurrences were suppressed, if any).
*/
class VLoggerRepetitionFilter
    {
    public:

        VLoggerRepetitionFilter();
        virtual ~VLoggerRepetitionFilter() {}

        /**
        Some VLogger subclasses may want to simply disable filtering.
        This allows them to do so in their constructor, for example.
        @param enabled true if enabling, false if disabling
        */
        void setEnabled(bool enabled) { mEnabled = enabled; }
        /**
        Returns true if the filter is enabled.
        @return true if the filter is enabled
        */
        bool isEnabled() const { return mEnabled; }

        /**
        Clears any pending message so that the filter is back
        to an initial state. This does not change the enabled state.
        */
        void reset();

        /**
        Checks the proposed log message; may save it or increment the internal counter;
        may emit a pending saved message; returns true if the caller should proceed to
        emit the message normally.
        @param    logLevel    the level of detail of the message
        @param    file        the file name that is emitting the log message;
                                NULL will suppress file and line in output
        @param    line        the line number that is emitting the log message
        @param    message     the text to emit
        @return true if the caller should proceed to emit the message, false if not
        */
        bool checkMessage(VLogger* logger, int logLevel, const char* file, int line, const VString& message);

        /**
        Checks to see if a long time has elapsed since the pending repeat has been sitting
        here. This is called by the logger before checking the log level. This prevents the case
        where a repeat never gets output just because there's nothing after it that fits the
        log level.
        */
        void checkTimeout(VLogger* logger);

    private:

        void _emitSuppressedMessages(VLogger* logger);

        bool        mEnabled; // Certain logger subclasses may want to turn off filtering entirely.

        // Information defining the saved output.
        bool        mHasSavedMessage;
        int         mNumSuppressedOccurrences;
        VInstant    mTimeOfLastOccurrence;
        int         mLogLevel;
        const char* mFile;
        int         mLine;
        VString     mMessage;
    };

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
        Deletes all loggers, presumably at program termination.
        */
        static void shutdown();
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
        Returns the named logger, or the default logger if there is no logger
        with the specified name, but will return NULL if the resulting logger
        (specified or default) would not emit at the specified log level nor
        would any of its forward loggers. So you must check for a NULL result.
        This function is most useful in conditional logging macros. Note that
        the returned logger itself might not log at the specified level, but
        it or one of its forward loggers will.
        @param    name      the name of the logger to access
        @param    logLevel  the log level at which output would log
        @return a logger that will emit at the specified level, or NULL if none
            will, search starting at the specified logger, falling back to the
            default logger if the specified logger is not found
        */
        static VLogger* getLoggerConditional(const VString& name, int logLevel);
        /**
        Returns the named logger, or NULL if there is no logger with that name.
        @param    name    the name of the logger to access
        @return the specified logger, or NULL if not found
        */
        static VLogger* findLogger(const VString& name);
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
        @param  logLevel        the level of detail for messages to emit
        @param  name            the name of the logger, used for finding by name
        @param  nextLoggerName  the name of the next logger in chain, or empty if none
        */
        VLogger(int logLevel, const VString& name, const VString& nextLoggerName);
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
        @param    message   the text to emit
        */
        void log(int logLevel, const char* file, int line, const VString& message);
        void log(int logLevel, const VString& message);

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
        @param    message the text to be logged
        */
        void rawLog(const VString& message);

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

        static const int kOff   = 0;    ///< Level of detail to suppress all output.
        static const int kFatal = 1;    ///< Level of detail to only show failures likely to be fatal.
        static const int kError = 20;   ///< Level of detail to add error messages.
        static const int kWarn  = 40;   ///< Level of detail to add basic warning messages.
        static const int kInfo  = 60;   ///< Level of detail to add coarse-grained status messages.
        static const int kDebug = 80;   ///< Level of detail to add fine-grained status messages.
        static const int kTrace = 100;  ///< Level of detail to add trace-level messages.
        static const int kAll   = 100;  ///< Level of detail to output all messages.

        static const VString kDefaultLoggerName;

        /**
        Returns the log level as a short descriptive string ("warn", "error", etc.)
        for use in formatted message output.
        @param    logLevel    the level of detail
        @param    name        the string to format
        */
        static void getLevelName(int logLevel, VString& name);

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
        @param    message     the text to emit
        */
        virtual void emit(int logLevel, const char* file, int line, const VString& message);

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
        static void format(VString& stringToFormat, int logLevel, const char* file, int line, const VString& message);

        VMutex  mMutex;     ///< A mutex to protect against multiple threads' messages from being intertwined;
                                // subclasses may access this carefully; note that it is locked prior to any
                                // call to emit() or emitRawLine(), so implementors of those functions must
                                // not re-lock because to do so would cause a deadlock.

        VLoggerRepetitionFilter mRepetitionFilter;  ///< Used to prevent repetitive info from clogging output.

    private:

        /**
        Creates and sets a default logger. This is called if needed by getLogger()
        if there are no loggers installed and a default one is needed.
        */
        static void installDefaultLogger();

        static void _breakpointLocationForLog();
        static void _breakpointLocationForEmit();

        /**
        Forwards the log output to the next logger in the chain if applicable.
        */
        void _forwardLog(int logLevel, const char* file, int line, const VString& message);
        /**
        Forwards the hex dump log output to thenext logger in the chain if applicable.
        */
        void _forwardLogHexDump(int logLevel, const VString& message, const Vu8* buffer, Vs64 length);
        /**
        Returns the next logger in the chain, or NULL if mNextLoggerName is empty or resolves
        to this logger itself.
        */
        VLogger* _getNextLogger() const;

        int     mLogLevel;          ///< Only messages with a detail level <= this will be emitted.
        VString mName;              ///< This logger's unique name so it can be looked up.
        VString mNextLoggerName;    ///< Name of the next logger in the chain, or empty if none.

        static VLoggerList  gLoggers;       ///< The list of installed loggers.
        static VLogger*     gDefaultLogger; ///< The default logger returned if get by name fails.

        friend class VLoggerRepetitionFilter; // It will call our emit() function to emit saved messages.
    };

/**
VForwardingLogger is a logger that produces no output of its own but can
be initialized with a "next" logger to which output is forwarded; its
purpose is to allow chaining of loggers to facilitate multiple named
loggers having output implemented in one logger. For example, you could
define a file logger named "abc", and have several forwarding loggers
with particular names whose output goes to the "abc" logger. You can do
this with other logger classes but they also emit their output to
concrete destinations. This one just forwards.
*/
class VForwardingLogger : public VLogger
    {
    public:

        /**
        Constructs a new forwarding logger object.
        @param  name            the name of the logger, used for finding by name
        @param  nextLoggerName  the name of the next logger in chain; empty makes no sense but is allowed
        */
        VForwardingLogger(const VString& name, const VString& nextLoggerName);
        /**
        Destructor.
        */
        virtual ~VForwardingLogger() {}
    };

/**
VSuppressionLogger is a logger that eats everything. You could install
it as the default logger to make sure nothing is emitted to cout since
normally a VCoutLogger is installed as the default, if that's what you
want. This type of logger never chains to a next logger because its
purpose is to consume all of its output.
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
        @param  logLevel        the level of detail for messages to emit
        @param  name            the name of the logger, used for finding by name
        @param  nextLoggerName  the name of the next logger in chain, or empty if none
        @param  filePath        the path of the file to create and write to
        */
        VFileLogger(int logLevel, const VString& name, const VString& nextLoggerName, const VString& filePath);
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
        @param  logLevel        the level of detail for messages to emit
        @param  name            the name of the logger, used for finding by name
        @param  nextLoggerName  the name of the next logger in chain, or empty if none
        */
        VCoutLogger(int logLevel, const VString& name, const VString& nextLoggerName);
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
        Constructs a new intercept logger object.
        @param  logLevel        the level of detail for messages to emit
        @param  name            the name of the logger, used for finding by name
        @param  nextLoggerName  the name of the next logger in chain, or empty if none
        */
        VInterceptLogger(int logLevel, const VString& name, const VString& nextLoggerName);
        /**
        Destructor.
        */
        virtual ~VInterceptLogger() {}

        bool sawExpectedMessage(const VString& inMessage);
        const VString& getLastMessage() const { return mLastLoggedMessage; }

        virtual void reset();

    protected:

        virtual void emit(int logLevel, const char* file, int line, const VString& message);
        virtual void emitRawLine(const VString& line);

    private:

        VString mLastLoggedMessage;
    };

/**
VRawFileLogger is a VFileLogger that does not format the output in any way
(no timestamp or log level indication).
*/
class VRawFileLogger : public VFileLogger
    {
    public:

        /**
        Constructs a new raw file logger object.
        @param  logLevel        the level of detail for messages to emit
        @param  name            the name of the logger, used for finding by name
        @param  nextLoggerName  the name of the next logger in chain, or empty if none
        @param  filePath        the path of the file to create and write to
        */
        VRawFileLogger(int logLevel, const VString& name, const VString& nextLoggerName, const VString& filePath);
        /**
        Destructor.
        */
        virtual ~VRawFileLogger() {}

    protected:

        virtual void emit(int logLevel, const char* file, int line, const VString& message);

    };

#endif /* vlogplus_h */

