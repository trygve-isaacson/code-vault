/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vtypes.h"

#include "vstring.h"
#include "vexception.h"

#include <errno.h>

Vs64 vault::VgetMemoryUsage() {
    return 0; // FIXME - find an API to use on Mac
}

// On Mac OS X, it is usually most convenient if we use Unix line endings
// (0x0A) rather than Classic Mac OS 9 line endings (0x0D), because many
// of the Unix tools barf on Classic line endings.

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

bool VSystemError::_isLikePosixError(int posixErrorCode) const {
    // We are POSIX. No translation necessary.
    return (posixErrorCode == mErrorCode);
}
