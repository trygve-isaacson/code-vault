/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

#ifndef vlogger_h
#define vlogger_h

/** @file */

#include "vtypes.h"

#include "vmutex.h"
#include "vbufferedfilestream.h"
#include "vtextiostream.h"
#include <boost/shared_ptr.hpp>

class VSettings;
class VSettingsNode;
class VBentoNode;

/**

    @defgroup vlogger Vault Logging

    <h1>Overview</h1>

    The VLogger facility has been improved in Vault 3.3, keeping the macro API for emitting
    log output exactly the same, while changing the internals and changing some of the APIs
    needed when programmatically accessing logger objects or creating custom subclasses.

    VLogger is the static API for accessing loggers directly and modifying the configuration
    on the fly. It is not an instantiable class.

    A VNamedLogger is a filtering object that receives log data based on name match, and
    restricts output by log level. It sends filtered output to one or more appenders.

    VLogAppender is an abstract class from which concrete classes are derived that write output
    to a specific destination such as a file or the console. Appenders receive level-filtered
    output from loggers that reference them. An appender can be referenced by more than one logger,
    and can be put in a "global appender" list to which all loggers also send their level-filtered
    output.

    A factory pattern is used to allow a concrete appender object to be instantiated when a logger
    needs it. By registering your own factory before initialization, you can have custom appender
    classes referenced in the configuration that is set up during initialization.

    In Vault 3.2 and earlier, an instantiated VLogger combined the functionality of VNamedLogger
    and VLogAppender, which was simpler but lacked flexibility in routing output at different
    log levels. With these improvements in 3.3, you now have the ability to route multiple loggers
    at different levels to the same appender, and route any logger's output to multiple appenders.

    <h1>Emitting Log Output</h1>

    Application code that wants to emit log output should simply use the various VLOGGER_xxxx()
    macros to write output. There are two flavors; the ones with shorter names of the form
    VLOGGER_<level>() write to the default logger at the specified level, while the ones with
    the longer names of the form VLOGGER_NAMED_<level>() have an extra parameter that is the
    name of the logger to write to.

    These macros expand to code that does not evaluate the string value unless the output may actually
    be emitted. Therefore, the caller does not have to be concerned about any overhead of formatting
    a complex string that would not be logged at the current level. For example, if you were to log
    the following expensive-to-build string at DEBUG level, when no loggers are currently configured
    at DEBUG level, the expense is not incurred:

    <pre>
        VLOGGER_DEBUG(VSTRING_FORMAT("State is: %d.", expensiveStateCalculation()));
    </pre>

    So there is no need to test the debug level before formatting in that case. However, if you do
    have a case where you want to avoid all overhead (say, in a tight loop or for precalculating some
    value to be used in subsequent logging), you can simply call VLOGGER_WOULD_LOG(level) or
    VLOGGER_NAMED_WOULD_LOG(name, level) in advance.

    If a specified named logger is not found, the system emits to another logger. In the simple case,
    this just means using the default logger. However, you can set up a naming hierarchy where logger
    names use a "dot.separated.naming.convention", because the fallback search is done by repeatedly
    stripping off the last segment of such a name and looking for a match. Ultimately it falls back
    to the default logger. The hierarchical search only occurs if you actually use the dot separated
    naming convention in the name you supply when emitting. You can treat this much like log4j uses
    Java package names, but since C++ does not have such a hierarchy built-in you need to decide what
    the hierarchy is, and you can use arbitrary hierarchies for arbitrary purposes (for example, the
    hierarchy could be unrelated to the class structure of the code, and instead describe some
    hierarchy of entities and their ids).

    <h1>Configuration XML</h1>

    If you do nothing to configure logging, a logger at INFO level is created upon first use, and
    it creates an appender to the console output upon first need. So you get all output at the console.

    The preferred way to configure logging is to call VLogger::configure() at startup, supplying it
    a VSettings object (which was loaded from XML) to define the loggers and appenders that are
    created.

    Here is a simple example configuration that sets up logging to log at DEBUG level to a file "my.log".

    <pre>
        <loggers>
            <appender name="default" kind="file" file="my.log" />
            <logger name="default" level="80" appender="default" />
        </loggers>
    </pre>

    Here is a more complex configuration with two loggers and two appenders, where one of the loggers
    writes to both of the appenders.

    <pre>
        <loggers>
            <appender name="primary" kind="file" file="my.log" />
            <appender name="console" kind="console" />
            <logger name="default" level="60" appender="primary" />
            <logger name="combo" level="80">
                <appender name="primary" />
                <appender name="console" />
            </logger>
        </loggers>
    </pre>

    For a given kind of appender, you can configure default property values to be applied to any
    particular appender of the kind where the values are not specified. For example, to set the
    default value of the "format-output" boolean to false for all file loggers, you would include:

    <pre>
        <loggers>
            <appender-defaults kind="file" format-output="false" />
        </loggers>
    </pre>

    You can specify in that manner any property appropriate to the appender kind indicated.

    The documentation for each concrete appender class describes the settings is supports.
    The following settings are defined by the base class VLogAppender, so they can be specified
    for any appender individually or via defaults:
    - "format-output" (boolean)
      Defaults to true. This determines whether an appender formats each message before writing it.
      Typically the formatting means prefixing the message with a current timestamp, the log level name, and
      the current thread name. In some cases it may be useful to omit such prefix data, for example
      when using logging to emit output during tests that will need to be "diff'ed" without regard
      to the time each line was written.
    - "print-stack-level" (int)
      Defaults to 0 (VLoggerLevel::OFF). If non-zero, this indicates a log level that when the
      appender receives a message at that level or less, the appender also emits a stack trace.
    - "print-stack-count" (int)
      Defaults to -1. If greater than zero, this is the limit on the number of times a stack trace
      will be emitted via the "print-stack-level" trigger before being disabled. This can be used
      to prevent runaway repeated stack tracing.
    - "print-stack-duration" (duration string such as "30s")
      Defaults to infinity ("INFINITY") meaning no limit. If specific, this is a time limit on how
      long stack tracing will continue to emit once triggered. It is another way of preventing runaway
      repeated stack tracing.

    <h1>Custom Appenders</h1>

    Call VLogger::registerLogAppenderFactory() to make your custom appender available to the system.
    Do so prior to calling VLogger::configure() if you want the XML configuration to be able to
    use your custom appenders.

    At a minimum, derive your appender from VLogAppender, and override emitRaw() to write the specified
    string to the destination output medium. If you wish to alter the normal format of the output,
    instead (or in addition) override emit() which must format the actual string to be emitted and then
    emitRaw() it. Look at the provided appenders as examples.

*/

