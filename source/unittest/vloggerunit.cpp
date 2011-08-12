/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vloggerunit.h"
#include "vlogger.h"
#include "vmessage.h"

/**
V2xInterceptLogger is a VInterceptLogger that captures a 2nd previous message for
our VLoggerRepetitionFilter testing.
*/
class V2xInterceptLogger : public VInterceptLogger
    {
    public:

        V2xInterceptLogger(int logLevel, const VString& name) : VInterceptLogger(logLevel, name, VString::EMPTY()), mNumRawLinesEmitted(0), mLastLoggedMessage(), m2ndLastLoggedMessage() {}
        virtual ~V2xInterceptLogger() {}

        const VString& getLastMessage() const { return mLastLoggedMessage; }
        const VString& get2ndLastMessage() const { return m2ndLastLoggedMessage; }
        virtual void reset() { VInterceptLogger::reset(); mLastLoggedMessage = VString::EMPTY(); m2ndLastLoggedMessage = VString::EMPTY(); mNumRawLinesEmitted = 0; }
        int getNumRawLinesEmitted() const { return mNumRawLinesEmitted; }
        void setFilterEnabled(bool enabled) { mRepetitionFilter.setEnabled(enabled); }

    protected:

        virtual void emitRawLine(const VString& line) { m2ndLastLoggedMessage = mLastLoggedMessage; mLastLoggedMessage = line; VInterceptLogger::emitRawLine(line); ++mNumRawLinesEmitted; }

    private:

        int mNumRawLinesEmitted;
        VString mLastLoggedMessage;
        VString m2ndLastLoggedMessage;
    };

// VLoggerUnit ------------------------------------------------------------------------

VLoggerUnit::VLoggerUnit(bool logOnSuccess, bool throwOnError) :
VUnit("VLoggerUnit", logOnSuccess, throwOnError)
    {
    }

void VLoggerUnit::run()
    {
    this->_testMacros();
    this->_testLogOutput();
    this->_testStringLoggers();
    this->_testMaxActiveLogLevel();
    this->_testLoggerPathNames();
//    this->_testOptimizationPerformance();
    }

void VLoggerUnit::_testMacros()
    {
    // Test every macro to make sure they all compile and run.
    // They won't all actually emit here in the unit test, because the actual log level is what it is.
    VString s("example buffer of data");
    VString name("dummy-logger-name");

    try
        {
        VLOGGER_FATAL_AND_THROW("Example of VLOGGER_FATAL_AND_THROW.");
        } catch (...) {}

    VLOGGER_LEVEL(VLogger::kInfo, "Example of VLOGGER_LEVEL @ kInfo.");
    VLOGGER_LEVEL_FILELINE(VLogger::kInfo, "Example of VLOGGER_LEVEL_FILELINE @ kInfo.", __FILE__, __LINE__); // (not normally called except by other macros)
    VLOGGER_FATAL("Example of VLOGGER_FATAL.");
    VLOGGER_ERROR("Example of VLOGGER_ERROR.");
    VLOGGER_WARN("Example of VLOGGER_WARN.");
    VLOGGER_INFO("Example of VLOGGER_INFO.");
    VLOGGER_DEBUG("Example of VLOGGER_DEBUG.");
    VLOGGER_TRACE("Example of VLOGGER_TRACE.");
    VLOGGER_LINE(VLogger::kInfo, "Example of VLOGGER_LINE @ kInfo.");
    VLOGGER_HEXDUMP(VLogger::kInfo, "Example of VLOGGER_HEXDUMP @ kInfo.", s.getDataBufferConst(), s.length() + 1);
    if (VLOGGER_WOULD_LOG(VLogger::kInfo)) {}

    VLOGGER_NAMED_LEVEL(name, VLogger::kInfo, "Example of VLOGGER_NAMED_LEVEL @ kInfo.");
    VLOGGER_NAMED_LEVEL_FILELINE(name, VLogger::kInfo, "Example of VLOGGER_NAMED_LEVEL_FILELINE @ kInfo.", __FILE__, __LINE__); // (not normally called except by other macros)
    VLOGGER_NAMED_FATAL(name, "Example of VLOGGER_NAMED_FATAL.");
    VLOGGER_NAMED_ERROR(name, "Example of VLOGGER_NAMED_ERROR.");
    VLOGGER_NAMED_WARN(name, "Example of VLOGGER_NAMED_WARN.");
    VLOGGER_NAMED_INFO(name, "Example of VLOGGER_NAMED_INFO.");
    VLOGGER_NAMED_DEBUG(name, "Example of VLOGGER_NAMED_DEBUG.");
    VLOGGER_NAMED_TRACE(name, "Example of VLOGGER_NAMED_TRACE.");
    VLOGGER_NAMED_LINE(name, VLogger::kInfo, "Example of VLOGGER_NAMED_LINE @ kInfo.");
    VLOGGER_NAMED_HEXDUMP(name, VLogger::kInfo, "Example of VLOGGER_NAMED_HEXDUMP @ kInfo.", s.getDataBufferConst(), s.length() + 1);
    if (VLOGGER_NAMED_WOULD_LOG(name, VLogger::kInfo)) {}

    VLOGGER_MESSAGE_LEVEL(VLogger::kInfo, "Example of VLOGGER_MESSAGE_LEVEL @ kInfo.");
    VLOGGER_MESSAGE_FATAL("Example of VLOGGER_MESSAGE_FATAL.");
    VLOGGER_MESSAGE_ERROR("Example of VLOGGER_MESSAGE_ERROR.");
    VLOGGER_MESSAGE_WARN("Example of VLOGGER_MESSAGE_WARN.");
    VLOGGER_MESSAGE_INFO("Example of VLOGGER_MESSAGE_INFO.");
    VLOGGER_MESSAGE_DEBUG("Example of VLOGGER_MESSAGE_DEBUG.");
    VLOGGER_MESSAGE_TRACE("Example of VLOGGER_MESSAGE_TRACE.");
    VLOGGER_MESSAGE_HEXDUMP("Example of VLOGGER_MESSAGE_HEXDUMP @ kInfo.", s.getDataBufferConst(), s.length() + 1);
    }

