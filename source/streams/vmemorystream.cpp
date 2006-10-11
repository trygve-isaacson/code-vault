/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vmemorystream.h"

#include "vexception.h"

VMemoryStream::VMemoryStream(Vs64 initialBufferSize, Vs64 resizeIncrement) :
VStream(),
mBufferSize(initialBufferSize),
mIOOffset(0),
mEOFOffset(0),
mResizeIncrement(resizeIncrement),
mBuffer(NULL)
    {
    mBuffer = VStream::newBuffer(mBufferSize);

    if (mBuffer == NULL)
        throw VException("VMemoryStream::VMemoryStream failed to allocate buffer, size %lld.", mBufferSize);

    ASSERT_INVARIANT();
    }

VMemoryStream::VMemoryStream(Vu8* buffer, Vs64 suppliedBufferSize, Vs64 suppliedEOFOffset, Vs64 resizeIncrement) :
VStream(),
mBufferSize(suppliedBufferSize),
mIOOffset(0),
mEOFOffset(suppliedEOFOffset),
mResizeIncrement(resizeIncrement),
mBuffer(buffer)
    {
    ASSERT_INVARIANT();
    }

VMemoryStream::~VMemoryStream()
    {
    delete [] mBuffer;
    }

Vs64 VMemoryStream::read(Vu8* targetBuffer, Vs64 numBytesToRead)
    {
    ASSERT_INVARIANT();

    Vs64    bytesRemainingInBuffer = mEOFOffset - mIOOffset;
    Vs64    actualNumBytesToCopy = V_MIN(numBytesToRead, bytesRemainingInBuffer);
    
    VStream::copyMemory(targetBuffer, mBuffer + mIOOffset, actualNumBytesToCopy);
    
    this->_finishRead(actualNumBytesToCopy);

    ASSERT_INVARIANT();

    return actualNumBytesToCopy;
    }

Vs64 VMemoryStream::write(const Vu8* buffer, Vs64 numBytesToWrite)
    {
    ASSERT_INVARIANT();

    this->_prepareToWrite(numBytesToWrite);    // throws if we can't write requested size
    
    VStream::copyMemory(mBuffer + mIOOffset, buffer, numBytesToWrite);
    
    this->_finishWrite(numBytesToWrite);

    ASSERT_INVARIANT();

    return numBytesToWrite;
    }

void VMemoryStream::flush()
    {
    // Nothing to flush.
    }

bool VMemoryStream::skip(Vs64 numBytesToSkip)
    {
    ASSERT_INVARIANT();

    Vs64    bytesRemainingInBuffer = mEOFOffset - mIOOffset;
    Vs64    actualNumBytesToSkip = V_MIN(numBytesToSkip, bytesRemainingInBuffer);
    
    mIOOffset += actualNumBytesToSkip;

    ASSERT_INVARIANT();

    return (numBytesToSkip == actualNumBytesToSkip);
    }

bool VMemoryStream::seek(Vs64 inOffset, int whence)
    {
    ASSERT_INVARIANT();

    Vs64    requestedOffset;
    Vs64    constrainedOffset;

    switch (whence)
        {
        case SEEK_SET:
            requestedOffset = inOffset;
            break;

        case SEEK_CUR:
            requestedOffset = mIOOffset + inOffset;
            break;

        case SEEK_END:
            requestedOffset = mEOFOffset;
            break;
            
        default:
            requestedOffset = CONST_S64(0);
            break;
        }

    if (requestedOffset < 0)
        constrainedOffset = 0;
    else if (requestedOffset > mEOFOffset)
        {
        constrainedOffset = requestedOffset;

        // write zeroes as we extend the buffer
        Vs64 numZeroesToWrite = requestedOffset - mEOFOffset;
        Vs64 oldEOF = mEOFOffset;
        
        mIOOffset = mEOFOffset;
        this->_prepareToWrite(numZeroesToWrite);
    
        while (oldEOF < mEOFOffset)
            {
            *(mBuffer + mIOOffset) = 0;
            oldEOF++;
            }
            
        this->_finishWrite(numZeroesToWrite);
        }
    else
        constrainedOffset = requestedOffset;
    
    mIOOffset = constrainedOffset;
    
    ASSERT_INVARIANT();

    return (constrainedOffset == requestedOffset);
    }

Vs64 VMemoryStream::offset() const
    {
    ASSERT_INVARIANT();

    return mIOOffset;
    }

Vs64 VMemoryStream::available() const
    {
    return mEOFOffset - mIOOffset;
    }

Vu8* VMemoryStream::_getReadIOPtr() const
    {
    /*
    For VMemoryStream, there is no distinction between read and write
    i/o offset, so they are the same, and both read and write copying
    is supported.
    */
    ASSERT_INVARIANT();

    return mBuffer + mIOOffset;
    }

Vu8* VMemoryStream::_getWriteIOPtr() const
    {
    /*
    For VMemoryStream, there is no distinction between read and write
    i/o offset, so they are the same, and both read and write copying
    is supported.
    */
    ASSERT_INVARIANT();

    return mBuffer + mIOOffset;
    }

Vs64 VMemoryStream::_prepareToRead(Vs64 numBytesToRead) const
    {
    ASSERT_INVARIANT();

    Vs64    bytesRemainingInBuffer = mEOFOffset - mIOOffset;
    Vs64    actualNumBytesToRead = V_MIN(numBytesToRead, bytesRemainingInBuffer);
    
    return actualNumBytesToRead;
    }