/**
    @ingroup vlogger
*/

// This first set of macros sends output to the default logger.
#define VLOGGER_LEVEL(level, message) do { if (!VLogger::isDefaultLogLevelActive(level)) break; VLogger::getDefaultLogger()->log(level, NULL, 0, message); } while (false)
#define VLOGGER_LEVEL_FILELINE(level, message, file, line) do { if (!VLogger::isDefaultLogLevelActive(level)) break; VLogger::getDefaultLogger()->log(level, file, line, message); } while (false)
#define VLOGGER_LINE(level, message) VLOGGER_LEVEL_FILELINE(level, message, __FILE__, __LINE__)
#define VLOGGER_FATAL_AND_THROW(message) do { VLogger::getDefaultLogger()->log(VLoggerLevel::FATAL, __FILE__, __LINE__, message); throw VStackTraceException(message); } while (false)
#define VLOGGER_FATAL(message) VLOGGER_LEVEL_FILELINE(VLoggerLevel::FATAL, message, __FILE__, __LINE__)
#define VLOGGER_ERROR(message) VLOGGER_LEVEL_FILELINE(VLoggerLevel::ERROR, message, __FILE__, __LINE__)
#define VLOGGER_WARN(message) VLOGGER_LEVEL(VLoggerLevel::WARN, message)
#define VLOGGER_INFO(message) VLOGGER_LEVEL(VLoggerLevel::INFO, message)
#define VLOGGER_DEBUG(message) VLOGGER_LEVEL(VLoggerLevel::DEBUG, message)
#define VLOGGER_TRACE(message) VLOGGER_LEVEL(VLoggerLevel::TRACE, message)
#define VLOGGER_HEXDUMP(level, message, buffer, length) do { if (!VLogger::isDefaultLogLevelActive(level)) break; VLogger::getDefaultLogger()->logHexDump(level, message, buffer, length); } while (false)
#define VLOGGER_WOULD_LOG(level) (VLogger::isDefaultLogLevelActive(level))

// This set of macros sends output to a specified named logger.
#define VLOGGER_NAMED_LEVEL(loggername, level, message) do { if (!VLogger::isLogLevelActive(level)) break; VNamedLoggerPtr nl = VLogger::findNamedLoggerForLevel(loggername, level); if (nl != NULL) nl->log(level, NULL, 0, message); } while (false)
#define VLOGGER_NAMED_LEVEL_FILELINE(loggername, level, message, file, line) do { if (!VLogger::isLogLevelActive(level)) break; VNamedLoggerPtr nl = VLogger::findNamedLoggerForLevel(loggername, level); if (nl != NULL) nl->log(level, file, line, message); } while (false)
#define VLOGGER_NAMED_LINE(loggername, level, message) VLOGGER_NAMED_LEVEL_FILELINE(loggername, level, message, __FILE__, __LINE__)
#define VLOGGER_NAMED_FATAL(loggername, message) VLOGGER_NAMED_LEVEL_FILELINE(loggername, VLoggerLevel::FATAL, message, __FILE__, __LINE__)
#define VLOGGER_NAMED_ERROR(loggername, message) VLOGGER_NAMED_LEVEL_FILELINE(loggername, VLoggerLevel::ERROR, message, __FILE__, __LINE__)
#define VLOGGER_NAMED_WARN(loggername, message) VLOGGER_NAMED_LEVEL(loggername, VLoggerLevel::WARN, message)
#define VLOGGER_NAMED_INFO(loggername, message) VLOGGER_NAMED_LEVEL(loggername, VLoggerLevel::INFO, message)
#define VLOGGER_NAMED_DEBUG(loggername, message) VLOGGER_NAMED_LEVEL(loggername, VLoggerLevel::DEBUG, message)
#define VLOGGER_NAMED_TRACE(loggername, message) VLOGGER_NAMED_LEVEL(loggername, VLoggerLevel::TRACE, message)
#define VLOGGER_NAMED_HEXDUMP(loggername, level, message, buffer, length) do { if (!VLogger::isLogLevelActive(level)) break; VNamedLoggerPtr nl = VLogger::findNamedLoggerForLevel(loggername, level); if (nl != NULL) nl->logHexDump(level, message, buffer, length); } while (false)
#define VLOGGER_NAMED_WOULD_LOG(loggername, level) (VLogger::isLogLevelActive(level) && (VLogger::findNamedLoggerForLevel(loggername, level) != NULL))

#define VLOGGER_APPENDER_EMIT(appender, level, message) do { (appender).emit(level, (level <= VLoggerLevel::ERROR) ? __FILE__ : NULL, (level <= VLoggerLevel::ERROR) ? __LINE__ : 0, true, message, false, VString::EMPTY()); } while (false)
#define VLOGGER_APPENDER_EMIT_FILELINE(appender, level, message, file, line) do { (appender).emit(level, file, line, true, message, false, VString::EMPTY()); } while (false)

/**
VLogAppender is an abstract base class that defines the API for writing output to a destination.
*/
class VLogAppender {
    public:

        // These constants can be used for the formatOutput constructor parameter.
        static const bool DO_FORMAT_OUTPUT = true;
        static const bool DONT_FORMAT_OUTPUT = false;

