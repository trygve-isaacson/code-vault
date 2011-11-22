/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

#ifndef vexception_h
#define vexception_h

/** @file */

#include "vstring.h"
#include "vlogger.h"
#include "vthread.h"

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
        @param  recordStackTrace if true, will attempt to collect stack trace into mCallStack
        */
        VException(bool recordStackTrace=false);
        /**
        Constructs a copy of another VException. Note that if the mErrorMessage
        is non-null, both objects will share a (const char*) value, which neither
        deletes on destruction since it is owned by the caller.
        @param  other   the exception to copy
        */
        VException(const VException& other, bool recordStackTrace=false);
        /**
        Constructs a VException with error code and static message.
        @param    error            the error code
        @param    errorMessage    a static error message
        */
        VException(int error, const char* errorMessage, bool recordStackTrace=false);
        /**
        Constructs a VException with error code and VString message.
        @param    error        the error code
        @param    errorString    the error message
        */
        VException(int error, const VString& errorString, bool recordStackTrace=false);
        /**
        Constructs a VException with default error code and static message.
        @param    errorMessage    a static error message
        */
        VException(const char* errorMessage, bool recordStackTrace=false);
        /**
        Constructs a VException with default error code and VString message.
        @param    errorString    the error message
        */
        VException(const VString& errorString, bool recordStackTrace=false);
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
        int getError() const;
        
        static const int kGenericError = -1;    ///< The default error code.
        
        /**
        Returns a reference to the string vector containing the stack trace, if collected.
        The vector will be empty if no stack trace was collected. The reference is invalid
        when the exception is destructed.
        @return the stack trace information, if it was collected
        */
        const VStringVector& getStackTrace() const;

        /**
        On Windows, and if V_TRANSLATE_WIN32_STRUCTURED_EXCEPTIONS is enabled, this function
        installs a structured exception handler for the current thread, that will throw a
        VException when the SE translator callback is invoked by the runtime. Normally such
        events (writing to a null pointer, etc.) will just crash or trigger the stack crawl
        fatal log message. This allows you to see those errors as C++ exceptions, if you want to.
        VThread calls this automatically. Your main thread (or your VThread::main) needs to
        call it manually if desired.
        If the build is not SEH enabled (at compile time), this function is a no-op.
        */
        static void installWin32SEHandler();
        /**
        On Windows, and if and if V_TRANSLATE_WIN32_STRUCTURED_EXCEPTIONS is enabled, this function
        lets you disable (or re-enable) the installWin32SEHandler() function, which is normally enabled.
        The purpose of this function is to let you enable SEH at compile time via the
        V_TRANSLATE_WIN32_STRUCTURED_EXCEPTIONS symbol, but then optionally disable it at startup if
        you have some kind of runtime configuration flag to trigger this function.
        Note that this flag only has an effect on installWin32SEHandler() when that function is called;
        it is called by VThread to install the handler when the thread starts. So it is not able to
        disable SEH on a thread that is already running; it is a way to prevent subsequent threads from
        installing the SEH (or the main thread, if you do this before the main thread installs the SEH).
        If the build is not SEH enabled (at compile time), this function is a no-op.
        @param  enabled if false, subsequent calls to installWin32SEHandler() will do nothing; if true,
                            subsequent calls to installWin32SEHandler() will install the SEH if compiled in
        */
        static void enableWin32SEHandler(bool enabled);
        static bool isWin32SEHandlerEnabled();

    private:
    
        /** Asserts if any invariant is broken. */
        void _assertInvariant() const;
        
        /** Called during construction if a stack trace was requested. */
        void _recordStackTrace();

        /** Called by each ctor to make for a convenient place to set a
        breakpoint that will be hit on any exception. */
        static void _breakpointLocation();
        
        int         mError;         ///< The error code.
        VString     mErrorString;   ///< The error string if NOT supplied as const char*.
        const char* mErrorMessage;  ///< The error string if supplied as const char*, else NULL.
        VStringVector mStackTrace;   ///< Optional stack frame info strings.
        
        static bool gWin32SEHEnabled; ///< If false, compile-time-enabled installWin32SEHandler() does nothing at runtime.
    };

/**
    @ingroup vexception
*/

