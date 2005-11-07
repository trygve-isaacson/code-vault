/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vexception.h"

// static
void VException::breakpointLocation()
    {
    // Put a breakpoint here if you want to break on all VExceptions.
    }

VException::VException()
    {
    mError = kGenericError;
    mErrorMessage = "";    // if mErrorString is empty, we assert that mErrorMessage is not null, so assign it something
    
    ASSERT_INVARIANT();
    
    VException::breakpointLocation();
    }

VException::VException(int error, const char* errorMessage)
    {
    mError = error;
    mErrorMessage = errorMessage;
    
    ASSERT_INVARIANT();
    
    VException::breakpointLocation();
    }

//lint -e818 -e960 "Violates MISRA Required Rule 69, function has variable number of arguments"
VException::VException(int error, char* format, ...)
    {
    mError = error;
    mErrorMessage = NULL;

     va_list    args;
    va_start(args, format);

    mErrorString.vaFormat(format, args);

    va_end(args);
    
    ASSERT_INVARIANT();
    
    VException::breakpointLocation();
    }

VException::VException(int error, const VString& errorString)
: mErrorString(errorString)
    {
    mError = error;
    mErrorMessage = NULL;
    
    ASSERT_INVARIANT();
    
    VException::breakpointLocation();
    }

VException::VException(const char* errorMessage)
    {
    mError = kGenericError;
    mErrorMessage = errorMessage;
    
    ASSERT_INVARIANT();
    
    VException::breakpointLocation();
    }

//lint -e818 -e960 "Violates MISRA Required Rule 69, function has variable number of arguments"
VException::VException(char* format, ...)
    {
    mError = kGenericError;
    mErrorMessage = NULL;

     va_list    args;
    va_start(args, format);

    mErrorString.vaFormat(format, args);

    va_end(args);
    
    ASSERT_INVARIANT();
    
    VException::breakpointLocation();
    }

VException::VException(const VString& errorString)
: mErrorString(errorString)
    {
    mError = kGenericError;
    mErrorMessage = NULL;
    
    ASSERT_INVARIANT();
    
    VException::breakpointLocation();
    }

VException::~VException() throw()
    {
    // do NOT delete mErrorMessage, it's someone's static string constant
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