        /**
        Constructs the appender with the specified name. A VNamedLogger that refers to this appender
        by name will route its output (after level filtering) to this appender.
        @param  name            the name of this appender
        @param  formatOutput    true if the logger should normally format its output, false for raw output
        */
        VLogAppender(const VString& name, bool formatOutput);
        /**
        Constructs the appender from settings. A VNamedLogger that refers to this appender
        by name will route its output (after level filtering) to this appender.
        @param  settings        settings containing appender "name" and optional "format-output" flag
        */
        VLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults);

        virtual ~VLogAppender();

        const VString& getName() const { return mName; }    ///< Returns the appender name. @return obvious
        bool isDefaultAppender() const;                     ///< Returns true if this appender is the default appender. Diagnostic only. @return obvious

        /**
        Entry point called by named loggers to cause an appender to append to its output.
        The level is not used for filtering, but rather as information that usually appears in an appender's output format.
        Aside from the level, filename, linenumber, we have two strings, either of which can be optional.
        This allows loggers to supply a normal message, or a header message followed by a raw line, or just a raw line,
        without having to do more complex locking or loops. You can see this in how VNamedLogger is able to use this
        API in a single place for multiple uses. It is possible for both emitMessage and emitRawLine to be true.
        Primarily the ability to emit a raw line is used when doing hex dump (VLOGGER_HEXDUMP) or stack trace logging,
        where there is one header message that needs normal log formatting, followed by additional lines that are
        effectively just continuations of that same message, on additional lines.

        Concrete subclasses rarely need to override this method; they typically instead implement the protected
        method _emitRawLine().

        @param  level       the level at which the message is being logged, and has already been filtered
        @param  file        if not null, the __FILE__ value indicating the source file that emitted the message
        @param  line        if not 0, the __LINE__ value indicating the line number in the source file that emitted the message
        @param  emitMessage if true, the message param should be emitted with the appender's normal formatting (e.g., time stamp, log level)
        @param  message     the message to emit if emitMessage is true
        @param  emitRawLine if true, the rawLine param should be emitted as is
        @param  rawLine     the raw line to message to emit if emitRawLine is true
        */
        virtual void emit(int level, const char* file, int line, bool emitMessage, const VString& message, bool emitRawLine, const VString& rawLine);
        /**
        This utility API is used to call emit with a raw message.
        @param  message     the message to be emitted in raw form
        */
        void emitRaw(const VString& message);

        /**
        For diagnostic purposes, adds the properties/state of this appender to the supplied Bento node.
        Concrete classes should call inherited and then add their type-specific info.
        @param  infoNode    the Bento node to which to add this appender's properties
        */
        virtual void addInfo(VBentoNode& infoNode) const;

    protected:

        /**
        Emits a message using the appender's normal formatting of log output. The base class implementation
        calls this->_formatMessage() to format the message if mFormatOutput is set. Then it calls
        _emitRawLine() to emit the resulting string (formatted or not).
        @param  level       the level at which the message is being logged, and has already been filtered
        @param  file        if not null, the __FILE__ value indicating the source file that emitted the message
        @param  line        if not 0, the __LINE__ value indicating the line number in the source file that emitted the message
        @param  message     the message to format and then emit to output
        */
        virtual void _emitMessage(int level, const char* file, int line, const VString& message);
        /**
        Formats a message prior to output. If the standard formatting supplied here is not what is
        desired, an appender can override this method. If this appender has mFormatOutput turned off, then this
        function is simply not called by _emitMessage() in the first place.
        (todo: This looks like a further opportunity to use a factory pattern to install a custom formatter
        on any appender.)
        @param  level       the level at which the message is being logged, and has already been filtered
        @param  file        if not null, the __FILE__ value indicating the source file that emitted the message
        @param  line        if not 0, the __LINE__ value indicating the line number in the source file that emitted the message
        @param  message     the message to format
        @return the formatted message string
        */
        virtual VString _formatMessage(int level, const char* file, int line, const VString& message);
        /**
        This is the method that most concrete appenders must implement in order to write a message
        (whether it is in raw form or has already been formatted) to the output medium.
        The reason an emty implementation is provided here, rather than it being pure virtual, is that
        some appenders override _emitMessage() instead because they always bypass formatting.
        @param  line    the actual line to be emitted as is (it has already been formatted if that was needed)
        */
        virtual void _emitRawLine(const VString& /*line*/) {}

        // These helper functions are meant to be used by subclasses constructing from settings.
        // Such constructors usually need to get settings, and fall back first to configured defaults, then to a specific default value.
        static bool _getBooleanInitSetting(const VString& attributePath, const VSettingsNode& settings, const VSettingsNode& defaults, bool defaultValue);
        static int _getIntInitSetting(const VString& attributePath, const VSettingsNode& settings, const VSettingsNode& defaults, int defaultValue);
        static VString _getStringInitSetting(const VString& attributePath, const VSettingsNode& settings, const VSettingsNode& defaults, const VString& defaultValue);

        VMutex mMutex;          ///< A mutex to protect against multiple threads' messages from being intertwined;
        // subclasses may access this carefully; note that it is locked prior to any
        // call to emitMessage() or emitRawLine(), so implementors of those functions must
        // not re-lock because to do so would cause a deadlock.
        VString mName;          ///< The name of the appender, used for lookup by loggers.
        bool    mFormatOutput;  ///< True if this appender should format messages it is asked to emit.

    private:

        VString _toString() const; ///< For diagnostics, returns a string representation of this appender and its name.

        static void _breakpointLocationForEmit(); ///< A convenient place to set a debugger breakpoint for any appender emitting output.
};

typedef boost::shared_ptr<VLogAppender> VLogAppenderPtr;
typedef boost::shared_ptr<const VLogAppender> VLogAppenderConstPtr;
typedef std::vector<VLogAppenderPtr> VLogAppenderPtrList;

class VNamedLogger;

/**
VLoggerRepetitionFilter provides VLogger a simple way of preventing runaway repetitive log
output. If the same text is emitted at the same level multiple times in succession, only
the first and last occurrences are emitted (the last one is adorned with an indication of
how many occurrences were suppressed, if any).
*/
class VLoggerRepetitionFilter {
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
        @param  logger      the logger to which any backlog of messages will be emitted
        @param  level       the level of detail of the message
        @param  file        the file name that is emitting the log message;
                                NULL will suppress file and line in output
        @param  line        the line number that is emitting the log message
        @param  message     the text to emit
        @return true if the caller should proceed to emit this message, false if not
        */
        bool checkMessage(VNamedLogger& logger, int level, const char* file, int line, const VString& message);

        /**
        Checks to see if a long time has elapsed since the pending repeat has been sitting
        here. This is called by the logger before checking the log level. This prevents the case
        where a repeat never gets output just because there's nothing after it that fits the
        log level.
        @param  logger      the logger to which any backlog of messages will be emitted
        */
        void checkTimeout(VNamedLogger& logger);

    private:

        VLoggerRepetitionFilter(const VLoggerRepetitionFilter&); // not copyable
        VLoggerRepetitionFilter& operator=(const VLoggerRepetitionFilter&); // not assignable

