/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vloggerunit.h"
#include "vlogger.h"
#include "vmessage.h"
#include "vbento.h"
#include "vsettings.h"

typedef std::vector<VNamedLogger*> VLoggerUnitLoggerList;

// VLoggerUnit ------------------------------------------------------------------------

static void _printLoggerInfo(const VString& label) {
    std::cout << "***** " << label << " *****" << std::endl;
    std::cout << VLogger::commandGetInfoString() << std::endl;
}

VLoggerUnit::VLoggerUnit(bool logOnSuccess, bool throwOnError) :
    VUnit("VLoggerUnit", logOnSuccess, throwOnError) {
}

void VLoggerUnit::run() {
    /*
    _printLoggerInfo("BEFORE _testNewInfrastructure");
        this->_testNewInfrastructure();
    _printLoggerInfo("AFTER _testNewInfrastructure");
    */
    this->_testMacros();
    this->_testStringLoggers();
    this->_testMaxActiveLogLevel();
    this->_testLoggerPathNames();
    this->_testSmartPtrLifecycle();
//    this->_testOptimizationPerformance();
}

void VLoggerUnit::_testMacros() {
    // Test every macro to make sure they all compile and run.
    // They won't all actually emit here in the unit test, because the actual log level is what it is.
    VString s("example buffer of data");
    VString name("dummy-logger-name");

    try {
        VLOGGER_FATAL_AND_THROW("Example of VLOGGER_FATAL_AND_THROW.");
    } catch (...) {}

    VLOGGER_LEVEL(VLoggerLevel::INFO, "Example of VLOGGER_LEVEL @ kInfo.");
    VLOGGER_LEVEL_FILELINE(VLoggerLevel::INFO, "Example of VLOGGER_LEVEL_FILELINE @ kInfo.", __FILE__, __LINE__); // (not normally called except by other macros)
    VLOGGER_FATAL("Example of VLOGGER_FATAL.");
    VLOGGER_ERROR("Example of VLOGGER_ERROR.");
    VLOGGER_WARN("Example of VLOGGER_WARN.");
    VLOGGER_INFO("Example of VLOGGER_INFO.");
    VLOGGER_DEBUG("Example of VLOGGER_DEBUG.");
    VLOGGER_TRACE("Example of VLOGGER_TRACE.");
    VLOGGER_LINE(VLoggerLevel::INFO, "Example of VLOGGER_LINE @ kInfo.");
    VLOGGER_HEXDUMP(VLoggerLevel::INFO, "Example of VLOGGER_HEXDUMP @ kInfo.", s.getDataBufferConst(), s.length() + 1);
    if (VLOGGER_WOULD_LOG(VLoggerLevel::INFO)) {}

    VLOGGER_NAMED_LEVEL(name, VLoggerLevel::INFO, "Example of VLOGGER_NAMED_LEVEL @ kInfo.");
    VLOGGER_NAMED_LEVEL_FILELINE(name, VLoggerLevel::INFO, "Example of VLOGGER_NAMED_LEVEL_FILELINE @ kInfo.", __FILE__, __LINE__); // (not normally called except by other macros)
    VLOGGER_NAMED_FATAL(name, "Example of VLOGGER_NAMED_FATAL.");
    VLOGGER_NAMED_ERROR(name, "Example of VLOGGER_NAMED_ERROR.");
    VLOGGER_NAMED_WARN(name, "Example of VLOGGER_NAMED_WARN.");
    VLOGGER_NAMED_INFO(name, "Example of VLOGGER_NAMED_INFO.");
    VLOGGER_NAMED_DEBUG(name, "Example of VLOGGER_NAMED_DEBUG.");
    VLOGGER_NAMED_TRACE(name, "Example of VLOGGER_NAMED_TRACE.");
    VLOGGER_NAMED_LINE(name, VLoggerLevel::INFO, "Example of VLOGGER_NAMED_LINE @ kInfo.");
    VLOGGER_NAMED_HEXDUMP(name, VLoggerLevel::INFO, "Example of VLOGGER_NAMED_HEXDUMP @ kInfo.", s.getDataBufferConst(), s.length() + 1);
    if (VLOGGER_NAMED_WOULD_LOG(name, VLoggerLevel::INFO)) {}

}

