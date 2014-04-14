/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
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

    <h3>Platform-Specific Error Codes and Messages</h3>

    On Unix-based systems, when a function call fails you are often directed to
    check 'errno' (which is usually a macro for a thread-local stored error value)
    as an error code, and can call strerror(errno) to get a system error message
    string that describes the error code.

    On Windows, the analog of errno and strerror() are GetLastError() (or for WinSock APIs,
    WSAGetLastError()) and a complicated API FormatMessage().

    It's often desirable to throw a VException that contains the error code and system error
    message, along with a string that describes your operation that failed. To facilitate this
    in a cross-platform way, all of the VException classes have constructors that take a
    VSystemError object. You can supply such an object by calling one of two static functions
    of VSystemError -- getSystemError() and getSocketError(). For most errors use the former,
    and for socket API errors use the latter. The distinction is because on Windows there is
    a difference (internally there are two functions for getting error codes, one specific to
    sockets).

    For example, if we fail to open a file successfully, we can write:
        throw VException(VSystemError(), VSTRING_FORMAT("Failed to open file '%s'.", path.chars()));
    Or if we fail on a socket connect() call, we can write:
        VSystemError e = VSystemError::getSocketError();
        vault::closeSocket(socketID);
        throw VException(e, VSTRING_FORMAT("Failed to connect to '%s'.", ipAddress.chars()));

    Note the second example above showing a situation where we need to put the VSystemError object into
    a local variable before throwing the exception, because we first close() the bad socket, and
    the act of calling close() means that presumably close() will succeed and thus set the system
    error code to 0. We need to "stash" the error code that happened on the connect() failure,
    because if we retrieve the socket error code as we construct the exception, we'll be retrieving
    the 0 value that resulted from a successful closeSocket() call.

    The VException will be filled in with the error code, and the error message you supply will have
    the system error message (including the numeric value) appended. For example, in the above, the
    exceptions might contain these error strings:
        "Failed to open file 'foo'. Error 2: File not found."
        "Failed to connect to '1.2.3.4'. Error 51: Network is unreachable."
    The VException numeric error code values would be 2 and 51, respectively.

    This style of building exceptions relieves you of having to worry about how to consistently
    retrieve and format the system level error codes and error messages. Just say what operation
    failed (with details as specific as you need, such as an item name/description and a function
    return code) and pass in the VSystemError object that is relevant.

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
VSystemError captures the current system error code and the corresponding error message text. A
static API is provided to specifically capture the current socket error code. Alternate constructors
let you supply an error code and an error message.

You can supply one of these objects to a VException as a convenient way to get the system
error code and text represented in the exception text, appended in a standard way to the exception
error message text that you provide. This allows more easily throwing exceptions that capture the
system error code and message in a platform-independent way without having to carefully construct
an exception error string that includes all of it explicitly.

Here's precisely what I mean in the terminology here:
- The "current system error code" is simply the current thread's global error code variable. On Unix
it's "errno", and on Windows it's "GetLastError()".
- The "error message text" is simply the system-supplied string that is obtained from a given error
code. On Unix it's "::strerror()", and on Windows it's "::FormatMessage()", supplying the error code.
- The "current socket error code" is not separate from the "current system error code" on Unix, but
on Windows it's "WSAGetLastError()" (WS referring to WinSock).
*/
class VSystemError {

    public:

        /**
        The default constructor captures the current system error code and its error message.
        This is usually sufficient to supply to a thrown VException.
        */
        VSystemError();
        /**
        This constructor lets you supply the error code, and the error message will be obtained
        automatically. This can be useful if an API returns a specific error code and the current
        system error code is not relevant.
        @param  errorCode   an error code to store; the OS will be asked to obtain the relevant
                            error message for that error code
        */
        VSystemError(int errorCode);
        /**
        This constructor lets you supply both the error code and the error message. The OS will
        not be asked to form the error message. An example from the Unix socket implementation is
        ::getaddrinfo() which doesn't use ::strerror() for obtaining an error message.
        @param  errorCode       an error code to store
        @param  errorMessage    an error message to store
        */
        VSystemError(int errorCode, const VString& errorMessage);
        ~VSystemError() {}

        /**
        This static API builds a system error object by getting the current socket error code.
        On Windows current socket error code is separate from the current system error code, so
        for socket-related APIs on any platform you should use this to get a system error object.
        @return a VSystemError containing the current socket error code and message
        */
        static VSystemError getSocketError();