        /**
        The helper method used by checkMessage() and checkTimeout() when they want to emit
        a backlog of messages.
        @param  logger      the logger to which the backlog of messages will be emitted
        */
        void _emitSuppressedMessages(VNamedLogger& logger);

        bool        mEnabled; ///< True if suppression filtering is allowed. Certain logger subclasses may want to turn off filtering entirely.

        // Information defining the saved output. The (level, file, line, message) combination
        // defines a repeated message. If any of those differ, then it's considered a different
        // message that is not a repeat.
        bool        mHasSavedMessage;           ///< True when we have suppressed and saved something.
        int         mNumSuppressedOccurrences;  ///< The count of repeat messages we have suppressed.
        VInstant    mTimeOfLastOccurrence;      ///< The instant at which we last suppressed a message.
        int         mLevel;                     ///< The log level of the suppressed messages.
        const char* mFile;                      ///< The __FILE__ value of the suppressed messages.
        int         mLine;                      ///< The __LINE__ value of the suppressed messages.
        VString     mMessage;                   ///< The text of the suppressed messages.
};

/**
This class encapsulates the configuration and state for when a named logger should decide to emit a
stack trace upon emitting a message. See how VNamedLogger::mPrintStackConfig is used.
*/
class VLoggerPrintStackConfig {
    public:

        VLoggerPrintStackConfig();
        ~VLoggerPrintStackConfig() {}

        int getLevel() const { return mLevel; } ///< Returns the level that triggers a stack trace.
        void configure(int level, int maxNumOccurrences, const VDuration& timeLimit);
        bool shouldPrintStack(int level, VNamedLogger& logger);

    private:

        int         mLevel;         ///< Messages at this level and lower can trigger a stack crawl.
        int         mMaxCount;      ///< A max number of stack crawls before automatically turning level back to OFF.
        VDuration   mDuration;      ///< A max duration of time before automatically turning level back to OFF.
        int         mCountdown;     ///< Internal counter when counting down from max count to 0 and turning back to OFF.
        VInstant    mExpiration;    ///< Internal instant for when the configured duration expires and we turn back to OFF
};

/**
VNamedLogger defines an object to which log output is initially sent. A logger has a name (that is used
to locate it and direct output to it) and a level (which the logger uses to filter what it receives).
After filtering its input by log level and possibly by the use of a repetition filter, a logger emits
a message that has passed the level and repetition filters, to zero or more appenders, as follows:
- If a "specific appender" instance was supplied to the constructor, it emits to that appender.
- Each named appender in its list of appender names is referenced and emitted to.
  - An empty string signifies the default appender.
  - If no specific appender and no appenders names are supplied to the constructor, the constructor
    adds an empty string so the the logger will emit to the default appender.
- It emits to all "global appenders".
*/
class VNamedLogger {
    public:

        /**
        Constructs a logger. If neither any appender names nor a specific appender is supplied here, the logger will
        be initialized with 1 appender name that is empty, which in turn causes it to emit to the default logger.
        (It will also emit to all "global appenders".) The appenders names can also be cleared, updated, or set later.
        @param  name                a name for the logger, used to find it if it is logged to by name
        @param  level               the level above which log output is filtered; for example, a logger at level INFO will not emit DEBUG messages
        @param  appenderNames       an optional list of appender names to which filtered output will be emitted (see note above)
        @param  specificAppender    an optional specific appender instance to which filtered output will be emitted (see note above)
        */
        VNamedLogger(const VString& name, int level, const VStringVector& appenderNames, VLogAppenderPtr specificAppender = VLogAppenderPtr());
        virtual ~VNamedLogger();

        /**
        Removes all appender names.
        */
        void clearAppenders();
        /**
        Clears all appenders names and adds one specific name.
        @param  appenderName    the appender name to set
        */
        void setAppender(const VString& appenderName);
        /**
        Adds an appender name to the existing list.
        @param  appenderName    the appender name to add
        */
        void addAppender(const VString& appenderName);

        // The following log/emit APIs should generally be accessed through the VLOGGER macros.
        // However, it is legal to call them if you have a reference to a logger.

        /**
        Logs a message (subject to filtering). The file and line parameters may be formatted into the text
        that is actually logged by a particular appender, and that may depend on the log level.
        @param  level   the level of the message
        @param  file    the source file name where the message was logged (from __FILE__ symbol)
        @param  line    the line in the source file where the message was logged (from __LINE__ symbol)
        @param  message the message to be logged
        */
        void log(int level, const char* file, int line, const VString& message);
        /**
        Logs a message (subject to filtering).
        @param  level   the level of the message
        @param  message the message to be logged
        */
        void log(int level, const VString& message);
        /**
        Logs a hex dump of the specified data (subject to filtering).
        @param  level   the level of the message
        @param  message the message to be logged as the line of output preceding the hex data
        @param  buffer  a pointer to some data
        @param  length  the number of bytes of the data that should be examined and written in hex form
        */
        void logHexDump(int level, const VString& message, const Vu8* buffer, Vs64 length);
        /**
        Emits a string in its raw form, without filtering; presumably called by a stack trace function.
        @param  message the string to be logged
        */
        void emitStackCrawlLine(const VString& message);

        const VString& getName() const { return mName; }            ///< Returns the logger's name. @return obvious
        bool isEnabledFor(int level) { return level <= mLevel; }    ///< Returns true if the logger's current level would allow the specified level to emit. @param level obvious @return obvious
        int getLevel() const { return mLevel; }                     ///< Returns the logger's current level. @return obvious
        void setLevel(int level);                                   ///< Sets the logger's level. @param level the level above which messages are filtered

        void setRepetitionFilterEnabled(bool enabled) { mRepetitionFilter.setEnabled(enabled); }    ///< Enabled or disables repetition filtering by this logger. @param enabled obvious

        /**
        Returns the log level at which the logger will cause a stack trace to be emitted along with
        the logged message. For example, if the print stack level is ERROR, then any log output at
        ERROR level or more severe (e.g. FATAL) will emit the message and also emit a stack trace.
        @return the current level that triggers a stack trace to be logged
        */
        int getPrintStackLevel() const { return mPrintStackConfig.getLevel(); }
        /**
        Configures the stack trace printing for this logger.
        @param  printStackLevel     log messages at this level or lower will also emit a stack trace
        @param  maxNumOccurrences   after this many stack traces are emitted, further stack traces will not occur; 0 or -1 disables stack trace
        @param  timeLimit           after this amount of time elapses, further stack traces will not occur; VInstant::INFINITE_FUTURE() means no time limit
        */
        void setPrintStackInfo(int printStackLevel, int maxNumOccurrences, const VDuration& timeLimit) { mPrintStackConfig.configure(printStackLevel, maxNumOccurrences, timeLimit); }
        /**
        Returns true if this logger is currently the default logger.
        @return obvious
        */
        bool isDefaultLogger() const;

