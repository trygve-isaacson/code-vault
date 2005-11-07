/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

#ifndef vlistenerthread_h
#define vlistenerthread_h

/** @file */

#include "vsocketthread.h"
#include "vsocket.h"
#include "vmutex.h"

class VSocketFactory;
class VSocketThreadFactory;

/**
    @ingroup vsocket vthread
*/

/**
A VListenerThread is a thread whose run() method listens on a socket and
creates a new VSocket for each incoming connection and a VSocketThread object
to manage each such VSocket.

You control the kind of VSocket- and VSocketThread-derived subclass that
is instantiated by passing a factory object for each thing in the
VListenerThread constructor.

Implementing a listener is trivially simple:

1. Define your VSocketThread subclass and override the run() method. In
this method you will probably create a VIOStream based on a VSocketStream
based on the VSocketThread's mSocket. Do reads on the stream to read
requests from the client, and do writes on the stream to write your
responses to the client. When you see that this->isStopped() returns true,
return from your run() method.

2. Define your VSocketThreadFactory subclass and override the createThread()
method to create an instance of your VSocketThread subclass.

3. When you want to shut down the listener, call its stop() method.

That's it!
*/
class VListenerThread : public VThread
    {
    public:
    
        /**
        Constructs the listener thread to listen on a specified port.
        
        If you are using a VManagementInterface to manage your server behavior,
        you can supply it to the VListenerThread so that it can let the
        manager know when the thread starts and ends.
        
        @param    name                a name for the thread, useful for debugging purposes
        @param    deleteSelfAtEnd        @see VThread
        @param    createDetached        @see VThread
        @param    manager                the object that receives notifications for this thread, or NULL
        @param    portNumber            the port number to listen on
        @param    socketFactory        a factory for creating a VSocket for each
                                    incoming connection
        @param    threadFactory        a factory for creating a VSocketThread for
                                    each incoming connection
        @param    initiallyListening    true if the thread should be listening when it first starts;
                                    false means it won't listen until you call startListening()
        */
        VListenerThread(const VString& name, bool deleteSelfAtEnd, bool createDetached, VManagementInterface* manager, int portNumber, VSocketFactory* socketFactory, VSocketThreadFactory* threadFactory, bool initiallyListening=true);
        /**
        Destructor.
        */
        virtual ~VListenerThread();
        
        /**
        Stops the thread; for VListenerThread this also stops listening
        and stops the socket threads (threads running connections established
        from this listener).
        */
        virtual void stop();
        /**
        Run method, listens and then goes into a loop that accepts incoming
        connections until the thread has been externally stopped.
        
        Each new VSocketThread is kept track of internally.
        */
        virtual void    run();

        /**
        Handles bookkeeping upon the termination of a VSocketThread that was previously
        created. The object notifies us of its termination.
        @param    socketThread    the thread that ended
        */
        void socketThreadEnded(VSocketThread* socketThread);
        
        /**
        Returns the port number we're listening on.
        @return    the port number
        */
        int    getPortNumber() const;
        
        /**
        Returns a list of information about all of this listener's
        current socket threads; note that because this information is
        so dynamic, the caller receives a snapshot of the information,
        which may be stale at any moment. For example, by the time
        you look at the information, it may refer to sockets that have
        since been closed.
        */
        VSocketInfoVector enumerateActiveSockets();
        
        /**
        Attempts to stop the specified socket thread that was created by this
        listener. Throws a VException if that socket thread no longer exists.
        These two parameters are used to identify the socket because the sock
        ID can re-used by another socket after a given socket is closed.
        
        @param    sockID            the socket ID
        @param    localPortNumber    the socket's port number
        */
        void stopSocketThread(VSockID sockID, int localPortNumber);

        /**
        Attempts to stop all socket threads that were created by this
        listener.
        */
        void stopAllSocketThreads();
        /**
        Sets the thread to listen if it isn't already. If the thread is
        currently listening, nothing changes. If it's sleeping in non-listening
        mode, next time through the loop it will start listening again.
        */
        void startListening() { mShouldListen = true; }
        /**
        Sets the thread to stop listening if it's currently listening. If the
        thread is not currently listening, nothing changes. If it's running in
        the listen loop, the loop will bail out within a few seconds, and go
        into non-listening mode until a call to startListening() is made.
        */
        void stopListening() { mShouldListen = false; }
        /**
        Returns true if the thread is in listening mode. Note that this doesn't
        mean it's actually listening at this moment, but rather whether it is
        set to listen; during transition phases between listening and not listening,
        this flag may reflect the pending state rather than the current state.
        */
        bool isListening() const { return mShouldListen; }

    protected:
    
        /**
        Performs the run() loop operations needed when we should be listening.
        The run() method calls this when we are listening. So 
        */
        void runListening();
    
        int                        mPortNumber;            ///< The port number we are listening on.
        bool                    mShouldListen;            ///< True if we should be listening; false if we should not. Controls run loops.
        VSocketFactory*            mSocketFactory;            ///< A factory for each incoming connection's VSocket.
        VSocketThreadFactory*    mThreadFactory;            ///< A factory for each incoming connection's VSocketThread.
        VSocketThreadPtrVector    mSocketThreads;            ///< The VSocketThread objects we have created.
        VMutex                    mSocketThreadsMutex;    ///< Mutex to protect our VSocketThread vector.
    };

/**
VListenerThreadPtrVector is simply a vector of VListenerThread object pointers.
*/
typedef std::vector<VListenerThread*> VListenerThreadPtrVector;

#endif /* vlistenerthread_h */
