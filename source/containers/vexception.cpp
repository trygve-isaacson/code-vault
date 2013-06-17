/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

/** @file */

#include "vexception.h"

#include "vassert.h"
#include "vtypes_internal.h"

// VSystemError ---------------------------------------------------------------

//static
VSystemError VSystemError::getSocketError() {
    return VSystemError(VSystemError::_getSocketErrorCode());
}

VSystemError::VSystemError()
    : mErrorCode(VSystemError::_getSystemErrorCode())
    , mErrorMessage(VSystemError::_getSystemErrorMessage(mErrorCode))
    {
}

VSystemError::VSystemError(int errorCode)
    : mErrorCode(errorCode)
    , mErrorMessage(VSystemError::_getSystemErrorMessage(errorCode))
    {
}

VSystemError::VSystemError(int errorCode, const VString& errorMessage)
    : mErrorCode(errorCode)
    , mErrorMessage(errorMessage)
    {
}

// VException -----------------------------------------------------------------

// Is ASSERT_INVARIANT enabled/disabled specifically for VException?
#ifdef V_ASSERT_INVARIANT_VEXCEPTION_ENABLED
    #undef ASSERT_INVARIANT
    #if V_ASSERT_INVARIANT_VEXCEPTION_ENABLED == 1
        #define ASSERT_INVARIANT() this->_assertInvariant() ///< Macro to call this->_assertInvariant().
    #else
        #define ASSERT_INVARIANT() ((void) 0) ///< No-op.
    #endif
#endif

// enableWin32SEHandler(false) will clear this flag and therefore disable installWin32SEHandler() functionality.
bool VException::gWin32SEHEnabled = true;

// static
void VException::_breakpointLocation() {
    // Put a breakpoint here if you want to break on all VExceptions.
}

VException::VException(bool recordStackTrace)
    : std::exception()
    , mError(kGenericError)
    , mErrorString()
    , mErrorMessage("") // if mErrorString is empty, we assert that mErrorMessage is not null, so assign it something
    , mStackTrace()
    {
    ASSERT_INVARIANT();

    VException::_breakpointLocation();

    if (recordStackTrace) {
        this->_recordStackTrace();
    }
}

VException::VException(const VException& other, bool recordStackTrace)
    : std::exception()
    , mError(other.getError())
    , mErrorString(other.mErrorString)
    , mErrorMessage(other.mErrorMessage)
    , mStackTrace(other.mStackTrace)
    {
    ASSERT_INVARIANT();

    VException::_breakpointLocation();

    if (recordStackTrace) {
        this->_recordStackTrace();
    }
}

VException::VException(int error, const char* errorMessage, bool recordStackTrace)
    : std::exception()
    , mError(error)
    , mErrorString()
    , mErrorMessage(errorMessage)
    , mStackTrace()
    {
    ASSERT_INVARIANT();

    VException::_breakpointLocation();

    if (recordStackTrace) {
        this->_recordStackTrace();
    }
}

VException::VException(int error, const VString& errorString, bool recordStackTrace)
    : std::exception()
    , mError(error)
    , mErrorString(errorString)
    , mErrorMessage(NULL)
    , mStackTrace()
    {
    ASSERT_INVARIANT();

    VException::_breakpointLocation();

    if (recordStackTrace) {
        this->_recordStackTrace();
    }
}

VException::VException(const char* errorMessage, bool recordStackTrace)
    : std::exception()
    , mError(kGenericError)
    , mErrorString()
    , mErrorMessage(errorMessage)
    , mStackTrace()
    {
    ASSERT_INVARIANT();

    VException::_breakpointLocation();

    if (recordStackTrace) {
        this->_recordStackTrace();
    }
}

VException::VException(const VString& errorString, bool recordStackTrace)
    : std::exception()
    , mError(kGenericError)
    , mErrorString(errorString)
    , mErrorMessage(NULL)
    , mStackTrace()
    {
    ASSERT_INVARIANT();

    VException::_breakpointLocation();

    if (recordStackTrace) {
        this->_recordStackTrace();
    }
}

VException::VException(const VSystemError& error, const VString& errorString, bool recordStackTrace)
    : std::exception()
    , mError(error.getErrorCode())
    , mErrorString(VSTRING_FORMAT("%s Error %d: %s.", errorString.chars(), error.getErrorCode(), error.getErrorMessage().chars()))
    , mErrorMessage(NULL)
    , mStackTrace()
    {
    ASSERT_INVARIANT();

    VException::_breakpointLocation();

    if (recordStackTrace) {
        this->_recordStackTrace();
    }
}

VException::~VException() throw() {
    // do NOT delete mErrorMessage, it's someone's static string constant
    mErrorMessage = NULL;
}

VException& VException::operator=(const VException& other) {
    ASSERT_INVARIANT();

    mError = other.getError();
    mErrorString = other.mErrorString;
    mErrorMessage = other.mErrorMessage;
    mStackTrace = other.mStackTrace;

    ASSERT_INVARIANT();

    return *this;
}

const char* VException::what() const throw() {
    ASSERT_INVARIANT();

    if (mErrorMessage == NULL)
        return mErrorString;
    else
        return mErrorMessage;
}

int VException::getError() const {
    ASSERT_INVARIANT();

    return mError;
}

const VStringVector& VException::getStackTrace() const {
    ASSERT_INVARIANT();

    return mStackTrace;
}