        /**
        For diagnostic purposes, adds the properties/state of this logger to the supplied Bento node.
        Calls _addInfo which a subclass should override to change the type value.
        Subclasses should call inherited and then add their type-specific info.
        */
        virtual void addInfo(VBentoNode& infoNode) const;

    protected:

        /**
        Emits the specified message and/or raw line to all appenders appropriate to this logger (see class-level
        doc for VNamedLogger).
        @param  level       the level of the message
        @param  file        the source file name where the message was logged (from __FILE__ symbol)
        @param  line        the line in the source file where the message was logged (from __LINE__ symbol)
        @param  emitMessage true if the message parameter is to be emitted
        @param  message     the message to be emitted if emitMessage is true (the appenders typically format the message)
        @param  emitRawLine true if the rawLine parameter is to be emitted
        @param  rawLine     the raw line to be emitted if emitRawLine is true (the appenders should not format the raw line)
        */
        virtual void _emitToAppenders(int level, const char* file, int line, bool emitMessage, const VString& message, bool emitRawLine, const VString& rawLine);

    private:

        VString _toString() const; ///< For diagnostics, returns a string representation of this appender and its name.

        static void _breakpointLocationForLog(); ///< A convenient place to set a debugger breakpoint for any appender emitting output.

        VString                 mName;              ///< The logger name. Used to find it if logged to by name.
        int                     mLevel;             ///< The level above which log output is filtered.
        mutable VMutex          mAppendersMutex;    ///< A mutex we use to ensure the mutable data below is stable when we access it, even if multiple threads log to us simultaneously.
        VStringVector           mAppenderNames;     ///< A list of appender names to which we emit. An empty string means the default appender.
        VLogAppenderPtr         mSpecificAppender;  ///< If not null, a specific appender instance we emit to.
        VLoggerRepetitionFilter mRepetitionFilter;  ///< Used to prevent repetitive info from clogging output.
        VLoggerPrintStackConfig mPrintStackConfig;  ///< Settings that control whether we add a stack trace for log messages at certain levels.

        friend class VLoggerRepetitionFilter; // it can call our _emitToAppenders when we call it from our log() function
        friend class VLoggerPrintStackConfig; // ditto
};

typedef boost::shared_ptr<VNamedLogger> VNamedLoggerPtr;
typedef boost::shared_ptr<const VNamedLogger> VNamedLoggerConstPtr;

/**
The abstract base class is what you implement to allow an appender class to be
dynamically instantiated from settings, typically during a call to configure() at startup.
*/
class VLogAppenderFactory {
    public:

        /**
        Instantiates the concreted appender class from the specified settings.
        @param  settings    settings to use to configure the appender
        @param  defaults    optional default settings for the appender to use if not specified in settings
        */
        virtual VLogAppenderPtr instantiateLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults) const = 0;

        /**
        For diagnostic purposes, adds the properties/state of this factory to the supplied Bento node.
        Concrete classes should add their type-specific info, primarily the factory and appender class names.
        @param  infoNode    a Bento node to which to add properties describing this factory, specifically
                                a string of name "type" whose value is the appender class name
        */
        virtual void addInfo(VBentoNode& infoNode) const = 0;

};

typedef boost::shared_ptr<const VLogAppenderFactory> VLogAppenderFactoryPtr;

class VLoggerLevel {
    public:

        // Log level constants.
        static const int OFF   = 0;    ///< Level of detail to suppress all output.
        static const int FATAL = 1;    ///< Level of detail to only show failures likely to be fatal.
        static const int ERROR = 20;   ///< Level of detail to add error messages.
        static const int WARN  = 40;   ///< Level of detail to add basic warning messages.
        static const int INFO  = 60;   ///< Level of detail to add coarse-grained status messages.
        static const int DEBUG = 80;   ///< Level of detail to add fine-grained status messages.
        static const int TRACE = 100;  ///< Level of detail to add trace-level messages.
        static const int ALL   = 100;  ///< Level of detail to output all messages.

        static VString getName(int level);
};

// Primary outward facing class for logging. All static methods. The other stuff is internal.
/**
The VLogger class provides the static APIs for configuring the logging system, adding, removing, and
finding appenders and loggers, etc.
*/
class VLogger {
    public:

        /**
        Registers an appender factory. If this is done prior to a call to configure(), the factory can be
        used to dynamically instantiate an appender from the configuration. The configure() method itself
        registers the appender factories supplied by this module. You only need to register factories for
        custom appenders you have written, and only if you want them to be accessible during configure().
        @param  appenderKind    a unique short string defining the appender class as it will be specified
                                    in the config settings XML; e.g., "cout", "file", etc.
        @param  factory         an appender factory
        */
        static void registerLogAppenderFactory(const VString& appenderKind, VLogAppenderFactoryPtr factory);

        /**
        Configures logging by reading settings info.
        The supplied baseLogDirectory can be an empty path, but otherwise it specifies a directory
        within which any file-based appenders should store their files and subdirectories; they
        will obtain this implicit base path from the global variable gBaseLogDirectory.
        The supplied settings node has the following child nodes:
        - A child node named "appender" defining each VLogAppender to be instantiated and registered.
          - Appender nodes have at a minimum a "name" string property and a "kind" string property.
          - All appenders have an optional "format-output" boolean property that, if false, specifies
            that the appender should not apply any formatting (such as a timestamp, level name, etc.)
            to its output. The default behavior is to format the output.
          - Other properties required or optional depend on the concrete appender class features.
        - A child node named "logger" defining each VNamedLogger to be instantiated and registered.
          - Logger nodes have at a minimum a "name" string property and a "level" int property.
          - Typically they also have an "appender" string property that defines the appender to use.
          - For multiple appenders, the logger node can have child nodes named "appender" that have
            a "name" string property.
        @param
        @param  loggingSettings the settings to process
        */
        static void configure(const VFSNode& baseLogDirectory, const VSettingsNode& loggingSettings);
        /**
        This function should always be called when terminating the application, as late as possible
        to ensure that no attempt is made to log after this is called. It cleans up and removes all
        logging objects.
        */
        static void shutdown();

