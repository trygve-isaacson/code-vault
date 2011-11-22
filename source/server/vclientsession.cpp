/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

#include "vclientsession.h"
#include "vtypes_internal.h"

#include "vmutexlocker.h"
#include "vserver.h"
#include "vlogger.h"
#include "vmessage.h"
#include "vmessageinputthread.h"
#include "vmessageoutputthread.h"
#include "vsocket.h"
#include "vbento.h"

// VClientSession --------------------------------------------------------------

VClientSession::VClientSession(const VString& sessionBaseName, VServer* server, const VString& clientType, VSocket* socket, const VDuration& standbyTimeLimit, Vs64 maxQueueDataSize) :
mName(sessionBaseName),
mMutex(VString::EMPTY()/*name will be set in body*/), // -> unlocked
mServer(server),
mClientType(clientType),
mClientIP(), // -> empty
mClientPort(0),
mClientAddress(), // -> empty
mInputThread(NULL),
mOutputThread(NULL),
mIsShuttingDown(false),
mStartupStandbyQueue(), // -> empty
mStandbyStartTime(VInstant::NEVER_OCCURRED()),
mStandbyTimeLimit(standbyTimeLimit),
mMaxClientQueueDataSize(maxQueueDataSize),
mSocket(socket),
mSocketStream(socket, "VClientSession"), // FIXME: find a way to get the IP address here or to set in ctor
mIOStream(mSocketStream),
mReferenceCount(0),
mReferenceCountMutex(VString::EMPTY()/*name will be set in body*/)
    {
    socket->getHostName(mClientIP);
    mClientPort = socket->getPortNumber();
    mClientAddress.format("%s:%d", mClientIP.chars(), mClientPort);
    mName.format("%s:%s:%d", sessionBaseName.chars(), mClientIP.chars(), mClientPort);
    mMutex.setName(VSTRING_FORMAT("VClientSession[%s]::mMutex", mName.chars()));
    mReferenceCountMutex.setName(VSTRING_FORMAT("VClientSession[%s]::mReferenceCountMutex", mName.chars()));

    if (mServer == NULL)
        {
        VString message(VSTRING_ARGS("[%s] VClientSession: No server specified.", this->getClientAddress().chars()));
        VLOGGER_ERROR(message);
        throw VStackTraceException(message);
        }
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

    delete mSocket;
    }

void VClientSession::shutdown(VThread* callingThread)
    {
    VMutexLocker locker(&mMutex, VSTRING_FORMAT("[%s]VClientSession::shutdown() %s", this->getName().chars(), (callingThread == NULL ? "":callingThread->getName().chars())));

    mIsShuttingDown = true;

    if (callingThread == NULL)
        VLOGGER_DEBUG(VSTRING_FORMAT("[%s] VClientSession::shutdown: Server requested shutdown of VClientSession@0x%08X.", this->getName().chars(), this));

    if (mInputThread != NULL)
        {
        if (callingThread == mInputThread)
            {
            mInputThread = NULL;
            VLOGGER_DEBUG(VSTRING_FORMAT("[%s] VClientSession::shutdown: Input Thread [%s] requested shutdown of VClientSession@0x%08X.", this->getName().chars(), callingThread->getName().chars(), this));
            }
        else
            mInputThread->stop();
        }

    if (mOutputThread != NULL)
        {
        if (callingThread == mOutputThread)
            {
            mOutputThread = NULL;
            VLOGGER_DEBUG(VSTRING_FORMAT("[%s] VClientSession::shutdown: Output Thread [%s] requested shutdown of VClientSession@0x%08X.", this->getName().chars(), callingThread->getName().chars(), this));
            }
        else
            mOutputThread->stop();
        }

    // Remove this session from the server's lists of active sessions,
    // so that it can be garbage collected.
    locker.unlock(); // Must release mMutex to avoid possibility of deadlock with a thread that could be posting broadcast right now, which has server lock, needs our lock. removeClientSession may need server lock. Deadlock.
    mServer->removeClientSession(this);
    mServer->clientSessionTerminating(this);
    }

