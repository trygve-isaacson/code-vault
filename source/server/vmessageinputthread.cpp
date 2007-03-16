/*
Copyright c1997-2007 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

#include "vmessageinputthread.h"

#include "vexception.h"
#include "vlogger.h"
#include "vmessage.h"
#include "vmessagepool.h"
#include "vclientsession.h"
#include "vbento.h"

// VMessageInputThread --------------------------------------------------------

VMessageInputThread::VMessageInputThread(const VString& name, VSocket* socket, VListenerThread* ownerThread, VServer* server, VMessagePool* messagePool)
: VSocketThread(name, socket, ownerThread),
mMessagePool(messagePool),
mSocketStream(socket, "VMessageInputThread"),
mInputStream(mSocketStream),
mConnected(false),
mSession(NULL),
mServer(server)
	{
	}

VMessageInputThread::~VMessageInputThread()
	{
	mMessagePool = NULL;
	mSession = NULL;
	mServer = NULL;
	}

void VMessageInputThread::run()
	{
	/*
	We process messages until we're "done". Done is flagged by the subclass
	marking the thread done when it sees fit. The subclass must catch any
	exceptions that are not catastrophic, because we are the last resort,
	and if we catch an exception we complete the thread, which will cause
	the connection to be shut down (the subclass may need to shut down
	additional resources by overridding run() and post-processing it).
	*/
	
	try
		{
		while (this->isRunning())
			this->_processNextRequest(); // Blocking read on socket; then message is dispatched.
		}
	catch (const VEOFException& /*ex*/)
		{
		// Usually just means the client has closed the connection.
		VLOGGER_MESSAGE_LEVEL(VLogger::kDebug, VString("[%s] VMessageInputThread connection closed.", mName.chars()));
		}
	catch (const VException& ex)
		{
		VLOGGER_MESSAGE_ERROR(VString("[%s] VMessageInputThread exiting due to top level exception #%d '%s'.", mName.chars(), ex.getError(), ex.what()));
		}
	catch (std::exception& ex)
		{
		VLOGGER_MESSAGE_ERROR(VString("[%s] VMessageInputThread exiting due to top level exception '%s'.", mName.chars(), ex.what()));
		}
	catch (...)
		{
		VLOGGER_MESSAGE_ERROR(VString("[%s] VMessageInputThread exiting due to top level unknown exception.", mName.chars()));
		}

	// This thread is done. Print our accumulated stats.

    mMessagePool->printStats();

	if (mSession != NULL)
		mSession->shutdown(this);
	}

void VMessageInputThread::attachSession(VClientSession* session)
	{
	mSession = session;
	}

//lint -e429 "Custodial pointer 'message' has not been freed or returned"
void VMessageInputThread::_processNextRequest()
	{
	VMessage* message = mMessagePool->get();

	/*
	RULES FOR EXCEPTION HANDLING IN REQUEST PROCESSING FUNCTIONS.
	(This text is referenced from the other implementations of
	_processNextRequest().)
	Rules for exception handling here:
	1. Receive may throw to us; we release and re-throw. This situation
		indicates a serious stream error, and we should bail out and
		shut down the socket (re-throwing does this).
	2. Dispatch may NOT throw to us unless the error is serious enough
		to warrant shutting down the socket; the implementation must catch
		all recoverable errors, so that we may continue processing
		subsequent incoming messages, under the assumption that the
		previous message was correctly formatted even if we encountered a
		problem while attempting to handle it.
	*/
	try
		{
		message->receive(mName, mInputStream);
		this->_dispatchMessage(message);
		}
	catch (...)
		{
		// If an exception reached us, we are responsible for releasing the message.
		VMessagePool::releaseMessage(message, mMessagePool);
		throw;
		}
	}

void VMessageInputThread::_dispatchMessage(VMessage* message)
	{
	VMessageHandler* handler = VMessageHandler::get(message, mServer, mSession, this);
	
	if (handler == NULL)
		{
		VLOGGER_MESSAGE_ERROR(VString("[%s] VMessageInputThread::_dispatchMessage: No message hander defined for message %d.", mName.chars(), (int) message->getMessageID()));
        this->_handleNoMessageHandler(message);
		VMessagePool::releaseMessage(message, mMessagePool);
		}
	else
		{
    	VInstant messageHandlerStart;

		/*
		PLEASE SEE COMMENTS IN VMessageInputThread::_processNextRequest() FOR THE
		RULES ON EXCEPTION HANDLING DURING REQUEST PROCESSING.
		*/
		try
			{
            this->_beforeProcessMessage(handler, message);
            this->_callProcessMessage(handler);
            this->_afterProcessMessage(handler);
			}
		catch (const VException& ex)
			{
			VLOGGER_MESSAGE_ERROR(VString("[%s] VMessageInputThread::_dispatchMessage: Caught exception for message %d: #%d %s", mName.chars(), (int) message->getMessageID(), ex.getError(), ex.what()));
			}
		catch (const std::exception& e)
			{
			VLOGGER_MESSAGE_ERROR(VString("[%s] VMessageInputThread::_dispatchMessage: Caught exception for message ID %d: %s", mName.chars(), (int) message->getMessageID(), e.what()));
			}
		catch (...)
			{
			VLOGGER_MESSAGE_ERROR(VString("[%s] VMessageInputThread::_dispatchMessage: Caught unknown exception for message ID %d.", mName.chars(), (int) message->getMessageID()));
			}
		
		handler->releaseMessage();	// handler might not actually release message (processMessage may have re-used it)
		delete handler;
		}
	}

void VMessageInputThread::_callProcessMessage(VMessageHandler* handler)
    {
    handler->processMessage();
    }

// VBentoMessageInputThread ---------------------------------------------------

VBentoMessageInputThread::VBentoMessageInputThread(const VString& name, VSocket* socket, VListenerThread* ownerThread, VServer* server, VMessagePool* messagePool) :
VMessageInputThread(name, socket, ownerThread, server, messagePool)
    {
    }

void VBentoMessageInputThread::_handleNoMessageHandler(VMessage* message)
    {
    VBentoNode responseData("response");
    responseData.addInt("result", -1);
    responseData.addString("error", VString("Invalid message ID %d. No handler defined.", (int) message->getMessageID()));

    VString bentoText;
    responseData.writeToBentoTextString(bentoText);
    VLOGGER_MESSAGE_ERROR(VString("[%s] Error Reply: %s", mName.chars(), bentoText.chars()));

	VMessage* response = mMessagePool->get();
    responseData.writeToStream(*response);
    VBinaryIOStream io(mSocketStream);
    response->send(mName, io);
    VMessagePool::releaseMessage(response, mMessagePool);
    }
    
void VBentoMessageInputThread::_callProcessMessage(VMessageHandler* handler)
    {
    try
        {
        VMessageInputThread::_callProcessMessage(handler);
        }
    catch (const std::exception& ex)
        {
        VBentoNode responseData("response");
        responseData.addInt("result", -1);
        responseData.addString("error", VString("An error occurred processing the message: %s", ex.what()));

        VString bentoText;
        responseData.writeToBentoTextString(bentoText);
        VLOGGER_MESSAGE_ERROR(VString("[%s] Error Reply: %s", mName.chars(), bentoText.chars()));

        VMessage* response = mMessagePool->get();
        responseData.writeToStream(*response);
        VBinaryIOStream io(mSocketStream);
        response->send(mName, io);
        VMessagePool::releaseMessage(response, mMessagePool);
        }
    }

