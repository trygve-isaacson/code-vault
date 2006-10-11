/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vexception.h"

// static
void VException::_breakpointLocation()
    {
    // Put a breakpoint here if you want to break on all VExceptions.
    }

VException::VException() :
mError(kGenericError),
// mErrorString constructs to empty string
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
// mErrorString constructs to empty string
mErrorMessage(errorMessage)
    {
    ASSERT_INVARIANT();
    
    VException::_breakpointLocation();
    }

//lint -e818 -e960 "Violates MISRA Required Rule 69, function has variable number of arguments"
VException::VException(int error, char* format, ...) :
mError(error),
// mErrorString constructs to empty string
mErrorMessage(NULL)
    {
    va_list    args;
    va_start(args, format);

    mErrorString.vaFormat(format, args);

    va_end(args);
    
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
// mErrorString constructs to empty string
mErrorMessage(errorMessage)
    {
    ASSERT_INVARIANT();
    
    VException::_breakpointLocation();
    }

//lint -e818 -e960 "Violates MISRA Required Rule 69, function has variable number of arguments"
VException::VException(char* format, ...) :
mError(kGenericError),
// mErrorString constructs to empty string
mErrorMessage(NULL)
    {
    va_list    args;
    va_start(args, format);

    mErrorString.vaFormat(format, args);

    va_end(args);
    
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

void VException::assertInvariant() const
    {
    /*
    Either mErrorString is used, or mErrorMessage is used. I guess the
    real way of detecting trouble is if mErrorString contains something,
    and mErrorMessage is non-null, then it's a good assumption that
    mErrorMessage is uninitialized.
    */
    
    if (mErrorString.isEmpty())
        V_ASSERT(mErrorMessage != NULL);
    else
        V_ASSERT(mErrorMessage == NULL);
    }

