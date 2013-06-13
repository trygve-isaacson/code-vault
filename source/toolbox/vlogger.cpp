/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

/** @file */

#include "vlogger.h"

#include "vthread.h"
#include "vmutexlocker.h"
#include "vsettings.h"

#include <boost/scoped_ptr.hpp>

static const VNamedLoggerPtr NULL_NAMED_LOGGER_PTR;
static const VLogAppenderPtr NULL_LOG_APPENDER_PTR;

volatile int VLogger::gMaxActiveLevel = 0;
VNamedLoggerPtr VLogger::gDefaultLogger = NULL_NAMED_LOGGER_PTR;
VLogAppenderPtr VLogger::gDefaultAppender = NULL_LOG_APPENDER_PTR;
VFSNode VLogger::gBaseLogDirectory(".");

// VNamedLogger ---------------------------------------------------------------

VNamedLogger::VNamedLogger(const VString& name, int level, const VStringVector& appenderNames, VLogAppenderPtr specificAppender)
    : boost::enable_shared_from_this<VNamedLogger>()
    , mName(name)
    , mLevel(level)
    , mAppendersMutex(VSTRING_FORMAT("VNamedLogger[%s]", name.chars()), true/*suppress logging*/)
    , mAppenderNames(appenderNames)
    , mSpecificAppender(specificAppender)
    , mRepetitionFilter()
    , mPrintStackConfig() {
    if (appenderNames.empty() && (specificAppender == NULL_LOG_APPENDER_PTR)) {
        mAppenderNames.push_back(VString::EMPTY());
    }
}

VNamedLogger::~VNamedLogger() {
}

void VNamedLogger::clearAppenders() {
    mAppenderNames.clear();
}

void VNamedLogger::setAppender(const VString& appenderName) {
    mAppenderNames.clear();
    mAppenderNames.push_back(appenderName);
}

void VNamedLogger::addAppender(const VString& appenderName) {
    mAppenderNames.push_back(appenderName);
}

void VNamedLogger::log(int level, const char* file, int line, const VString& message) {
    if (level > mLevel) {
        return;
    }

    VNamedLogger::_breakpointLocationForLog();

    if (mRepetitionFilter.isEnabled()) { // avoid mutex if no need to check filter
        VMutexLocker timeoutLocker(&mAppendersMutex, "VNamedLogger::log() checkTimeout");
        mRepetitionFilter.checkTimeout(*this);
    }

    VMutexLocker locker(&mAppendersMutex, "VNamedLogger::log");
    if (mRepetitionFilter.checkMessage(*this, level, file, line, message)) {
        this->_emitToAppenders(level, file, line, true, message, false, VString::EMPTY());
        if (mPrintStackConfig.shouldPrintStack(level, *this)) {
            locker.unlock(); // avoid recursive deadlock, we're done with our data until we recur
            VThread::logStackCrawl(message, VNamedLoggerPtr(shared_from_this()), false);
        }
    }
}

void VNamedLogger::addInfo(VBentoNode& infoNode) const {
    infoNode.addString("name", mName);
    infoNode.addInt("level", mLevel);

    if (this->isDefaultLogger()) {
        infoNode.addBool("is-default-logger", true);
    }

    if (mAppenderNames.size() == 1) {
        infoNode.addString("appender", mAppenderNames[0]);
    } else if (mAppenderNames.size() > 1) {
        (void) infoNode.addStringArray("appenders", mAppenderNames);
    }

    infoNode.addBool("repetition-filter-enabled", mRepetitionFilter.isEnabled());
    infoNode.addInt("print-stack-level", mPrintStackConfig.getLevel());
}

void VNamedLogger::log(int level, const VString& message) {
    this->log(level, NULL, 0, message);
}

void VNamedLogger::logHexDump(int level, const VString& message, const Vu8* buffer, Vs64 length) {
    if (level > mLevel) {
        return;
    }

    VNamedLogger::_breakpointLocationForLog();

    // Try to be efficient here:
    // Form the hex dump only if the length is > 0.
    // But do it once ahead of time, then interate over the appenders, writing to each one.

    VString hexString;
    if (length > 0) {
        VMemoryStream   tempBuffer;
        VTextIOStream   stream(tempBuffer);
        VHex            hex(&stream);
        hex.printHex(buffer, length);
        hexString.copyFromBuffer((const char*)tempBuffer.getBuffer(), 0, (int) tempBuffer.getEOFOffset());
    }

    VMutexLocker locker(&mAppendersMutex, "VNamedLogger::logHexDump");
    this->_emitToAppenders(level, NULL, 0, true, message, true, hexString);
}

void VNamedLogger::emitStackCrawlLine(const VString& message) {
    VMutexLocker locker(&mAppendersMutex, "VNamedLogger::emitStackCrawlLine");
    this->_emitToAppenders(VLoggerLevel::TRACE /* not used for raw line emit */, NULL, 0, false, VString::EMPTY(), true, message);
}

void VNamedLogger::setLevel(int level) {
    int oldLevel = mLevel;
    mLevel = level;
    VLogger::checkMaxActiveLogLevelForChangedLogger(oldLevel, level);
}

bool VNamedLogger::isDefaultLogger() const {
    return VLogger::gDefaultLogger.get() == this;
}

void VNamedLogger::_emitToAppenders(int level, const char* file, int line, bool emitMessage, const VString& message, bool emitRawLine, const VString& rawLine) {
    if (mSpecificAppender != NULL_LOG_APPENDER_PTR) {
        mSpecificAppender->emit(level, file, line, emitMessage, message, emitRawLine, rawLine);
    }

    for (VStringVector::const_iterator i = mAppenderNames.begin(); i != mAppenderNames.end(); ++i) {
        VLogAppenderPtr appender = (*i).isEmpty() ? VLogger::getDefaultAppender() : VLogger::getAppender(*i);
        appender->emit(level, file, line, emitMessage, message, emitRawLine, rawLine);
    }

    VLogger::emitToGlobalAppenders(level, file, line, emitMessage, message, emitRawLine, rawLine);
}

VString VNamedLogger::_toString() const {
    VString s(VSTRING_ARGS("VNamedLogger '%s' (%d) ->", mName.chars(), mLevel));

    VMutexLocker locker(&mAppendersMutex, "VNamedLogger::_toString");
    for (VStringVector::const_iterator i = mAppenderNames.begin(); i != mAppenderNames.end(); ++i) {
        s += VSTRING_FORMAT(" '%s'", (*i).chars());
    }

    return s;
}

// static
void VNamedLogger::_breakpointLocationForLog() {
    // Put a breakpoint here if you want to break on every message that is logged,
    // once basic level filtering has been passed.
}

// Provided factories ---------------------------------------------------------

