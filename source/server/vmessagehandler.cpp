/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

#include "vmessagehandler.h"

#include "vexception.h"
#include "vsocketthread.h"
#include "vmessagepool.h"
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

VMessageHandler::VMessageHandler(const VString& name, VMessage* m, VServer* server, VClientSession* session, VSocketThread* thread, VMessagePool* pool, VMutex* mutex) :
mName(name),
mMessage(m),
mServer(server),
mSessionReference(session),
mThread(thread),
mPool(pool),
mStartTime(/*now*/),
mLocker(mutex, VString("VMessageHandler(%s)", name.chars())),
mUnblockTime(/*now*/), // Note that if we block locking the mutex, mUnblockTime - mStartTime will indicate how long we were blocked here.
mSessionName() // initialized below
    {
    if (session == NULL)  // 2010.01.07 ranstrom/trygve ARGO-22496  allow NULL mThread. Not realistic, but useful for some debugging.
        mSessionName = (mThread == NULL) ? "" : mThread->getName();
    else
        mSessionName = session->getName();
        
    VLOGGER_MESSAGE_LEVEL(VMessage::kMessageHandlerLifecycleLevel, VString("[%s] %s@0x%08X for message ID=%d constructed.", mSessionName.chars(), mName.chars(), this, (int) m->getMessageID()));
    }

VMessageHandler::~VMessageHandler()
    {
    try
        {
        VLOGGER_MESSAGE_LEVEL(VMessage::kMessageHandlerLifecycleLevel, VString("[%s] %s@0x%08X destructed.", mSessionName.chars(), mName.chars(), this));
        }
    catch (...) {} // prevent exception from propagating
    }

void VMessageHandler::releaseMessage()
    {
    if (mMessage != NULL)
        {
        VMessagePool::releaseMessage(mMessage, mPool);
        mMessage = NULL;
        }
    }

VMessage* VMessageHandler::getMessage(VMessageID messageID)
    {
    if (mPool == NULL)
        {
        VString error("VMessageHandler::getMessage: mPool is null.");
        VLOGGER_MESSAGE_FATAL(error);
        throw VException(error);
        }

    return mPool->get(messageID);
    }

void VMessageHandler::logMessageContentRecord(const VString& details, VLogger* logger) const
    {
    if (logger == NULL)
        logger = this->_getMessageContentRecordLogger();
    
    if (logger != NULL)
        logger->log(VMessage::kMessageContentRecordingLevel, NULL, 0, VString("[%s] %s", mSessionName.chars(), details.chars()));
    }

void VMessageHandler::logMessageContentFields(const VString& details, VLogger* logger) const
    {
    if (logger == NULL)
        logger = this->_getMessageContentFieldsLogger();
    
    if (logger != NULL)
        logger->log(VMessage::kMessageContentFieldsLevel, NULL, 0, VString("[%s] %s", mSessionName.chars(), details.chars()));
    }

void VMessageHandler::logProcessMessageStart() const
    {
    VLOGGER_MESSAGE_LEVEL(VMessage::kMessageHandlerDispatchLevel, VString("[%s] %s start.", mSessionName.chars(), mName.chars()));
    }

void VMessageHandler::logProcessMessageEnd() const
    {
    VDuration elapsed = VInstant(/*now*/) - mStartTime;
    if (mUnblockTime == mStartTime)
        {
        VLOGGER_MESSAGE_LEVEL(VMessage::kMessageHandlerDispatchLevel, VString("[%s] %s end. (Elapsed time: %lldms)", mSessionName.chars(), mName.chars(), elapsed.getDurationMilliseconds()));
        }
    else
        {
        // We were evidently blocked for at least 1ms during construction, waiting for the mutex to be released.
        // If the duration of blocked time exceeded a certain amount, emit this at info level so it is even more visible.
        VDuration blockedTime = mUnblockTime - mStartTime;
        VLOGGER_MESSAGE_LEVEL((blockedTime > 25 * VDuration::MILLISECOND()) ? VLogger::kInfo : (int)VMessage::kMessageHandlerDispatchLevel, // strangely, gcc gave linker error w/o int cast
            VString("[%s] %s end. (Elapsed time: %lldms. Blocked for: %lldms.)", mSessionName.chars(), mName.chars(), elapsed.getDurationMilliseconds(), blockedTime.getDurationMilliseconds()));
        }
    }

void VMessageHandler::_logDetailedDispatch(const VString& dispatchInfo) const
    {
    VLOGGER_MESSAGE_LEVEL(VMessage::kMessageHandlerDetailLevel, VString("[%s] %s", mSessionName.chars(), dispatchInfo.chars()));
    }

void VMessageHandler::_logMessageContentRecord(const VString& contentInfo) const
    {
    VLOGGER_MESSAGE_LEVEL(VMessage::kMessageContentRecordingLevel, VString("[%s] %s", mSessionName.chars(), contentInfo.chars()));
    }

void VMessageHandler::_logMessageContentFields(const VString& contentInfo) const
    {
    VLOGGER_MESSAGE_LEVEL(VMessage::kMessageContentFieldsLevel, VString("[%s] %s", mSessionName.chars(), contentInfo.chars()));
    }

void VMessageHandler::_logMessageContentHexDump(const VString& info, const Vu8* buffer, Vs64 length) const
    {
    VLOGGER_MESSAGE_HEXDUMP(VString("[%s] %s", mSessionName.chars(), info.chars()), buffer, length);
    }

VLogger* VMessageHandler::_getMessageContentRecordLogger() const
    {
    VLogger* logger = VLogger::getLogger(VMessage::kMessageLoggerName);
    
    if (logger->isEnabledFor(VMessage::kMessageContentRecordingLevel))
        return logger;
    else
        return NULL;
    }

VLogger* VMessageHandler::_getMessageContentFieldsLogger() const
    {
    VLogger* logger = VLogger::getLogger(VMessage::kMessageLoggerName);
    
    if (logger->isEnabledFor(VMessage::kMessageContentFieldsLevel))
        return logger;
    else
        return NULL;
    }