bool VClientSession::postOutputMessage(VMessage* message, bool isForBroadcast)
    {
    if (isForBroadcast && !message->isBeingBroadcast()) // This is a programmer error indicating incorrect API usage.
        {
        VLOGGER_ERROR(VSTRING_FORMAT("[%s] VClientSession::postOutputMessage: Message ID=%d posted for broadcast but not marked for broadcast.", this->getName().chars(), message->getMessageID()));
        return false;
        }

    bool posted = false;

    VMutexLocker locker(&mMutex, VSTRING_FORMAT("[%s]VClientSession::postOutputMessage()", this->getName().chars())); // protect the mStartupStandbyQueue during queue operations

    if ((! mIsShuttingDown) && (! this->isClientGoingOffline())) // don't post if client is doing a disconnect
        {
        if (isForBroadcast && ! this->isClientOnline())
            {
            // This branch is entered only for broadcasting to an offline session.
            // We either post to the session's standby queue, or if we hit a limit we start killing the session.

            VInstant now;

            if (mStandbyStartTime == VInstant::NEVER_OCCURRED())
                {
                mStandbyStartTime = now;
                }
            
            Vs64 currentQueueDataSize = mStartupStandbyQueue.getQueueDataSize();
            if ((mMaxClientQueueDataSize > 0) && (currentQueueDataSize >= mMaxClientQueueDataSize))
                {
                // We have hit the queue size limit. Do not post. Initiate a shutdown of this session.
                VLOGGER_ERROR(VSTRING_FORMAT("[%s] VClientSession::postOutputMessage: Reached output queue limit of " VSTRING_FORMATTER_S64 " bytes. Not posting message ID=%d. Closing socket to force shutdown of session and its i/o threads.", this->getName().chars(), mMaxClientQueueDataSize, message->getMessageID()));
                mSocket->close();
                }
            else if ((mStandbyTimeLimit == VDuration::ZERO()) || (now <= mStandbyStartTime + mStandbyTimeLimit))
                {
                VLOGGER_DEBUG(VSTRING_FORMAT("[%s] VClientSession::postOutputMessage: Placing message ID=%d on standby queue for not-yet-started session.", this->getName().chars(), message->getMessageID()));

                mStartupStandbyQueue.postMessage(message);
                message->addBroadcastTarget();
                posted = true;
                }
            else
                {
                // We have hit the standby time limit. Do not post. Initiate a shutdown of this session.
                VLOGGER_ERROR(VSTRING_FORMAT("[%s] VClientSession::postOutputMessage: Reached standby time limit of %s. Not posting message ID=%d. Closing socket to force shutdown of session and its i/o threads.", this->getName().chars(), mStandbyTimeLimit.getDurationString().chars(), message->getMessageID()));
                mSocket->close();
                }
            }
        else if (isForBroadcast)
            {
            // This branch is entered only for broadcasting to an online session.
            // It is a requirement that sessions supporting broadcast have an async output thread, so null means the output
            // thread has vanished underneath us, presumably because the socket has failed.
            if (mOutputThread == NULL)
                {
                VLOGGER_ERROR(VSTRING_FORMAT("[%s] VClientSession::postOutputMessage: Unable to post message ID=%d to session because output thread has become null. Closing socket to force shutdown of session and its i/o threads.", this->getName().chars(), message->getMessageID()));
                mSocket->close();
                }
            else
                {
                // Post it to our async output thread, which will perform the actual i/o.
                // Note that mOutputThread->postOutputMessage() stops its own thread if posting fails, triggering session end. We don't need to take action.
                posted = mOutputThread->postOutputMessage(message);
                if (posted)
                    message->addBroadcastTarget();
                }
            }
        else if (mOutputThread != NULL)
            {
            // This branch is entered only for non-broadcast when this session has an async output thread.
            // Typical for posting a message directly to 1 session.
            // We need to post to the output thread, but there is no broadcast counting.
            // Note that mOutputThread->postOutputMessage() stops its own thread if posting fails, triggering session end. We don't need to take action.
            posted = mOutputThread->postOutputMessage(message);
            }
        else // not for broadcast, no output thread
            {
            // This branch is entered for non-broadcast synchronous-session posting. Just send on the socket stream.
            // This would only be for sessions that are synchronous and do not use a separate output thread.
            // Write the message directly to our output stream and release it.
            message->send(this->getName(), mIOStream);
            VMessage::deleteMessage(message);
            posted = true;
            }
        }

    // If we are not broadcasting, we are direct posting to single session, and we failed to post, so we must consume/delete the message.
    if (!isForBroadcast && !posted)
        VMessage::deleteMessage(message);

    return posted;
    }