void VLoggerUnit::_testStringLoggers() {
    // For the VStringLogger/VStringVectorLogger tests, note that the logged
    // output captured in the logger includes timestamp/level text, so we can't
    // do an exact match test. Instead, do a contains() or endsWith() test.

    const VString FATAL_MESSAGE("A fatal message.");
    const VString ERROR_MESSAGE("An error message.");
    const VString WARN_MESSAGE("A warning message.");
    const VString INFO_MESSAGE("An info message.");
    const VString DEBUG_MESSAGE("A debug message.");
    const VString TRACE_MESSAGE("A trace message.");

    VStringLogger vsl("VLoggerUnit's VStringLogger", VLoggerLevel::INFO);
    vsl.log(VLoggerLevel::FATAL, FATAL_MESSAGE);
    vsl.log(VLoggerLevel::ERROR, ERROR_MESSAGE);
    vsl.log(VLoggerLevel::WARN, WARN_MESSAGE);
    vsl.log(VLoggerLevel::INFO, INFO_MESSAGE);
    vsl.log(VLoggerLevel::DEBUG, DEBUG_MESSAGE);
    vsl.log(VLoggerLevel::TRACE, TRACE_MESSAGE);

    this->test(vsl.getLines().contains(FATAL_MESSAGE), "VStringLogger contains fatal message");
    this->test(vsl.getLines().contains(ERROR_MESSAGE), "VStringLogger contains error message");
    this->test(vsl.getLines().contains(WARN_MESSAGE), "VStringLogger contains warn message");
    this->test(vsl.getLines().contains(INFO_MESSAGE), "VStringLogger contains info message");
    this->test(! vsl.getLines().contains(DEBUG_MESSAGE), "VStringLogger does not contain debug message");
    this->test(! vsl.getLines().contains(TRACE_MESSAGE), "VStringLogger does not contain trace message");

    this->logStatus(VSTRING_FORMAT("VStringLogger contents:\n%s", vsl.getLines().chars()));

    VStringVectorLogger vsvl("VLoggerUnit's VStringVectorLogger", VLoggerLevel::INFO, NULL);
    vsvl.log(VLoggerLevel::FATAL, FATAL_MESSAGE);
    vsvl.log(VLoggerLevel::ERROR, ERROR_MESSAGE);
    vsvl.log(VLoggerLevel::WARN, WARN_MESSAGE);
    vsvl.log(VLoggerLevel::INFO, INFO_MESSAGE);
    vsvl.log(VLoggerLevel::DEBUG, DEBUG_MESSAGE);
    vsvl.log(VLoggerLevel::TRACE, TRACE_MESSAGE);

    const VStringVector& actualOutputLines = vsvl.getLines();
    this->test(actualOutputLines.size() == 4, "VStringVectorLogger size = 4");
    this->test(actualOutputLines.at(0).endsWith(FATAL_MESSAGE), "VStringVectorLogger lines[0]");
    this->test(actualOutputLines.at(1).endsWith(ERROR_MESSAGE), "VStringVectorLogger lines[1]");
    this->test(actualOutputLines.at(2).endsWith(WARN_MESSAGE), "VStringVectorLogger lines[2]");
    this->test(actualOutputLines.at(3).endsWith(INFO_MESSAGE), "VStringVectorLogger lines[3]");

    this->logStatus("VStringVectorLogger contents follow, each as unit test status element:");
    for (VStringVector::const_iterator i = actualOutputLines.begin(); i != actualOutputLines.end(); ++i)
        this->logStatus(*i);

    /*
    Test that when there is frozen or simulated time, the logger emits both time values.
    */

    VDuration frozenFutureShift = 123 * VDuration::MILLISECOND();
    VInstant frozenStartTime; // get the time now as we start manipulating time
    VInstant shiftedFrozenTime = frozenStartTime + frozenFutureShift;
    VInstant::freezeTime(shiftedFrozenTime); // Until we un-freeze, all apparent VInstant "now" values will be this specified time that is shifted into the future.
    
    VString testTimeFormat("yMMddHHmmssSSS"); // Control the format we generate and therefore expect to see.
    VInstantFormatter testTimeFormatter(testTimeFormat);
    VStringLogger trueTimeLogOutputLogger("frozen time logging test logger", VLoggerLevel::INFO, VLogAppender::DO_FORMAT_OUTPUT, VString::EMPTY(), testTimeFormat);
    VString frozenTimeString = shiftedFrozenTime.getLocalString(testTimeFormatter);
    // Ever so slightly tricky: we'd like to verify that the true current time appears, but it can change between when we look at it
    // and when the logger emits it! To be 100% robust and not have rare test failures, let's allow the current time or the current
    // time plus 1ms to appear in the log output.
    VInstant trueNow; trueNow.setTrueNow(); // It will have the true current time, not the frozen time.
    VString trueNowString = trueNow.getLocalString(testTimeFormatter);
    trueNow += VDuration::MILLISECOND();
    VString trueNowPlus1msString = trueNow.getLocalString(testTimeFormatter);
    trueTimeLogOutputLogger.log(VLoggerLevel::WARN, VSTRING_FORMAT("This warning should have two timestamps to the left: the true time '%s' (or 1ms later = '%s') and the %s-shifted frozen time '%s'.", trueNowString.chars(), trueNowPlus1msString.chars(), frozenFutureShift.getDurationString().chars(), frozenTimeString.chars()));
    this->logStatus(VSTRING_FORMAT("frozen time logging test logger contents: %s", trueTimeLogOutputLogger.getLines().chars()));

    this->test(trueTimeLogOutputLogger.getLines().contains(frozenTimeString), VSTRING_FORMAT("frozen time logging test output contains frozen time %s into the future", frozenFutureShift.getDurationString().chars()));
    this->test(trueTimeLogOutputLogger.getLines().contains(trueNowString) || trueTimeLogOutputLogger.getLines().contains(trueNowPlus1msString), "frozen time logging test output contains true time");

    VInstant::unfreezeTime();
}