/**
This simple helper class throws an exception with a stack trace included,
without having to pass the boolean as you do with the base class.
*/
class VStackTraceException : public VException
    {
    public:
    
        VStackTraceException(const VString& errorString) :
            VException(errorString, true) {}
        VStackTraceException(int error, const VString& errorString) :
            VException(error, errorString, true) {}
        virtual ~VStackTraceException() throw() {}
    };

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
VSocketClosedException is a VException that indicates that a read or write on
a socket has failed because the socket has closed. The reason for separating
this from a VException or VEOFException is so that the catch block can distinguish
between abnormal conditions (those exceptions) and a simple socket closure (this
exception). An example is VMessageInputThread and VMessageOutputThread, which
log a VSocketClosedException at a very high level, since it doesn't really indicate
an "error" per se, but rather just a condition requiring the VSocket to be cleaned
up and the thread to finish.
*/
class VSocketClosedException : public VException
    {
    public:
    
        /**
        Constructs the exception with default error code and VString message.
        @param    errorString    the error message
        */
        VSocketClosedException(int error, const VString& errorString) : VException(error, errorString) {}
        /**
        Destructor.
        */
        virtual ~VSocketClosedException() throw() {}
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
        VRangeException(const char* errorMessage, bool recordStackTrace=true) : VException(errorMessage, recordStackTrace) {}
        /**
        Constructs the exception with default error code and VString message.
        @param    errorString    the error message
        */
        VRangeException(const VString& errorString, bool recordStackTrace=true) : VException(errorString, recordStackTrace) {}
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
        VUnimplementedException(const char* errorMessage, bool recordStackTrace=true) : VException(errorMessage, recordStackTrace) {}
        /**
        Constructs the exception with default error code and VString message.
        @param    errorString    the error message
        */
        VUnimplementedException(const VString& errorString, bool recordStackTrace=true) : VException(errorString, recordStackTrace) {}
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
        static void throwIfError(OSStatus err, const VString& message, bool recordStackTrace=true) { if (0 != err) throw VOSStatusException(err, message, recordStackTrace); }
        
        /**
        Constructs the exception with the OSStatus value. The value is
        stored in the VException error code.
        @param err the error code
        */
        VOSStatusException(OSStatus err, const VString& message, bool recordStackTrace=true) : VException(static_cast<int>(err), message, recordStackTrace) {}
        /**
        Destructor.
        */
        virtual ~VOSStatusException() throw() {}
    };
    
#endif /* VAULT_CORE_FOUNDATION_SUPPORT */

/**
Function template for performing a dynamic_cast with a try/catch that can log the location
of the bad cast. In some environments, for some garbage pointer values, dynamic_cast will
throw an exception rather than crash. We can diagnose those by logging the point of cast
and even log a stack crawl.
You can call this function template directly, but in most cases it is easier to use the
macros defined below since they supply the __FILE__ and __LINE__ and booleans for you.
Please note: There is a small amount of overhead compared to a plain dynamic_cast because
of the extra function call. The presence of the try/catch block does not seem to have a cost.
Limitation: (Initial testing with VC++) Currently the template definition does not like
being passed an auto_ptr<> nor the get() from an auto_ptr<>; doing so yields a compile error.
@param  obj                 the object being cast
@param  file                (usually via __FILE__) the source file of the call
@param  line                (usually via __LINE__) the line number of the call
@param  rethrowException    normally true; if false, upon exception, null is returned rather
than an exception being re-thrown
@param  logException        if true, upon exception the exception info is logged
@param  logStackCrawl       if true and logException is true, upon the log output is a stack crawl
(unless stack crawl support is not available)
@return the dynamic_cast result, or NULL if the cast threw an exception and rethrowException is false
@throws VException that re-formats any raw exception message by adding the file and line number
*/
template<typename cast_to_type, typename an_object_ptr>
static cast_to_type VcheckedDynamicCast(an_object_ptr& obj, const char* file, int line, bool rethrowException, bool logException, bool logStackCrawl)
    {
    try
        {
        return dynamic_cast<cast_to_type>(obj);
        }
    catch (const std::exception& ex)
        {
        // Avoid message construction overhead if no flags require it.
        if (logException || rethrowException)
            {
            VString message(VSTRING_ARGS("Exception in dynamic_cast operation at %s:%d: '%s'", file, line, ex.what()));

            if (logException)
                {
                if (logStackCrawl)
                    VThread::logStackCrawl(message, VLogger::getDefaultLogger(), false);

                VLogger::getDefaultLogger()->log(VLoggerLevel::ERROR, file, line, message);
                }

            if (rethrowException)
                throw VException(message);
            }

        return NULL;
        }
    }

/// Performs a checked dynamic cast that will propagate any dynamic cast exception after first logging a stack crawl.
#define V_CHECKED_DYNAMIC_CAST(cast_to_type, an_object) VcheckedDynamicCast<cast_to_type>((an_object), __FILE__, __LINE__, true, true, true)
/// Performs a checked dynamic cast that will, upon a dynamic cast exception, log a stack crawl, eat the exception, and return NULL instead.
#define V_CHECKED_DYNAMIC_CAST_NOTHROW(cast_to_type, an_object) VcheckedDynamicCast<cast_to_type>((an_object), __FILE__, __LINE__, false, true, true)

#endif /* vexception_h */
