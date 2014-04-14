/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
*/

/** @file */

#include "vbufferedfilestream.h"
#include "vtypes_internal.h"

#include "vexception.h"

VBufferedFileStream::VBufferedFileStream()
    : VAbstractFileStream()
    , mFile(NULL)
    , mCloseOnDestruct(true)
    {
}

VBufferedFileStream::VBufferedFileStream(const VFSNode& node)
    : VAbstractFileStream(node)
    , mFile(NULL)
    , mCloseOnDestruct(true)
    {
}

VBufferedFileStream::VBufferedFileStream(FILE* f, bool closeOnDestruct)
    : VAbstractFileStream()
    , mFile(f)
    , mCloseOnDestruct(closeOnDestruct)
    {
}

VBufferedFileStream::~VBufferedFileStream() {
    if (mCloseOnDestruct) {
        VBufferedFileStream::close();
    }

    mFile = NULL;
}

void VBufferedFileStream::setFile(FILE* f, bool closeOnDestruct) {
    mFile = f;
    mCloseOnDestruct = closeOnDestruct;
}

void VBufferedFileStream::openReadOnly() {
    mFile = VFileSystem::fopen(mNode.getPath(), "rb");

    this->_throwIfOpenFailed("VBufferedFileStream::openReadOnly", mNode.getPath());
}

void VBufferedFileStream::openReadWrite() {
    /*
    The semantics of ::fopen() and ::open() run counter to the normal desire
    to open a file r/w and have it created if it doesn't exist. For example,
    mode "r" does not create the file. Mode "w" does. So instead, we check for
    existence first, and then open it exactly the way we intend.
    */

    mFile = VFileSystem::fopen(mNode.getPath(), mNode.exists() ? "r+b" : "w+b");

    this->_throwIfOpenFailed("VBufferedFileStream::openReadWrite", mNode.getPath());
}

void VBufferedFileStream::openWrite() {
    mFile = VFileSystem::fopen(mNode.getPath(), "wb");

    this->_throwIfOpenFailed("VBufferedFileStream::openWrite", mNode.getPath());
}

bool VBufferedFileStream::isOpen() const {
    return (mFile != NULL);
}

void VBufferedFileStream::close() {
    if (this->isOpen()) {
        (void) VFileSystem::fclose(mFile);
        mFile = NULL;
    }
}

Vs64 VBufferedFileStream::read(Vu8* targetBuffer, Vs64 numBytesToRead) {
    /*
    Most of the work here is to deal with the fact that we are providing
    a SInt64 count in our API, but the underlying OS API may only
    allow a size_t count. So we may have to read in a loop until the
    requested number of bytes is read or EOF is reached. In the typical
    case, of course, we only go through the loop once.
    */

    bool    needsSizeConversion = (sizeof(Vs64) != sizeof(size_t));
    Vs64    numBytesRemaining = numBytesToRead;
    Vs64    numBytesRead = 0;
    size_t  requestCount;
    size_t  actualCount;

    do {
        if (needsSizeConversion) {
            requestCount = static_cast<size_t>(V_MIN(((Vs64)(0x7FFFFFFF)), numBytesRemaining));
        } else {
            requestCount = static_cast<size_t>(numBytesRemaining);
        }

        actualCount = VFileSystem::fread(targetBuffer + numBytesRead, 1, requestCount, mFile);

        numBytesRead += actualCount;
        numBytesRemaining -= actualCount;

    } while ((actualCount == requestCount) && (numBytesRemaining > 0));

    return numBytesRead;
}

Vs64 VBufferedFileStream::write(const Vu8* buffer, Vs64 numBytesToWrite) {
    /*
    Most of the work here is to deal with the fact that we are providing
    a SInt64 count in our API, but the underlying OS API may only
    allow a size_t count. So we may have to write in a loop until the
    requested number of bytes is written. In the typical
    case, of course, we only go through the loop once.
    */

    bool    needsSizeConversion = VStream::needSizeConversion(numBytesToWrite);
    Vs64    numBytesRemaining = numBytesToWrite;
    Vs64    numBytesWritten = 0;
    size_t  requestCount;
    size_t  actualCount;

    do {
        if (needsSizeConversion) {
            requestCount = static_cast<size_t>(V_MIN((static_cast<Vs64>(LONG_MAX)), numBytesRemaining));
        } else {
            requestCount = static_cast<size_t>(numBytesRemaining);
        }

        actualCount = VFileSystem::fwrite(buffer + numBytesWritten, 1, requestCount, mFile);

        numBytesWritten += actualCount;
        numBytesRemaining -= actualCount;

    } while ((actualCount == requestCount) && (numBytesRemaining > 0));

    if (numBytesWritten != numBytesToWrite) {
        VString path;
        mNode.getPath(path);

        throw VException(VSystemError(), VSTRING_FORMAT("VBufferedFileStream::write to '%s' only wrote " VSTRING_FORMATTER_S64 " of " VSTRING_FORMATTER_S64 " requested bytes.", path.chars(), numBytesWritten, numBytesToWrite));
    }

    return numBytesWritten;
}

void VBufferedFileStream::flush() {
    int result = VFileSystem::fflush(mFile);

    if (result != 0) {
        VString path;
        mNode.getPath(path);
        throw VException(VSystemError(), VSTRING_FORMAT("VBufferedFileStream::flush to '%s' failed with result %d.", path.chars(), result));
    }
}

bool VBufferedFileStream::skip(Vs64 numBytesToSkip) {
    /*
    Most of the work here is to deal with the fact that we are providing
    a SInt64 count in our API, but the underlying OS API may only
    allow a long count. So we may have to seek in a loop until the
    requested offset is seeked. In the typical case, of course, we only go
    through the loop once.
    */

    bool success;
    bool needsSizeConversion = VStream::needSizeConversion(numBytesToSkip);
    Vs64 numBytesRemaining = numBytesToSkip;
    Vs64 requestCount;

    do {
        if (needsSizeConversion) {
            requestCount = V_MIN((static_cast<Vs64>(LONG_MAX)), numBytesRemaining);
        } else {
            requestCount = numBytesRemaining;
        }

        success = this->seek(numBytesToSkip, SEEK_CUR);

        numBytesRemaining -= requestCount;

    } while (success && (numBytesRemaining > 0));

    return success;
}

bool VBufferedFileStream::seek(Vs64 offset, int whence) {
    // FIXME: need to deal with Vs64-to-long conversion
    return (VFileSystem::fseek(mFile, static_cast<long>(offset), whence) == 0);
}

Vs64 VBufferedFileStream::getIOOffset() const {
    return VFileSystem::ftell(mFile);
}

Vs64 VBufferedFileStream::available() const {
    Vs64 currentOffset = this->getIOOffset();
    Vs64 eofOffset;

    // const_cast: WORKAROUND. Save/restore state.
    const_cast<VBufferedFileStream*>(this)->seek(0, SEEK_END);
    eofOffset = this->getIOOffset();
    const_cast<VBufferedFileStream*>(this)->seek(currentOffset, SEEK_SET);    // restore original position

    return eofOffset - currentOffset;
}