        /**
        Instantiates and registers an appender from settings, via the already-registered factory for the
        concrete VLogAppender subclass indicated by "kind". The settings node has the properties mentioned
        above for configure(). If there is no default appender at the time of this call, the instantiated
        appender becomes the default appender.
        @param  appenderSettings the settings to process
        @param  appenderDefaults optional default properties for the kind of appender specified in appenderSettings
        */
        static void installNewLogAppender(const VSettingsNode& appenderSettings, const VSettingsNode& appenderDefaults);
        /**
        Instantiates and registers a VNamedLogger from settings. The settings node has the properties mentioned
        above for configure(). If there is no default logger at the time of this call, the instantiated
        appender becomes the default logger.
        @param  loggerSettings the settings to process
        */
        static void installNewNamedLogger(const VSettingsNode& loggerSettings);
        /**
        Instantiates and registers a VNamedLogger. If there is no default logger at the time of this call, the instantiated
        appender becomes the default logger.
        @param  name            the name for the logger; if an existing logger with that name already exists,
                                    it will be replaced in the map of registered loggers
        @param  level           the log leve for the logger
        @param  appenderNames   a list of appender names to which this logger will emit its filtered output
        */
        static void installNewNamedLogger(const VString& name, int level, const VStringVector& appenderNames);
        /**
        Instantiates and registers a VNamedLogger. If there is no default logger at the time of this call, the instantiated
        logger becomes the default logger.
        @param  name            the name for the logger; if an existing logger with that name already exists,
                                    it will be replaced in the map of registered loggers
        @param  level           the log leve for the logger
        @param  appenderName    an appender name to which this logger will emit its filtered output
        */
        static void installNewNamedLogger(const VString& name, int level, const VString& appenderName);
        /**
        Registers the supplied appender. If there is no default appender at the time of this call, the
        appender becomes the default appender.
        @param  appender            the appender to register
        @param  asDefaultAppender   if true, the appender becomes the default appender
        */
        static void registerLogAppender(VLogAppenderPtr appender, bool asDefaultAppender = false);
        /**
        Registers the supplied appender, and also adds it to the list of global appenders to which all loggers
        implicitly emit their filtered output. If there is no default appender at the time of this call, the
        appender becomes the default appender.
        @param  appender            the appender to register
        @param  asDefaultAppender   if true, the appender becomes the default appender
        */
        static void registerGlobalAppender(VLogAppenderPtr appender, bool asDefaultAppender = false);
        /**
        Registers the supplied logger. If there is no default logger at the time of this call, the
        logger becomes the default logger.
        @param  name            the name for the logger; if an existing logger with that name already exists,
                                    it will be replaced in the map of registered loggers
        @param  level           the log leve for the logger
        @param  appenderName    an appender name to which this logger will emit its filtered output
        */
        static void registerLogger(VNamedLoggerPtr namedLogger, bool asDefaultLogger = false);
        /**
        Removes the specified appender from the registry.
        @param  appender    the appender to deregister
        */
        static void deregisterLogAppender(VLogAppenderPtr appender);
        /**
        Finds the specified appender by name, and if found, removes it from the registry.
        @param  name    the name of the appender to find and deregister
        */
        static void deregisterLogAppender(const VString& name);
        /**
        Removes the specified logger from the registry.
        @param  namedLogger    the logger to deregister
        */
        static void deregisterLogger(VNamedLoggerPtr namedLogger);
        /**
        Finds the specified logger by name, and if found, removes it from the registry.
        @param  name    the name of the logger to find and deregister
        */
        static void deregisterLogger(const VString& name);
        /**
        Used internally for bookkeeping when a logger is removed, to maintain the "max active log level" optimization.
        @param  removedActiveLevel  the log level of the logger that is being removed
        */
        static void checkMaxActiveLogLevelForRemovedLogger(int removedActiveLevel);
        /**
        Used internally for bookkeeping when a logger's level is changed, to maintain the "max active log level" optimization.
        @param  oldActiveLevel  the old log level of the logger that is changing
        @param  newActiveLevel  the new log level of the logger that is changing
        */
        static void checkMaxActiveLogLevelForChangedLogger(int oldActiveLevel, int newActiveLevel);

        /**
        Returns true if the default logger is active for the specified level.
        @param  level   the level to check
        @return true if the default logger would log at that level
        */
        static bool isDefaultLogLevelActive(int level);
        /**
        Returns true if any registered logger is active for the specified level
        @param  level   the level to check
        @return true if some registered logger would log at that level
        */
        static bool isLogLevelActive(int level);

        // The following "getters" and "finders" have the following consistent naming convention:
        // - "get" always returns a valid object; it may need to create the object in question
        // - "find" will return null if there is no such object; it will not create an object

        // Loggers:
        /**
        Returns the default logger; creates, installs, and returns a new one if there is no default logger.
        @return the default logger (@NotNull)
        */
        static VNamedLoggerPtr getDefaultLogger();
        /**
        Makes the specified logger the default logger.
        @param  namedLogger the logger to make the default logger
        */
        static void setDefaultLogger(VNamedLoggerPtr namedLogger);
        /**
        Returns a logger by looking for the one specified, or returns the default logger if none with that name
        exists.
        @param  name    the name of the logger to find
        @return the found logger, or the default logger if not found (@NotNull)
        */
        static VNamedLoggerPtr getLogger(const VString& name);
        /**
        Returns the default logger, if it already exists; null otherwise.
        @return the default logger (@Nullable)
        */
        static VNamedLoggerPtr findDefaultLogger();
        /**
        Returns the default logger, if it already exists, and is active for the specified level; null otherwise.
        @param  level   the level to check as active for the default logger
        @return the default logger (@Nullable)
        */
        static VNamedLoggerPtr findDefaultLoggerForLevel(int level);
        /**
        Returns the specified logger, if it exists; null otherwise.
        @param  name    the name of the logger to find
        @return a logger (@Nullable)
        */
        static VNamedLoggerPtr findNamedLogger(const VString& name);
        /**
        Returns the specified logger, if it exists AND it is active for the specified level; null otherwise.
        @param  name    the name of the logger to find
        @param  level   the level to check as active for the found logger
        @return a logger (@Nullable)
        */
        static VNamedLoggerPtr findNamedLoggerForLevel(const VString& name, int level);

