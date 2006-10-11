/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

#ifndef vexception_h
#define vexception_h

/** @file */

#include "vstring.h"

/**

    @defgroup vexception Vault Exceptions

    <h3>Overview</h3>
    
    The Vault does not return error codes; instead it always throws
    exceptions of type VException or its
    subclass VEOFException. As is necessary for proper C++ behavior,
    these exceptions are always thrown "by value", and should be caught
    "by reference". For example:
    
    - throw VException("D'oh!");
    
    - catch (VException& ex) { cout << ex.what(); }
    
    When throwing, you can construct a VException with several different constructor
    forms, including hardcoded strings, sprintf-like syntax, etc.,
    with or without integer error codes. For example:
    
    - throw VException("Unable to process command.");
    
    - throw VException("Unable to process command '%%s'.", command);

    - throw VException(kFakeIDErrorCode, "Fake ID detected.");
    
    - throw VException(kFakeIDErrorCode, "Fake ID detected. %%d is not a valid ID number.", id);
    
    When catching, you can get the error message as a (char*), and can
    get the integer error code. For example:
    
    - catch (VException& ex) { cout << "Error number is " << ex.getError(); }
    
    - catch (VException& ex) { cout << "Error string is '" << ex.what() << "'"; }
    
    - catch (VException& ex)\n
        {\n
        VLOG(VLog::kErrorDetailLevel, "Caught error #%%d: '%%s'", ex.getError(), ex.what());\n
        }
    
    <h3>Specialized Exceptions</h3>
    
    VEOFException is thrown by the stream i/o classes when they unexpectedly
    encounter EOF during a read operation. (Of course, socket reads simply
    block if there is no data; an EOF exception would indicate that the socket
    has been closed and there really cannot be any more data to block on.)
    In some cases, you may want to
    use this as a "normal" signal, and in other cases it may mean a problem
    with the data you are reading; it depends on what you are doing.
    
    Just as VEOFException is, you can derive from VException to define your
    own specialized exceptions that might contain additional data or simply be
    separately identifiable in your catch blocks.
    
*/
    
/**
    @ingroup vexception
*/

/**
VException is the base class for all exceptions thrown by the Vault.

A VException contains an error code integer and an error message
strings. Both are optional.

If you need throw an exception due to an out-of-memory situation, you
should only use the VException constructors that take a static message
value (const char*). This ensures that you do not cause the VException
to allocate yet more memory for the VString that would be created otherwise.
VException retains the static message's char pointer and does NOT free or
delete it on destruction.
*/

class VException : public std::exception
    {
    public:
    
        /**
        Constructs a VException with default error code and empty message.
        */
        VException();
        /**
        Constructs a copy of another VException. Note that if the mErrorMessage
        is non-null, both objects will share a (const char*) value, which neither
        deletes on destruction since it is owned by the caller.
        @param  other   the exception to copy
        */
        VException(const VException& other);
        /**
        Constructs a VException with error code and static message.
        @param    error            the error code
        @param    errorMessage    a static error message
        */
        VException(int error, const char* errorMessage);
        /**
        Constructs a VException with error code and vararg message.
        @param    error    the error code
        @param    format    the format string for the vararg parameters
        */
        VException(int error, char* format, ...);
        /**
        Constructs a VException with error code and VString message.
        @param    error        the error code
        @param    errorString    the error message
        */
        VException(int error, const VString& errorString);
        /**
        Constructs a VException with default error code and static message.
        @param    errorMessage    a static error message
        */
        VException(const char* errorMessage);
        /**
        Constructs a VException with default error code and vararg message.
        @param    format    the format string for the vararg parameters
        */
        VException(char* format, ...);
        /**
        Constructs a VException with default error code and VString message.
        @param    errorString    the error message
        */
        VException(const VString& errorString);
        /**
        Destructor. The throw() declaration is required to satisfy the
        base class std::exception definition.
        */
        virtual ~VException() throw();
        
        /**
        Assignment operator. Note that if the mErrorMessage
        is non-null, both objects will share a (const char*) value, which neither
        deletes on destruction since it is owned by the caller.
        @param  other   the exception to copy
        */
        VException& operator=(const VException& other);

        /**
        Override of the base class method for extracting the message.
        The throw() declaration is required to satisfy the base class std::exception
        definition.
        @return the error message
        */
        virtual const char* what() const throw();
        
        /**
        Returns the exception's error code.
        @return    the error code
        */
        int            getError() const;
        
        static const int kGenericError = -1;    ///< The default error code.

    private:
    
        /** Asserts if any invariant is broken. */
        void assertInvariant() const;
        
        /** Called by each ctor to make for a convenient place to set a
        breakpoint that will be hit on any exception. */
        static void _breakpointLocation();

        int            mError;            ///< The error code.
        VString        mErrorString;    ///< The error string if NOT supplied as const char*.
        const char*    mErrorMessage;    ///< The error string if supplied as const char*, else NULL.
        
    };