void VMemoryStream::_prepareToWrite(Vs64 numBytesToWrite)
    {
    ASSERT_INVARIANT();

    Vs64    requiredBufferSize = mIOOffset + numBytesToWrite;
    
    if (requiredBufferSize > mBufferSize)
        {
        // Reallocate a larger buffer, copy old contents to it.
        Vs64    newBufferSize;
        
        if (mResizeIncrement == kIncrement2x)
            {
            // Double the size repeatedly until large enough.
            newBufferSize = mBufferSize;
            
            // Just in case caller was dumb enough to ask us to double
            // an initial size of zero, pick something...
            if (newBufferSize == 0)
                newBufferSize = 1024;

            do
                {
                newBufferSize = 2 * newBufferSize;
                } while (newBufferSize < requiredBufferSize);
            }
        else
            {
            // Grow to fit requirement, but if necessary round up to
            // align on increment boundary.
            newBufferSize = requiredBufferSize;
            
            Vs64    misalignment = newBufferSize % mResizeIncrement;

            if (misalignment != 0)
                newBufferSize += (mResizeIncrement - misalignment);
            }

        Vu8*    newBuffer = VStream::newBuffer(newBufferSize);
        
        // FIXME: is this check now superfluous because new throws rather than returning NULL? (used to call malloc)
        if (newBuffer == NULL)
            throw VException("VMemoryStream::_prepareToWrite failure expanding buffer size from %lld to %lld.", mBufferSize, newBufferSize);

        VStream::copyMemory(newBuffer, mBuffer, mEOFOffset);

        delete [] mBuffer;
        mBuffer = newBuffer;
        mBufferSize = newBufferSize;
        }

    ASSERT_INVARIANT();
    }

void VMemoryStream::_finishRead(Vs64 numBytesRead)
    {
    ASSERT_INVARIANT();

    mIOOffset += numBytesRead;

    ASSERT_INVARIANT();
    }

void VMemoryStream::_finishWrite(Vs64 numBytesWritten)
    {
    ASSERT_INVARIANT();

    mIOOffset += numBytesWritten;
    
    // If we advanced past "eof", move eof forward.
    if (mIOOffset > mEOFOffset)
        mEOFOffset = mIOOffset;

    ASSERT_INVARIANT();
    }

void VMemoryStream::adoptBuffer(Vu8* buffer, Vs64 suppliedBufferSize, Vs64 suppliedEOFOffset, bool deleteOldBuffer)
    {
    ASSERT_INVARIANT();

    if (deleteOldBuffer && (mBuffer != NULL))
        {
        delete [] mBuffer;
        mBuffer = NULL;
        }

    mBufferSize = suppliedBufferSize;
    mEOFOffset = suppliedEOFOffset;
    mIOOffset = 0;
    
    mBuffer = buffer;

    ASSERT_INVARIANT();
    }

Vu8* VMemoryStream::getBuffer() const
    {
    ASSERT_INVARIANT();

    return mBuffer;
    }

Vs64 VMemoryStream::bufferSize() const
    {
    ASSERT_INVARIANT();

    return mBufferSize;
    }

Vs64 VMemoryStream::eofOffset() const
    {
    ASSERT_INVARIANT();

    return mEOFOffset;
    }

Vs64 VMemoryStream::ioOffset() const
    {
    ASSERT_INVARIANT();

    return mIOOffset;
    }

void VMemoryStream::setEOF(Vs64 eofOffset)
    {
    ASSERT_INVARIANT();

    mEOFOffset = V_MIN(mBufferSize, eofOffset);
    mIOOffset = V_MIN(mEOFOffset, mIOOffset);

    ASSERT_INVARIANT();
    }

//lint -e421 Caution -- function 'abort(void)' is considered dangerous [MISRA Rule 126]"
void VMemoryStream::assertInvariant() const
    {
    V_ASSERT(mBuffer != NULL);
    V_ASSERT(mBufferSize >= 0);
    V_ASSERT(mEOFOffset <= mBufferSize);    // EOF cannot be beyond end of buffer (can be at next byte)
    V_ASSERT(mIOOffset <= mEOFOffset);    // IO offset cannot be past EOF
    }

bool operator==(const VMemoryStream& m1, const VMemoryStream& m2)
    {
    Vs64    length = m1.eofOffset();

    // If the lengths don't match, we consider the streams not equal.
    if (m2.eofOffset() != length)
        return false;
    
    // If the length is zero, then we consider the streams equal and don't
    // need to call memcmp.
    if (length == 0)
        return true;

    // If the lengths fit size_t or size_t is 64 bits, we can just call memcmp once.
    if (! VStream::needSizeConversion(length))
        return ::memcmp(m1.getBuffer(), m2.getBuffer(), static_cast<size_t>(m1.eofOffset())) == 0;

    /*
    If we get here, then we have too much data for memcmp to handle in one
    shot. We need to loop over chunks of data sized that memcmp can handle,
    until we finish or find an inequality.
    */
    bool    equalSoFar = true;
    Vs64    numBytesRemaining = length;
    Vs64    offset = 0;
    size_t  compareChunkSize;
    
    do
        {
        compareChunkSize = static_cast<size_t> (V_MIN(V_MAX_S32, numBytesRemaining));
        
        equalSoFar = ::memcmp(m1.getBuffer() + offset, m2.getBuffer() + offset, compareChunkSize) == 0;

        numBytesRemaining -= compareChunkSize;

        } while (equalSoFar && numBytesRemaining > 0);
    
    return equalSoFar;
    }