void VLoggerUnit::_testLogOutput()
    {
    V2xInterceptLogger logger(VLogger::kInfo, "VLoggerUnit's V2xInterceptLogger");
    
    const VString TEST_MESSAGE_1("Test message 1");
    logger.log(VLogger::kInfo, TEST_MESSAGE_1);
    this->test(logger.getLastMessage() == TEST_MESSAGE_1, TEST_MESSAGE_1);
    
    const VString TEST_MESSAGE_2("Test message 2");
    logger.log(VLogger::kInfo, TEST_MESSAGE_2);
    this->test(logger.getLastMessage() == TEST_MESSAGE_2, TEST_MESSAGE_2);
    
    const VString TEST_MESSAGE_SENTINEL("Test message sentinel");
    const VString TEST_MESSAGE_REPEATING("Test message that repeats");
    const VString TEST_MESSAGE_REPEATING_3X("[3x] Test message that repeats");

    // If we emit a given message twice, then emit another, we should see the repeated message
    // emitted in normal fashion, because a mere 1x repeat is not shortened by noting "[1x]" in
    // the repeat indicator output.
    logger.reset();
    logger.log(VLogger::kInfo, TEST_MESSAGE_SENTINEL);
    this->test(logger.getLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getNumRawLinesEmitted() == 1, "emitted 1");

    logger.log(VLogger::kInfo, TEST_MESSAGE_REPEATING); // original
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getLastMessage() == TEST_MESSAGE_REPEATING, TEST_MESSAGE_REPEATING);
    this->test(logger.getNumRawLinesEmitted() == 2, "emitted 2");

    logger.log(VLogger::kInfo, TEST_MESSAGE_REPEATING); // 1x repeat, should get buffered
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getLastMessage() == TEST_MESSAGE_REPEATING, TEST_MESSAGE_REPEATING);
    this->test(logger.getNumRawLinesEmitted() == 2, "still emitted 2");

    logger.log(VLogger::kInfo, TEST_MESSAGE_SENTINEL); // cause repeat to be output without indicator
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_REPEATING, TEST_MESSAGE_REPEATING);
    this->test(logger.getLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getNumRawLinesEmitted() == 4, "emitted 4");
    
    // Now do the same but repeat more than 1x, so that the eventual output contains
    // the correct indicator of the number of repeats.
    logger.reset();
    logger.log(VLogger::kInfo, TEST_MESSAGE_SENTINEL);
    this->test(logger.getLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getNumRawLinesEmitted() == 1, "emitted 1");

    logger.log(VLogger::kInfo, TEST_MESSAGE_REPEATING); // original
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getLastMessage() == TEST_MESSAGE_REPEATING, TEST_MESSAGE_REPEATING);
    this->test(logger.getNumRawLinesEmitted() == 2, "emitted 2");

    logger.log(VLogger::kInfo, TEST_MESSAGE_REPEATING); // 1x repeat, should get buffered
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getLastMessage() == TEST_MESSAGE_REPEATING, TEST_MESSAGE_REPEATING);
    this->test(logger.getNumRawLinesEmitted() == 2, "still emitted 2");

    logger.log(VLogger::kInfo, TEST_MESSAGE_REPEATING); // 2x repeat, should get buffered
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getLastMessage() == TEST_MESSAGE_REPEATING, TEST_MESSAGE_REPEATING);
    this->test(logger.getNumRawLinesEmitted() == 2, "still emitted 2");

    logger.log(VLogger::kInfo, TEST_MESSAGE_REPEATING); // 3x repeat, should get buffered
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getLastMessage() == TEST_MESSAGE_REPEATING, TEST_MESSAGE_REPEATING);
    this->test(logger.getNumRawLinesEmitted() == 2, "still emitted 2");

    logger.log(VLogger::kInfo, TEST_MESSAGE_SENTINEL); // cause repeat to be output WITH [3x] indicator
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_REPEATING_3X, TEST_MESSAGE_REPEATING_3X);
    this->test(logger.getLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getNumRawLinesEmitted() == 4, "emitted 4");
    
    // Make sure that we get all output emitted if the filter is disabled.
    logger.setFilterEnabled(false);

    logger.log(VLogger::kInfo, TEST_MESSAGE_1);
    this->test(logger.getLastMessage() == TEST_MESSAGE_1, "Unfiltered 1");
    
    logger.log(VLogger::kInfo, TEST_MESSAGE_2);
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_1, "Unfiltered 2a");
    this->test(logger.getLastMessage() == TEST_MESSAGE_2, "Unfiltered 2b");
    
    logger.log(VLogger::kInfo, TEST_MESSAGE_2);
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_2, "Unfiltered 3a");
    this->test(logger.getLastMessage() == TEST_MESSAGE_2, "Unfiltered 3b");
    
    logger.log(VLogger::kInfo, TEST_MESSAGE_1);
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_2, "Unfiltered 4a");
    this->test(logger.getLastMessage() == TEST_MESSAGE_1, "Unfiltered 4b");
    }