        // Appenders:
        /**
        Returns the default appender; creates, installs, and returns a new VCoutAppender if there is no default appender.
        @return the default appender (@NotNull)
        */
        static VLogAppenderPtr getDefaultAppender();
        /**
        Returns an appender by looking for the one specified, or returns the default appender if none with that name
        exists.
        @param  appenderName    the name of the appender to find
        @return the found appender, or the appender logger if not found (@NotNull)
        */
        static VLogAppenderPtr getAppender(const VString& appenderName);
        /**
        Returns a new list containing all appenders including the global appenders. It's possible for the
        list to be empty if no appenders have been registered. All values will be non-null shared_ptr copies.
        @return the list of all appenders
        */
        static VLogAppenderPtrList getAllAppenders();
        /**
        Returns the default appender, if it already exists; null otherwise.
        @return the default appender (@Nullable)
        */
        static VLogAppenderPtr findDefaultAppender();
        /**
        Returns the specified appender, if it exists; null otherwise.
        @param  name    the name of the appender to find
        @return an appender (@Nullable)
        */
        static VLogAppenderPtr findAppender(const VString& name);

        // These functions whose name is prefixed with "command" are intended for use at runtime from
        // a command facility that allows reconfiguring logging on the fly.
        static VBentoNode* commandGetInfo();    ///< Returns a Bento structure describing the logging structures in effect, for diagnostic output. @return obvious
        static VString commandGetInfoString();  ///< Returns a string taken from the Bento results of commandGetInfo(). @return obvious
        static void commandNewAppender(const VString& appenderName, const VString& kind, const VString& appenderParam); ///< Calls through to installNewLogAppender().
        static void commandRemoveAppender(const VString& appenderName);                                                 ///< Calls through to deregisterAppender().
        static void commandRollAppender(const VString& appenderName);                                                   ///< Finds the specified (or all, if appenderName is empty) VRollingFileLogAppender and tells them to roll over to a new file.
        static void commandNewLogger(const VString& loggerName, const VStringVector& appenderNames);                    ///< Calls through to installNewNamedLogger().
        static void commandRemoveLogger(const VString& loggerName);                                                     ///< Calls through to deregisterLogger().
        static void commandSetLoggerAppenders(const VString& loggerName, const VStringVector& appenderNames);           ///< Calls through to clearAppenders() and setAppender() for the specified logger.
        static void commandAddLoggerAppenders(const VString& loggerName, const VStringVector& appenderNames);           ///< Calls through to addAppender() for the specified logger.
        static void commandRemoveLoggerAppenders(const VString& loggerName, const VStringVector& appenderNames);        ///< Calls through to removeAppender() for the specified logger.
        static void commandSetLogLevel(const VString& loggerName, int level);                                           ///< Calls through to setLevel for the specified (or all, if loggerName is empty) loggers.
        static void commandSetPrintStackLevel(const VString& loggerName, int printStackLevel, int count, const VDuration& timeLimit); ///< Calls through to setPrintStackInfo() for the specified (or all, if loggerName is empty) loggers.

        // Used specifically by VNamedLogger::_emitToAppenders to emit to all "global appenders" with correct locking. Should not be called elsewhere.
        static void emitToGlobalAppenders(int level, const char* file, int line, bool emitMessage, const VString& message, bool emitRawLine, const VString& rawLine);

    private:

        VLogger(const VLogger&); // not copyable
        VLogger& operator=(const VLogger&); // not assignable

        // These helper methods, like private methods in general, assume the caller has locked.
        static void _registerAppender(VLogAppenderPtr appender, bool asDefaultAppender = false, bool asGlobalAppender = false);
        static void _registerLogger(VNamedLoggerPtr namedLogger, bool asDefaultLogger = false);
        static VBentoNode* _commandGetInfo();
        static VString _commandGetInfoString();

        // These methods maintain the gMaxActiveLogLevel.
        static void _checkMaxActiveLogLevelForNewLogger(int newActiveLevel); // Called when a new logger is created, since that may change the max active log level.
        static void _checkMaxActiveLogLevelForRemovedLogger(int removedActiveLevel); // Called when a logger is removed, since that may change the max active log level.
        static void _checkMaxActiveLogLevelForChangedLogger(int oldActiveLevel, int newActiveLevel); // Called when a logger's level is changed, since that may change the max active log level.
        static void _recalculateMaxActiveLogLevel(); // Called when one of the _check... methods decides the max active log level may indeed have changed, and must be recalculated.

        // These two methods are how we really search for a specified named logger.
        static VNamedLoggerPtr _findNamedLoggerFromExactName(const VString& name);      ///< Return the logger with the specified name, or null if it doesn't exist. (@Nullable)
        static VNamedLoggerPtr _findNamedLoggerFromPathName(const VString& pathName);   ///< Return a logger using a dot-separated path name, falling back to an exact name find. (@Nullable)

        // _mutexInstance() must be used internally whenever referencing these variables:
        volatile static int     gMaxActiveLevel;    ///< The max level of any registered logger. Used to optimize the VLOGGER macros so they can return early if a log statement won't pass level filters.
        static VNamedLoggerPtr  gDefaultLogger;     ///< The default logger that is logged to by the simple VLOGGER macros and by the VLOGGER_NAMED macros if the named logger is not found. Created on first reference if needed.
        static VLogAppenderPtr  gDefaultAppender;   ///< The default appender that is emitted to by a logger if the logger has no appender specified. A VCoutAppender is created on first reference if needed.
        static VFSNode          gBaseLogDirectory;  ///< The directory within which any file-oriented loggers should write all their data.

        friend class VLoggerUnit;  // unit tests directly examine our state
        friend bool VNamedLogger::isDefaultLogger() const;
        friend bool VLogAppender::isDefaultAppender() const;