void VException::_assertInvariant() const {
    /*
    Either mErrorString is used, or mErrorMessage is used. I guess the
    real way of detecting trouble is if mErrorString contains something,
    and mErrorMessage is non-null, then it's a good assumption that
    mErrorMessage is uninitialized.
    */
    VASSERT_NOT_EQUAL(mErrorMessage, VCPP_DEBUG_BAD_POINTER_VALUE);
    if (mErrorString.isEmpty())
        VASSERT_NOT_NULL(mErrorMessage);
    else
        VASSERT_NULL(mErrorMessage);
}

void VException::_recordStackTrace() {
#ifdef VAULT_USER_STACKCRAWL_SUPPORT
    // If error supplied as literal, move it to our VString error message to combine with stack trace.
    if (mErrorMessage != NULL) {
        mErrorString += mErrorMessage;
        mErrorMessage = NULL;
    }

    // Use a VStringLogger so we can log directly to it and then capture its lines and add them to the exception.
    boost::shared_ptr<VStringLogger> logger(new VStringLogger(VString::EMPTY(), VLoggerLevel::TRACE));
    VThread::logStackCrawl("Stack:", logger, false);
    mErrorString += VString::NATIVE_LINE_ENDING();
    mErrorString += logger->getLines();
#endif
}

#ifndef V_TRANSLATE_WIN32_STRUCTURED_EXCEPTIONS

void VException::installWin32SEHandler() {}

#else

#include "eh.h"

#define CASE_RETURN_BREAK(symbolname) case symbolname: return #symbolname; break;

// see: <http://msdn.microsoft.com/en-us/library/aa363082(VS.85).aspx>
static const char* _getExceptionLabel(const EXCEPTION_RECORD& er) {
    switch (er.ExceptionCode) {
            CASE_RETURN_BREAK(EXCEPTION_ACCESS_VIOLATION)
            CASE_RETURN_BREAK(EXCEPTION_ARRAY_BOUNDS_EXCEEDED)
            CASE_RETURN_BREAK(EXCEPTION_BREAKPOINT)
            CASE_RETURN_BREAK(EXCEPTION_DATATYPE_MISALIGNMENT)
            CASE_RETURN_BREAK(EXCEPTION_FLT_DENORMAL_OPERAND)
            CASE_RETURN_BREAK(EXCEPTION_FLT_DIVIDE_BY_ZERO)
            CASE_RETURN_BREAK(EXCEPTION_FLT_INEXACT_RESULT)
            CASE_RETURN_BREAK(EXCEPTION_FLT_INVALID_OPERATION)
            CASE_RETURN_BREAK(EXCEPTION_FLT_OVERFLOW)
            CASE_RETURN_BREAK(EXCEPTION_FLT_STACK_CHECK)
            CASE_RETURN_BREAK(EXCEPTION_FLT_UNDERFLOW)
            CASE_RETURN_BREAK(EXCEPTION_ILLEGAL_INSTRUCTION)
            CASE_RETURN_BREAK(EXCEPTION_IN_PAGE_ERROR)
            CASE_RETURN_BREAK(EXCEPTION_INT_DIVIDE_BY_ZERO)
            CASE_RETURN_BREAK(EXCEPTION_INT_OVERFLOW)
            CASE_RETURN_BREAK(EXCEPTION_INVALID_DISPOSITION)
            CASE_RETURN_BREAK(EXCEPTION_NONCONTINUABLE_EXCEPTION)
            CASE_RETURN_BREAK(EXCEPTION_PRIV_INSTRUCTION)
            CASE_RETURN_BREAK(EXCEPTION_SINGLE_STEP)
            CASE_RETURN_BREAK(EXCEPTION_STACK_OVERFLOW)
        default: return "OTHER_EXCEPTION_CODE"; break;
    }
}

static void _win32SEHandler(unsigned code, EXCEPTION_POINTERS* ep) {
    if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_NONCONTINUABLE_EXCEPTION) {
        VString msg("_win32SEHandler: ExceptionCode = EXCEPTION_NONCONTINUABLE_EXCEPTION. Exiting.");
        std::cout << msg << std::endl;
        std::cerr << msg << std::endl;
        VLOGGER_FATAL(msg);
        ::ExitProcess(1);
    } else if (ep->ExceptionRecord->ExceptionFlags == EXCEPTION_NONCONTINUABLE) {
        VString msg("_win32SEHandler: ExceptionFlags = EXCEPTION_NONCONTINUABLE. Exiting.");
        std::cout << msg << std::endl;
        std::cerr << msg << std::endl;
        VLOGGER_FATAL(msg);
        ::ExitProcess(1);
    } else {
        VStringLoggerPtr logger(new VStringLogger(VString::EMPTY(), VLoggerLevel::TRACE, false));
        VThread::logStackCrawl(_getExceptionLabel(*(ep->ExceptionRecord)), logger, false);
        throw VException((int) code, logger->getLines());
    }
}

// static
void VException::installWin32SEHandler() {
    if (gWin32SEHEnabled) {
        _set_se_translator(_win32SEHandler);
    }
}

#endif

// static
void VException::enableWin32SEHandler(bool enabled) {
    gWin32SEHEnabled = enabled;
}

// static
bool VException::isWin32SEHandlerEnabled() {
    return gWin32SEHEnabled;
}
