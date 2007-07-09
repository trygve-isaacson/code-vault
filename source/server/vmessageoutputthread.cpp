/*
Copyright c1997-2007 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

#include "vmessageoutputthread.h"

#include "vexception.h"
#include "vclientsession.h"
#include "vmessagepool.h"

// VMessageOutputThread -------------------------------------------------------

VMessageOutputThread::VMessageOutputThread(const VString& name, VSocket* socket, VListenerThread* ownerThread, VServer* server, VClientSession* session, VMessagePool* messagePool)
: VSocketThread(name, socket, ownerThread),
mMessagePool(messagePool),
// mOutputQueue -> empty
mSocketStream(socket, "VMessageOutputThread"),
mOutputStream(mSocketStream),
mServer(server),
mSession(session)
	{
	}

VMessageOutputThread::~VMessageOutputThread()
	{
	mOutputQueue.releaseAllMessages();

	/*
	We share the socket w/ the input thread. We sort of let the input
	thread be the owner. So we don't want our superclass to see
	mSocket and clean it up. Just set it to NULL so that the other
	class will be the one to do so.
	*/
	mSocket = NULL;
	mMessagePool = NULL;
	mServer = NULL;
	mSession = NULL;
	}

void VMessageOutputThread::run()
	{
	try
		{
		while (this->isRunning())
			this->_processNextOutboundMessage();
		}
	catch (const VException& ex)
		{
		/*
		Unlike the input threads, we shouldn't normally get an EOF exception to indicate that the
		connection has been closed normally, because we are an output thread. So any exceptions
		that land here uncaught are socket i/o errors and are logged as such.
		*/
		VLOGGER_MESSAGE_ERROR(VString("[%s] VMessageOutputThread::run: Exiting due to top level exception #%d '%s'.", mName.chars(), ex.getError(), ex.what()));
		}
	catch (std::exception& ex)
		{
		VLOGGER_MESSAGE_ERROR(VString("[%s] VMessageOutputThread::run: Exiting due to top level exception '%s'.", mName.chars(), ex.what()));
		}
	catch (...)
		{
		VLOGGER_MESSAGE_ERROR(VString("[%s] VMessageOutputThread::run: Exiting due to top level unknown exception.", mName.chars()));
		}

	if (mSession != NULL)
		mSession->shutdown(this);
	}

void VMessageOutputThread::stop()
    {
    VSocketThread::stop();
    mOutputQueue.wakeUp(); // if it's blocked, this is needed to kick it back to its run loop
    }

void VMessageOutputThread::attachSession(VClientSession* session)
	{
	mSession = session;
	}

void VMessageOutputThread::postOutputMessage(VMessage* message)
    {
    mOutputQueue.postMessage(message);
    }

void VMessageOutputThread::releaseAllQueuedMessages()
    {
    mOutputQueue.releaseAllMessages();
    }

int VMessageOutputThread::getOutputQueueSize() const
    {
    return static_cast<int>(mOutputQueue.getQueueSize());
    }

void VMessageOutputThread::_processNextOutboundMessage()
	{
	VMessage*	message = mOutputQueue.blockUntilNextMessage();

	if (message == NULL)
		{
		// OK -- means we were awakened from block but w/o a message actually available
		}
	else
		{
        if (mSession != NULL)
            mSession->sendMessageToClient(message, mName, mOutputStream);
        else
            {
            // We are just a client. No "session". Just send.
            VLOGGER_CONDITIONAL_MESSAGE_LEVEL(VMessage::kMessageQueueOpsLevel, VString("[%s] VMessageOutputThread::_processNextOutboundMessage: Sending message@0x%08X.", mName.chars(), message));
            message->send(mName, mOutputStream);
            }

		VMessagePool::releaseMessage(message, mMessagePool);
		}
	}

