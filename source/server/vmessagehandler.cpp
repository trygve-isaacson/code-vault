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
	this->_logDetailedDispatch(VString("VMessageHandler@0x%08X for message ID=%d constructed.", this, (int) m->getMessageID()));
	}

VMessageHandler::~VMessageHandler()
	{
	try
		{
		this->_logDetailedDispatch(VString("VMessageHandler@0x%08X destructed.", this));
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
		VLOGGER_MESSAGE_FATAL(error);
		throw VException(error);
    	}

    return mPool->get(messageID);
	}

void VMessageHandler::_logSimpleDispatch(const VString& dispatchInfo) const
	{
	VLOGGER_MESSAGE_LEVEL(VMessage::kDispatchLifecycleLevel, VString("[%s] %s", (mSession == NULL ? mThread->name().chars() : mSession->getName().chars()), dispatchInfo.chars()));
	}

void VMessageHandler::_logDetailedDispatch(const VString& dispatchInfo) const
	{
	VLOGGER_MESSAGE_LEVEL(VMessage::kDispatchDetailLevel, VString("[%s] %s", (mSession == NULL ? mThread->name().chars() : mSession->getName().chars()), dispatchInfo.chars()));
	}

void VMessageHandler::_logMessageContentInfo(const VString& contentInfo) const
	{
	VLOGGER_MESSAGE_LEVEL(VMessage::kContentInfoLevel, VString("[%s] %s", (mSession == NULL ? mThread->name().chars() : mSession->getName().chars()), contentInfo.chars()));
	}

void VMessageHandler::_logMessageContentHexDump(const VString& info, const Vu8* buffer, Vs64 length) const
	{
	VLOGGER_MESSAGE_HEXDUMP(VString("[%s] %s", (mSession == NULL ? mThread->name().chars() : mSession->getName().chars()), info.chars()), buffer, length);
	}

VLogger* VMessageHandler::_getDetailsLogger() const
	{
	VLogger* logger = VLogger::getLogger(VMessage::kMessageLoggerName);
	
	if (logger->isEnabledFor(VMessage::kContentInfoLevel))
		return logger;
	else
		return NULL;
	}

void VMessageHandler::logMessageDetails(const VString& details, VLogger* logger) const
	{
	if (logger == NULL)
		logger = this->_getDetailsLogger();
	
	if (logger != NULL)
		logger->log(VMessage::kContentInfoLevel, NULL, 0, VString("[%s] %s", (mSession == NULL ? mThread->name().chars() : mSession->getName().chars()), details.chars()));
	}