/**
    @ingroup vexception
*/

/**
VEOFException is a VException that indicates that a stream reader has hit the
end of the stream while reading. Normally this happens when reading past EOF
in a file, or when a socket is closed while there is a blocking read waiting
for data on the socket.
*/
class VEOFException : public VException
    {
    public:
    
        /**
        Constructs the exception with an error message.
        @param    errorMessage    the message
        */
        VEOFException(const char* errorMessage) : VException(errorMessage) {}
        /**
        Constructs the exception with default error code and VString message.
        @param    errorString    the error message
        */
        VEOFException(const VString& errorString) : VException(errorString) {}
        /**
        Destructor.
        */
        virtual ~VEOFException() throw() {}
    };

/**
VRangeException is a VException that indicates that a value is outside the
valid range of data. For example, when you instantiate a VDate or call its
set() function with a bad month or day value, it will throw a VRangeException
rather than asserting.
*/
class VRangeException : public VException
    {
    public:
    
        /**
        Constructs the exception with an error message.
        @param    errorMessage    the message
        */
        VRangeException(const char* errorMessage) : VException(errorMessage) {}
        /**
        Constructs the exception with default error code and VString message.
        @param    errorString    the error message
        */
        VRangeException(const VString& errorString) : VException(errorString) {}
        /**
        Destructor.
        */
        virtual ~VRangeException() throw() {}
    };

/**
VUnimplementedException is a VException that indicates that a feature in the
code is not yet implemented. Hopefully this is simply a work-in-progress.
*/
class VUnimplementedException : public VException
    {
    public:
    
        /**
        Constructs the exception with an error message.
        @param    errorMessage    the message
        */
        VUnimplementedException(const char* errorMessage) : VException(errorMessage) {}
        /**
        Constructs the exception with default error code and VString message.
        @param    errorString    the error message
        */
        VUnimplementedException(const VString& errorString) : VException(errorString) {}
        /**
        Destructor.
        */
        virtual ~VUnimplementedException() throw() {}
    };

#ifdef VAULT_CORE_FOUNDATION_SUPPORT
/**
VOSStatusException is provided for exceptions caused by non-zero OSStatus
values returned by Mac OS API functions.
*/
class VOSStatusException : public VException
    {
    public:
        
        /**
        Throws a VOSStatusException if err is non-zero; the error value is used for the
        VException error code.
        */
        static void throwIfError(OSStatus err, const VString& message) { if (0 != err) throw VOSStatusException(err, message); }
        
        /**
        Constructs the exception with the OSStatus value. The value is
        stored in the VException error code.
        @param err the error code
        */
        VOSStatusException(OSStatus err, const VString& message) : VException(static_cast<int>(err), message) {}
        /**
        Destructor.
        */
        virtual ~VOSStatusException() throw() {}
    };
    
#endif /* VAULT_CORE_FOUNDATION_SUPPORT */

#endif /* vexception_h */
