/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

/** @file */

#include "vmemorystream.h"

#include "vexception.h"
#include "vassert.h"

// Is ASSERT_INVARIANT enabled/disabled specifically for VMemoryStream?
#ifdef V_ASSERT_INVARIANT_VMEMORYSTREAM_ENABLED
    #undef ASSERT_INVARIANT
    #if V_ASSERT_INVARIANT_VMEMORYSTREAM_ENABLED == 1
        #define ASSERT_INVARIANT() this->_assertInvariant() ///< Macro to call this->_assertInvariant().
    #else
        #define ASSERT_INVARIANT() ((void) 0) ///< No-op.
    #endif
#endif

// VMemoryStream --------------------------------------------------------------------------

VMemoryStream::VMemoryStream(Vs64 initialBufferSize, Vs64 resizeIncrement)
    : VStream()
    , mBufferSize(initialBufferSize)
    , mIOOffset(0)
    , mEOFOffset(0)
    , mResizeIncrement(resizeIncrement)
    , mOwnsBuffer(true)
    , mAllocationType(kAllocatedByOperatorNew)
    , mBuffer(NULL)
    {
    mBuffer = this->_createNewBuffer(mBufferSize, mAllocationType);

    ASSERT_INVARIANT();
}

VMemoryStream::VMemoryStream(const VMemoryStream& other)
    : VStream(other.mName)
    , mBufferSize(other.mBufferSize)
    , mIOOffset(other.mIOOffset)
    , mEOFOffset(other.mEOFOffset)
    , mResizeIncrement(other.mResizeIncrement)
    , mOwnsBuffer(false)
    , mAllocationType(other.mAllocationType)
    , mBuffer(other.mBuffer)
    {
    /*
    All we need to do is decide how to manage the buffer, based on the ownership:

    1.
    If the other stream does NOT own the buffer, then we're all set: we're sharing a
    buffer that some third stream owns. We don't own it, and just point to it like the
    other stream.

    2.
    If the other stream owns the buffer, then we need to make a copy. It is not safe
    for us to simply point to the existing buffer in a non-owned fashion, because
    some calling code might assume it can delete the other stream, which would delete
    the buffer it owns that we'd still point to if we weren't deleted first.
    */

    if (other.mOwnsBuffer) {
        // Case 2 -- we need to make our own copy of the buffer.
        BufferAllocationType newAllocationType;
        Vu8* newBuffer = this->_createNewBuffer(mBufferSize, newAllocationType);

        VStream::copyMemory(newBuffer, mBuffer, mEOFOffset);

        mBuffer = newBuffer;
        mAllocationType = newAllocationType;
        mOwnsBuffer = true;
    }

    ASSERT_INVARIANT();
}

VMemoryStream::VMemoryStream(Vu8* buffer, BufferAllocationType allocationType, bool adoptsBuffer, Vs64 suppliedBufferSize, Vs64 suppliedEOFOffset, Vs64 resizeIncrement)
    : VStream()
    , mBufferSize(suppliedBufferSize)
    , mIOOffset(0)
    , mEOFOffset(suppliedEOFOffset)
    , mResizeIncrement(resizeIncrement)
    , mOwnsBuffer(adoptsBuffer)
    , mAllocationType(allocationType)
    , mBuffer(buffer)
    {
    ASSERT_INVARIANT();
}

VMemoryStream::~VMemoryStream() {
    this->_releaseBuffer();
}

VMemoryStream& VMemoryStream::operator=(const VMemoryStream& other) {
    ASSERT_INVARIANT();

    if (this != &other) {
        mName = other.mName;
        mBufferSize = other.mBufferSize;
        mIOOffset = other.mIOOffset;
        mEOFOffset = other.mEOFOffset;
        mResizeIncrement = other.mResizeIncrement;
        mOwnsBuffer = false;
        mAllocationType = other.mAllocationType;
        mBuffer = other.mBuffer;

        if (other.mOwnsBuffer) {
            // Case 2 -- we need to make our own copy of the buffer.
            BufferAllocationType newAllocationType;
            Vu8* newBuffer = this->_createNewBuffer(mBufferSize, newAllocationType);

            VStream::copyMemory(newBuffer, mBuffer, mEOFOffset);

            mBuffer = newBuffer;
            mAllocationType = newAllocationType;
            mOwnsBuffer = true;
        }
    }

    ASSERT_INVARIANT();

    return *this;
}