void VLoggerUnit::_testMaxActiveLogLevel() {
    // We assume the existing max logger level is less than 90.
    // We need to special case the use of the "VUnit" logger which may be present for routing
    // all unit test output. We temporarily downgrade its log level. Try/catch to ensure it
    // gets restored even if we throw an exception here.

    int oldVUnitLevel = -1;
    VNamedLoggerPtr vunitLogger = VLogger::findNamedLogger("VUnit");
    if (vunitLogger != NULL) {
        oldVUnitLevel = vunitLogger->getLevel();
        vunitLogger->setLevel(80);
    }

    try {
        _printLoggerInfo("BEFORE INSTALLING LOGGERS");

        int oldMaxActiveLevel = VLogger::gMaxActiveLevel;

        VLogger::installNewNamedLogger("90", 90, VStringVector());
        VUNIT_ASSERT_EQUAL_LABELED(VLogger::gMaxActiveLevel, 90, "max active level");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(89), "level -1 is active");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(90), "level == is active");
        VUNIT_ASSERT_FALSE_LABELED(VLogger::isLogLevelActive(91), "level +1 is not active");

        VLogger::installNewNamedLogger("94", 94, VStringVector());
        VUNIT_ASSERT_EQUAL_LABELED(VLogger::gMaxActiveLevel, 94, "max active level");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(93), "level -1 is active");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(94), "level == is active");
        VUNIT_ASSERT_FALSE_LABELED(VLogger::isLogLevelActive(95), "level +1 is not active");

        VLogger::installNewNamedLogger("92", 92, VStringVector());
        VUNIT_ASSERT_EQUAL_LABELED(VLogger::gMaxActiveLevel, 94, "max active level");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(93), "level -1 is active");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(94), "level == is active");
        VUNIT_ASSERT_FALSE_LABELED(VLogger::isLogLevelActive(95), "level +1 is not active");

        _printLoggerInfo("AFTER INSTALLING LOGGERS");

        VLogger::deregisterLogger("90");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::findNamedLogger("90") == NULL, "level 90 logger deleted");
        VUNIT_ASSERT_EQUAL_LABELED(VLogger::gMaxActiveLevel, 94, "max active level");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(93), "level -1 is active");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(94), "level == is active");
        VUNIT_ASSERT_FALSE_LABELED(VLogger::isLogLevelActive(95), "level +1 is not active");

        VLogger::deregisterLogger("94");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::findNamedLogger("94") == NULL, "level 94 logger deleted");
        VUNIT_ASSERT_EQUAL_LABELED(VLogger::gMaxActiveLevel, 92, "max active level");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(91), "level -1 is active");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(92), "level == is active");
        VUNIT_ASSERT_FALSE_LABELED(VLogger::isLogLevelActive(93), "level +1 is not active");

        VLogger::deregisterLogger("92");
        _printLoggerInfo("AFTER DEREGISTERING LOGGERS");

        VUNIT_ASSERT_TRUE_LABELED(VLogger::findNamedLogger("92") == NULL, "level 92 logger deleted");
        VUNIT_ASSERT_EQUAL_LABELED(VLogger::gMaxActiveLevel, oldMaxActiveLevel, "max active level");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(oldMaxActiveLevel - 1), "level -1 is active");
        VUNIT_ASSERT_TRUE_LABELED(VLogger::isLogLevelActive(oldMaxActiveLevel), "level == is active");
        VUNIT_ASSERT_FALSE_LABELED(VLogger::isLogLevelActive(oldMaxActiveLevel + 1), "level +1 is not active");
    } catch (...) {
        if (vunitLogger != NULL)
            vunitLogger->setLevel(oldVUnitLevel);

        throw;
    }
}

