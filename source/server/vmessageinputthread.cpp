/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

#include "vmessageinputthread.h"

#include "vexception.h"
#include "vlogger.h"
#include "vmessage.h"
#include "vclientsession.h"
#include "vbento.h"

// VMessageInputThread --------------------------------------------------------

VMessageInputThread::VMessageInputThread(const VString& name, VSocket* socket, VListenerThread* ownerThread, VServer* server, const VMessageFactory* messageFactory)
    : VSocketThread(name, socket, ownerThread)
    , mSocketStream(socket, "VMessageInputThread")
    , mInputStream(mSocketStream)
    , mConnected(false)
    , mSessionReference(NULL)
    , mServer(server)
    , mMessageFactory(messageFactory)
    , mHasOutputThread(false)
    {
}

VMessageInputThread::~VMessageInputThread() {
    // If we have a session, it is responsible for deleting the mSocket, not us.
    // This is because a session has input and output threads, the order of whose
    // destruction is unpredictable; so we cannot let our base class delete the mSocket.
    if (mSessionReference.getSession() != NULL) {
        mSocket = NULL;
    }

    mServer = NULL;
    mMessageFactory = NULL;
}

void VMessageInputThread::run() {
    /*
    We process messages until we're "done". Done is flagged by the subclass
    marking the thread done when it sees fit. The subclass must catch any
    exceptions that are not catastrophic, because we are the last resort,
    and if we catch an exception we complete the thread, which will cause
    the connection to be shut down (the subclass may need to shut down
    additional resources by overridding run() and post-processing it).
    Note that in the "error" exceptions below, we don't bother logging if
    we know that the exception is due to expected input thread shutdown,
    recognized by the fact that we are no longer in running state.
    */

    try {
        while (this->isRunning()) {
            this->_processNextRequest(); // Blocking read on socket; then message is dispatched.
        }
    } catch (const VEOFException& /*ex*/) {
        // Usually just means the client has closed the connection.
        VLOGGER_MESSAGE_DEBUG(VSTRING_FORMAT("[%s] VMessageInputThread: Socket has closed (EOF), thread will end.", mName.chars()));
    } catch (const VSocketClosedException& /*ex*/) {
        VLOGGER_MESSAGE_DEBUG(VSTRING_FORMAT("[%s] VMessageInputThread: Socket has closed, thread will end.", mName.chars()));
    } catch (const VException& ex) {
        if (this->isRunning()) {
            VLOGGER_MESSAGE_ERROR(VSTRING_FORMAT("[%s] VMessageInputThread: Exiting due to top level exception #%d '%s'.", mName.chars(), ex.getError(), ex.what()));
        }
    } catch (const std::exception& ex) {
        if (this->isRunning()) {
            VLOGGER_MESSAGE_ERROR(VSTRING_FORMAT("[%s] VMessageInputThread: Exiting due to top level exception '%s'.", mName.chars(), ex.what()));
        }
    } catch (...) {
        if (this->isRunning()) {
            VLOGGER_MESSAGE_ERROR(VSTRING_FORMAT("[%s] VMessageInputThread: Exiting due to top level unknown exception.", mName.chars()));
        }
    }

    if (mSessionReference.getSession() != NULL) {
        mSessionReference.getSession()->shutdown(this);
    }

    // If we are dependent on an output thread, we must spin here until it clears the flag.
    const VDuration warnLimit = 15 * VDuration::SECOND();
    const VInstant startTime;
    bool warned = false;
    while (mHasOutputThread) {
        VThread::sleep(50 * VDuration::MILLISECOND());
        if (!warned) {
            const VInstant now;
            const VDuration duration = now - startTime;
            if (duration > warnLimit) {
                warned = true;
                VLOGGER_MESSAGE_WARN(VSTRING_FORMAT("[%s] VMessageInputThread: Still waiting for output thread to end after %s. Will warn again when output thread ends.", mName.chars(), duration.getDurationString().chars()));
            }
        }
    }

    if (warned) {
        const VInstant now;
        const VDuration duration = now - startTime;
        VLOGGER_MESSAGE_WARN(VSTRING_FORMAT("[%s] VMessageInputThread: Finally saw output thread end after %s.", mName.chars(), duration.getDurationString().chars()));
    }
}

void VMessageInputThread::attachSession(VClientSession* session) {
    mSessionReference.setSession(session);
}

