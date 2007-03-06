/*
Copyright c1997-2007 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

#include "vmessagehandler.h"

#include "vexception.h"
#include "vsocketthread.h"
#include "vmessagepool.h"

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
	VLOGGER_NAMED_LEVEL(VMessage::kMessageLoggerName, kMessageDispatchLifecycleLogLevel, VString("[%s] VMessageHandler@0x%08X begin.", mThread->name().chars(), this));
	}

VMessageHandler::~VMessageHandler()
	{
	try
		{
		VLOGGER_NAMED_LEVEL(VMessage::kMessageLoggerName, kMessageDispatchLifecycleLogLevel, VString("[%s] VMessageHandler@0x%08X end.", mThread->name().chars(), this));
		}
	catch (...) {} // prevent exception from propagating
	}

void VMessageHandler::releaseMessage()
	{
	if (mMessage != NULL)
		VMessagePool::releaseMessage(mMessage, mPool);
	}

VMessage* VMessageHandler::getMessage(VMessageID messageID)
	{
	if (mPool == NULL)
	    {
		VString error("VMessageHandler::getMessage: mPool is null.");
		VLOGGER_NAMED_ERROR(VMessage::kMessageLoggerName, error);
		throw VException(error);
    	}

    return mPool->get(messageID);
	}

void VMessageHandler::_logSimpleDispatch(const VString& messageHandlerName) const
	{
	VLOGGER_NAMED_LEVEL(VMessage::kMessageLoggerName, kMessageDispatchSimpleLogLevel, VString("[%s] VMessageHandler@0x%08X %s", mThread->name().chars(), this, messageHandlerName.chars()));
	}

VLogger* VMessageHandler::_getDetailsLogger() const
	{
	VLogger*	logger = VLogger::getLogger(VMessage::kMessageLoggerName);
	
	if (logger->isEnabledFor(kMessageDispatchDetailLogLevel))
		return logger;
	else
		return NULL;
	}

void VMessageHandler::logMessageDetails(const VString& details, VLogger* logger) const
	{
	if (logger == NULL)
		logger = this->_getDetailsLogger();
	
	if (logger != NULL)
		logger->log(kMessageDispatchDetailLogLevel, NULL, 0, VString("[%s] %s", mThread->name().chars(), details.chars()));
	}