        // Internal development debugging methods. Only enabled as needed.
//#define VLOGGER_INTERNAL_DEBUGGING
#ifdef VLOGGER_INTERNAL_DEBUGGING
        static void _reportLoggerChange(bool before, const VString& label, const VNamedLoggerPtr& was, const VNamedLoggerPtr& is);
        static void _reportAppenderChange(bool before, const VString& label, const VLogAppenderPtr& was, const VLogAppenderPtr& is);
#else
        static void _reportLoggerChange(bool /*before*/, const VString& /*label*/, const VNamedLoggerPtr& /*was*/, const VNamedLoggerPtr& /*is*/) {}
        static void _reportAppenderChange(bool /*before*/, const VString& /*label*/, const VLogAppenderPtr& /*was*/, const VLogAppenderPtr& /*is*/) {}
#endif /* VLOGGER_INTERNAL_DEBUGGING */
};

/**
An appender that emits to the console using the std::cout stream.
It defines no additional settings properties.
*/
class VCoutLogAppender : public VLogAppender {
    public:
        VCoutLogAppender(const VString& name, bool formatOutput);
        VCoutLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults);
        virtual ~VCoutLogAppender() {}
        virtual void addInfo(VBentoNode& infoNode) const;
    protected:
        virtual void _emitRawLine(const VString& line);
};

/**
An appender that emits to one big file.
It defines the following additional properties:
- "path" (string)
  Defaults to the appender name. Specifies the file path for the log file.
*/
class VFileLogAppender : public VLogAppender {
    public:
        VFileLogAppender(const VString& name, bool formatOutput, const VString& filePath);
        VFileLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults);
        virtual ~VFileLogAppender() {}
        virtual void addInfo(VBentoNode& infoNode) const;
    protected:
        virtual void _emitRawLine(const VString& line);
    private:
        void _openFile(); // constructor helper
        VBufferedFileStream mFileStream;    ///< The underlying file stream we open and write to.
        VTextIOStream       mOutputStream;  ///< The high-level text stream we write to.
};

/**
Not yet implemented. An appender that emits to rolling log files.
It will define several properties to control the limits on log file size
and the lifecycle for cleaning up "old" files. I plan to define a class
that has a global housekeeping object which all rolling file appenders register with,
to indicate their file patterns, directories, lifetimes, and currently in use file.
The housekeeper task will periodically remove sufficiently old files as long as they
are not currently in use. Another approach might be to only fire a cleaning job upon
rollover, since although a file can become old due to the passing of time, if no rollover
occurs then there is no growth in the number of files on disk.
*/
class VRollingFileLogAppender : public VLogAppender {
    public:
        VRollingFileLogAppender(const VString& name, bool formatOutput, const VString& dirPath, const VString& fileNamePrefix, int maxNumLines);
        VRollingFileLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults);
        virtual ~VRollingFileLogAppender() {}
        virtual void addInfo(VBentoNode& infoNode) const;
    protected:
        virtual void _emitRawLine(const VString& line);
};

/**
An appender that discards everything emitted to it.
It defines no additional settings properties.
*/
class VSilentLogAppender : public VLogAppender {
    public:
        VSilentLogAppender(const VString& name) : VLogAppender(name, true/*this won't matter*/) {}
        VSilentLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults) : VLogAppender(settings, defaults) {}
        virtual ~VSilentLogAppender() {}
        virtual void addInfo(VBentoNode& infoNode) const;
        virtual void emit(int /*level*/, const char* /*file*/, int /*line*/, bool /*emitMessage*/, const VString& /*message*/, bool /*emitRawLine*/, const VString& /*rawLine*/) {}
};

/**
An appender that appends each emitted message to a multi-line string instance variable that
can be obtained afterwards. You would not want to use such a logger indefinitely, since it
would run out of memory; it's intended to capture log output emitted for a specific action
of limited duration.
It defines no additional settings properties.
*/
class VStringLogAppender : public VLogAppender {
    public:
        VStringLogAppender(const VString& name, bool formatOutput);
        VStringLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults);
        virtual ~VStringLogAppender() {}
        virtual void addInfo(VBentoNode& infoNode) const;

        VMutex& getMutex() { return mMutex; }
        const VString& getLines() const { return mLines; }
        const char* orphanLines() { return mLines.orphanDataBuffer(); }

    protected:
        virtual void _emitRawLine(const VString& line);
    private:
        VString mLines;
};

/**
An appender that appends each emitted message to a string list that can be obtained afterwards.
You would not want to use such a logger indefinitely, since it would run out of memory; it's
intended to capture log output emitted for a specific action of limited duration.
It defines no additional settings properties.
*/
class VStringVectorLogAppender : public VLogAppender {
    public:
        VStringVectorLogAppender(const VString& name, bool formatOutput, VStringVector* storage);
        VStringVectorLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults);
        virtual ~VStringVectorLogAppender();
        virtual void addInfo(VBentoNode& infoNode) const;

        VMutex& getMutex() { return mMutex; }
        const VStringVector& getLines() const { return *mStorage; }

    protected:
        virtual void _emitRawLine(const VString& line);
    private:
        VStringVector* mStorage;
        VStringVector mLines;
};

/**
A special logger subclass meant to be declared on the stack (not "registered") and explicitly logged
to, which uses an embedded VStringLogAppender to capture the emitted messages to a multi-line string.
*/
class VStringLogger : public VNamedLogger {
    public:

        VStringLogger(const VString& name, int level, bool formatOutput = VLogAppender::DO_FORMAT_OUTPUT);
        virtual ~VStringLogger() {}
        virtual void addInfo(VBentoNode& infoNode) const;

        const VString& getLines() const { return mAppender.getLines(); }
        const char* orphanLines() { return mAppender.orphanLines(); }

    protected:

        virtual void _emitToAppenders(int level, const char* file, int line, bool emitMessage, const VString& message, bool emitRawLine, const VString& rawLine);

    private:

        VStringLogAppender mAppender;

};

/**
A special logger subclass meant to be declared on the stack (not "registered") and explicitly logged
to, which uses an embedded VStringVectorLogAppender to capture the emitted messages to a string list.
*/
class VStringVectorLogger : public VNamedLogger {
    public:

        VStringVectorLogger(const VString& name, int level, VStringVector* storage, bool formatOutput = VLogAppender::DO_FORMAT_OUTPUT);
        virtual ~VStringVectorLogger() {}
        virtual void addInfo(VBentoNode& infoNode) const;

        const VStringVector& getLines() const { return mAppender.getLines(); }

    protected:

        virtual void _emitToAppenders(int level, const char* file, int line, bool emitMessage, const VString& message, bool emitRawLine, const VString& rawLine);

    private:

        VStringVectorLogAppender mAppender;

};

#endif /* vlogger_h */