static VString _createValueString(int i) {
    return VSTRING_FORMAT("value[%d]", i);
}

void VLoggerUnit::_testLoggerPathNames() {
    // Create a defined hierarchy of logger names.
    // Use different log levels.
    // Route output to the loggers by using paths.
    // Verify that the right loggers are found, and the right levels are honored.
    // VStringLogger is useful because we can examine its contents easily after logging.

    std::vector<VNamedLogger*> loggers;
    loggers.push_back(new VStringLogger("diagnostics",                          VLoggerLevel::ERROR));
    loggers.push_back(new VStringLogger("diagnostics.sensors",                  VLoggerLevel::WARN));
    loggers.push_back(new VStringLogger("diagnostics.sensors.transponders",     VLoggerLevel::INFO));
    loggers.push_back(new VStringLogger("diagnostics.sensors.transponders.42",  VLoggerLevel::DEBUG));
    std::vector<std::vector<int> > loggerNWanted;

    for (std::vector<VNamedLogger*>::const_iterator i = loggers.begin(); i != loggers.end(); ++i)
        VLogger::registerLogger(VNamedLoggerPtr(*i));

    // Verify that we find the right loggers explicitly. Test things like extra trailing or internal path separator.
    VNamedLoggerPtr defaultLogger = VLogger::getDefaultLogger();
    VNamedLoggerPtr foundLogger;

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
    s = _createValueString(++val); VLOGGER_NAMED_WARN("diagnostics",                  s);  // "s" should NOT get logged.
    s = _createValueString(++val); VLOGGER_NAMED_FATAL("diagnostics.blahblahblah",     s); logger0Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_ERROR("diagnostics.blahblahblah",     s); logger0Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_WARN("diagnostics.blahblahblah",     s);  // "s" should NOT get logged.
    s = _createValueString(++val); VLOGGER_NAMED_FATAL("diagnostics.blah.blah.blah",   s); logger0Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_ERROR("diagnostics.blah.blah.blah",   s); logger0Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_WARN("diagnostics.blah.blah.blah",   s);  // "s" should NOT get logged.
    loggerNWanted.push_back(logger0Wanted);

    // for "diagnostics.sensors": WARN and below (we test WARN and ERROR, do not want INFO)
    std::vector<int> logger1Wanted;
    s = _createValueString(++val); VLOGGER_NAMED_ERROR("diagnostics.sensors",                  s); logger1Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_WARN("diagnostics.sensors",                  s); logger1Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_INFO("diagnostics.sensors",                  s);  // "s" should NOT get logged.
    s = _createValueString(++val); VLOGGER_NAMED_ERROR("diagnostics.sensors.blahblahblah",     s); logger1Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_WARN("diagnostics.sensors.blahblahblah",     s); logger1Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_INFO("diagnostics.sensors.blahblahblah",     s);  // "s" should NOT get logged.
    s = _createValueString(++val); VLOGGER_NAMED_ERROR("diagnostics.sensors.blah.blah.blah",   s); logger1Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_WARN("diagnostics.sensors.blah.blah.blah",   s); logger1Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_INFO("diagnostics.sensors.blah.blah.blah",   s);  // "s" should NOT get logged.
    loggerNWanted.push_back(logger1Wanted);

    // for "diagnostics.sensors.transponders": INFO and below (we test INFO and WARN, do not want DEBUG)
    std::vector<int> logger2Wanted;
    s = _createValueString(++val); VLOGGER_NAMED_WARN("diagnostics.sensors.transponders",                  s); logger2Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_INFO("diagnostics.sensors.transponders",                  s); logger2Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_DEBUG("diagnostics.sensors.transponders",                  s); // "s" should NOT get logged.
    s = _createValueString(++val); VLOGGER_NAMED_WARN("diagnostics.sensors.transponders.blahblahblah",     s); logger2Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_INFO("diagnostics.sensors.transponders.blahblahblah",     s); logger2Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_DEBUG("diagnostics.sensors.transponders.blahblahblah",     s); // "s" should NOT get logged.
    s = _createValueString(++val); VLOGGER_NAMED_WARN("diagnostics.sensors.transponders.blah.blah.blah",   s); logger2Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_INFO("diagnostics.sensors.transponders.blah.blah.blah",   s); logger2Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_DEBUG("diagnostics.sensors.transponders.blah.blah.blah",   s); // "s" should NOT get logged.
    loggerNWanted.push_back(logger2Wanted);

    // for "diagnostics.sensors.transponders.42": DEBUG and below (we test DEBUG and INFO, do not want TRACE)
    std::vector<int> logger3Wanted;
    s = _createValueString(++val); VLOGGER_NAMED_INFO("diagnostics.sensors.transponders.42",                  s); logger3Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_DEBUG("diagnostics.sensors.transponders.42",                  s); logger3Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_TRACE("diagnostics.sensors.transponders.42",                  s); // "s" should NOT get logged.
    s = _createValueString(++val); VLOGGER_NAMED_INFO("diagnostics.sensors.transponders.42.blahblahblah",     s); logger3Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_DEBUG("diagnostics.sensors.transponders.42.blahblahblah",     s); logger3Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_TRACE("diagnostics.sensors.transponders.42.blahblahblah",     s); // "s" should NOT get logged.
    s = _createValueString(++val); VLOGGER_NAMED_INFO("diagnostics.sensors.transponders.42.blah.blah.blah",   s); logger3Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_DEBUG("diagnostics.sensors.transponders.42.blah.blah.blah",   s); logger3Wanted.push_back(val);
    s = _createValueString(++val); VLOGGER_NAMED_TRACE("diagnostics.sensors.transponders.42.blah.blah.blah",   s); // "s" should NOT get logged.
    loggerNWanted.push_back(logger3Wanted);

    // Now, for each logger, test that it contains the value strings it should (no-yes-yes per each
    // triplet shown above), and none of the other entire set. We want to test the entire set in order
    // to prove there is no cross-contamination of the loggers from mixed-up path name searching, etc.

    const int endVal = val;

    int loggerNIndex = 0;
    for (VLoggerUnitLoggerList::const_iterator loggersIterator = loggers.begin(); loggersIterator != loggers.end(); ++loggersIterator) {
        VStringLogger* vsl = static_cast<VStringLogger*>(*loggersIterator);
        const VString& lines = vsl->getLines();
        this->logStatus(VSTRING_FORMAT("Logger '%s' lines:\n%s", vsl->getName().chars(), lines.chars()));
        for (int testVal = 0; testVal < endVal; ++testVal) {
            VString si = _createValueString(testVal);
            if (std::find(loggerNWanted[loggerNIndex].begin(), loggerNWanted[loggerNIndex].end(), testVal) != loggerNWanted[loggerNIndex].end())
                this->test(lines.contains(si), VSTRING_FORMAT("%s / %s present", vsl->getName().chars(), si.chars()));
            else
                this->test(! lines.contains(si), VSTRING_FORMAT("%s / %s not present", vsl->getName().chars(), si.chars()));
        }

        ++loggerNIndex;
    }

    for (VLoggerUnitLoggerList::const_iterator i = loggers.begin(); i != loggers.end(); ++i) {
        VString loggerName = (*i)->getName();
        VLogger::deregisterLogger(loggerName); // destroys the VLogger object with that name
        VUNIT_ASSERT_TRUE_LABELED(VLogger::findNamedLogger(loggerName) == NULL, VSTRING_FORMAT("logger '%s' deleted", loggerName.chars()));
    }

    // Note: We just deleted the loggers in the loop above. The VLoggerUnitLoggerList (loggers) now contains garbage pointers.
    // Do not reference them after the loop. Destructing the list is OK.
}

