/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

/** @file */

#include "vdirectiofilestream.h"
#include "vtypes_internal.h"

#include "vexception.h"

VDirectIOFileStream::VDirectIOFileStream()
    : VAbstractFileStream()
    , mFile(-1)
    , mCloseOnDestruct(true)
    {
}

VDirectIOFileStream::VDirectIOFileStream(const VFSNode& node)
    : VAbstractFileStream(node)
    , mFile(-1)
    , mCloseOnDestruct(true)
    {
}

VDirectIOFileStream::VDirectIOFileStream(int fd, bool closeOnDestruct)
    : VAbstractFileStream()
    , mFile(fd)
    , mCloseOnDestruct(closeOnDestruct)
    {
}

VDirectIOFileStream::~VDirectIOFileStream() {
    if (mCloseOnDestruct) {
        VDirectIOFileStream::close();
    }
}

void VDirectIOFileStream::setFile(int fd, bool closeOnDestruct) {
    mFile = fd;
    mCloseOnDestruct = closeOnDestruct;
}

void VDirectIOFileStream::openReadOnly() {
    mFile = VFileSystemAPI::wrap_open(mNode.getPath(), READ_ONLY_MODE);

    this->_throwIfOpenFailed("VDirectIOFileStream::openReadOnly", mNode.getPath());
}

void VDirectIOFileStream::openReadWrite() {
    /*
    The semantics of ::fopen() and ::open() run counter to the normal desire
    to open a file r/w and have it created if it doesn't exist. For example,
    mode "r" does not create the file. Mode "w" does. So instead, we check for
    existence first, and then open it exactly the way we intend.
    */

    mFile = VFileSystemAPI::wrap_open(mNode.getPath(), mNode.exists() ? READWRITE_MODE : WRITE_CREATE_MODE);

    this->_throwIfOpenFailed("VDirectIOFileStream::openReadWrite", mNode.getPath());
}

void VDirectIOFileStream::openWrite() {
    mFile = VFileSystemAPI::wrap_open(mNode.getPath(), WRITE_CREATE_MODE);

    this->_throwIfOpenFailed("VDirectIOFileStream::openWrite", mNode.getPath());
}

bool VDirectIOFileStream::isOpen() const {
    return (mFile != -1);
}

void VDirectIOFileStream::close() {
    if (this->isOpen()) {
        (void) VFileSystemAPI::wrap_close(mFile);
        mFile = -1;
    }
}

Vs64 VDirectIOFileStream::read(Vu8* targetBuffer, Vs64 numBytesToRead) {
    /*
    Most of the work here is to deal with the fact that we are providing
    a SInt64 count in our API, but the underlying OS API may only
    allow a size_t count. So we may have to read in a loop until the
    requested number of bytes is read or EOF is reached. In the typical
    case, of course, we only go through the loop once.
    */

    bool    needsSizeConversion = VStream::needSizeConversion(numBytesToRead);
    Vs64    numBytesRemaining = numBytesToRead;
    Vs64    numBytesRead = 0;
    size_t  requestCount;
    size_t  actualCount;

    // FIXME: If needsSizeConversion is false, just do a read w/o the excess baggage here.
    // Also, we can set it to false if the actual size fits in 31 bits, as in VStream::copyMemory.
    do {
        if (needsSizeConversion) {
            requestCount = static_cast<size_t>(V_MIN((CONST_S64(0x7FFFFFFF)), numBytesRemaining));
        } else {
            requestCount = static_cast<size_t>(numBytesRemaining);
        }

        actualCount = static_cast<size_t>(VFileSystemAPI::wrap_read(mFile, targetBuffer + numBytesRead, requestCount));

        numBytesRead += actualCount;
        numBytesRemaining -= actualCount;

    } while ((actualCount == requestCount) && (numBytesRemaining > 0));

    return numBytesRead;
}

Vs64 VDirectIOFileStream::write(const Vu8* buffer, Vs64 numBytesToWrite) {
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

    // FIXME: If needsSizeConversion is false, just do a write w/o the excess baggage here.
    // Also, we can set it to false if the actual size fits in 31 bits, as in VStream::copyMemory.
    do {
        if (needsSizeConversion) {
            requestCount = static_cast<size_t>(V_MIN((static_cast<Vs64>(LONG_MAX)), numBytesRemaining));
        } else {
            requestCount = static_cast<size_t>(numBytesRemaining);
        }

        actualCount = static_cast<size_t>(VFileSystemAPI::wrap_write(mFile, buffer + numBytesWritten, requestCount));

        numBytesWritten += actualCount;
        numBytesRemaining -= actualCount;

    } while ((actualCount == requestCount) && (numBytesRemaining > 0));

    if (numBytesWritten != numBytesToWrite) {
        VString    path;
        mNode.getPath(path);

        throw VException(VSystemError(), VSTRING_FORMAT("VDirectIOFileStream::write to '%s' only wrote " VSTRING_FORMATTER_S64 " of " VSTRING_FORMATTER_S64 " requested bytes.", path.chars(), numBytesWritten, numBytesToWrite));
    }

    return numBytesWritten;
}

void VDirectIOFileStream::flush() {
    // Unbuffered file writes have no flush mechanism since they are not buffered.
}

bool VDirectIOFileStream::skip(Vs64 numBytesToSkip) {
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

    // FIXME: If needsSizeConversion is false, just do a seek w/o the excess baggage here.
    // Also, we can set it to false if the actual size fits in 31 bits, as in VStream::copyMemory.
    // FIXME: This calls VDirectIOFileStream::seek below, which is not as savvy as this--should it be?
    do {
        if (needsSizeConversion) {
            requestCount = V_MIN((static_cast<Vs64>(LONG_MAX)), numBytesRemaining);
        } else {
            requestCount = numBytesRemaining;
        }

        success = this->seek(requestCount, SEEK_CUR);

        numBytesRemaining -= requestCount;

    } while (success && (numBytesRemaining > 0));

    return success;
}

bool VDirectIOFileStream::seek(Vs64 offset, int whence) {
    // FIXME: need to deal with Vs64-to-off_t conversion
    return (VFileSystemAPI::wrap_lseek(mFile, static_cast<off_t>(offset), whence) == 0);
}

Vs64 VDirectIOFileStream::getIOOffset() const {
    return VFileSystemAPI::wrap_lseek(mFile, static_cast<off_t>(0), SEEK_CUR);
}

Vs64 VDirectIOFileStream::available() const {
    Vs64 currentOffset = this->getIOOffset();
    Vs64 eofOffset;

    // const_cast: WORKAROUND. Save/restore state.
    const_cast<VDirectIOFileStream*>(this)->seek(0, SEEK_END);
    eofOffset = this->getIOOffset();
    const_cast<VDirectIOFileStream*>(this)->seek(currentOffset, SEEK_SET);    // restore original position

    return eofOffset - currentOffset;
}