class VCoutLogAppenderFactory : public VLogAppenderFactory {
    public:
        VCoutLogAppenderFactory() : VLogAppenderFactory() {}
        virtual ~VCoutLogAppenderFactory() {}

        virtual VLogAppenderPtr instantiateLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults) const
            { return VLogAppenderPtr(new VCoutLogAppender(settings, defaults)); }
        virtual void addInfo(VBentoNode& infoNode) const
            { infoNode.addString("type", "VCoutLogAppenderFactory"); }
};

class VFileLogAppenderFactory : public VLogAppenderFactory {
    public:
        VFileLogAppenderFactory() : VLogAppenderFactory() {}
        virtual ~VFileLogAppenderFactory() {}

        virtual VLogAppenderPtr instantiateLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults) const
            { return VLogAppenderPtr(new VFileLogAppender(settings, defaults)); }
        virtual void addInfo(VBentoNode& infoNode) const
            { infoNode.addString("type", "VFileLogAppenderFactory"); }
};

class VRollingFileLogAppenderFactory : public VLogAppenderFactory {
    public:
        VRollingFileLogAppenderFactory() : VLogAppenderFactory() {}
        virtual ~VRollingFileLogAppenderFactory() {}

        virtual VLogAppenderPtr instantiateLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults) const
            { return VLogAppenderPtr(new VRollingFileLogAppender(settings, defaults)); }
        virtual void addInfo(VBentoNode& infoNode) const
            { infoNode.addString("type", "VRollingFileLogAppenderFactory"); }
};

class VSilentLogAppenderFactory : public VLogAppenderFactory {
    public:
        VSilentLogAppenderFactory() : VLogAppenderFactory() {}
        virtual ~VSilentLogAppenderFactory() {}

        virtual VLogAppenderPtr instantiateLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults) const
            { return VLogAppenderPtr(new VSilentLogAppender(settings, defaults)); }
        virtual void addInfo(VBentoNode& infoNode) const
            { infoNode.addString("type", "VSilentLogAppenderFactory"); }
};

class VStringLogAppenderFactory : public VLogAppenderFactory {
    public:
        VStringLogAppenderFactory() : VLogAppenderFactory() {}
        virtual ~VStringLogAppenderFactory() {}

        virtual VLogAppenderPtr instantiateLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults) const
            { return VLogAppenderPtr(new VStringLogAppender(settings, defaults)); }
        virtual void addInfo(VBentoNode& infoNode) const
            { infoNode.addString("type", "VStringLogAppenderFactory"); }
};

class VStringVectorLogAppenderFactory : public VLogAppenderFactory {
    public:
        VStringVectorLogAppenderFactory() : VLogAppenderFactory() {}
        virtual ~VStringVectorLogAppenderFactory() {}

        virtual VLogAppenderPtr instantiateLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults) const
            { return VLogAppenderPtr(new VStringVectorLogAppender(settings, defaults)); }
        virtual void addInfo(VBentoNode& infoNode) const
            { infoNode.addString("type", "VStringVectorLogAppenderFactory"); }
};

// VLogger -------------------------------------------------------------------

// This style of static mutex declaration and access ensures correct
// initialization if accessed during the static initialization phase.
static VMutex* _mutexInstance() {
    static VMutex gVLoggerMutex("gVLoggerMutex", true/*suppress logging of the logger mutex*/);
    return &gVLoggerMutex;
}

// _mutexInstance() must be used internally whenever referencing these static accessors:

typedef std::map<VString, VNamedLoggerPtr> VNamedLoggerMap;
static VNamedLoggerMap& _getLoggerMap() {
    static VNamedLoggerMap gLoggerMap;
    return gLoggerMap;
};

typedef std::map<VString, VLogAppenderPtr> VLogAppendersMap;
static VLogAppendersMap& _getAppendersMap() {
    static VLogAppendersMap gAppendersMap;
    return gAppendersMap;
}
static VLogAppendersMap& _getGlobalAppendersMap() {
    static VLogAppendersMap gGlobalAppendersMap;
    return gGlobalAppendersMap;
}

typedef std::map<VString, VLogAppenderFactoryPtr> VLogAppenderFactoriesMap;
static VLogAppenderFactoriesMap& _getAppenderFactoriesMap() {
    static VLogAppenderFactoriesMap gAppenderFactoriesMap;
    return gAppenderFactoriesMap;
}

// static
VString VLoggerLevel::getName(int level) {
    // We make all strings 5 characters long for clean layout in the log output.

    if (level == FATAL)
        return VSTRING_COPY("FATAL");
    else if (level == ERROR)
        return VSTRING_COPY("ERROR");
    else if (level == WARN)
        return VSTRING_COPY("WARN ");
    else if (level == INFO)
        return VSTRING_COPY("INFO ");
    else if (level == DEBUG)
        return VSTRING_COPY("DEBUG");
    else if (level == TRACE)
        return VSTRING_COPY("TRACE");
    else if (level > DEBUG)
        return VSTRING_FORMAT("DBG%2d", level);
    else if (level > INFO)
        return VSTRING_FORMAT("INF%2d", level);
    else if (level > WARN)
        return VSTRING_FORMAT("WRN%2d", level);
    else if (level > ERROR)
        return VSTRING_FORMAT("ERR%2d", level);
    else
        return VSTRING_FORMAT("%5d", level);
}

// static
void VLogger::installNewLogAppender(const VSettingsNode& appenderSettings, const VSettingsNode& appenderDefaults) {
    VMutexLocker locker(_mutexInstance(), "VLogger::installNewLogAppender");
    VLogAppenderFactoriesMap::const_iterator pos = _getAppenderFactoriesMap().find(appenderSettings.getString("kind"));
    if (pos != _getAppenderFactoriesMap().end()) {
        VLogAppenderPtr appender = pos->second->instantiateLogAppender(appenderSettings, appenderDefaults);

        locker.unlock();
        VLogger::registerLogAppender(appender);
    }
}