void VLoggerUnit::_testSmartPtrLifecycle() {

    // Regression test for bug in VNamedLogger::log() that incorrectly passed naked (this) to VNamedLoggerPtr() for VThread::logStackCrawl() parameter, causing premature destruction of logger on return.
    VLogger::getDefaultLogger()->setPrintStackInfo(VLoggerLevel::WARN, 1 /*only 1 stack crawl, then disable*/, VDuration::POSITIVE_INFINITY() /*use count limit only, no time limit*/);
    VLOGGER_WARN("This warning should appear as a [warn ] line in the logger, and also appear as the start of a stack trace on the next line (if stack trace is implemented).");
    VLOGGER_WARN("This warning should appear as a [warn ] line in the logger, but should not generate a stack crawl on the next line."); // Previous smartptr crash regression will be detected here.

}

#define OLDEST_VLOGGER_NAMED_DEBUG(loggername, message) VLogger::getLogger(loggername)->log(VLoggerLevel::DEBUG, message)
#define OLD_VLOGGER_NAMED_DEBUG(loggername, message) do { VNamedLoggerPtr vlcond = VLogger::findNamedLoggerForLevel(loggername, VLoggerLevel::DEBUG); if (vlcond != NULL) vlcond->log(VLoggerLevel::DEBUG, NULL, 0, message); } while (false)
// for reference, as of this writing, the new one basically expands to:
// #define VLOGGER_NAMED_DEBUG(loggername, message) do { if (!VLogger::isLogLevelActive(VLoggerLevel::DEBUG)) break; VLogger* vlcond = VLogger::getLoggerConditional(loggername, VLoggerLevel::DEBUG); if (vlcond != NULL) vlcond->log(VLoggerLevel::DEBUG, NULL, 0, message); } while (false)

