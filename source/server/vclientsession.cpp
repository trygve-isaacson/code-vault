/*
Copyright c1997-2007 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

#include "vclientsession.h"

#include "vmutexlocker.h"
#include "vserver.h"
#include "vlogger.h"
#include "vmessage.h"
#include "vmessageinputthread.h"
#include "vmessageoutputthread.h"
#include "vmessagepool.h"
#include "vsocket.h"
#include "vbento.h"

// VClientSession --------------------------------------------------------------

VClientSession::VClientSession(const VString& sessionBaseName, VServer* server, const VString& clientType, VSocket* socket) :
mName(sessionBaseName),
// mMutex -> unlocked
mServer(server),
mClientType(clientType),
// mClientIP -> empty
mClientPort(0),
// mClientAddress -> empty
mInputThread(NULL),
mOutputThread(NULL),
mIsShuttingDown(false),
// mStartupStandbyQueue -> empty
// mTasks -> empty
mSocketStream(socket, "VClientSession"), // FIXME: find a way to get the IP address here or to set in ctor
mIOStream(mSocketStream)
    {
	socket->getHostName(mClientIP);
	mClientPort = socket->getPortNumber();
    mClientAddress.format("%s:%d", mClientIP.chars(), mClientPort);
    mName.format("%s:%s:%d", sessionBaseName.chars(), mClientIP.chars(), mClientPort);
    }

VClientSession::~VClientSession()
    {
    try
        {
        this->_releaseQueuedClientMessages();
        }
    catch (...) {}

    mInputThread = NULL;
    mOutputThread = NULL;
    }
    
void VClientSession::attachTask(const VMessageHandlerTask* task)
    {
    VMutexLocker tasksLocker(&mMutex);
    mTasks.push_back(task);
    }

void VClientSession::detachTask(const VMessageHandlerTask* task)
    {
    VMutexLocker tasksLocker(&mMutex);
    SessionTaskList::iterator position = std::find(mTasks.begin(), mTasks.end(), task);

    if (position != mTasks.end())
        mTasks.erase(position); // we're removing the task from our list, not deleting the task itself
    }

void VClientSession::shutdown(VThread* callingThread)
    {
	VMutexLocker locker(&mMutex);

	mIsShuttingDown = true;

	if (callingThread == NULL)
		VLOGGER_DEBUG(VString("[%s] VClientSession::shutdown: Server requested shutdown of VClientSession@0x%08X.", this->getClientAddress().chars(), this));
	else
		VLOGGER_DEBUG(VString("[%s] VClientSession::shutdown: Thread [%s] requested shutdown of VClientSession@0x%08X.", this->getClientAddress().chars(), callingThread->name().chars(), this));

	if (mInputThread != NULL)
		{
		if (callingThread == mInputThread)
			mInputThread = NULL;
		else
			mInputThread->stop();
		}

	if (mOutputThread != NULL)
		{
		if (callingThread == mOutputThread)
			mOutputThread = NULL;
		else
			mOutputThread->stop();
		}

	if ((mInputThread == NULL) &&
		(mOutputThread == NULL))
		{
        if (mServer != NULL)
            mServer->removeClientSession(this);

		locker.unlock();// otherwise, we violate the "delete this" caveat of the next line
        this->_selfDestruct(); // illegal to refer to "this" after this line of code
		}
    }

bool VClientSession::postOutputMessage(VMessage* message, bool releaseIfNotPosted, bool queueStandbyIfStartingUp)
    {
	bool posted = false;

    VMutexLocker locker(&mMutex); // protect the mStartupStandbyQueue during queue operations

	if ((! mIsShuttingDown) && (! this->isClientGoingOffline())) // don't post if client is doing a disconnect
		{
		if (queueStandbyIfStartingUp && ! this->isClientOnline())
		    {
		    VLOGGER_DEBUG(VString("[%s] SPARCSClientSession::postOutputMessage: Placing message message@0x%08X on standby queue.", this->getClientAddress().chars(), message));
		    mStartupStandbyQueue.postMessage(message);
		    }
		else
		    {
            if (mOutputThread == NULL)
                {
                // Write the message directly to our output stream and release it.
                message->send(mInputThread->name(), mIOStream); // FIXME: better "session label" needed
                VMessagePool::releaseMessage(message, message->getPool());
                }
            else
                {
                // Post it to our async output thread, which will perform the actual i/o.
                mOutputThread->postOutputMessage(message);
                }
    		}

		posted = true;
		}

	if (releaseIfNotPosted && !posted)
		VMessagePool::releaseMessage(message, message->getPool());

	return posted;
    }

void VClientSession::sendMessageToClient(VMessage* message, const VString& sessionLabel, VBinaryIOStream& out)
    {
	VMutexLocker locker(&mMutex); // protect from session state changes while testing state and sending

    if (mIsShuttingDown || this->isClientGoingOffline())
        {
        locker.unlock(); // unlock ASAP, before logging
		VLOGGER_MESSAGE_WARN(VString("VClientSession::sendMessageToClient: NOT sending message@0x%08X to offline session [%s], presumably in process of session shutdown.", message, mClientAddress.chars()));
        }
    else
        {
        VLOGGER_CONDITIONAL_MESSAGE_LEVEL(VMessage::kMessageQueueOpsLevel, VString("[%s] VClientSession::sendMessageToClient: Sending message@0x%08X.", sessionLabel.chars(), message));
        message->send(sessionLabel, out);
        }
    }

VBentoNode* VClientSession::getSessionInfo() const
    {
    VBentoNode* result = new VBentoNode(mName);
    
    result->addString("name", mName);
    result->addString("type", mClientType);
    result->addString("address", mClientAddress);
    result->addString("shutting", mIsShuttingDown ? "yes":"no");
    
    return result;
    }

void VClientSession::_selfDestruct()
    {
    // Wait until any pending tasks finish.
    // Create a scope for the locker.
        {
        VMutexLocker tasksLocker(&mMutex);
        while (! mTasks.empty())
            {
            tasksLocker.unlock();
            VThread::sleep(VDuration::SECOND());
            tasksLocker.lock();
            }
        }
    
    delete this;	// illegal to refer to "this" after this line of code
    }

void VClientSession::_moveStandbyMessagesToAsyncOutputQueue()
    {
    // Note that we rely on the caller to lock the mMutex before calling us.
    // changeInitalizationState calls us but needs to lock a larger scope,
    // so we don't want to do the locking.
	VMessage* m = mStartupStandbyQueue.getNextMessage();

	while (m != NULL)
	    {
	    VLOGGER_DEBUG(VString("[%s] VClientSession::_moveStandbyMessagesToAsyncOutputQueue: Moving message message@0x%08X from standby queue to output queue.", this->getClientAddress().chars(), m));
	    mOutputThread->postOutputMessage(m);
	    m = mStartupStandbyQueue.getNextMessage();
	    }
    }

int VClientSession::_getOutputQueueSize() const
    {
    if (mOutputThread == NULL)
        return 0;
    else
        return mOutputThread->getOutputQueueSize();
    }

void VClientSession::_releaseQueuedClientMessages()
    {
	VMutexLocker locker(&mMutex); // protect the mStartupStandbyQueue during queue operations

	// Order probably does not matter, but it makes sense to pop them in the order they would have been sent.

    if (mOutputThread != NULL)
        mOutputThread->releaseAllQueuedMessages();

	mStartupStandbyQueue.releaseAllMessages();
    }

// VClientSessionFactory -----------------------------------------------------------

void VClientSessionFactory::addSessionToServer(VClientSession* session)
    {
    if (mServer != NULL)
        mServer->addClientSession(session);
    }