// static
void VLogger::installNewNamedLogger(const VSettingsNode& loggerSettings) {
    VString name = loggerSettings.getString("name");
    int level = loggerSettings.getInt("level", VLoggerLevel::INFO);
    VStringVector appenderNames;
    VString appenderName = loggerSettings.getString("appender", VString::EMPTY());
    if (appenderName.isNotEmpty()) {
        appenderNames.push_back(appenderName);
    }

    const int numAppenders = loggerSettings.countNamedChildren("appender");
    for (int i = 0; i < numAppenders; ++i) {
        const VSettingsNode* appenderNode = loggerSettings.getNamedChild("appender", i);
        appenderName = appenderNode->getString("name", VString::EMPTY());
        if (appenderName.isNotEmpty()) {
            appenderNames.push_back(appenderName);
        }
    }

    VNamedLoggerPtr logger(new VNamedLogger(name, level, appenderNames));
    int printStackLevel = loggerSettings.getInt("print-stack-level", VLoggerLevel::OFF);
    if (printStackLevel != VLoggerLevel::OFF) {
        int maxNumOccurrences = loggerSettings.getInt("print-stack-count", -1);
        VDuration timeLimit = loggerSettings.getDuration("print-stack-duration", VDuration::POSITIVE_INFINITY());
        logger->setPrintStackInfo(printStackLevel, maxNumOccurrences, timeLimit);
    }

    VMutexLocker locker(_mutexInstance(), "VLogger::installNewNamedLogger");
    VLogger::_registerLogger(logger, false);
}

// static
void VLogger::installNewNamedLogger(const VString& name, int level, const VStringVector& appenderNames) {
    VNamedLoggerPtr logger(new VNamedLogger(name, level, appenderNames));

    VMutexLocker locker(_mutexInstance(), "VLogger::installNewNamedLogger");
    VLogger::_registerLogger(logger, false);
}

// static
void VLogger::installNewNamedLogger(const VString& name, int level, const VString& appenderName) {
    VStringVector appenderNames;
    appenderNames.push_back(appenderName);
    VLogger::installNewNamedLogger(name, level, appenderNames);
}

// static
void VLogger::registerLogAppenderFactory(const VString& appenderKind, VLogAppenderFactoryPtr factory) {
    VMutexLocker locker(_mutexInstance(), "VLogger::registerLogAppenderFactory");
    _getAppenderFactoriesMap()[appenderKind] = factory;
}

// static
void VLogger::configure(const VFSNode& baseLogDirectory, const VSettingsNode& loggingSettings) {
    if (baseLogDirectory.getPath().isNotEmpty())
        gBaseLogDirectory = baseLogDirectory;

    // The caller is welcome to register other appender factory types before calling configure where we register the built-in types.
    VLogger::registerLogAppenderFactory("cout", VLogAppenderFactoryPtr(new VCoutLogAppenderFactory()));
    VLogger::registerLogAppenderFactory("file", VLogAppenderFactoryPtr(new VFileLogAppenderFactory()));
    VLogger::registerLogAppenderFactory("rolling-file", VLogAppenderFactoryPtr(new VRollingFileLogAppenderFactory()));
    VLogger::registerLogAppenderFactory("silent", VLogAppenderFactoryPtr(new VSilentLogAppenderFactory()));
    VLogger::registerLogAppenderFactory("string", VLogAppenderFactoryPtr(new VStringLogAppenderFactory()));
    VLogger::registerLogAppenderFactory("string-vector", VLogAppenderFactoryPtr(new VStringVectorLogAppenderFactory()));

    // Stash any per-appender defaults in a map while we configure, so we can pass them to the factories we call.
    std::map<VString, const VSettingsNode*> defaultsForAppenders;
    VSettings emptyDefaults;
    const int numAppenderDefaults = loggingSettings.countNamedChildren("appender-defaults");
    for (int i = 0; i < numAppenderDefaults; ++i) {
        const VSettingsNode* appenderDefaultsNode = loggingSettings.getNamedChild("appender-defaults", i);
        defaultsForAppenders[appenderDefaultsNode->getString("kind", VString::EMPTY())] = appenderDefaultsNode;
    }

    const int numAppenders = loggingSettings.countNamedChildren("appender");
    for (int i = 0; i < numAppenders; ++i) {
        const VSettingsNode* appenderNode = loggingSettings.getNamedChild("appender", i);
        const VSettingsNode* appenderDefaults = defaultsForAppenders[appenderNode->getString("kind")];
        if (appenderDefaults == NULL) {
            appenderDefaults = &emptyDefaults;
        }

        VLogger::installNewLogAppender(*appenderNode, *appenderDefaults);
    }

    const int numLoggers = loggingSettings.countNamedChildren("logger");
    for (int i = 0; i < numLoggers; ++i) {
        const VSettingsNode* loggerNode = loggingSettings.getNamedChild("logger", i);
        VLogger::installNewNamedLogger(*loggerNode);
    }
}

// static
void VLogger::shutdown() {
    VMutexLocker locker(_mutexInstance(), "VLogger::shutdown");

    // Clear all shared_ptr references. This will allow all referenced objects to be deleted (unless someone outside retains a reference).
    gDefaultLogger.reset();
    gDefaultAppender.reset();
    _getLoggerMap().clear();
    _getAppendersMap().clear();
    _getAppenderFactoriesMap().clear();

    gMaxActiveLevel = 0;
}

// static
void VLogger::registerLogAppender(VLogAppenderPtr appender, bool asDefaultAppender) {
    VMutexLocker locker(_mutexInstance(), "VLogger::registerLogAppender");
    VLogger::_registerAppender(appender, asDefaultAppender, false);
}

// static
void VLogger::registerGlobalAppender(VLogAppenderPtr appender, bool asDefaultAppender) {
    VMutexLocker locker(_mutexInstance(), "VLogger::registerLogAppender");
    VLogger::_registerAppender(appender, asDefaultAppender, true);
}

// static
void VLogger::registerLogger(VNamedLoggerPtr namedLogger, bool asDefaultLogger) {
    VMutexLocker locker(_mutexInstance(), "VLogger::registerLogger");
    VLogger::_registerLogger(namedLogger, asDefaultLogger);
}

// static
void VLogger::deregisterLogAppender(VLogAppenderPtr appender) {
    VMutexLocker locker(_mutexInstance(), "VLogger::deregisterLogAppender");

    if (gDefaultAppender == appender) {
        gDefaultAppender.reset();
    }

    VLogAppendersMap::iterator pos = _getAppendersMap().find(appender->getName());
    if (pos != _getAppendersMap().end()) {
        _getAppendersMap().erase(pos);
    }

    pos = _getGlobalAppendersMap().find(appender->getName());
    if (pos != _getGlobalAppendersMap().end()) {
        _getGlobalAppendersMap().erase(pos);
    }

}

// static
void VLogger::deregisterLogAppender(const VString& name) {
    VLogAppenderPtr appender = VLogger::findAppender(name);
    if (appender != NULL) {
        VLogger::deregisterLogAppender(appender);
    }
}

// static
void VLogger::deregisterLogger(VNamedLoggerPtr namedLogger) {
    VMutexLocker locker(_mutexInstance(), "VLogger::deregisterLogger");

    if (gDefaultLogger == namedLogger) {
        gDefaultLogger.reset();
    }

    VNamedLoggerMap::iterator pos = _getLoggerMap().find(namedLogger->getName());
    if (pos != _getLoggerMap().end()) {
        _getLoggerMap().erase(pos);
    }

    VLogger::_checkMaxActiveLogLevelForRemovedLogger(namedLogger->getLevel());
}