void VLoggerUnit::_testStringLoggers()
    {
    // For the VStringLogger/VStringVectorLogger tests, note that the logged
    // output captured in the logger includes timestamp/level text, so we can't
    // do an exact match test. Instead, do a contains() or endsWith() test.
    
    const VString FATAL_MESSAGE("A fatal message.");
    const VString ERROR_MESSAGE("An error message.");
    const VString WARN_MESSAGE("A warning message.");
    const VString INFO_MESSAGE("An info message.");
    const VString DEBUG_MESSAGE("A debug message.");
    const VString TRACE_MESSAGE("A trace message.");

    VStringLogger vsl(VLogger::kInfo, "VLoggerUnit's VStringLogger", VString::EMPTY());
    vsl.log(VLogger::kFatal, FATAL_MESSAGE);
    vsl.log(VLogger::kError, ERROR_MESSAGE);
    vsl.log(VLogger::kWarn, WARN_MESSAGE);
    vsl.log(VLogger::kInfo, INFO_MESSAGE);
    vsl.log(VLogger::kDebug, DEBUG_MESSAGE);
    vsl.log(VLogger::kTrace, TRACE_MESSAGE);
    
    this->test(vsl.getLines().contains(FATAL_MESSAGE), "VStringLogger contains fatal message");
    this->test(vsl.getLines().contains(ERROR_MESSAGE), "VStringLogger contains error message");
    this->test(vsl.getLines().contains(WARN_MESSAGE), "VStringLogger contains warn message");
    this->test(vsl.getLines().contains(INFO_MESSAGE), "VStringLogger contains info message");
    this->test(! vsl.getLines().contains(DEBUG_MESSAGE), "VStringLogger does not contain debug message");
    this->test(! vsl.getLines().contains(TRACE_MESSAGE), "VStringLogger does not contain trace message");
    
    this->logStatus(VSTRING_FORMAT("VStringLogger contents:\n%s", vsl.getLines().chars()));

    VStringVectorLogger vsvl(VLogger::kInfo, "VLoggerUnit's VStringLogger", VString::EMPTY());
    vsvl.log(VLogger::kFatal, FATAL_MESSAGE);
    vsvl.log(VLogger::kError, ERROR_MESSAGE);
    vsvl.log(VLogger::kWarn, WARN_MESSAGE);
    vsvl.log(VLogger::kInfo, INFO_MESSAGE);
    vsvl.log(VLogger::kDebug, DEBUG_MESSAGE);
    vsvl.log(VLogger::kTrace, TRACE_MESSAGE);
    
    const VStringVector& actualOutputLines = vsvl.getLines();
    this->test(actualOutputLines.size() == 4, "VStringVectorLogger size = 4");
    this->test(actualOutputLines.at(0).endsWith(FATAL_MESSAGE), "VStringVectorLogger lines[0]");
    this->test(actualOutputLines.at(1).endsWith(ERROR_MESSAGE), "VStringVectorLogger lines[1]");
    this->test(actualOutputLines.at(2).endsWith(WARN_MESSAGE), "VStringVectorLogger lines[2]");
    this->test(actualOutputLines.at(3).endsWith(INFO_MESSAGE), "VStringVectorLogger lines[3]");

    this->logStatus("VStringVectorLogger contents follow:");
    for (VStringVector::const_iterator i = actualOutputLines.begin(); i != actualOutputLines.end(); ++i)
        this->logStatus(*i);

    }

