/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

#ifndef vlogger_h
#define vlogger_h

/** @file */

#include "vmutex.h"
#include "vbinaryiostream.h"
#include "vbufferedfilestream.h"
#include "vsocketstream.h"
#include "vtextiostream.h"
#include "vinstant.h"

class VString;
class VBentoNode;

/**
    @ingroup toolbox
*/

/*
These macros make it easier to emit log messages without as much verbose typing.
They are also far more efficient than calling APIs directly, because they can avoid
message formatting overhead if the log level will not in fact emit anything.
Each macro has two versions: one using the default logger, one using a named logger.
At ERROR level or lower, these macros add the caller's file and line number to the
output automatically.

Most common use cases, using INFO level:
    VLOGGER_INFO("Hey!"); // writes to default logger
    VLOGGER_LEVEL(VLogger::kInfo, "Hey!"); // equivalent, but wordier; useful if level is a variable
    VLOGGER_NAMED_INFO("mylogger", "Hey!"); // outputs to "mylogger" if present, default logger otherwise
    VLOGGER_LINE(VLogger::kInfo, "Hey!"); // like VLOGGER_INFO, but adds file and line number
    VLOGGER_HEXDUMP(VLogger::kInfo, "Hey!", myBuffer, bufferLength); // hex dump of buffer to default logger at INFO level
*/

/** Emits a message at a specified level to the default logger. */
#define VLOGGER_LEVEL(level, message) do { if (!VLogger::isLogLevelActive(level)) break; VLogger* vlcond = VLogger::getLoggerConditional(VString::EMPTY(), level); if (vlcond != NULL) vlcond->log(level, NULL, 0, message); } while (false)
/** Emits a message at a specified level to the default logger, including file and line number passed in (used by macros below which have file/line). */
#define VLOGGER_LEVEL_FILELINE(level, message, file, line) do { if (!VLogger::isLogLevelActive(level)) break; VLogger* vlcond = VLogger::getLoggerConditional(VString::EMPTY(), level); if (vlcond != NULL) vlcond->log(level, file, line, message); } while (false)
/** Emits a message at kFatal level to the default logger, then throws that message in a VException.
Note that this is not inherently "fatal" unless your call stack decides to make it so. This is just a
convenience function to do both steps in one call. */
#define VLOGGER_FATAL_AND_THROW(message) do { VLogger::getDefaultLogger()->log(VLogger::kFatal, __FILE__, __LINE__, message); throw VStackTraceException(message); } while (false)
/** Emits a message at kFatal level to the default logger.
Note that emitting this log message is not a fatal action and does not terminate the application. */
#define VLOGGER_FATAL(message) VLogger::getDefaultLogger()->log(VLogger::kFatal, __FILE__, __LINE__, message)
/** Emits a message at kError level to the default logger. */
#define VLOGGER_ERROR(message) VLOGGER_LEVEL_FILELINE(VLogger::kError, message, __FILE__, __LINE__)
/** Emits a message at kWarn level to the default logger. */
#define VLOGGER_WARN(message) VLOGGER_LEVEL(VLogger::kWarn, message)
/** Emits a message at kInfo level to the default logger. */
#define VLOGGER_INFO(message) VLOGGER_LEVEL(VLogger::kInfo, message)
/** Emits a message at kDebug level to the default logger. */
#define VLOGGER_DEBUG(message) VLOGGER_LEVEL(VLogger::kDebug, message)
/** Emits a message at kTrace level to the default logger. */
#define VLOGGER_TRACE(message) VLOGGER_LEVEL(VLogger::kTrace, message)
/** Emits a message at a specified level, including file and line number, to the default logger. */
#define VLOGGER_LINE(level, message) VLOGGER_LEVEL_FILELINE(level, message, __FILE__, __LINE__)
/** Emits a hex dump at a specified level to the default logger. */
#define VLOGGER_HEXDUMP(level, message, buffer, length) do { if (!VLogger::isLogLevelActive(level)) break; VLogger* vlcond = VLogger::getLoggerConditional(VString::EMPTY(), level); if (vlcond != NULL) vlcond->logHexDump(level, message, buffer, length); } while (false)
/** Returns true if the default logger would emit at the specified level. */
#define VLOGGER_WOULD_LOG(level) (VLogger::isLogLevelActive(level) && (VLogger::getDefaultLogger()->getLevel() >= level))

