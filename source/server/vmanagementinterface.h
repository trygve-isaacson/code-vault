/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

#ifndef vmanagementinterface_h
#define vmanagementinterface_h

/** @file */

class VThread;
class VListenerThread;

/**
VManagementInterface defines the interface for a class you can provide
that will be notified as threads come and go, so that you can keep
track of listeners, start them, stop them, kill them, etc. You might
use this to create a management connection listener that processes
messages for managing the server with the above operations.

You pass an instance of VManagementInterface, or NULL, to each VThread
that is created by your factories. This specifies the management
interface object that is notified of that thread's lifecycle.
*/
class VManagementInterface
    {
    public:
    
        /**
        Constructs the interface object.
        */
        VManagementInterface() {}
        /**
        Destructor.
        */
        virtual ~VManagementInterface() {}
        
        /**
        Notifies the interface of a new thread thread whose run() is about
        to be invoked. The concrete class might typically add the thread
        pointer to a vector. The thread pointer is guaranteed to be valid
        until the interface is notified of threadEnded().
        @param    thread    the thread to add to the interface's list
        */
        virtual void threadStarting(VThread* thread) = 0;
        /**
        Notifies the interface of a thread whose run() has just
        reached its end. The concrete class might typically remove the
        thread pointer from a vector. Upon return from this notification,
        the interface must not reference the thread object if the thread's
        mDeleteAtEnd property is set to true, because in that case the
        threadMain function will immediately delete the thread object.
        @param    listenerThread    the thread to remove from the list
        */
        virtual void threadEnded(VThread* thread) = 0;
        
        /**
        Notifies the interface of a new listener thread whose runListening()
        is underway and is about to start listening. The concrete class
        might typically add the thread pointer to a vector. The thread
        pointer is guaranteed to be valid until the interface is notified
        of listenerEnded(). Important note: the management interface will
        also receive a threadStarting() notification for this listener
        thread, because it is also just a thread; that notification will
        occur before the listenerStarting() notification.
        @param    thread    the thread to add to the interface's list
        */
        virtual void listenerStarting(VListenerThread* listener) = 0;
        /**
        Notifies the interface of a listener thread whose runListening() has
        failed. This is typically due to the port being in use, and indicates
        the inability to listen on the port. The concrete class might typically
        decide to stop the listener thread (so it doesn't keep trying to listen)
        and shut down the server (since this may indicate a failure to start
        up properly). This notification will occur between calls to
        listenerStarting() and listenerEnded().
        @param    thread    the thread that failed to listen
        */
        virtual void listenerFailed(VListenerThread* listener, const VString& message) = 0;
        /**
        Notifies the interface of a listener thread whose runListening() has just
        reached its end. The concrete class might typically remove the
        thread pointer from a vector. Upon return from this notification,
        the interface must not reference the thread object if the thread's
        mDeleteAtEnd property is set to true, because in that case the
        threadMain function will immediately delete the thread object.
        Important note: the management interface will
        also receive a threadEnded() notification for this listener
        thread, because it is also just a thread; that notification will
        occur after the listenerStarting() notification.
        @param    listenerThread    the thread to remove from the list
        */
        virtual void listenerEnded(VListenerThread* listener) = 0;
    };

#endif /* vmanagementinterface_h */

