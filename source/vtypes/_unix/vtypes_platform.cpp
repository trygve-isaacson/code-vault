/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vtypes.h"

#include "vstring.h"
#include "vtypes_internal_platform.h"

#include <errno.h>

Vs64 vault::VgetMemoryUsage() {
    return 0; // FIXME - find an API to use on Unix
}

static const Vu8 kUnixLineEnding = 0x0A;

const Vu8* vault::VgetNativeLineEnding(int& numBytes) {
    numBytes = 1;
    return &kUnixLineEnding;
}

// VSystemError ---------------------------------------------------------------
// Platform-specific implementation of VSystemError internal accessors.

// static
int VSystemError::_getSystemErrorCode() {
    return errno;
}

// static
int VSystemError::_getSocketErrorCode() {
    return errno;
}

// static
VString VSystemError::_getSystemErrorMessage(int errorCode) {
    return ::strerror(errorCode);
}

// static
bool VSystemError::_isLikePosixError(int posixErrorCode) const {
    // We are POSIX. No translation necessary.
    return (posixErrorCode == mErrorCode);
}

// VAutoreleasePool -----------------------------------------------------------

// VAutoreleasePool is a no-op on Unix.
VAutoreleasePool::VAutoreleasePool() {}
void VAutoreleasePool::drain() {}
VAutoreleasePool::~VAutoreleasePool() {}