void VLoggerUnit::_testOptimizationPerformance() {
    const int numIterations = 10000000;
    const VString loggerName("speed-test-logger");
    VNamedLoggerPtr oldDefaultLogger = VLogger::getDefaultLogger();
    VNamedLoggerPtr logger(new VNamedLogger(loggerName, VLoggerLevel::INFO, VStringVector()));
    VLogger::registerLogger(logger, true /* as default logger */);

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
    if (do1) {
        VInstant start;
        for (int i = 0; i < numIterations; ++i) {
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
    if (do2) {
        VInstant start;
        for (int i = 0; i < numIterations; ++i) {
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
    if (do3) {
        VInstant start;
        for (int i = 0; i < numIterations; ++i) {
            VLOGGER_NAMED_DEBUG(loggerName, VSTRING_FORMAT("i=%d", i));
        }
        VDuration d(VInstant() - start);
        std::cout << "MODE 3: " << numIterations << " iterations in " << d.getDurationString() << std::endl;
    }

    VLogger::setDefaultLogger(oldDefaultLogger);
    VLogger::deregisterLogger(loggerName);
}

static void _println(const VString& s) {
    std::cout << s << std::endl;
}

static void _functionTakingConst(VNamedLoggerConstPtr p) {
    (void) p->getLevel();
}

void VLoggerUnit::_testNewInfrastructure() {
    VString settingsText(VSTRING_COPY(
                             "<logging>"
                             "<appender name=\"default\" kind=\"file\" filename=\"test-out.log\" />"
                             "<appender name=\"separate\" kind=\"file\" filename=\"separate.log\" />"
                             "<logger name=\"default\" level=\"60\" appender=\"default\" />"
                             "<logger name=\"special\" level=\"80\" appender=\"default\" > <appender name=\"separate\" /> </logger>"
                             "<logger name=\"separate\" level=\"80\" appender=\"separate\" />"
                             "</logging>"
                         ));

    VMemoryStream buf(settingsText.getDataBuffer(), VMemoryStream::kAllocatedByOperatorNew, false, settingsText.length(), settingsText.length());
    VTextIOStream in(buf);
    VSettings settings(in);

    VLogger::configure(VFSNode("."), *(settings.findNode("logging")));

    VLOGGER_INFO("this is an INFO message to default");
    VLOGGER_LEVEL(VLoggerLevel::DEBUG, "this is a DEBUG message to default (WHICH SHOULD NOT APPEAR SINCE LOGGER IS @INFO)");

    VLOGGER_NAMED_INFO("special", "this is an INFO message to special (which goes to both appenders)");
    VLOGGER_NAMED_LEVEL("special", VLoggerLevel::DEBUG, "this is a DEBUG message to special (which goes to both appenders)");

    VLOGGER_NAMED_INFO("separate", "this is an INFO message to the separate output file");
    VLOGGER_NAMED_LEVEL("separate", VLoggerLevel::DEBUG, "this is a DEBUG message to the separate output file");

    VLOGGER_NAMED_HEXDUMP("special", VLoggerLevel::DEBUG, "This is a hex dump of the XML string buffer used to configure logging  (which goes to both appenders)", settingsText.getDataBuffer(), 1 + settingsText.length());

    VStringLogger slx("slx", VLoggerLevel::INFO);
    slx.log(VLoggerLevel::INFO, NULL, 0, "1.slx.info");
    slx.log(VLoggerLevel::DEBUG, NULL, 0, "2.slx.debug");
    slx.log(VLoggerLevel::INFO, NULL, 0, "3.slx.info");
    VString slxOut = slx.getLines();
    _println("HERE IS WHAT SLX CAPTURED:");
    _println(slxOut);

    VStringLogAppender* slxap = new VStringLogAppender("slxa", false, VString::EMPTY(), VString::EMPTY());
    VLogAppenderPtr slxa(slxap);
    VNamedLogger slxx("slxx", VLoggerLevel::INFO, VStringVector(), slxa);
    slxx.log(VLoggerLevel::INFO, NULL, 0, "1.slxx.info");
    slxx.log(VLoggerLevel::DEBUG, NULL, 0, "2.slxx.debug");
    slxx.log(VLoggerLevel::INFO, NULL, 0, "3.slxx.info");
    VString slxxOut = slxap->getLines();
    _println("HERE IS WHAT SLXX CAPTURED:");
    _println(slxxOut);

    VStringVectorLogger svlx("svlx", VLoggerLevel::INFO, NULL);
    svlx.log(VLoggerLevel::INFO, NULL, 0, "1.svlx.info");
    svlx.log(VLoggerLevel::DEBUG, NULL, 0, "2.svlx.debug");
    svlx.log(VLoggerLevel::INFO, NULL, 0, "3.svlx.info");
    VStringVector svlxOut = svlx.getLines();
    _println("HERE IS WHAT SVLX CAPTURED:");
    for (VStringVector::const_iterator i = svlxOut.begin(); i != svlxOut.end(); ++i)
        _println(*i);

    // Test const behavior of shared_ptr ...
    VNamedLoggerConstPtr constLogger = VLogger::getDefaultLogger();
    //constLogger->setLevel(40);
    _functionTakingConst(constLogger);
    _functionTakingConst(VLogger::getDefaultLogger());

    VLogger::shutdown();
}