// static
void VLogger::deregisterLogger(const VString& name) {
    VNamedLoggerPtr logger = VLogger::findNamedLogger(name);
    if (logger != NULL) {
        VLogger::deregisterLogger(logger);
    }
}

// static
bool VLogger::isDefaultLogLevelActive(int level) {
    return VLogger::isLogLevelActive(level) && VLogger::getDefaultLogger()->isEnabledFor(level);
}

// static
bool VLogger::isLogLevelActive(int level) {
    // No locking necessary to simply read this volatile value.
    return (level <= gMaxActiveLevel);
}

// static
VNamedLoggerPtr VLogger::getDefaultLogger() {
    VMutexLocker locker(_mutexInstance(), "VLogger::getDefaultLogger");

    if (gDefaultLogger == NULL) {
        VLogger::_registerLogger(VNamedLoggerPtr(new VNamedLogger("auto-default-logger", VLoggerLevel::INFO, VStringVector())), true);
    }

    return gDefaultLogger;
}

#ifdef VLOGGER_INTERNAL_DEBUGGING
// static
void VLogger::_reportLoggerChange(bool before, const VString& label, const VNamedLoggerPtr& was, const VNamedLoggerPtr& is) {
    VString wasName = (was == NULL) ? "null" : was->getName();
    VString isName = is->getName();
    VString msg(VSTRING_ARGS("_reportLoggerChange %s %s: was '%s', is '%s'", (before ? "before" : "after"), label.chars(), wasName.chars(), isName.chars()));
    std::cout << msg << std::endl;
}
#endif /* VLOGGER_INTERNAL_DEBUGGING */

// static
void VLogger::setDefaultLogger(VNamedLoggerPtr namedLogger) {
    VMutexLocker locker(_mutexInstance(), "VLogger::getDefaultLogger");
    VLogger::_reportLoggerChange(true, "setDefaultLogger", gDefaultLogger, namedLogger);
    gDefaultLogger = namedLogger;
    VLogger::_reportLoggerChange(false, "setDefaultLogger", gDefaultLogger, namedLogger);
}

// static
VNamedLoggerPtr VLogger::getLogger(const VString& name) {
    VNamedLoggerPtr logger = findNamedLogger(name);

    if (logger == NULL) {
        logger = VLogger::getDefaultLogger();
    }

    return logger;
}

// static
VNamedLoggerPtr VLogger::findDefaultLogger() {
    VMutexLocker locker(_mutexInstance(), "VLogger::findDefaultLogger");
    return gDefaultLogger;
}

// static
VNamedLoggerPtr VLogger::findDefaultLoggerForLevel(int level) {
    VMutexLocker locker(_mutexInstance(), "VLogger::findDefaultLoggerForLevel");

    if (gDefaultLogger == NULL) {
        VLogger::_registerLogger(VNamedLoggerPtr(new VNamedLogger("default", VLoggerLevel::INFO, VStringVector())), true);
    }

    return gDefaultLogger->isEnabledFor(level) ? gDefaultLogger : NULL_NAMED_LOGGER_PTR;
}

// static
VNamedLoggerPtr VLogger::findNamedLogger(const VString& name) {
    VMutexLocker locker(_mutexInstance(), "VLogger::findNamedLogger");
    return VLogger::_findNamedLoggerFromPathName(name);
}

// static
VNamedLoggerPtr VLogger::findNamedLoggerForLevel(const VString& name, int level) {
    // Fast as possible short-circuit: If no logger is enabled at the level (global int test), further searching is not necessary.
    if (! VLogger::isLogLevelActive(level)) {
        return NULL_NAMED_LOGGER_PTR;
    }

    VNamedLoggerPtr logger = VLogger::findNamedLogger(name);

    // If found but level is too high, return null so it won't log.
    if ((logger != NULL) && (logger->getLevel() < level)) {
        return NULL_NAMED_LOGGER_PTR;
    }

    // If not found, get the default logger with a level check (returns null if level is too high).
    if (logger == NULL) {
        logger = VLogger::findDefaultLoggerForLevel(level);
    }

    return logger;
}

#ifdef VLOGGER_INTERNAL_DEBUGGING
// static
void VLogger::_reportAppenderChange(bool before, const VString& label, const VLogAppenderPtr& was, const VLogAppenderPtr& is) {
    VString wasName = (was == NULL) ? "null" : was->getName();
    VString isName = is->getName();
    VString msg(VSTRING_ARGS("_reportAppenderChange %s %s: was '%s', is '%s'", (before ? "before" : "after"), label.chars(), wasName.chars(), isName.chars()));
    std::cout << msg << std::endl;
}
#endif /* VLOGGER_INTERNAL_DEBUGGING */

// static
VLogAppenderPtr VLogger::getDefaultAppender() {
    VMutexLocker locker(_mutexInstance(), "VLogger::getDefaultAppender");

    if (gDefaultAppender == NULL) {
        VLogger::_registerAppender(VLogAppenderPtr(new VCoutLogAppender("auto-default-cout-appender", VLogAppender::DO_FORMAT_OUTPUT)), true);
    }

    return gDefaultAppender;
}

// static
VLogAppenderPtr VLogger::getAppender(const VString& appenderName) {
    /* locker scope */ {
        VMutexLocker locker(_mutexInstance(), "VLogger::getAppender");
        VLogAppendersMap::const_iterator pos = _getAppendersMap().find(appenderName);
        if (pos != _getAppendersMap().end()) {
            return pos->second;
        }
    }

    // Doesn't exist. Return default appender.
    return VLogger::getDefaultAppender();
}

// static
VLogAppenderPtrList VLogger::getAllAppenders() {
    VLogAppenderPtrList result;

    /* locker scope */ {
        VMutexLocker locker(_mutexInstance(), "VLogger::getAllAppenders");

        for (VLogAppendersMap::const_iterator i = _getAppendersMap().begin(); i != _getAppendersMap().end(); ++i) {
            result.push_back((*i).second);
        }

        for (VLogAppendersMap::const_iterator i = _getGlobalAppendersMap().begin(); i != _getGlobalAppendersMap().end(); ++i) {
            result.push_back((*i).second);
        }
    }

    return result;
}

// static
VLogAppenderPtr VLogger::findDefaultAppender() {
    VMutexLocker locker(_mutexInstance(), "VLogger::findDefaultAppender");
    return gDefaultAppender;
}

// static
VLogAppenderPtr VLogger::findAppender(const VString& name) {
    VLogAppendersMap::iterator pos = _getAppendersMap().find(name);
    if (pos != _getAppendersMap().end()) {
        return pos->second;
    }

    return NULL_LOG_APPENDER_PTR;
}