/** Emits a message at a specified level to the specified logger. */
#define VLOGGER_NAMED_LEVEL(loggername, level, message) do { if (!VLogger::isLogLevelActive(level)) break; VLogger* vlcond = VLogger::getLoggerConditional(loggername, level); if (vlcond != NULL) vlcond->log(level, NULL, 0, message); } while (false)
/** Emits a message at a specified level, including file and line number, to the specified logger. */
#define VLOGGER_NAMED_LEVEL_FILELINE(loggername, level, message, file, line) do { if (!VLogger::isLogLevelActive(level)) break; VLogger* vlcond = VLogger::getLoggerConditional(loggername, level); if (vlcond != NULL) vlcond->log(level, file, line, message); } while (false)
/** Emits a message at kFatal level to the specified logger. */
#define VLOGGER_NAMED_FATAL(loggername, message) VLogger::getLogger(loggername)->log(VLogger::kFatal, __FILE__, __LINE__, message)
/** Emits a message at kError level to the specified logger. */
#define VLOGGER_NAMED_ERROR(loggername, message) VLOGGER_NAMED_LEVEL_FILELINE(loggername, VLogger::kError, message, __FILE__, __LINE__)
/** Emits a message at kWarn level to the specified logger. */
#define VLOGGER_NAMED_WARN(loggername, message) VLOGGER_NAMED_LEVEL(loggername, VLogger::kWarn, message)
/** Emits a message at kInfo level to the specified logger. */
#define VLOGGER_NAMED_INFO(loggername, message) VLOGGER_NAMED_LEVEL(loggername, VLogger::kInfo, message)
/** Emits a message at kDebug level to the specified logger. */
#define VLOGGER_NAMED_DEBUG(loggername, message) VLOGGER_NAMED_LEVEL(loggername, VLogger::kDebug, message)
/** Emits a message at kTrace level to the specified logger. */
#define VLOGGER_NAMED_TRACE(loggername, message) VLOGGER_NAMED_LEVEL(loggername, VLogger::kTrace, message)
/** Emits a message at a specified level, including file and line number, to the specified logger. */
#define VLOGGER_NAMED_LINE(loggername, level, message) VLOGGER_NAMED_LEVEL_FILELINE(loggername, level, message, __FILE__, __LINE__)
/** Emits a hex dump at a specified level to the specified logger. */
#define VLOGGER_NAMED_HEXDUMP(loggername, level, message, buffer, length) do { if (!VLogger::isLogLevelActive(level)) break; VLogger* vlcond = VLogger::getLoggerConditional(loggername, level); if (vlcond != NULL) vlcond->logHexDump(level, message, buffer, length); } while (false)
/** Returns true if the specified logger would emit at the specified level. */
#define VLOGGER_NAMED_WOULD_LOG(loggername, level) (VLogger::isLogLevelActive(level) && (VLogger::getLogger(loggername)->getLevel() >= level))

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

        VLoggerRepetitionFilter(const VLoggerRepetitionFilter&); // not copyable
        VLoggerRepetitionFilter& operator=(const VLoggerRepetitionFilter&); // not assignable

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
        Returns true if the specified level is active for at least one logger.
        False means no logger would log at that level. This is used by the logging
        macros to efficiently avoid all work (including message formatting) if none
        of the loggers would currently log at the specified level.
        @param logLevel the log level intended to log at
        @return true if some logger would emit at that log level; false if not
        */
        static bool isLogLevelActive(int logLevel);
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
        Sets one or all loggers' print stack info. If you specify a logger name and
        it's not found, then nothing happens. See setPrintStackInfo for details on
        parameter use.
        @param    name        empty for all loggers; or the name of a logger
        @param  printStackLevel    the level of detail <= which will cause a stack
                                    crawl to be emitted
        @param  maxNumOccurrences   if non-zero, a limit on the number of stack
                                    crawls that will be printed before turning the
                                    level back to zero (off)
        @param  timeLimit           if specified, a time limit from now at which point
                                    the level will be turned back to zero (off)
        */
        static void setPrintStackInfos(const VString& name, int printStackLevel, int maxNumOccurrences, const VDuration& timeLimit);
        /**
        Returns display info about the installed loggers. The result is
        a Bento structure with one child node per logger. The caller owns the
        Bento data and should delete it when it's no longer needed. Each child
        node has the logger's attributes:
        - string name
        - int logLevel
        - string nextLoggerName
        - int printStackLevel
        - int printStackMaxCount
        - duration printStackDuration
        - instant printStackExpiration
        @return a Bento data hierarchy as described above
        */
        static VBentoNode* getLoggerInfo();

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
        Returns the current level of detail, above which messages are suppressed.
        @return the current level of detail
        */
        int getLevel() const { return mLogLevel; }
        /**
        Sets the logger's detail level; messages with higher detail levels will be
        suppressed when logged.
        @param    logLevel    the level of detail for messages to emit
        */
        void setLevel(int logLevel);
        /**
        Enables or disables the repetition filter.
        */
        void setRepetitionFilterEnabled(bool enabled) { mRepetitionFilter.setEnabled(enabled); }
        /**
        Returns the current print stack level of detail, above which
        messages do not emit a stack crawl (defaults to kOff, so there are no
        stack crawls emitted)
        @return the current print stack level of detail
        */
        int getPrintStackLevel() const { return mPrintStackLevel; }
        /**
        Sets the logger's print stack configuration. By setting a print stack level
        to non-zero (zero is kOff), it means that any log output that passes this
        logger's mLogLevel, and which is less than or equal to the mPrintStackLevel,
        will cause a stack crawl to be printed to the log output. The typical use
        is to set it to kError so that you can find out what piece of code is calling
        into some other lower-level code that emits the error. The limit parameters
        let you avoid having an endless spew of stack crawls, and just get a few of
        them for a short period of time around an error condition.
        @param  printStackLevel    the level of detail <= which will cause a stack
                                    crawl to be emitted
        @param  maxNumOccurrences   if non-zero, a limit on the number of stack
                                    crawls that will be printed before turning the
                                    level back to zero (off)
        @param  timeLimit           if specified, a time limit from now at which point
                                    the level will be turned back to zero (off)
        */
        void setPrintStackInfo(int printStackLevel, int maxNumOccurrences, const VDuration& timeLimit);
        /**
        This callback is to be used only by VStackCrawl_emitStackCrawl() when it is called
        by VLogger. It will emit the supplied string without any locking, nor any formatting,
        under the presumption that the VLogger has already locked its mutex and is prepared
        to emit multiple raw lines via this callback. This is like rawLog() but without
        locking.
        @param  line    the line of text to be logged
        */
        void emitStackCrawlLine(const VString& line);
        
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

        /**
        Add a log to chain to (replaces the old "next").
        @param    name    name of the "next" log to chain to
        */
        void setNextLogger(const VString& name);
        /**
        Return the next log in the chain (if any)
        */
        const VString& getNextLogger() const { return mNextLoggerName; }
        /**
        Removes a log to chain to
        @param    logLevel    the level of detail for messages to emit
        */
        void removeNextLogger();

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
        virtual void emitMessage(int logLevel, const char* file, int line, const VString& message);

        /**
        Emits a raw line to the logger's output. Normally this is the function that
        concrete logger classes override, because the base class formats the message
        and then calls this function to emit the message. However, as mentioned in
        the comments for emitMessage(), a database logger might just override emitMessage() and
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

        VMutex mMutex;  ///< A mutex to protect against multiple threads' messages from being intertwined;
                        // subclasses may access this carefully; note that it is locked prior to any
                        // call to emitMessage() or emitRawLine(), so implementors of those functions must
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
        
        static void _checkMaxActiveLogLevelForNewLogger(int newActiveLogLevel); ///< Maintains the gMaxActiveLogLevel; called whenever some logger's level is changed to the supplied value.
        static void _checkMaxActiveLogLevelForRemovedLogger(int removedActiveLogLevel); ///< Maintains the gMaxActiveLogLevel; called whenever some logger's level is changed to the supplied value.
        static void _checkMaxActiveLogLevelForChangedLogger(int oldActiveLogLevel, int newActiveLogLevel); ///< Maintains the gMaxActiveLogLevel; called whenever some logger's level is changed to the supplied value.
        static void _recalculateMaxActiveLogLevel(); ///< Maintains the gMaxActiveLogLevel; called whenever a full rescan is needed to find the max log level.

        /**
        Finds the specified logger using an exact name match, without locking (assumes caller has locked
        the _mutexInstance()).
        @param  name    the logger name to match exactly
        @return the found logger, or NULL if not found
        */
        static VLogger* _findLoggerFromExactName(const VString& name);
        /**
        Finds the specified logger using a path name search, without locking (assumes caller has locked
        the _mutexInstance()). Path names are like Java packages (e.g, "path.to.my.logger.name"). The search
        first repeatedly strips off the tail until it either finds a match. If it fails to match even the
        first element, we return NULL.
        @param  name    the logger path name to match against
        @return the found logger, or NULL if not found at any path level
        */
        static VLogger* _findLoggerFromPathName(const VString& pathName);

        /**
        Set's the logger's detail level, and assumes the caller (usually the public setLevel())
        has locked the _mutexInstance().
        @param    logLevel    the level of detail for messages to emit
        */
        void _setLevel(int logLevel);
        /**
        Forwards the log output to the next logger in the chain if applicable.
        */
        void _forwardLog(int logLevel, const char* file, int line, const VString& message);
        /**
        Forwards the hex dump log output to thenext logger in the chain if applicable.
        */
        void _forwardLogHexDump(int logLevel, const VString& message, const Vu8* buffer, Vs64 length);
        /**
        Returns true if this logger is configured to forward to a next logger in chain.
        This value should normally be checked before calling _getNextLogger(), as an
        optimization to avoid extra function call overhead.
        */
        bool _hasNextLogger() const { return mNextLoggerName.isNotEmpty(); }
        /**
        Returns the next logger in the chain, or NULL if mNextLoggerName is empty or resolves
        to this logger itself.
        */
        VLogger* _getNextLogger() const;
        /**
        Emits a stack crawl if the log level is appropriate and stack crawl printing is
        enabled and has not timed out or reached the count limit; maintains the limit tracking
        in doing so.
        @param    logLevel    the level of detail of the message
        @param    file        the file name that is emitting the log message;
                            NULL will suppress file and line in output
        @param    line        the line number that is emitting the log message
        @param    message     the text to emit
        */
        void _printStackCrawl(int logLevel, const char* file, int line, const VString& message);

        int      mLogLevel;             ///< Only messages with a detail level <= this will be emitted.
        VString  mName;                 ///< This logger's unique name so it can be looked up.
        VString  mNextLoggerName;       ///< Name of the next logger in the chain, or empty if none.
        int      mPrintStackLevel;      ///< Messages with a detail level <= this will call the stack crawl printing function. kOff defeats it.
        int      mPrintStackMaxCount;   ///< Positive value indicates max stacks printed per mPrintStackDuration interval.
        VDuration mPrintStackDuration;  ///< Non-infinite value indicates interval for resetting countdown of max stacks.
        int      mPrintStackCountdown;  ///< Positive value is num remaining print stacks until mPrintStackResetTime.
        VInstant mPrintStackExpiration; ///< Next instant when stack printing countdown will reset (INFINITE_FUTURE means never).

        static VLogger* gDefaultLogger; ///< The default logger returned if get by name fails.
        volatile static int gMaxActiveLogLevel; ///< The max log level of the registered loggers. Allows fast fail of log level check without having to examine each logger.

        friend class VLoggerRepetitionFilter; // It will call our emitMessage() function to emit saved messages.
        friend class VLoggerUnit; // Unit test can peek into our internal state to verify intended behavior.
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

        VBufferedFileStream mFileStream;    ///< The underlying files stream we open and write to.
        VTextIOStream        mOutputStream; ///< The high-level text stream we write to.
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
VInterceptLogger is a logger that looks for an expected message to be logged, and
sets a flag if it ever sees that message once. You can re-use it for multiple
messages by setting a new expected message, which resets the flag. This can be
useful when unit testing code that communicates failure via logger output instead
of via a throw. But in general, the need to use this indicates a weak error
handling design.
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

        /**
        Sets the expected message to look for, and resets the "seen" flag.
        @param  message the message we will look for
        */
        void setExpectedMessage(const VString& message);
        /**
        Returns true if the specified message was the last message seen.
        @param message the message to test for
        @return true if the last message was the one specified
        */
        bool sawExpectedMessage() const { return mSawExpectedMessage; }
        /**
        Returns the expected message, useful if the expected message was not seen and
        you want to report the missed message text.
        @return obvious
        */
        const VString& getExpectedMessage() const { return mExpectedMessage; }

        /**
        Override of base class reset; resets to initial state.
        */
        virtual void reset();

    protected:

        // Overrides of VLogger subclass interface; we save the line
        // instead of sending it to some output.
        virtual void emitMessage(int logLevel, const char* file, int line, const VString& message);
        virtual void emitRawLine(const VString& line);

    private:

        VString mExpectedMessage;
        bool mSawExpectedMessage;
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

        // Overrides of VLogger subclass interface; we write to the file without
        // any time stamp or log level notation prefix.
        virtual void emitMessage(int logLevel, const char* file, int line, const VString& message);

    };