        /**
        Returns the stored error code. The numeric values for error codes are generally very
        platform-specific.
        @return obvious
        */
        int getErrorCode() const { return mErrorCode; }
        /**
        Returns the stored error message. For system and socket error codes, this string will
        normally have been obtained by asking the OS to form a string that corresponds to the
        stored error code.
        @return obvious
        */
        VString getErrorMessage() const { return mErrorMessage; }
        /**
        Returns true if the internal error code is equivalent to the specified POSIX error
        code constant, for the platform. For example, on Windows an interrupted call would
        be the constant WSAEINTR, which is equivalent -- but not the same number -- as the
        POSIX constant EINTR; you could call isPosixError(EINTR) to test for that condition
        on either platform. You generally just throw an exception when you encounter an
        error, but in a few cases you need to decide which corrective action or which specific
        exception to throw, based on the exact error code.
        */
        bool isLikePosixError(int posixErrorCode) const { return this->_isLikePosixError(posixErrorCode); }

    private:

        // Platform-specific implementations for obtaining system and socket error codes and messages.
        // The implementation for these is in each platform's vtypes_platform.cpp file.
        static int _getSystemErrorCode();
        static int _getSocketErrorCode();
        static VString _getSystemErrorMessage(int errorCode);
        
        bool _isLikePosixError(int posixErrorCode) const;

        int     mErrorCode;     ///< The stored error code.
        VString mErrorMessage;  ///< The stored error message.
};

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

class VException : public std::exception {
    public:

        /**
        Constructs a VException with default error code and empty message.
        @param  recordStackTrace if true, will attempt to collect stack trace into mCallStack
        */
        VException(bool recordStackTrace = false);
        /**
        Constructs a copy of another VException. Note that if the mErrorMessage
        is non-null, both objects will share a (const char*) value, which neither
        deletes on destruction since it is owned by the caller.
        @param  other               the exception to copy
        @param  recordStackTrace    if true, will attempt to collect stack trace into mCallStack
        */
        VException(const VException& other, bool recordStackTrace = false);
        /**
        Constructs a VException with error code and static message.
        @param  error               the error code
        @param  errorMessage        a static error message
        @param  recordStackTrace    if true, will attempt to collect stack trace into mCallStack
        */
        VException(int error, const char* errorMessage, bool recordStackTrace = false);
        /**
        Constructs a VException with error code and VString message.
        @param  error               the error code
        @param  errorString         the error message
        @param  recordStackTrace    if true, will attempt to collect stack trace into mCallStack
        */
        VException(int error, const VString& errorString, bool recordStackTrace = false);
        /**
        Constructs a VException with default error code and static message.
        @param  errorMessage        a static error message
        @param  recordStackTrace    if true, will attempt to collect stack trace into mCallStack
        */
        VException(const char* errorMessage, bool recordStackTrace = false);
        /**
        Constructs a VException with default error code and VString message.
        @param  errorString         the error message
        @param  recordStackTrace    if true, will attempt to collect stack trace into mCallStack
        */
        VException(const VString& errorString, bool recordStackTrace = false);
        /**
        Constructs a VException with system error and VString message. The internal error
        message will be formatted from the two.
        @param  error               the error construct
        @param  errorString         the error message
        @param  recordStackTrace    if true, will attempt to collect stack trace into mCallStack
        */
        VException(const VSystemError& error, const VString& errorString, bool recordStackTrace = false);
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
};

/**
    @ingroup vexception
*/

/**
This simple helper class throws an exception with a stack trace included,
without having to pass the boolean as you do with the base class.
*/
class VStackTraceException : public VException {
    public:

        VStackTraceException(const VString& errorString) :
            VException(errorString, true) {}
        VStackTraceException(int error, const VString& errorString) :
            VException(error, errorString, true) {}
        VStackTraceException(const VSystemError& error, const VString& errorString) :
            VException(error, errorString, true) {}
        virtual ~VStackTraceException() throw() {}
};

/**
VEOFException is a VException that indicates that a stream reader has hit the
end of the stream while reading. Normally this happens when reading past EOF
in a file, or when a socket is closed while there is a blocking read waiting
for data on the socket.
*/
class VEOFException : public VException {
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
class VSocketClosedException : public VException {
    public:

        /**
        Constructs the exception with default error code and VString message.
        @param  error       the error code
        @param  errorString the error message
        */
        VSocketClosedException(int error, const VString& errorString) : VException(error, errorString) {}
        /**
        Constructs the exception with default error code and VString message.
        @param  error       the error construct
        @param  errorString the error message
        */
        VSocketClosedException(const VSystemError& error, const VString& errorString) : VException(error, errorString) {}
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
class VRangeException : public VException {
    public:

        /**
        Constructs the exception with an error message.
        @param  errorMessage        the message
        @param  recordStackTrace    if true, will attempt to collect stack trace into mCallStack
        */
        VRangeException(const char* errorMessage, bool recordStackTrace = true) : VException(errorMessage, recordStackTrace) {}
        /**
        Constructs the exception with default error code and VString message.
        @param  errorString         the error message
        @param  recordStackTrace    if true, will attempt to collect stack trace into mCallStack
        */
        VRangeException(const VString& errorString, bool recordStackTrace = true) : VException(errorString, recordStackTrace) {}
        /**
        Destructor.
        */
        virtual ~VRangeException() throw() {}
};

/**
VUnimplementedException is a VException that indicates that a feature in the
code is not yet implemented. Hopefully this is simply a work-in-progress.
*/
class VUnimplementedException : public VException {
    public:

        /**
        Constructs the exception with an error message.
        @param  errorMessage        the message
        @param  recordStackTrace    if true, will attempt to collect stack trace into mCallStack
        */
        VUnimplementedException(const char* errorMessage, bool recordStackTrace = true) : VException(errorMessage, recordStackTrace) {}
        /**
        Constructs the exception with default error code and VString message.
        @param  errorString         the error message
        @param  recordStackTrace    if true, will attempt to collect stack trace into mCallStack
        */
        VUnimplementedException(const VString& errorString, bool recordStackTrace = true) : VException(errorString, recordStackTrace) {}
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
class VOSStatusException : public VException {
    public:

        /**
        Throws a VOSStatusException if err is non-zero; the error value is used for the
        VException error code.
        @param  err                 the error code
        @param  errorMessage        the message
        @param  recordStackTrace    if true, will attempt to collect stack trace into mCallStack
        */
        static void throwIfError(OSStatus err, const VString& message, bool recordStackTrace = true) { if (0 != err) throw VOSStatusException(err, message, recordStackTrace); }

        /**
        Constructs the exception with the OSStatus value. The value is
        stored in the VException error code.
        @param  err                 the error code
        @param  errorString         the error message
        @param  recordStackTrace    if true, will attempt to collect stack trace into mCallStack
        */
        VOSStatusException(OSStatus err, const VString& message, bool recordStackTrace = true) : VException(static_cast<int>(err), message, recordStackTrace) {}
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
static cast_to_type VcheckedDynamicCast(an_object_ptr& obj, const char* file, int line, bool rethrowException, bool logException, bool logStackCrawl) {
    try {
        return dynamic_cast<cast_to_type>(obj);
    } catch (const std::exception& ex) {
        // Avoid message construction overhead if no flags require it.
        if (logException || rethrowException) {
            VString message(VSTRING_ARGS("Exception in dynamic_cast operation at %s:%d: '%s'", file, line, ex.what()));

            if (logException) {
                if (logStackCrawl) {
                    VThread::logStackCrawl(message, VLogger::getDefaultLogger(), false);
                }

                VLogger::getDefaultLogger()->log(VLoggerLevel::ERROR, file, line, message);
            }

            if (rethrowException) {
                throw VException(message);
            }
        }

        return NULL;
    }
}

/// Performs a checked dynamic cast that will propagate any dynamic cast exception after first logging a stack crawl.
#define V_CHECKED_DYNAMIC_CAST(cast_to_type, an_object) VcheckedDynamicCast<cast_to_type>((an_object), __FILE__, __LINE__, true, true, true)
/// Performs a checked dynamic cast that will, upon a dynamic cast exception, log a stack crawl, eat the exception, and return NULL instead.
#define V_CHECKED_DYNAMIC_CAST_NOTHROW(cast_to_type, an_object) VcheckedDynamicCast<cast_to_type>((an_object), __FILE__, __LINE__, false, true, true)

#endif /* vexception_h */