// static
const VFSNode& VLogger::getBaseLogDirectory() {
    return gBaseLogDirectory;
}

// static
VBentoNode* VLogger::commandGetInfo() {
    VMutexLocker locker(_mutexInstance(), "VLogger::commandGetInfo");
    return VLogger::_commandGetInfo();
}

// static
VBentoNode* VLogger::_commandGetInfo() {
    VBentoNode* rootNode = new VBentoNode("logger-info");
    VBentoNode* factoriesNode = rootNode->addNewChildNode("factories");
    VBentoNode* appendersNode = rootNode->addNewChildNode("appenders");
    VBentoNode* loggersNode = rootNode->addNewChildNode("loggers");

    rootNode->addInt("max-active-log-level", gMaxActiveLevel);

    const VLogAppenderFactoriesMap& factories = _getAppenderFactoriesMap();
    for (VLogAppenderFactoriesMap::const_iterator i = factories.begin(); i != factories.end(); ++i) {
        VBentoNode* factoryNode = factoriesNode->addNewChildNode("factory");
        (*i).second->addInfo(*factoryNode);
    }

    const VLogAppendersMap& appenders = _getAppendersMap();
    for (VLogAppendersMap::const_iterator i = appenders.begin(); i != appenders.end(); ++i) {
        VBentoNode* appenderNode = appendersNode->addNewChildNode("appender");
        (*i).second->addInfo(*appenderNode);
    }

    const VNamedLoggerMap& loggers = _getLoggerMap();
    for (VNamedLoggerMap::const_iterator i = loggers.begin(); i != loggers.end(); ++i) {
        VBentoNode* loggerNode = loggersNode->addNewChildNode("logger");
        (*i).second->addInfo(*loggerNode);
    }

    return rootNode;
}

// static
VString VLogger::commandGetInfoString() {
    VMutexLocker locker(_mutexInstance(), "VLogger::commandGetInfoString");
    return VLogger::_commandGetInfoString();
}

// static
VString VLogger::_commandGetInfoString() {
    boost::scoped_ptr<VBentoNode> bento(VLogger::_commandGetInfo());
    VString s;
    bento->writeToBentoTextString(s, true);
    return s;
}

// static
void VLogger::commandSetLogLevel(const VString& loggerName, int level) {
    
    std::vector<VNamedLoggerPtr> targetLoggers;

    // First, get all the desired loggers, with required locking in place.
    /* locker scope */ {
        VMutexLocker locker(_mutexInstance(), "VLogger::commandSetLogLevel()");
        const VNamedLoggerMap& loggers = _getLoggerMap();
        for (VNamedLoggerMap::const_iterator i = loggers.begin(); i != loggers.end(); ++i) {
            VNamedLoggerPtr logger = (*i).second;
            if (loggerName.isEmpty() || (logger->getName() == loggerName)) {
                targetLoggers.push_back(logger);
            }
        }
    }
    
    // Now, set each logger's level with the public API. It locks each time.
    for (std::vector<VNamedLoggerPtr>::const_iterator i = targetLoggers.begin(); i != targetLoggers.end(); ++i) {
        (*i)->setLevel(level);
    }
}

// static
void VLogger::commandSetPrintStackLevel(const VString& loggerName, int printStackLevel, int count, const VDuration& timeLimit) {
    VMutexLocker locker(_mutexInstance(), "VLogger::commandSetLogLevel()");
    const VNamedLoggerMap& loggers = _getLoggerMap();
    for (VNamedLoggerMap::const_iterator i = loggers.begin(); i != loggers.end(); ++i) {
        VNamedLoggerPtr logger = (*i).second;
        if (loggerName.isEmpty() || (logger->getName() == loggerName)) {
            logger->setPrintStackInfo(printStackLevel, count, timeLimit);
        }
    }
}

// static
void VLogger::emitToGlobalAppenders(int level, const char* file, int line, bool emitMessage, const VString& message, bool emitRawLine, const VString& rawLine) {
    VMutexLocker locker(_mutexInstance(), "VLogger::emitToGlobalAppenders");
    for (VLogAppendersMap::const_iterator i = _getGlobalAppendersMap().begin(); i != _getGlobalAppendersMap().end(); ++i) {
        VLogAppenderPtr appender = (*i).second;
        appender->emit(level, file, line, emitMessage, message, emitRawLine, rawLine);
    }
}

// static
void VLogger::_registerAppender(VLogAppenderPtr appender, bool asDefaultAppender, bool asGlobalAppender) {
    // ASSUMES CALLER HOLDS _mutexInstance().

    VLogger::_reportAppenderChange(true, "_registerAppender", gDefaultAppender, appender);

    if (asDefaultAppender || (gDefaultAppender == NULL)) {
        gDefaultAppender = appender;
    }

    _getAppendersMap()[appender->getName()] = appender;

    if (asGlobalAppender) {
        _getGlobalAppendersMap()[appender->getName()] = appender;
    }

    VLogger::_reportAppenderChange(false, "_registerAppender", gDefaultAppender, appender);
}


// static
void VLogger::_registerLogger(VNamedLoggerPtr namedLogger, bool asDefaultLogger) {
    // ASSUMES CALLER HOLDS _mutexInstance().

    VLogger::_reportLoggerChange(true, "_registerLogger", gDefaultLogger, namedLogger);

    if (asDefaultLogger || (gDefaultLogger == NULL)) {
        gDefaultLogger = namedLogger;
    }

    _getLoggerMap()[namedLogger->getName()] = namedLogger;

    VLogger::_checkMaxActiveLogLevelForNewLogger(namedLogger->getLevel());

    VLogger::_reportLoggerChange(false, "_registerLogger", gDefaultLogger, namedLogger);
}

// static
void VLogger::_checkMaxActiveLogLevelForNewLogger(int newActiveLevel) {
    // ASSUMES CALLER HOLDS _mutexInstance().

    // If the logger has a higher level, then its level is the new max.
    if (newActiveLevel > gMaxActiveLevel) {
        gMaxActiveLevel = newActiveLevel;
    }
}

// static
void VLogger::checkMaxActiveLogLevelForRemovedLogger(int removedActiveLevel) {
    VMutexLocker locker(_mutexInstance(), "checkMaxActiveLogLevelForRemovedLogger");
    _checkMaxActiveLogLevelForRemovedLogger(removedActiveLevel);
}

// static
void VLogger::_checkMaxActiveLogLevelForRemovedLogger(int removedActiveLevel) {
    // ASSUMES CALLER HOLDS _mutexInstance().

    // If the logger had the highest level, we need to search to find the new max.
    if (removedActiveLevel >= gMaxActiveLevel) {
        VLogger::_recalculateMaxActiveLogLevel();
    }
}