void VLoggerUnit::_testMaxActiveLogLevel()
    {
    // We assume the existing max logger level is less than 90.
    // We need to special case the use of the "VUnit" logger which may be present for routing
    // all unit test output. We temporarily downgrade its log level. Try/catch to ensure it
    // gets restored even if we throw an exception here.
    
    int oldVUnitLogLevel = -1;
    VLogger* vunitLogger = VLogger::findLogger("VUnit");
    if (vunitLogger != NULL)
        {
        oldVUnitLogLevel = vunitLogger->getLevel();
        vunitLogger->setLevel(80);
        }

    try
        {
        int oldMaxActiveLogLevel = VLogger::gMaxActiveLogLevel;

        VLogger::installLogger(new VLogger(90, "90", VString::EMPTY()));
        VUNIT_ASSERT_EQUAL_LABELED(VLogger::gMaxActiveLogLevel, 90, "max active level");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(89), "level -1 is active");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(90), "level == is active");
        VUNIT_ASSERT_FALSE_LABELED(VLogger::isLogLevelActive(91), "level +1 is not active");

        VLogger::installLogger(new VLogger(94, "94", VString::EMPTY()));
        VUNIT_ASSERT_EQUAL_LABELED(VLogger::gMaxActiveLogLevel, 94, "max active level");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(93), "level -1 is active");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(94), "level == is active");
        VUNIT_ASSERT_FALSE_LABELED(VLogger::isLogLevelActive(95), "level +1 is not active");

        VLogger::installLogger(new VLogger(92, "92", VString::EMPTY()));
        VUNIT_ASSERT_EQUAL_LABELED(VLogger::gMaxActiveLogLevel, 94, "max active level");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(93), "level -1 is active");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(94), "level == is active");
        VUNIT_ASSERT_FALSE_LABELED(VLogger::isLogLevelActive(95), "level +1 is not active");

        VLogger::deleteLogger("90");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::findLogger("90") == NULL, "level 90 logger deleted");
        VUNIT_ASSERT_EQUAL_LABELED(VLogger::gMaxActiveLogLevel, 94, "max active level");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(93), "level -1 is active");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(94), "level == is active");
        VUNIT_ASSERT_FALSE_LABELED(VLogger::isLogLevelActive(95), "level +1 is not active");

        VLogger::deleteLogger("94");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::findLogger("94") == NULL, "level 94 logger deleted");
        VUNIT_ASSERT_EQUAL_LABELED(VLogger::gMaxActiveLogLevel, 92, "max active level");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(91), "level -1 is active");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(92), "level == is active");
        VUNIT_ASSERT_FALSE_LABELED(VLogger::isLogLevelActive(93), "level +1 is not active");

        VLogger::deleteLogger("92");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::findLogger("92") == NULL, "level 92 logger deleted");
        VUNIT_ASSERT_EQUAL_LABELED(VLogger::gMaxActiveLogLevel, oldMaxActiveLogLevel, "max active level");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(oldMaxActiveLogLevel - 1), "level -1 is active");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(oldMaxActiveLogLevel), "level == is active");
        VUNIT_ASSERT_FALSE_LABELED(VLogger::isLogLevelActive(oldMaxActiveLogLevel + 1), "level +1 is not active");
        }
    catch (...)
        {
        if (vunitLogger != NULL)
            vunitLogger->setLevel(oldVUnitLogLevel);

        throw;
        }
    }

