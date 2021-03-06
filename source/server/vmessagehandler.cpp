/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

#include "vmessagehandler.h"

#include "vexception.h"
#include "vsocketthread.h"
#include "vclientsession.h"

// VMessageHandler ------------------------------------------------------------

VMessageHandlerFactoryMap* VMessageHandler::gFactoryMap = NULL;

// static
VMessageHandler* VMessageHandler::get(VMessagePtr m, VServer* server, VClientSessionPtr session, VSocketThread* thread) {
    VMessageHandlerFactory* factory = (*(VMessageHandler::mapInstance()))[m->getMessageID()];

    if (factory == NULL)
        return NULL;
    else
        return factory->createHandler(m, server, session, thread);
}

// static
void VMessageHandler::registerHandlerFactory(VMessageID messageID, VMessageHandlerFactory* factory) {
    (*(VMessageHandler::mapInstance()))[messageID] = factory;
}

// static
VMessageHandlerFactoryMap* VMessageHandler::mapInstance() {
    // We assume that creation occurs during static init, so we don't have to
    // be concerned about multiple threads stepping on each other during create.

    if (gFactoryMap == NULL)
        gFactoryMap = new VMessageHandlerFactoryMap();

    return gFactoryMap;
}

VMessageHandler::VMessageHandler(const VString& name, VMessagePtr m, VServer* server, VClientSessionPtr session, VSocketThread* thread, const VMessageFactory* messageFactory, VMutex* mutex)
    : mName(name)
    , mLoggerName(VSTRING_ARGS("vault.messages.VMessageHandler.%d", m->getMessageID()))
    , mMessage(m)
    , mServer(server)
    , mSession(session)
    , mThread(thread)
    , mMessageFactory(messageFactory)
    , mStartTime(/*now*/)
    , mLocker(mutex, VSTRING_FORMAT("VMessageHandler(%s)", name.chars()))
    , mUnblockTime(/*now*/) // Note that if we block locking the mutex, mUnblockTime - mStartTime will indicate how long we were blocked here.
    , mSessionName() // initialized below if session or thread was supplied
    {

    if (session != nullptr) { // A message handler doesn't need to be related to a session object.
        mSessionName = session->getName();
    } else if (mThread != NULL) { // Thread may be null for test case or other purposes.
        mSessionName = mThread->getName();
    }

    VLOGGER_NAMED_LEVEL(mLoggerName, VMessage::kMessageHandlerLifecycleLevel, VSTRING_FORMAT("[%s] %s@0x%08X for message ID=%d constructed.", mSessionName.chars(), mName.chars(), this, (int) m->getMessageID()));
}

VMessageHandler::~VMessageHandler() {
    try {
        VLOGGER_NAMED_LEVEL(mLoggerName, VMessage::kMessageHandlerLifecycleLevel, VSTRING_FORMAT("[%s] %s@0x%08X destructed.", mSessionName.chars(), mName.chars(), this));
    } catch (...) {} // prevent exception from propagating

    mServer = NULL;
    mThread = NULL;
    mMessageFactory = NULL;
}

VMessagePtr VMessageHandler::getMessage(VMessageID messageID) {
    VMessagePtr message = mMessageFactory->instantiateNewMessage(messageID);
    return message;
}

void VMessageHandler::logMessageContentRecord(const VString& details, VNamedLoggerPtr logger) const {
    if (logger == nullptr)
        logger = this->_getMessageContentRecordLogger();

    if (logger != nullptr)
        logger->log(VMessage::kMessageContentRecordingLevel, details);
}

void VMessageHandler::logMessageContentFields(const VString& details, VNamedLoggerPtr logger) const {
    if (logger == nullptr)
        logger = this->_getMessageContentFieldsLogger();

    if (logger != nullptr)
        logger->log(VMessage::kMessageContentFieldsLevel, details);
}

void VMessageHandler::logMessageDetailsFields(const VString& details, VNamedLoggerPtr logger) const {
    if (logger == nullptr)
        logger = VLogger::findNamedLoggerForLevel(mLoggerName, VMessage::kMessageTrafficDetailsLevel);

    if (logger != nullptr)
        logger->log(VMessage::kMessageTrafficDetailsLevel, details);
}

void VMessageHandler::logProcessMessageStart() const {
    VLOGGER_NAMED_LEVEL(mLoggerName, VMessage::kMessageHandlerDispatchLevel, VSTRING_FORMAT("%s start.", mName.chars()));
}

void VMessageHandler::logProcessMessageEnd() const {
    VDuration elapsed = VInstant(/*now*/) - mStartTime;
    if (mUnblockTime == mStartTime) {
        VLOGGER_NAMED_LEVEL(mLoggerName, VMessage::kMessageHandlerDispatchLevel, VSTRING_FORMAT("%s end. (Elapsed time: %s)", mName.chars(), elapsed.getDurationString().chars()));
    } else {
        // We were evidently blocked for at least 1ms during construction, waiting for the mutex to be released.
        // If the duration of blocked time exceeded a certain amount, emit this at info level so it is even more visible.
        VDuration blockedTime = mUnblockTime - mStartTime;
        VLOGGER_NAMED_LEVEL(mLoggerName, (blockedTime > 25 * VDuration::MILLISECOND()) ? VLoggerLevel::INFO : (int)VMessage::kMessageHandlerDispatchLevel, // strangely, gcc gave linker error w/o int cast
                              VSTRING_FORMAT("%s end. (Elapsed time: %s. Blocked for: %s.)", mName.chars(), elapsed.getDurationString().chars(), blockedTime.getDurationString().chars()));
    }
}

void VMessageHandler::_logDetailedDispatch(const VString& dispatchInfo) const {
    VLOGGER_NAMED_LEVEL(mLoggerName, VMessage::kMessageHandlerDetailLevel, dispatchInfo);
}

void VMessageHandler::_logMessageContentRecord(const VString& contentInfo) const {
    VLOGGER_NAMED_LEVEL(mLoggerName, VMessage::kMessageContentRecordingLevel, contentInfo);
}

void VMessageHandler::_logMessageContentFields(const VString& contentInfo) const {
    VLOGGER_NAMED_LEVEL(mLoggerName, VMessage::kMessageContentFieldsLevel, contentInfo);
}

void VMessageHandler::_logMessageContentHexDump(const VString& info, const Vu8* buffer, Vs64 length) const {
    VLOGGER_NAMED_HEXDUMP(mLoggerName, VMessage::kMessageContentHexDumpLevel, info, buffer, length);
}

VNamedLoggerPtr VMessageHandler::_getMessageContentRecordLogger() const {
    return VLogger::findNamedLoggerForLevel(mLoggerName, VMessage::kMessageContentRecordingLevel);
}

VNamedLoggerPtr VMessageHandler::_getMessageContentFieldsLogger() const {
    return VLogger::findNamedLoggerForLevel(mLoggerName, VMessage::kMessageContentFieldsLevel);
}