Vs64 VMemoryStream::read(Vu8* targetBuffer, Vs64 numBytesToRead) {
    ASSERT_INVARIANT();

    Vs64 bytesRemainingInBuffer = mEOFOffset - mIOOffset;
    Vs64 actualNumBytesToCopy = V_MIN(numBytesToRead, bytesRemainingInBuffer);

    VStream::copyMemory(targetBuffer, mBuffer + mIOOffset, actualNumBytesToCopy);

    this->_finishRead(actualNumBytesToCopy);

    ASSERT_INVARIANT();

    return actualNumBytesToCopy;
}

Vs64 VMemoryStream::write(const Vu8* buffer, Vs64 numBytesToWrite) {
    ASSERT_INVARIANT();

    this->_prepareToWrite(numBytesToWrite);    // throws if we can't write requested size

    VStream::copyMemory(mBuffer + mIOOffset, buffer, numBytesToWrite);

    this->_finishWrite(numBytesToWrite);

    ASSERT_INVARIANT();

    return numBytesToWrite;
}

void VMemoryStream::flush() {
    // Nothing to flush.
}

bool VMemoryStream::skip(Vs64 numBytesToSkip) {
    ASSERT_INVARIANT();

    Vs64 bytesRemainingInBuffer = mEOFOffset - mIOOffset;
    Vs64 actualNumBytesToSkip = V_MIN(numBytesToSkip, bytesRemainingInBuffer);

    mIOOffset += actualNumBytesToSkip;

    ASSERT_INVARIANT();

    return (numBytesToSkip == actualNumBytesToSkip);
}

bool VMemoryStream::seek(Vs64 offset, int whence) {
    ASSERT_INVARIANT();

    Vs64 requestedOffset;
    Vs64 constrainedOffset;

    switch (whence) {
        case SEEK_SET:
            requestedOffset = offset;
            break;

        case SEEK_CUR:
            requestedOffset = mIOOffset + offset;
            break;

        case SEEK_END:
            requestedOffset = mEOFOffset;
            break;

        default:
            requestedOffset = CONST_S64(0);
            break;
    }

    if (requestedOffset < 0) {
        constrainedOffset = 0;
    } else if (requestedOffset > mEOFOffset) {
        constrainedOffset = requestedOffset;

        // write zeroes as we extend the buffer
        Vs64 numZeroesToWrite = requestedOffset - mEOFOffset;
        Vs64 oldEOF = mEOFOffset;

        mIOOffset = mEOFOffset;
        this->_prepareToWrite(numZeroesToWrite);

        while (oldEOF < mEOFOffset) {
            *(mBuffer + mIOOffset) = 0;
            oldEOF++;
        }

        this->_finishWrite(numZeroesToWrite);
    } else {
        constrainedOffset = requestedOffset;
    }

    mIOOffset = constrainedOffset;

    ASSERT_INVARIANT();

    return (constrainedOffset == requestedOffset);
}

Vs64 VMemoryStream::getIOOffset() const {
    ASSERT_INVARIANT();

    return mIOOffset;
}

Vs64 VMemoryStream::available() const {
    return mEOFOffset - mIOOffset;
}

Vu8* VMemoryStream::_getReadIOPtr() const {
    /*
    For VMemoryStream, there is no distinction between read and write
    i/o offset, so they are the same, and both read and write copying
    is supported.
    */
    ASSERT_INVARIANT();

    return mBuffer + mIOOffset;
}

Vu8* VMemoryStream::_getWriteIOPtr() const {
    /*
    For VMemoryStream, there is no distinction between read and write
    i/o offset, so they are the same, and both read and write copying
    is supported.
    */
    ASSERT_INVARIANT();

    return mBuffer + mIOOffset;
}

Vs64 VMemoryStream::_prepareToRead(Vs64 numBytesToRead) const {
    ASSERT_INVARIANT();

    Vs64 bytesRemainingInBuffer = mEOFOffset - mIOOffset;
    Vs64 actualNumBytesToRead = V_MIN(numBytesToRead, bytesRemainingInBuffer);

    return actualNumBytesToRead;
}

