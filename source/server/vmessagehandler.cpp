/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

#include "vmessagehandler.h"

#include "vexception.h"
#include "vsocketthread.h"
#include "vclientsession.h"

// VMessageHandler ------------------------------------------------------------

VMessageHandlerFactoryMap* VMessageHandler::gFactoryMap = NULL;

// static
VMessageHandler* VMessageHandler::get(VMessage* m, VServer* server, VClientSession* session, VSocketThread* thread)
    {
    VMessageHandlerFactory* factory = (*(VMessageHandler::mapInstance()))[m->getMessageID()];
    
    if (factory == NULL)
        return NULL;
    else
        return factory->createHandler(m, server, session, thread);
    }

// static
void VMessageHandler::registerHandlerFactory(VMessageID messageID, VMessageHandlerFactory* factory)
    {
    (*(VMessageHandler::mapInstance()))[messageID] = factory;
    }

// static
VMessageHandlerFactoryMap* VMessageHandler::mapInstance()
    {
    // We assume that creation occurs during static init, so we don't have to
    // be concerned about multiple threads stepping on each other during create.

    if (gFactoryMap == NULL)
        gFactoryMap = new VMessageHandlerFactoryMap();

    return gFactoryMap;
    }

VMessageHandler::VMessageHandler(const VString& name, VMessage* m, VServer* server, VClientSession* session, VSocketThread* thread, const VMessageFactory* messageFactory, VMutex* mutex) :
mName(name),
mMessage(m),
mServer(server),
mSessionReference(session),
mThread(thread),
mMessageFactory(messageFactory),
mStartTime(/*now*/),
mLocker(mutex, VSTRING_FORMAT("VMessageHandler(%s)", name.chars())),
mUnblockTime(/*now*/), // Note that if we block locking the mutex, mUnblockTime - mStartTime will indicate how long we were blocked here.
mSessionName() // initialized below
    {
    if (session == NULL)
        mSessionName = (mThread == NULL) ? "" : mThread->getName(); // Allow thread to be null for test case or other purposes.
    else
        mSessionName = session->getName();
        
    VLOGGER_MESSAGE_LEVEL(VMessage::kMessageHandlerLifecycleLevel, VSTRING_FORMAT("[%s] %s@0x%08X for message ID=%d constructed.", mSessionName.chars(), mName.chars(), this, (int) m->getMessageID()));
    }

VMessageHandler::~VMessageHandler()
    {
    try
        {
        VLOGGER_MESSAGE_LEVEL(VMessage::kMessageHandlerLifecycleLevel, VSTRING_FORMAT("[%s] %s@0x%08X destructed.", mSessionName.chars(), mName.chars(), this));
        }
    catch (...) {} // prevent exception from propagating
    
    mMessage = NULL;
    mServer = NULL;
    mThread = NULL;
    mMessageFactory = NULL;
    }

void VMessageHandler::releaseMessage()
    {
    VMessage::release(mMessage);
    mMessage = NULL;
    }

VMessage* VMessageHandler::getMessage(VMessageID messageID)
    {
    VMessage* message = mMessageFactory->instantiateNewMessage(messageID);
    return message;
    }

void VMessageHandler::logMessageContentRecord(const VString& details, VNamedLoggerPtr logger) const
    {
    if (logger == NULL)
        logger = this->_getMessageContentRecordLogger();
    
    if (logger != NULL)
        logger->log(VMessage::kMessageContentRecordingLevel, NULL, 0, VSTRING_FORMAT("[%s] %s", mSessionName.chars(), details.chars()));
    }

void VMessageHandler::logMessageContentFields(const VString& details, VNamedLoggerPtr logger) const
    {
    if (logger == NULL)
        logger = this->_getMessageContentFieldsLogger();
    
    if (logger != NULL)
        logger->log(VMessage::kMessageContentFieldsLevel, NULL, 0, VSTRING_FORMAT("[%s] %s", mSessionName.chars(), details.chars()));
    }

void VMessageHandler::logMessageDetailsFields(const VString& details, VNamedLoggerPtr logger) const
    {
    if (logger == NULL)
        logger = VLogger::findNamedLoggerForLevel(VMessage::kMessageLoggerName, VMessage::kMessageTrafficDetailsLevel);
    
    if (logger != NULL)
        logger->log(VMessage::kMessageTrafficDetailsLevel, NULL, 0, VSTRING_FORMAT("[%s] %s", mSessionName.chars(), details.chars()));
    }

void VMessageHandler::logProcessMessageStart() const
    {
    VLOGGER_MESSAGE_LEVEL(VMessage::kMessageHandlerDispatchLevel, VSTRING_FORMAT("[%s] %s start.", mSessionName.chars(), mName.chars()));
    }

void VMessageHandler::logProcessMessageEnd() const
    {
    VDuration elapsed = VInstant(/*now*/) - mStartTime;
    if (mUnblockTime == mStartTime)
        {
        VLOGGER_MESSAGE_LEVEL(VMessage::kMessageHandlerDispatchLevel, VSTRING_FORMAT("[%s] %s end. (Elapsed time: %s)", mSessionName.chars(), mName.chars(), elapsed.getDurationString().chars()));
        }
    else
        {
        // We were evidently blocked for at least 1ms during construction, waiting for the mutex to be released.
        // If the duration of blocked time exceeded a certain amount, emit this at info level so it is even more visible.
        VDuration blockedTime = mUnblockTime - mStartTime;
        VLOGGER_MESSAGE_LEVEL((blockedTime > 25 * VDuration::MILLISECOND()) ? VLoggerLevel::INFO : (int)VMessage::kMessageHandlerDispatchLevel, // strangely, gcc gave linker error w/o int cast
            VSTRING_FORMAT("[%s] %s end. (Elapsed time: %s. Blocked for: %s.)", mSessionName.chars(), mName.chars(), elapsed.getDurationString().chars(), blockedTime.getDurationString().chars()));
        }
    }

void VMessageHandler::_logDetailedDispatch(const VString& dispatchInfo) const
    {
    VLOGGER_MESSAGE_LEVEL(VMessage::kMessageHandlerDetailLevel, VSTRING_FORMAT("[%s] %s", mSessionName.chars(), dispatchInfo.chars()));
    }

void VMessageHandler::_logMessageContentRecord(const VString& contentInfo) const
    {
    VLOGGER_MESSAGE_LEVEL(VMessage::kMessageContentRecordingLevel, VSTRING_FORMAT("[%s] %s", mSessionName.chars(), contentInfo.chars()));
    }

void VMessageHandler::_logMessageContentFields(const VString& contentInfo) const
    {
    VLOGGER_MESSAGE_LEVEL(VMessage::kMessageContentFieldsLevel, VSTRING_FORMAT("[%s] %s", mSessionName.chars(), contentInfo.chars()));
    }

void VMessageHandler::_logMessageContentHexDump(const VString& info, const Vu8* buffer, Vs64 length) const
    {
    VLOGGER_MESSAGE_HEXDUMP(VSTRING_FORMAT("[%s] %s", mSessionName.chars(), info.chars()), buffer, length);
    }

VNamedLoggerPtr VMessageHandler::_getMessageContentRecordLogger() const
    {
    return VLogger::findNamedLoggerForLevel(VMessage::kMessageLoggerName, VMessage::kMessageContentRecordingLevel);
    }

VNamedLoggerPtr VMessageHandler::_getMessageContentFieldsLogger() const
    {
    return VLogger::findNamedLoggerForLevel(VMessage::kMessageLoggerName, VMessage::kMessageContentFieldsLevel);
    }

