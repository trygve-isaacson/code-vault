/*
Copyright c1997-2007 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

#include "vmessagehandler.h"

#include "vexception.h"
#include "vsocketthread.h"
#include "vmessagepool.h"
#include "vclientsession.h"

// VMessageHandler ------------------------------------------------------------

VMessageHandlerFactoryMap* VMessageHandler::smFactoryMap = NULL;

// static
VMessageHandler* VMessageHandler::get(VMessage* m, VServer* server, VClientSession* session, VSocketThread* thread)
	{
	VMessageHandlerFactory*	factory = (*(VMessageHandler::mapInstance()))[m->getMessageID()];
	
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

	if (smFactoryMap == NULL)
		smFactoryMap = new VMessageHandlerFactoryMap();

	return smFactoryMap;
	}

VMessageHandler::VMessageHandler(VMessage* m, VServer* server, VClientSession* session, VSocketThread* thread, VMessagePool* pool, VMutex* mutex) :
mMessage(m),
mServer(server),
mSession(session),
mThread(thread),
mPool(pool),
mLocker(mutex)
	{
	VLOGGER_MESSAGE_LEVEL(VMessage::kMessageHandlerLifecycleLevel, VString("[%s] VMessageHandler@0x%08X for message ID=%d constructed.", (mSession == NULL ? mThread->name().chars() : mSession->getName().chars()), this, (int) m->getMessageID()));
	}

VMessageHandler::~VMessageHandler()
	{
	try
		{
        VLOGGER_MESSAGE_LEVEL(VMessage::kMessageHandlerLifecycleLevel, VString("[%s] VMessageHandler@0x%08X destructed.", (mSession == NULL ? mThread->name().chars() : mSession->getName().chars()), this));
		}
	catch (...) {} // prevent exception from propagating
	}

void VMessageHandler::releaseMessage()
	{
	if (mMessage != NULL)
		{
		VMessagePool::releaseMessage(mMessage, mPool);
		mMessage = NULL;		// 2007.03.12 JHR ARGO-??? Avoid double-deletions
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
		logger->log(VMessage::kMessageContentRecordingLevel, NULL, 0, VString("[%s] %s", (mSession == NULL ? mThread->name().chars() : mSession->getName().chars()), details.chars()));
	}

void VMessageHandler::logMessageContentFields(const VString& details, VLogger* logger) const
	{
	if (logger == NULL)
		logger = this->_getMessageContentFieldsLogger();
	
	if (logger != NULL)
		logger->log(VMessage::kMessageContentFieldsLevel, NULL, 0, VString("[%s] %s", (mSession == NULL ? mThread->name().chars() : mSession->getName().chars()), details.chars()));
	}

void VMessageHandler::_logSimpleDispatch(const VString& dispatchInfo) const
	{
	VLOGGER_MESSAGE_LEVEL(VMessage::kMessageHandlerDispatchLevel, VString("[%s] %s", (mSession == NULL ? mThread->name().chars() : mSession->getName().chars()), dispatchInfo.chars()));
	}

void VMessageHandler::_logDetailedDispatch(const VString& dispatchInfo) const
	{
	VLOGGER_MESSAGE_LEVEL(VMessage::kMessageHandlerDetailLevel, VString("[%s] %s", (mSession == NULL ? mThread->name().chars() : mSession->getName().chars()), dispatchInfo.chars()));
	}

void VMessageHandler::_logMessageContentRecord(const VString& contentInfo) const
	{
	VLOGGER_MESSAGE_LEVEL(VMessage::kMessageContentRecordingLevel, VString("[%s] %s", (mSession == NULL ? mThread->name().chars() : mSession->getName().chars()), contentInfo.chars()));
	}

void VMessageHandler::_logMessageContentFields(const VString& contentInfo) const
	{
	VLOGGER_MESSAGE_LEVEL(VMessage::kMessageContentFieldsLevel, VString("[%s] %s", (mSession == NULL ? mThread->name().chars() : mSession->getName().chars()), contentInfo.chars()));
	}

void VMessageHandler::_logMessageContentHexDump(const VString& info, const Vu8* buffer, Vs64 length) const
	{
	VLOGGER_MESSAGE_HEXDUMP(VString("[%s] %s", (mSession == NULL ? mThread->name().chars() : mSession->getName().chars()), info.chars()), buffer, length);
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