// static
void VLogger::checkMaxActiveLogLevelForChangedLogger(int oldActiveLevel, int newActiveLevel) {
    VMutexLocker locker(_mutexInstance(), "checkMaxActiveLogLevelForChangedLogger");
    _checkMaxActiveLogLevelForChangedLogger(oldActiveLevel, newActiveLevel);
}

// static
void VLogger::_checkMaxActiveLogLevelForChangedLogger(int oldActiveLevel, int newActiveLevel) {
    // ASSUMES CALLER HOLDS _mutexInstance().

    // If the logger's new level is higher than current max, then its level is the new max.
    // Otherwise, if the old level was the max, and the new level is lower than it, we need to search to find the new max.
    if (newActiveLevel > gMaxActiveLevel) {
        gMaxActiveLevel = newActiveLevel;
    } else if ((oldActiveLevel >= gMaxActiveLevel) && (newActiveLevel < gMaxActiveLevel)) {
        VLogger::_recalculateMaxActiveLogLevel();
    }
}

// static
void VLogger::_recalculateMaxActiveLogLevel() {
    // ASSUMES CALLER HOLDS _mutexInstance().

    // This value is less than previous max. Scan all loggers to see what the new max is.
    int newMax = 0;
    for (VNamedLoggerMap::const_iterator i = _getLoggerMap().begin(); i != _getLoggerMap().end(); ++i) {
        newMax = V_MAX(newMax, (*i).second->getLevel());
    }

    gMaxActiveLevel = newMax;
}

// static
VNamedLoggerPtr VLogger::_findNamedLoggerFromExactName(const VString& name) {
    VNamedLoggerMap::const_iterator pos = _getLoggerMap().find(name);
    if (pos == _getLoggerMap().end()) {
        return NULL_NAMED_LOGGER_PTR;
    }

    return pos->second;
}

// static
VNamedLoggerPtr VLogger::_findNamedLoggerFromPathName(const VString& pathName) {
    VString nextNameToSearch(pathName);

    while (nextNameToSearch.contains('.')) {
        VNamedLoggerPtr foundLogger = VLogger::_findNamedLoggerFromExactName(nextNameToSearch);
        if (foundLogger != NULL) {
            return foundLogger;
        }

        nextNameToSearch.substringInPlace(0, nextNameToSearch.lastIndexOf('.'));
    }

    return VLogger::_findNamedLoggerFromExactName(nextNameToSearch);
}

// VLogAppender ------------------------------------------------------

VLogAppender::VLogAppender(const VString& name, bool formatOutput)
    : mMutex(VSTRING_FORMAT("VLogAppender(%s)", name.chars()), true/*this mutex itself must not log*/)
    , mName(name)
    , mFormatOutput(formatOutput)
    {
}

VLogAppender::VLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults)
    : mMutex(VSTRING_FORMAT("VLogAppender(%s)", settings.getString("name").chars()), true/*this mutex itself must not log*/)
    , mName(settings.getString("name"))
    , mFormatOutput(VLogAppender::_getBooleanInitSetting("format-output", settings, defaults, DO_FORMAT_OUTPUT))
    {
}

VLogAppender::~VLogAppender() {
}

void VLogAppender::addInfo(VBentoNode& infoNode) const {
    infoNode.addString("name", mName);

    if (this->isDefaultAppender()) {
        infoNode.addBool("is-default-appender", true);
    }

    if (!mFormatOutput) {
        infoNode.addBool("format-output", DONT_FORMAT_OUTPUT);
    }
}

void VLogAppender::emit(int level, const char* file, int line, bool emitMessage, const VString& message, bool emitRawLine, const VString& rawLine) {
    VLogAppender::_breakpointLocationForEmit();

    VMutexLocker locker(&mMutex, "emit"); // ensure multiple threads don't intertwine lines of a single emission

    if (emitMessage) {
        this->_emitMessage(level, file, line, message);
    }

    if (emitRawLine) {
        this->_emitRawLine(rawLine);
    }
}

void VLogAppender::emitRaw(const VString& message) {
    this->emit(VLoggerLevel::TRACE, NULL, 0, false, VString::EMPTY(), true, message);
}

bool VLogAppender::isDefaultAppender() const {
    return VLogger::gDefaultAppender.get() == this;
}

// static
bool VLogAppender::_getBooleanInitSetting(const VString& attributePath, const VSettingsNode& settings, const VSettingsNode& defaults, bool defaultValue) {
    return settings.getBoolean(attributePath, defaults.getBoolean(attributePath, defaultValue));
}

// static
int VLogAppender::_getIntInitSetting(const VString& attributePath, const VSettingsNode& settings, const VSettingsNode& defaults, int defaultValue) {
    return settings.getInt(attributePath, defaults.getInt(attributePath, defaultValue));
}

// static
VString VLogAppender::_getStringInitSetting(const VString& attributePath, const VSettingsNode& settings, const VSettingsNode& defaults, const VString& defaultValue) {
    return settings.getString(attributePath, defaults.getString(attributePath, defaultValue));
}

void VLogAppender::_emitMessage(int level, const char* file, int line, const VString& message) {
    if (mFormatOutput) {
        VString formattedMessage = this->_formatMessage(level, file, line, message);
        this->_emitRawLine(formattedMessage);
    } else {
        this->_emitRawLine(message); // directly, without applying formatting
    }
}

VString VLogAppender::_formatMessage(int level, const char* file, int line, const VString& message) {
    VInstant    now;
    VString     timeStampString;
    now.getLocalLogString(timeStampString);

    // If we are running in simulated time, display both the current and simulated time.
    if ((VInstant::getSimulatedClockOffset() != VDuration::ZERO()) || VInstant::isTimeFrozen()) {
        now.setTrueNow();
        VString     trueTimeStampString;
        now.getLocalString(trueTimeStampString);

        timeStampString = trueTimeStampString + " " + timeStampString;
    }

    VString levelName = VLoggerLevel::getName(level);
    const VString threadName = VThread::getCurrentThreadName();

    // If there's file/line number info, then always show it.
    if (file == NULL) {
        return VSTRING_FORMAT("%s %s | %s | %s", timeStampString.chars(), levelName.chars(), threadName.chars(), message.chars());
    } else {
        return VSTRING_FORMAT("%s %s | %s | @ %s:%d: %s", timeStampString.chars(), levelName.chars(), threadName.chars(), file, line, message.chars());
    }
}

VString VLogAppender::_toString() const {
    return VSTRING_FORMAT("VLogAppender '%s'", mName.chars());
}