void VClientSession::sendMessageToClient(VMessage* message, const VString& sessionLabel, VBinaryIOStream& out)
    {
    // No longer a need to lock mMutex, because session is ref counted and cannot disappear from under us here.
    if (mIsShuttingDown || this->isClientGoingOffline())
        {
        VLOGGER_MESSAGE_WARN(VSTRING_FORMAT("VClientSession::sendMessageToClient: NOT sending message@0x%08X to offline session [%s], presumably in process of session shutdown.", message, mClientAddress.chars()));
        }
    else
        {
        VLOGGER_MESSAGE_LEVEL(VMessage::kMessageQueueOpsLevel, VSTRING_FORMAT("[%s] VClientSession::sendMessageToClient: Sending message@0x%08X.", sessionLabel.chars(), message));
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

    int standbyQueueSize = (int) mStartupStandbyQueue.getQueueSize();
    if (standbyQueueSize != 0)
        {
        result->addInt("standby-queue-size", (int) mStartupStandbyQueue.getQueueSize());
        result->addS64("standby-queue-data-size", mStartupStandbyQueue.getQueueDataSize());
        }

    if (mOutputThread != NULL)
        {
        result->addInt("output-queue-size", mOutputThread->getOutputQueueSize());
        }

    return result;
    }

void VClientSession::_moveStandbyMessagesToAsyncOutputQueue()
    {
    // Note that we rely on the caller to lock the mMutex before calling us.
    // changeInitalizationState calls us but needs to lock a larger scope,
    // so we don't want to do the locking.
    VMessage* m = mStartupStandbyQueue.getNextMessage();

    while (m != NULL)
        {
        VLOGGER_TRACE(VSTRING_FORMAT("[%s] VClientSession::_moveStandbyMessagesToAsyncOutputQueue: Moving message message@0x%08X from standby queue to output queue.", this->getName().chars(), m));
        this->_postStandbyMessageToAsyncOutputQueue(m);
        m = mStartupStandbyQueue.getNextMessage();
        }

    mStandbyStartTime = VInstant::NEVER_OCCURRED(); // We are no longer in standby queuing mode (until next time we queue).
    }

int VClientSession::_getOutputQueueSize() const
    {
    if (mOutputThread == NULL)
        return 0;
    else
        return mOutputThread->getOutputQueueSize();
    }

void VClientSession::_postStandbyMessageToAsyncOutputQueue(VMessage* message)
    {
    mOutputThread->postOutputMessage(message, false /* do not respect the queue limits, just move all messages onto the queue */);
    }

void VClientSession::_releaseQueuedClientMessages()
    {
    VMutexLocker locker(&mMutex, VSTRING_FORMAT("[%s]VClientSession::_releaseQueuedClientMessages()", this->getName().chars())); // protect the mStartupStandbyQueue during queue operations

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

// VClientSessionReference ---------------------------------------------------------

VClientSessionReference::VClientSessionReference(VClientSession* session) :
mSession(NULL)
    {
    this->setSession(session);
    }

VClientSessionReference::~VClientSessionReference()
    {
    this->setSession(NULL);
    }

void VClientSessionReference::setSession(VClientSession* session)
    {
    // Un-reference the existing session, if any.
    if (mSession != NULL)
        {
        VMutexLocker locker(&mSession->mReferenceCountMutex, VSTRING_FORMAT("VClientSessionReference::setSession() decr %s", mSession->getName().chars()));
        --(mSession->mReferenceCount);
        mSession = NULL;
        }

    // Reference the new session, if supplied.
    if (session != NULL)
        {
        VMutexLocker locker(&session->mReferenceCountMutex, VSTRING_FORMAT("VClientSessionReference::setSession() incr %s", session->getName().chars()));
        ++(session->mReferenceCount);
        mSession = session;
        }
    }