void VMemoryStream::_prepareToWrite(Vs64 numBytesToWrite) {
    ASSERT_INVARIANT();

    Vs64 requiredBufferSize = mIOOffset + numBytesToWrite;

    if (requiredBufferSize > mBufferSize) {
        // If we don't own the buffer, we are not permitted to reallocate it, so we throw
        // an EOF exception.
        if (! mOwnsBuffer) {
            throw VEOFException("VMemoryStream::_prepareToWrite: Invalid attempt to expand non-owned buffer.");
        }

        // Reallocate a larger buffer, copy old contents to it.
        Vs64 newBufferSize;

        if (mResizeIncrement == kIncrement2x) {
            // Double the size repeatedly until large enough.
            newBufferSize = mBufferSize;

            // Just in case caller was dumb enough to ask us to double
            // an initial size of zero, pick something...
            if (newBufferSize == 0) {
                newBufferSize = 1024;
            }

            do {
                newBufferSize = 2 * newBufferSize;
            } while (newBufferSize < requiredBufferSize);

        } else {
            // Grow to fit requirement, but if necessary round up to
            // align on increment boundary.
            newBufferSize = requiredBufferSize;

            Vs64 misalignment = newBufferSize % mResizeIncrement;

            if (misalignment != 0) {
                newBufferSize += (mResizeIncrement - misalignment);
            }
        }

        BufferAllocationType newAllocationType;
        Vu8* newBuffer = this->_createNewBuffer(newBufferSize, newAllocationType);

        VStream::copyMemory(newBuffer, mBuffer, mEOFOffset);

        this->_releaseBuffer();
        mBuffer = newBuffer;
        mBufferSize = newBufferSize;
        mAllocationType = newAllocationType;
    }

    ASSERT_INVARIANT();
}

void VMemoryStream::_finishRead(Vs64 numBytesRead) {
    ASSERT_INVARIANT();

    mIOOffset += numBytesRead;

    ASSERT_INVARIANT();
}

void VMemoryStream::_finishWrite(Vs64 numBytesWritten) {
    ASSERT_INVARIANT();

    mIOOffset += numBytesWritten;

    // If we advanced past "eof", move eof forward.
    if (mIOOffset > mEOFOffset) {
        mEOFOffset = mIOOffset;
    }

    ASSERT_INVARIANT();
}

void VMemoryStream::adoptBuffer(Vu8* buffer, BufferAllocationType allocationType, bool adoptsBuffer, Vs64 suppliedBufferSize, Vs64 suppliedEOFOffset) {
    ASSERT_INVARIANT();

    this->_releaseBuffer();

    mBufferSize = suppliedBufferSize;
    mEOFOffset = suppliedEOFOffset;
    mIOOffset = 0;
    mOwnsBuffer = adoptsBuffer;
    mAllocationType = allocationType;

    mBuffer = buffer;

    ASSERT_INVARIANT();
}

Vu8* VMemoryStream::getBuffer() const {
    ASSERT_INVARIANT();

    return mBuffer;
}

void VMemoryStream::orphanBuffer() {
    ASSERT_INVARIANT();

    mOwnsBuffer = false;
}

Vs64 VMemoryStream::getBufferSize() const {
    ASSERT_INVARIANT();

    return mBufferSize;
}

Vs64 VMemoryStream::getEOFOffset() const {
    ASSERT_INVARIANT();

    return mEOFOffset;
}

void VMemoryStream::setEOF(Vs64 eofOffset) {
    ASSERT_INVARIANT();

    mEOFOffset = V_MIN(mBufferSize, eofOffset);
    mIOOffset = V_MIN(mEOFOffset, mIOOffset);

    ASSERT_INVARIANT();
}

void VMemoryStream::_releaseBuffer() {
    if (mOwnsBuffer) {
        switch (mAllocationType) {
            case kAllocatedByOperatorNew:
                delete [] mBuffer;
                mBuffer = NULL;
                break;
            case kAllocatedByMalloc:
                ::free(mBuffer);
                mBuffer = NULL;
                break;
            case kAllocatedOnStack:
                mBuffer = NULL;
                break;
            case kAllocatedUnknown:
                mBuffer = NULL;
                break;
            default:
                break;
        }
    } else {
        mBuffer = NULL;
    }
}