class VStringLogger : public VLogger
    {
    public:

        /**
        Constructs a logger that accumulates all output into one large VString,
        with '\n' embedded at the end of each emitted "line".
        @param  logLevel        the level of detail for messages to emit
        @param  name            the name of the logger, used for finding by name
        @param  nextLoggerName  the name of the next logger in chain, or empty if none
        */
        VStringLogger(int logLevel, const VString& name, const VString& nextLoggerName);
        /**
        Destructor.
        */
        virtual ~VStringLogger() {}
        
        /**
        Depending on how you use this logger, you may need to be careful about
        locking the mutex while accessing the lines. The rule is that during a period
        in which this logger may be logged to, you must not depend on the immutability
        of the mLines, except while you have locked the mutex; and while you have
        locked the mutex, another thread that attempts to log will block. The typical
        use case of this logger will be to capture some logger output locally, and then use
        those lines before continuing; such a use case doesn't require you to lock the mutex.
        @return a reference to the mutex controlling access to this logger's data
        */
        VMutex& getMutex() { return mMutex; }
        /**
        Returns a reference to the VString holding the captured log lines; if you need to
        retain access to the lines after this logger is destructed (e.g. beyond a scope block),
        then you must assign the result to a separate VString, or use orphanLines().
        @return a reference to the string of captured log data
        */
        const VString& getLines() const { return mLines; }
        /**
        Returns the raw char buffer underlying the captured log data, transfers ownership and
        delete[] responsibility of that buffer to the caller, and as a necessary side-effect resets
        the capture string to empty. This is most useful if you wish to efficiently use the captured
        log data beyond the lifecycle scope of this VStringLogger object once it has done the capture.
        @return the captured log data raw char buffer, now owned by the caller
        */
        const char* orphanLines() { return mLines.orphanDataBuffer(); }

    protected:

        /**
        Override of base class function, emits the line to cout.
        @param    line    the string to be emitted
        */
        virtual void emitRawLine(const VString& line);

    private:
    
        VString mLines;
    };