static VString _createValueString(int i)
    {
    return VSTRING_FORMAT("value[%d]", i);
    }

void VLoggerUnit::_testLoggerPathNames()
    {
    // Create a defined hierarchy of logger names.
    // Use different log levels.
    // Route output to the loggers by using paths.
    // Verify that the right loggers are found, and the right levels are honored.
    // VStringLogger is useful because we can examine its contents easily after logging.
    
    VLoggerList loggers;
    loggers.push_back(new VStringLogger(VLogger::kError,    "diagnostics",                          VString::EMPTY()));
    loggers.push_back(new VStringLogger(VLogger::kWarn,     "diagnostics.sensors",                  VString::EMPTY()));
    loggers.push_back(new VStringLogger(VLogger::kInfo,     "diagnostics.sensors.transponders",     VString::EMPTY()));
    loggers.push_back(new VStringLogger(VLogger::kDebug,    "diagnostics.sensors.transponders.42",  VString::EMPTY()));
    std::vector<std::vector<int> > loggerNWanted;
    
    for (VLoggerList::const_iterator i = loggers.begin(); i != loggers.end(); ++i)
        VLogger::installLogger(*i);

    // Verify that we find the right loggers explicitly. Test things like extra trailing or internal path separator.
    VLogger* defaultLogger = VLogger::getDefaultLogger();
    VLogger* foundLogger = NULL;
    
    foundLogger = VLogger::getLogger("diag.nostics");                               VUNIT_ASSERT_EQUAL(foundLogger->getName(), defaultLogger->getName());
    foundLogger = VLogger::getLogger("diagnostics");                                VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics");
    foundLogger = VLogger::getLogger("diagnostics.");                               VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics");
    foundLogger = VLogger::getLogger("diagnostics..");                              VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics");
    foundLogger = VLogger::getLogger("diagnostics.nonexistent");                    VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics");
    foundLogger = VLogger::getLogger("diagnostics.nonexistent.");                   VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics");
    foundLogger = VLogger::getLogger("diagnostics.nonexistent..");                  VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics");
    foundLogger = VLogger::getLogger("diagnostics..sensors");                       VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics");
    foundLogger = VLogger::getLogger("diagnostics.sensors");                        VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors");
    foundLogger = VLogger::getLogger("diagnostics.sensors.");                       VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors");
    foundLogger = VLogger::getLogger("diagnostics.sensors..");                      VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors");
    foundLogger = VLogger::getLogger("diagnostics.sensors.nonexistent");            VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors");
    foundLogger = VLogger::getLogger("diagnostics.sensors..nonexistent.");          VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors");
    foundLogger = VLogger::getLogger("diagnostics.sensors.nonexistent.");           VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors");
    foundLogger = VLogger::getLogger("diagnostics.sensors.nonexistent..");          VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors");
    foundLogger = VLogger::getLogger("diagnostics.sensors..transponders");          VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors");
    foundLogger = VLogger::getLogger("diagnostics.sensors.transponders");           VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors.transponders");
    foundLogger = VLogger::getLogger("diagnostics.sensors.transponders");           VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors.transponders");
    foundLogger = VLogger::getLogger("diagnostics.sensors.transponders.");          VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors.transponders");
    foundLogger = VLogger::getLogger("diagnostics.sensors.transponders..");         VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors.transponders");
    foundLogger = VLogger::getLogger("diagnostics.sensors.transponders.44");        VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors.transponders");
    foundLogger = VLogger::getLogger("diagnostics.sensors.transponders.44");        VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors.transponders");
    foundLogger = VLogger::getLogger("diagnostics.sensors.transponders..44");       VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors.transponders");
    foundLogger = VLogger::getLogger("diagnostics.sensors.transponders.44.");       VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors.transponders");
    foundLogger = VLogger::getLogger("diagnostics.sensors.transponders.44..");      VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors.transponders");
    foundLogger = VLogger::getLogger("diagnostics.sensors.transponders..42");       VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors.transponders");
    foundLogger = VLogger::getLogger("diagnostics.sensors.transponders.42");        VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors.transponders.42");
    foundLogger = VLogger::getLogger("diagnostics.sensors.transponders.42.");       VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors.transponders.42");
    foundLogger = VLogger::getLogger("diagnostics.sensors.transponders.42..");      VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors.transponders.42");
    foundLogger = VLogger::getLogger("diagnostics.sensors.transponders.42.xyz");    VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors.transponders.42");
    foundLogger = VLogger::getLogger("diagnostics.sensors.transponders.42.xyz.");   VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors.transponders.42");
    foundLogger = VLogger::getLogger("diagnostics.sensors.transponders.42.xyz..");  VUNIT_ASSERT_EQUAL(foundLogger->getName(), "diagnostics.sensors.transponders.42");


    // Test logging to each logger by path, something below, at, and above the log level, to test proper output.
    // Include some non-existent path tails to test that we find the right parent path name.
    // Note that we have to create the string using ++val before each logger call, so we actually increment it each time;
    // otherwise, the logger optimization will skip the increment by avoiding logging.
    int val = -1; // we will pre-increment so that we can push_back after each wanted creation; so will effectively be 0-based
    VString s;
    
    // for "diagnostics": ERROR and below (we test ERROR and FATAL, do not want WARN)
    std::vector<int> logger0Wanted;
    s = _createValueString(++val); VLOGGER_NAMED_FATAL("diagnostics",                  s); logger0Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_ERROR("diagnostics",                  s); logger0Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_WARN ("diagnostics",                  s); // "s" should NOT get logged.
    s = _createValueString(++val); VLOGGER_NAMED_FATAL("diagnostics.blahblahblah",     s); logger0Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_ERROR("diagnostics.blahblahblah",     s); logger0Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_WARN ("diagnostics.blahblahblah",     s); // "s" should NOT get logged.
    s = _createValueString(++val); VLOGGER_NAMED_FATAL("diagnostics.blah.blah.blah",   s); logger0Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_ERROR("diagnostics.blah.blah.blah",   s); logger0Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_WARN ("diagnostics.blah.blah.blah",   s); // "s" should NOT get logged.
    loggerNWanted.push_back(logger0Wanted);
    
    // for "diagnostics.sensors": WARN and below (we test WARN and ERROR, do not want INFO)
    std::vector<int> logger1Wanted;
    s = _createValueString(++val); VLOGGER_NAMED_ERROR("diagnostics.sensors",                  s); logger1Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_WARN ("diagnostics.sensors",                  s); logger1Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_INFO ("diagnostics.sensors",                  s); // "s" should NOT get logged.
    s = _createValueString(++val); VLOGGER_NAMED_ERROR("diagnostics.sensors.blahblahblah",     s); logger1Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_WARN ("diagnostics.sensors.blahblahblah",     s); logger1Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_INFO ("diagnostics.sensors.blahblahblah",     s); // "s" should NOT get logged.
    s = _createValueString(++val); VLOGGER_NAMED_ERROR("diagnostics.sensors.blah.blah.blah",   s); logger1Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_WARN ("diagnostics.sensors.blah.blah.blah",   s); logger1Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_INFO ("diagnostics.sensors.blah.blah.blah",   s); // "s" should NOT get logged.
    loggerNWanted.push_back(logger1Wanted);
    
    // for "diagnostics.sensors.transponders": INFO and below (we test INFO and WARN, do not want DEBUG)
    std::vector<int> logger2Wanted;
    s = _createValueString(++val); VLOGGER_NAMED_WARN ("diagnostics.sensors.transponders",                  s); logger2Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_INFO ("diagnostics.sensors.transponders",                  s); logger2Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_DEBUG("diagnostics.sensors.transponders",                  s); // "s" should NOT get logged.
    s = _createValueString(++val); VLOGGER_NAMED_WARN ("diagnostics.sensors.transponders.blahblahblah",     s); logger2Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_INFO ("diagnostics.sensors.transponders.blahblahblah",     s); logger2Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_DEBUG("diagnostics.sensors.transponders.blahblahblah",     s); // "s" should NOT get logged.
    s = _createValueString(++val); VLOGGER_NAMED_WARN ("diagnostics.sensors.transponders.blah.blah.blah",   s); logger2Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_INFO ("diagnostics.sensors.transponders.blah.blah.blah",   s); logger2Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_DEBUG("diagnostics.sensors.transponders.blah.blah.blah",   s); // "s" should NOT get logged.
    loggerNWanted.push_back(logger2Wanted);
    
    // for "diagnostics.sensors.transponders.42": DEBUG and below (we test DEBUG and INFO, do not want TRACE)
    std::vector<int> logger3Wanted;
    s = _createValueString(++val); VLOGGER_NAMED_INFO ("diagnostics.sensors.transponders.42",                  s); logger3Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_DEBUG("diagnostics.sensors.transponders.42",                  s); logger3Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_TRACE("diagnostics.sensors.transponders.42",                  s); // "s" should NOT get logged.
    s = _createValueString(++val); VLOGGER_NAMED_INFO ("diagnostics.sensors.transponders.42.blahblahblah",     s); logger3Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_DEBUG("diagnostics.sensors.transponders.42.blahblahblah",     s); logger3Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_TRACE("diagnostics.sensors.transponders.42.blahblahblah",     s); // "s" should NOT get logged.
    s = _createValueString(++val); VLOGGER_NAMED_INFO ("diagnostics.sensors.transponders.42.blah.blah.blah",   s); logger3Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_DEBUG("diagnostics.sensors.transponders.42.blah.blah.blah",   s); logger3Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_TRACE("diagnostics.sensors.transponders.42.blah.blah.blah",   s); // "s" should NOT get logged.
    loggerNWanted.push_back(logger3Wanted);
    
    // Now, for each logger, test that it contains the value strings it should (no-yes-yes per each
    // triplet shown above), and none of the other entire set. We want to test the entire set in order
    // to prove there is no cross-contamination of the loggers from mixed-up path name searching, etc.
    
    const int endVal = val;
    
    int loggerNIndex = 0;
    for (VLoggerList::const_iterator loggersIterator = loggers.begin(); loggersIterator != loggers.end(); ++loggersIterator)
        {
        VStringLogger* vsl = static_cast<VStringLogger*>(*loggersIterator);
        const VString& lines = vsl->getLines();
        this->logStatus(VSTRING_FORMAT("Logger '%s' lines:\n%s", vsl->getName().chars(), lines.chars()));
        for (int testVal = 0; testVal < endVal; ++testVal)
            {
            VString si = _createValueString(testVal);
            if (std::find(loggerNWanted[loggerNIndex].begin(), loggerNWanted[loggerNIndex].end(), testVal) != loggerNWanted[loggerNIndex].end())
                this->test(lines.contains(si), VSTRING_FORMAT("%s / %s present", vsl->getName().chars(), si.chars()));
            else
                this->test(! lines.contains(si), VSTRING_FORMAT("%s / %s not present", vsl->getName().chars(), si.chars()));
            }
        
        ++loggerNIndex;
        }
    
    for (VLoggerList::const_iterator i = loggers.begin(); i != loggers.end(); ++i)
        {
        VString deletedLoggerName = (*i)->getName();
        VLogger::deleteLogger((*i)->getName()); // destroys the VLogger object with that name
        VUNIT_ASSERT_TRUE_LABELED(VLogger::findLogger(deletedLoggerName) == NULL, VSTRING_FORMAT("logger '%s' deleted", deletedLoggerName.chars()));
        }
    
    // Note: We just deleted the loggers in the loop above. The VLoggerList (loggers) now contains garbage pointers.
    // Do not reference them after the loop. Destructing the list is OK.
    }