// static
void VLogAppender::_breakpointLocationForEmit() {
    // Put a breakpoint here if you want to break on every message that is actually
    // emitted to an appender after the log level filtering and routing has passed.
}

// VCoutLogAppender ----------------------------------------------------------

VCoutLogAppender::VCoutLogAppender(const VString& name, bool formatOutput)
    : VLogAppender(name, formatOutput)
    {
    std::cout << std::endl;
}

VCoutLogAppender::VCoutLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults)
    : VLogAppender(settings, defaults)
    {
    std::cout << std::endl;
}

void VCoutLogAppender::addInfo(VBentoNode& infoNode) const {
    VLogAppender::addInfo(infoNode);
    infoNode.addString("type", "VCoutLogAppender");
}

void VCoutLogAppender::_emitRawLine(const VString& line) {
    std::cout << line.chars() << std::endl;
    (void) ::fflush(stdout);
}

// VFileLogAppender ----------------------------------------------------------

VFileLogAppender::VFileLogAppender(const VString& name, bool formatOutput, const VString& filePath)
    : VLogAppender(name, formatOutput)
    , mFileStream(VFSNode(filePath))
    , mOutputStream(mFileStream)
    {
    this->_openFile();
}

VFileLogAppender::VFileLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults)
    : VLogAppender(settings, defaults)
    , mFileStream()
    , mOutputStream(mFileStream)
    {
    // If no path is specified, we'll use "<appendername>.log" in the base log directory, simply
    // using the appender's "name" property as our base file name.
    VString defaultPath;
    VLogger::getBaseLogDirectory().getChildPath(settings.getString("name") + ".log", defaultPath);
    mFileStream.setNode(VFSNode(_getStringInitSetting("path", settings, defaults, defaultPath)));
    
    this->_openFile();
}

void VFileLogAppender::_openFile() {
    VFSNode newLogFileDir;
    mFileStream.getNode().getParentNode(newLogFileDir);
    newLogFileDir.mkdirs();
    
    mFileStream.openReadWrite();
    mFileStream.seek(CONST_S64(0), SEEK_END);

    mOutputStream.writeLineEnd();
}

void VFileLogAppender::addInfo(VBentoNode& infoNode) const {
    VLogAppender::addInfo(infoNode);
    infoNode.addString("type", "VFileLogAppender");
    infoNode.addString("file", mFileStream.getNode().getPath());
}

void VFileLogAppender::_emitRawLine(const VString& line) {
    mOutputStream.writeLine(line);
    mOutputStream.flush();
}

// VRollingFileLogAppender ---------------------------------------------------

VRollingFileLogAppender::VRollingFileLogAppender(const VString& name, bool formatOutput, const VString& /*dirPath*/, const VString& /*fileNamePrefix*/, int /*maxNumLines*/)
    : VLogAppender(name, formatOutput)
    {
}

VRollingFileLogAppender::VRollingFileLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults)
    : VLogAppender(settings, defaults)
    {
    /* todo: use these settings properties to construct, something like:
        mDirPath = VLogAppender::_getStringInitSetting("dir", settings, defaults, VLogger::getBaseLogDirectory().getPath());
        mPrefix = VLogAppender::_getStringInitSetting("prefix", settings, defaults, settings.getString("name"));
        mMaxLines = VLogAppender::_getIntInitSetting("max-lines", settings, defaults, 10000);
    */
}

void VRollingFileLogAppender::addInfo(VBentoNode& infoNode) const {
    VLogAppender::addInfo(infoNode);
    infoNode.addString("type", "VRollingFileLogAppender");
    // TODO: current file etc.
}

void VRollingFileLogAppender::_emitRawLine(const VString& /*line*/) {
    // TODO!
}

// VSilentLogAppender ----------------------------------------------------------

void VSilentLogAppender::addInfo(VBentoNode& infoNode) const {
    VLogAppender::addInfo(infoNode);
    infoNode.addString("type", "VSilentLogAppender");
}

// VStringLogAppender ----------------------------------------------------------

VStringLogAppender::VStringLogAppender(const VString& name, bool formatOutput)
    : VLogAppender(name, formatOutput)
    , mLines()
    {
}

VStringLogAppender::VStringLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults)
    : VLogAppender(settings, defaults)
    , mLines()
    {
}

void VStringLogAppender::addInfo(VBentoNode& infoNode) const {
    VLogAppender::addInfo(infoNode);
    infoNode.addString("type", "VStringLogAppender");
}

void VStringLogAppender::_emitRawLine(const VString& line) {
    mLines += line;
    mLines += VString::NATIVE_LINE_ENDING();
}

// VStringVectorLogAppender ---------------------------------------------------

VStringVectorLogAppender::VStringVectorLogAppender(const VString& name, bool formatOutput, /*@Nullable*/VStringVector* storage)
    : VLogAppender(name, formatOutput)
    , mStorage(storage)
    , mLines()
    {
    if (mStorage == NULL) { // If not supplied, use our internal lines vector storage.
        mStorage = &mLines;
    }
}

VStringVectorLogAppender::VStringVectorLogAppender(const VSettingsNode& settings, const VSettingsNode& defaults)
    : VLogAppender(settings, defaults)
    , mStorage(NULL)
    , mLines()
    {
    mStorage = &mLines;
}

VStringVectorLogAppender::~VStringVectorLogAppender() {
    mStorage = NULL; // We don't own the pointer.
}

void VStringVectorLogAppender::addInfo(VBentoNode& infoNode) const {
    VLogAppender::addInfo(infoNode);
    infoNode.addString("type", "VStringVectorLogAppender");
    infoNode.addBool("external-storage", mStorage != NULL);
}

void VStringVectorLogAppender::_emitRawLine(const VString& line) {
    mStorage->push_back(line);
}

// VStringLogger -------------------------------------------------------------

VStringLogger::VStringLogger(const VString& name, int level, bool formatOutput)
    : VNamedLogger(name, level, VStringVector())
    , mAppender(name + ".appender", formatOutput)
    {
}

void VStringLogger::addInfo(VBentoNode& infoNode) const {
    VNamedLogger::addInfo(infoNode);
    infoNode.addString("type", "VStringLogger");
}

void VStringLogger::_emitToAppenders(int level, const char* file, int line, bool emitMessage, const VString& message, bool emitRawLine, const VString& rawLine) {
    mAppender.emit(level, file, line, emitMessage, message, emitRawLine, rawLine);
}

// VStringVectorLogger -------------------------------------------------------------

VStringVectorLogger::VStringVectorLogger(const VString& name, int level, /*@Nullable*/VStringVector* storage, bool formatOutput)
    : VNamedLogger(name, level, VStringVector())
    , mAppender(name + ".appender", formatOutput, storage)
    {
}