//lint -e429 "Custodial pointer 'message' has not been freed or returned" [OK: try or catch branches guarantee message is released.]
void VMessageInputThread::_processNextRequest() {
    VMessage* message = mMessageFactory->instantiateNewMessage();

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
    try {
        message->receive(mName, mInputStream);
        this->_dispatchMessage(message);
    } catch (...) {
        // If an exception reached us, we are responsible for releasing the message.
        VMessage::release(message);
        throw;
    }
}

void VMessageInputThread::_dispatchMessage(VMessage* message) {
    VMessageHandler* handler = VMessageHandler::get(message, mServer, mSessionReference.getSession(), this);

    if (handler == NULL) {
        VLOGGER_MESSAGE_ERROR(VSTRING_FORMAT("[%s] VMessageInputThread::_dispatchMessage: No message hander defined for message %d.", mName.chars(), (int) message->getMessageID()));
        this->_handleNoMessageHandler(message);
        VMessage::release(message);
    } else {
        VInstant messageHandlerStart;

        /*
        PLEASE SEE COMMENTS IN VMessageInputThread::_processNextRequest() FOR THE
        RULES ON EXCEPTION HANDLING DURING REQUEST PROCESSING.
        */
        try {
            this->_beforeProcessMessage(handler, message);
            this->_callProcessMessage(handler);
            this->_afterProcessMessage(handler);
        } catch (const VException& ex) {
            VLOGGER_MESSAGE_ERROR(VSTRING_FORMAT("[%s] VMessageInputThread::_dispatchMessage: Caught exception for message %d: #%d %s", mName.chars(), (int) message->getMessageID(), ex.getError(), ex.what()));
        } catch (const std::exception& e) {
            VLOGGER_MESSAGE_ERROR(VSTRING_FORMAT("[%s] VMessageInputThread::_dispatchMessage: Caught exception for message ID %d: %s", mName.chars(), (int) message->getMessageID(), e.what()));
        } catch (...) {
            VLOGGER_MESSAGE_ERROR(VSTRING_FORMAT("[%s] VMessageInputThread::_dispatchMessage: Caught unknown exception for message ID %d.", mName.chars(), (int) message->getMessageID()));
        }

        handler->releaseMessage();    // handler might not actually release message (processMessage may have re-used it)
        delete handler;
    }
}

void VMessageInputThread::_callProcessMessage(VMessageHandler* handler) {
    handler->logProcessMessageStart();
    handler->processMessage();
    handler->logProcessMessageEnd();
}

// VBentoMessageInputThread ---------------------------------------------------

VBentoMessageInputThread::VBentoMessageInputThread(const VString& name, VSocket* socket, VListenerThread* ownerThread, VServer* server, const VMessageFactory* messageFactory) :
    VMessageInputThread(name, socket, ownerThread, server, messageFactory) {
}

void VBentoMessageInputThread::_handleNoMessageHandler(VMessage* message) {
    VBentoNode responseData("response");
    responseData.addInt("result", -1);
    responseData.addString("error-message", VSTRING_FORMAT("Invalid message ID %d. No handler defined.", (int) message->getMessageID()));

    VString bentoText;
    responseData.writeToBentoTextString(bentoText);
    VLOGGER_MESSAGE_ERROR(VSTRING_FORMAT("[%s] Error Reply: %s", mName.chars(), bentoText.chars()));

    VMessage* response = mMessageFactory->instantiateNewMessage();
    responseData.writeToStream(*response);
    VBinaryIOStream io(mSocketStream);
    response->send(mName, io);
    VMessage::release(response);
}

void VBentoMessageInputThread::_callProcessMessage(VMessageHandler* handler) {
    try {
        VMessageInputThread::_callProcessMessage(handler);
    } catch (const std::exception& ex) {
        VBentoNode responseData("response");
        responseData.addInt("result", -1);
        responseData.addString("error-message", VSTRING_FORMAT("An error occurred processing the message: %s", ex.what()));

        VString bentoText;
        responseData.writeToBentoTextString(bentoText);
        VLOGGER_MESSAGE_ERROR(VSTRING_FORMAT("[%s] Error Reply: %s", mName.chars(), bentoText.chars()));

        VMessage* response = mMessageFactory->instantiateNewMessage();
        responseData.writeToStream(*response);
        VBinaryIOStream io(mSocketStream);
        response->send(mName, io);
        VMessage::release(response);
    }
}