#define OLDEST_VLOGGER_NAMED_DEBUG(loggername, message) VLogger::getLogger(loggername)->log(VLogger::kDebug, message)
#define OLD_VLOGGER_NAMED_DEBUG(loggername, message) do { VLogger* vlcond = VLogger::getLoggerConditional(loggername, VLogger::kDebug); if (vlcond != NULL) vlcond->log(VLogger::kDebug, NULL, 0, message); } while (false)
// for reference, as of this writing, the new one basically expands to:
// #define VLOGGER_NAMED_DEBUG(loggername, message) do { if (!VLogger::isLogLevelActive(VLogger::kDebug)) break; VLogger* vlcond = VLogger::getLoggerConditional(loggername, VLogger::kDebug); if (vlcond != NULL) vlcond->log(VLogger::kDebug, NULL, 0, message); } while (false)

void VLoggerUnit::_testOptimizationPerformance()
    {
    const int numIterations = 10000000;
    const VString loggerName("speed-test-logger");
    VLogger* oldDefaultLogger = VLogger::getDefaultLogger();
    VLogger* logger = new VLogger(VLogger::kInfo, loggerName, VString::EMPTY());
    VLogger::installLogger(logger);
    VLogger::setDefaultLogger(logger);
    
    // We have installed a level 60 (info) logger as default, and it should be the max level.
    // We will now log zillions of level 80 (debug) messages, which will emit nothing.
    // The question is how much overhead is there to log when the levels is such that nothing is emitted.
    // In a perfect world there is zero overhead.
    
    bool do1 = true;
    bool do2 = true;
    bool do3 = true;
    
    // Worst possible way:
    // Simply call the logger with the formatted message, and let the logger decide whether to emit.
    // This macro is the ancient original way it was done.
    // Note: results of first test run of 10 million iterations: 48.945 seconds.
    if (do1)
        {
        VInstant start;
        for (int i = 0; i < numIterations; ++i)
            {
            OLDEST_VLOGGER_NAMED_DEBUG(loggerName, VSTRING_FORMAT("i=%d", i));
            }
        VDuration d(VInstant() - start);
        std::cout << "MODE 1: " << numIterations << " iterations in " << d.getDurationString() << std::endl;
        }

    // Smarter macro:
    // Avoids formatting when we won't log, but still has to search the loggers to find the one that is named,
    // and check its level before deciding whether to call it at all.
    // This macro is how it has been done until recently.
    // Note: results of first test run of 10 million iterations: 19.439 seconds.
    if (do2)
        {
        VInstant start;
        for (int i = 0; i < numIterations; ++i)
            {
            OLD_VLOGGER_NAMED_DEBUG(loggerName, VSTRING_FORMAT("i=%d", i));
            }
        VDuration d(VInstant() - start);
        std::cout << "MODE 2: " << numIterations << " iterations in " << d.getDurationString() << std::endl;
        }

    // New, improved macro:
    // First it calls the new "max log level" API to check whether any logger at all meets the level.
    // Only then does it bother to go look for the specified logger.
    // This is the new technique that virtually eliminates overhead.
    // Note: results of first test run of 10 million iterations: 0.095 seconds.
    if (do3)
        {
        VInstant start;
        for (int i = 0; i < numIterations; ++i)
            {
            VLOGGER_NAMED_DEBUG(loggerName, VSTRING_FORMAT("i=%d", i));
            }
        VDuration d(VInstant() - start);
        std::cout << "MODE 3: " << numIterations << " iterations in " << d.getDurationString() << std::endl;
        }

    VLogger::setDefaultLogger(oldDefaultLogger);
    VLogger::deleteLogger(loggerName);
    }