void VStringVectorLogger::addInfo(VBentoNode& infoNode) const {
    VNamedLogger::addInfo(infoNode);
    infoNode.addString("type", "VStringVectorLogger");
}

void VStringVectorLogger::_emitToAppenders(int level, const char* file, int line, bool emitMessage, const VString& message, bool emitRawLine, const VString& rawLine) {
    mAppender.emit(level, file, line, emitMessage, message, emitRawLine, rawLine);
}

// VLoggerRepetitionFilter ---------------------------------------------------

VLoggerRepetitionFilter::VLoggerRepetitionFilter()
    : mEnabled(true)
    , mHasSavedMessage(false)
    , mNumSuppressedOccurrences(0)
    , mTimeOfLastOccurrence(VInstant::NEVER_OCCURRED())
    , mLevel(0)
    , mFile(NULL)
    , mLine(0)
    , mMessage()
    {
}

void VLoggerRepetitionFilter::reset() {
    mHasSavedMessage = false;
    mNumSuppressedOccurrences = 0;
    mTimeOfLastOccurrence = VInstant::NEVER_OCCURRED();
    mLevel = 0;
    mFile = NULL;
    mLine = 0;
    mMessage = VString::EMPTY();
}

bool VLoggerRepetitionFilter::checkMessage(VNamedLogger& logger, int level, const char* file, int line, const VString& message) {
    if (!mEnabled) {
        return true;
    }

    bool isRepeatMessage = mHasSavedMessage &&
                           (level == mLevel) &&
                           (file == mFile) &&
                           (line == mLine) &&
                           (message == mMessage);

    if (isRepeatMessage) {
        // This is a repeat message. Update our info and return false to indicate that
        // this message should not yet be emitted.
        ++mNumSuppressedOccurrences;
        mTimeOfLastOccurrence.setNow();
    } else {
        // This is not a repeat message. Emit any pending saved recurring message,
        // then reset to store this message, and return true to indicate that
        // this message should be emitted (the first occurrence of a message is
        // always emitted).

        // Emit pending saved message.
        if (mHasSavedMessage && (mNumSuppressedOccurrences > 0)) {
            this->_emitSuppressedMessages(logger);
        }

        // Reset and store this new message.
        mHasSavedMessage = true;
        mNumSuppressedOccurrences = 0;
        mTimeOfLastOccurrence.setNow();
        mLevel = level;
        mFile = file;
        mLine = line;
        mMessage = message;
    }

    return ! isRepeatMessage;
}

void VLoggerRepetitionFilter::checkTimeout(VNamedLogger& logger) {
    if (!mEnabled) {
        return;
    }

    if (mHasSavedMessage && (mNumSuppressedOccurrences > 0)) {
        VInstant now;
        if ((now - mTimeOfLastOccurrence) > VDuration::MINUTE()) {
            this->_emitSuppressedMessages(logger);
        }
    }
}

void VLoggerRepetitionFilter::_emitSuppressedMessages(VNamedLogger& logger) {
    // If there was only 1 suppressed message, no need to mark it.
    if (mNumSuppressedOccurrences > 1) {
        VString tweakedMessage(VSTRING_ARGS("[%dx] %s", mNumSuppressedOccurrences, mMessage.chars()));
        logger._emitToAppenders(mLevel, mFile, mLine, true, tweakedMessage, false, VString::EMPTY());
    } else {
        logger._emitToAppenders(mLevel, mFile, mLine, true, mMessage, false, VString::EMPTY());
    }

    mHasSavedMessage = false;
    mNumSuppressedOccurrences = 0;
}

// VLoggerPrintStackConfig ----------------------------------------------------

VLoggerPrintStackConfig::VLoggerPrintStackConfig()
    : mLevel(VLoggerLevel::OFF)
    , mMaxCount(-1)
    , mDuration(VDuration::POSITIVE_INFINITY())
    , mCountdown(-1)
    , mExpiration(VInstant::INFINITE_FUTURE())
    {
}

void VLoggerPrintStackConfig::configure(int level, int maxNumOccurrences, const VDuration& timeLimit) {
    mLevel = level;
    mMaxCount = (maxNumOccurrences > 0) ? maxNumOccurrences : -1; // 0 really means off (-1)
    mDuration = timeLimit;
    mCountdown = mMaxCount;

    if (timeLimit.isSpecific()) {
        mExpiration = VInstant(/*now*/) + timeLimit;
    } else {
        mExpiration = VInstant::INFINITE_FUTURE();
    }
}

bool VLoggerPrintStackConfig::shouldPrintStack(int level, VNamedLogger& logger) {
    if (level > mLevel) {
        return false;
    }

    // mCountdown of -1 means no count limit, just a timeout limit, then turn off
    // mExpiration of infinite means no time limit, just a count limit, then turn off
    // if both are set, it means count down to zero, then suppress until timeout is reached
    // if neither is set, we always print the stack crawl for this level

    bool resetCountdown = false;
    bool turnOff = false;
    bool printStack = false;
    VInstant now;

    if (mCountdown == -1) {
        if (mDuration == VDuration::POSITIVE_INFINITY()) {
            // no count limit, no time limit: always print
            printStack = true;
        } else {
            // no count limit, but time limit defined: print if not expired, turn off if expired
            printStack = (now < mExpiration);
            turnOff = !printStack;
        }
    } else if (mCountdown == 0) {
        if (mDuration == VDuration::POSITIVE_INFINITY()) {
            // count limit reached, no time limit: turn off printing completely
            turnOff = true;
        } else {
            // count limit reached, time limit defined: check time limit and reset if expired
            resetCountdown = (now >= mExpiration);
            printStack = resetCountdown;
        }
    } else { // mCountdown > 0
        // count limit exists but not yet reached, so print
        printStack = true;

        if (mDuration == VDuration::POSITIVE_INFINITY()) {
            // no time limit: nothing to do
        } else {
            // time limit defined: reset if expired
            resetCountdown = (now >= mExpiration);
        }
    }

    if (resetCountdown) {
        mExpiration = (now + mDuration); // easier than looping increment from previous to future; good enough

        if (mMaxCount > 0) {
            mCountdown = mMaxCount;
        }
    }

    if (printStack) {
        if (mCountdown > 0) {
            --mCountdown;
        }
    }

    if (turnOff) {
        mLevel = VLoggerLevel::OFF;
        mMaxCount = -1;
        mDuration = VDuration::POSITIVE_INFINITY();
        mCountdown = -1;
        mExpiration = VInstant::INFINITE_FUTURE();
        logger._emitToAppenders(VLoggerLevel::INFO, NULL, 0, true, "Print stack crawl for this logger is auto-disabling now.", false, VString::EMPTY());
    }

    return printStack;
}

