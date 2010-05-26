/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

#ifndef vsocketthreadfactory_h
#define vsocketthreadfactory_h

/** @file */

/* Note that this is an abstract base class with no implementation
besides this header file's inline declarations. */

class VSocket;
class VSocketThread;
class VListenerThread;
class VManagementInterface;

/**
VSocketThreadFactory is an abstract base class that you must override
to create the kind of VSocketThread subclass you want. You simply
override the createThread method. You will typically pass such an
object to a VListenerThread so that it can create your kind of
socket thread to manage an incoming connection.
*/
class VSocketThreadFactory
    {
    public:
    
        VSocketThreadFactory() : mManager(NULL) {}
        /**
        Constructs the factory with the optional management interface that will
        be supplied to each socket thread.
        */
        VSocketThreadFactory(VManagementInterface* manager) : mManager(manager) {}
        /**
        Destructor, declared for completeness.
        */
        virtual ~VSocketThreadFactory() {}
        
        /**
        Sets the management interface to receive notifications. May be NULL if
        no notifications are to be given.
        @param  manager the manager to notify, or NULL
        */
        void setManager(VManagementInterface* manager) { mManager = manager; }
        
        /**
        Creates a VSocketThread object to communicate on the specified socket.
        @param    socket        the socket to initialize the VSocketThread with
        @param    ownerThread    the owner thread to initialize the VSocketThread with
        @return    the new VSocketThread object
        */
        virtual VSocketThread* createThread(VSocket* socket, VListenerThread* ownerThread) = 0;

    protected:
    
        VManagementInterface* mManager; ///< The management interface supplied to each thread.

    private:

        VSocketThreadFactory(const VSocketThreadFactory&); // not copyable
        VSocketThreadFactory& operator=(const VSocketThreadFactory&); // not assignable
    };

#endif /* vsocketthreadfactory_h */