Vu8* VMemoryStream::_createNewBuffer(Vs64 bufferSize, BufferAllocationType& newAllocationType) {
    Vu8* buffer = NULL;
    newAllocationType = mAllocationType; // only stack case changes this below
    switch (mAllocationType) {
        case kAllocatedByOperatorNew:
            buffer = VStream::newNewBuffer(bufferSize);
            break;
        case kAllocatedByMalloc:
            buffer = VStream::mallocNewBuffer(bufferSize);
            break;
        case kAllocatedOnStack:
        case kAllocatedUnknown:
            buffer = VStream::newNewBuffer(bufferSize);
            newAllocationType = kAllocatedByOperatorNew;
            break;
        default:
            throw VStackTraceException(VSTRING_FORMAT("VMemoryStream::_createNewBuffer: Invalid allocation type %d.", (int) mAllocationType));
            break;
    }

    return buffer;
}

void VMemoryStream::_assertInvariant() const {
    VASSERT_NOT_NULL(mBuffer);
    VASSERT_NOT_EQUAL(mBuffer, VCPP_DEBUG_BAD_POINTER_VALUE);
    VASSERT_GREATER_THAN_OR_EQUAL(mBufferSize, 0);
    VASSERT_LESS_THAN_OR_EQUAL(mEOFOffset, mBufferSize);
    VASSERT_LESS_THAN_OR_EQUAL(mIOOffset, mEOFOffset);
    VASSERT_VALUE((mAllocationType == kAllocatedByOperatorNew || mAllocationType == kAllocatedByMalloc || mAllocationType == kAllocatedOnStack || mAllocationType == kAllocatedUnknown), mAllocationType, VSTRING_INT((int)mAllocationType));
}

bool operator==(const VMemoryStream& m1, const VMemoryStream& m2) {
    Vs64 length = m1.getEOFOffset();

    // If the lengths don't match, we consider the streams not equal.
    if (m2.getEOFOffset() != length) {
        return false;
    }

    // If the length is zero, then we consider the streams equal and don't
    // need to call memcmp.
    if (length == 0) {
        return true;
    }

    // If the lengths fit size_t or size_t is 64 bits, we can just call memcmp once.
    if (! VStream::needSizeConversion(length)) {
        return ::memcmp(m1.getBuffer(), m2.getBuffer(), static_cast<size_t>(m1.getEOFOffset())) == 0;
    }

    /*
    If we get here, then we have too much data for memcmp to handle in one
    shot. We need to loop over chunks of data sized that memcmp can handle,
    until we finish or find an inequality.
    */
    bool    equalSoFar = true;
    Vs64    numBytesRemaining = length;
    Vs64    offset = 0;
    size_t  compareChunkSize;

    do {
        compareChunkSize = static_cast<size_t>(V_MIN(V_MAX_S32, numBytesRemaining));

        equalSoFar = ::memcmp(m1.getBuffer() + offset, m2.getBuffer() + offset, compareChunkSize) == 0;

        numBytesRemaining -= compareChunkSize;

    } while (equalSoFar && numBytesRemaining > 0);

    return equalSoFar;
}

// VReadOnlyMemoryStream --------------------------------------------------------------------------

VReadOnlyMemoryStream::VReadOnlyMemoryStream(Vu8* buffer, Vs64 suppliedEOFOffset) :
    VMemoryStream(buffer, kAllocatedUnknown, false, suppliedEOFOffset, suppliedEOFOffset) {
}

VReadOnlyMemoryStream::VReadOnlyMemoryStream(const VReadOnlyMemoryStream& other) :
    VMemoryStream(other) {
}

VReadOnlyMemoryStream& VReadOnlyMemoryStream::operator=(const VReadOnlyMemoryStream& other) {
    VMemoryStream::operator=(other);
    return *this;
}

void VReadOnlyMemoryStream::adoptBuffer(Vu8* buffer, BufferAllocationType allocationType, Vs64 suppliedBufferSize, Vs64 suppliedEOFOffset) {
    VMemoryStream::adoptBuffer(buffer, allocationType, false, suppliedBufferSize, suppliedEOFOffset);
}

Vs64 VReadOnlyMemoryStream::write(const Vu8* /*buffer*/, Vs64 /*numBytesToWrite*/) {
    throw VEOFException("VReadOnlyMemoryStream::write: Invalid attempt to write to a read-only stream.");
}

void VReadOnlyMemoryStream::flush() {
    throw VEOFException("VReadOnlyMemoryStream::flush: Invalid attempt to flush a read-only stream.");
}