class VStringVectorLogger : public VLogger
    {
    public:

        /**
        Constructs a logger that accumulates all output into a VStringVector.
        @param  logLevel        the level of detail for messages to emit
        @param  name            the name of the logger, used for finding by name
        @param  nextLoggerName  the name of the next logger in chain, or empty if none
        @param  storage         if not NULL, an existing vector where the output is collected, rather than here;
                                    the supplied pointer must remain valid as long as this logger can log
        */
        VStringVectorLogger(int logLevel, const VString& name, const VString& nextLoggerName, VStringVector* storage=NULL);
        /**
        Destructor.
        */
        virtual ~VStringVectorLogger();

        /**
        Depending on how you use this logger, you may need to be careful about
        locking the mutex while accessing the lines. The rule is that during a period
        in which this logger may be logged to, you must not depend on the immutability
        of the mLines, except while you have locked the mutex; and while you have
        locked the mutex, another thread that attempts to log will block. The typical
        use case of this logger will be to capture some logger output locally, and then use
        those lines before continuing; such a use case doesn't require you to lock the mutex.
        @return a reference to the mutex controlling access to this logger's data
        */
        VMutex& getMutex() { return mMutex; }
        /**
        Returns a reference to the VString holding the captured log lines; if you need to
        retain access to the lines after this logger is destructed (e.g. beyond a scope block),
        then you must assign the result to a separate VStringVector.
        @return a reference to the string of captured log data
        */
        const VStringVector& getLines() const { return *mStorage; }

    protected:

        /**
        Override of base class function, emits the line to cout.
        @param    line    the string to be emitted
        */
        virtual void emitRawLine(const VString& line);

    private:
    
        VStringVector*  mStorage;   ///< Points to supplied storage if non-NULL storage was supplied in constructor; points to mLines otherwise.
        VStringVector   mLines;     ///< If no storage was supplied, we store the log data here.
    };

#endif /* vlogger_h */

