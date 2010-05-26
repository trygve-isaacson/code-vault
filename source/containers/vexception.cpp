/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vexception.h"

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
void VException::_breakpointLocation()
    {
    // Put a breakpoint here if you want to break on all VExceptions.
    }

VException::VException() :
mError(kGenericError),
mErrorString(), // -> empty
mErrorMessage("") // if mErrorString is empty, we assert that mErrorMessage is not null, so assign it something
    {
    ASSERT_INVARIANT();
    
    VException::_breakpointLocation();
    }

VException::VException(const VException& other) :
std::exception(),
mError(other.getError()),
mErrorString(other.mErrorString),
mErrorMessage(other.mErrorMessage)
    {
    ASSERT_INVARIANT();

    VException::_breakpointLocation();
    }

VException::VException(int error, const char* errorMessage) :
mError(error),
mErrorString(), // -> empty
mErrorMessage(errorMessage)
    {
    ASSERT_INVARIANT();
    
    VException::_breakpointLocation();
    }

VException::VException(int error, const VString& errorString) :
mError(error),
mErrorString(errorString),
mErrorMessage(NULL)
    {
    ASSERT_INVARIANT();
    
    VException::_breakpointLocation();
    }

VException::VException(const char* errorMessage) :
mError(kGenericError),
mErrorString(), // -> empty
mErrorMessage(errorMessage)
    {
    ASSERT_INVARIANT();
    
    VException::_breakpointLocation();
    }

VException::VException(const VString& errorString) :
mError(kGenericError),
mErrorString(errorString),
mErrorMessage(NULL)
    {
    ASSERT_INVARIANT();
    
    VException::_breakpointLocation();
    }

VException::~VException() throw()
    {
    // do NOT delete mErrorMessage, it's someone's static string constant
    }

VException& VException::operator=(const VException& other)
    {
    ASSERT_INVARIANT();

    mError = other.getError();
    mErrorString = other.mErrorString;
    mErrorMessage = other.mErrorMessage;

    ASSERT_INVARIANT();
    
    return *this;
    }

const char* VException::what() const throw()
    {
    ASSERT_INVARIANT();

    if (mErrorMessage == NULL)
        return mErrorString;
    else
        return mErrorMessage;
    }
    
int VException::getError() const
    {
    ASSERT_INVARIANT();

    return mError;
    }

void VException::_assertInvariant() const
    {
    /*
    Either mErrorString is used, or mErrorMessage is used. I guess the
    real way of detecting trouble is if mErrorString contains something,
    and mErrorMessage is non-null, then it's a good assumption that
    mErrorMessage is uninitialized.
    */
    V_ASSERT(mErrorMessage != VCPP_DEBUG_BAD_POINTER_VALUE);
    if (mErrorString.isEmpty())
        V_ASSERT(mErrorMessage != NULL);
    else
        V_ASSERT(mErrorMessage == NULL);
    }

#ifndef V_TRANSLATE_WIN32_STRUCTURED_EXCEPTIONS

void VException::installWin32SEHandler() {}

#else

#include "eh.h"

#define CASE_RETURN_BREAK(symbolname) case symbolname: return #symbolname; break;

// see: <http://msdn.microsoft.com/en-us/library/aa363082(VS.85).aspx>
static const char* _getExceptionLabel(const EXCEPTION_RECORD& er)
    {
    switch (er.ExceptionCode)
        {
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

static void _win32SEHandler(unsigned code, EXCEPTION_POINTERS* ep)
    {
    if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_NONCONTINUABLE_EXCEPTION)
        {
        VString msg("_win32SEHandler: ExceptionCode = EXCEPTION_NONCONTINUABLE_EXCEPTION. Exiting.");
        std::cout << msg << std::endl;
        std::cerr << msg << std::endl;
        VLOGGER_FATAL(msg);
        ::ExitProcess(1);
        }
    else if (ep->ExceptionRecord->ExceptionFlags == EXCEPTION_NONCONTINUABLE)
        {
        VString msg("_win32SEHandler: ExceptionFlags = EXCEPTION_NONCONTINUABLE. Exiting.");
        std::cout << msg << std::endl;
        std::cerr << msg << std::endl;
        VLOGGER_FATAL(msg);
        ::ExitProcess(1);
        }
    else
        {
        VStringLogger logger(VLogger::kTrace, VString::EMPTY(), VString::EMPTY());
        VThread::logStackCrawl(_getExceptionLabel(*(ep->ExceptionRecord)), &logger, false);
        throw VException((int) code, logger.getLines());
        }
    }

// static
void VException::installWin32SEHandler()
    {
    if (gWin32SEHEnabled)
        _set_se_translator(_win32SEHandler);
    }

#endif

// static
void VException::enableWin32SEHandler(bool enabled)
    {
    gWin32SEHEnabled = enabled;
    }

// static
bool VException::isWin32SEHandlerEnabled()
    {
    return gWin32SEHEnabled;
    }
