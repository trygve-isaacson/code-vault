/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vstream.h"

#include "vexception.h"

VStream::VStream()
// mName constructs to empty string
    {
    }

VStream::VStream(const VString& name) :
mName(name)
    {
    }

void VStream::readGuaranteed(Vu8* targetBuffer, Vs64 numBytesToRead)
    {
    Vs64    numBytesRead = this->read(targetBuffer, numBytesToRead);
    
    if (numBytesRead != numBytesToRead)
        throw VEOFException("VStream::readGuaranteed encountered end of stream.");
    }

// static
Vs64 /*VStream::*/streamCopy(VStream& fromStream, VStream& toStream, Vs64 numBytesToCopy, Vs64 tempBufferSize)
    {
    Vs64    numBytesCopied = 0;
    
    /*
    First we figure out which (if either) of the streams can give us a buffer
    pointer. Either or both of these may be NULL.
    */
    Vu8*    fromBuffer = fromStream._getReadIOPtr();
    Vu8*    toBuffer = toStream._getWriteIOPtr();
    
    /*
    If the source stream gave us a buffer to read from, we have to ask it
    how much data it really has, so we know how much we're really going to be
    copying.
    */
    if (fromBuffer != NULL)
        numBytesToCopy = fromStream._prepareToRead(numBytesToCopy);
    
    /*
    If the target stream gave us a buffer to write to, we have to ask it
    again after first giving it a chance to expand the buffer to fit the
    requested copy size.
    */
    if (toBuffer != NULL)
        {
        toStream._prepareToWrite(numBytesToCopy);
        toBuffer = toStream._getWriteIOPtr();
        }

    /*
    Now we can proceed with the copy. The matrix of possibities is the
    two possible sources (buffer or stream) and the two possible targets
    (buffer or stream). We handle each case optimally.
    */
    if ((fromBuffer == NULL) && (toBuffer != NULL))
        {
        // stream-to-buffer copy
        numBytesCopied = fromStream.read(toBuffer, numBytesToCopy);
        toStream._finishWrite(numBytesCopied);
        }
    else if ((fromBuffer != NULL) && (toBuffer == NULL))
        {
        // buffer-to-stream copy
        numBytesCopied = toStream.write(fromBuffer, numBytesToCopy);
        fromStream._finishRead(numBytesCopied);
        }
    else if ((fromBuffer != NULL) && (toBuffer != NULL))
        {
        // buffer-to-buffer copy
        VStream::copyMemory(toBuffer, fromBuffer, numBytesToCopy);
        numBytesCopied = numBytesToCopy;

        fromStream._finishRead(numBytesCopied);
        toStream._finishWrite(numBytesCopied);
        }
    else
        {
        /*
        Worst case scenario: direct copy between streams without their own
        buffers, so we have to create a buffer to do the transfer.
        */

        Vu8*    tempBuffer;
        Vs64    numBytesRemaining;
        Vs64    numTempBytesToCopy;
        Vs64    numTempBytesRead;
        Vs64    numTempBytesWritten;
        
        numBytesRemaining = numBytesToCopy;
        tempBufferSize = V_MIN(numBytesToCopy, tempBufferSize);

        tempBuffer = VStream::newBuffer(tempBufferSize);

        while (numBytesRemaining > 0)
            {
            numTempBytesToCopy = V_MIN(numBytesRemaining, tempBufferSize);
            
            numTempBytesRead = fromStream.read(tempBuffer, numTempBytesToCopy);

            // If we detect EOF, we're done.
            if (numTempBytesRead == 0)
                break;

            numTempBytesWritten = toStream.write(tempBuffer, numTempBytesRead);

            numBytesRemaining -= numTempBytesWritten;
            numBytesCopied += numTempBytesWritten;
            
            // If we couldn't write any bytes, we have a problem and should stop here.
            if (numTempBytesWritten == 0)
                break;
            }
        
        delete [] tempBuffer;
        }

    return numBytesCopied;
    }

const VString& VStream::name() const
    {
    return mName;
    }

// static
bool VStream::needSizeConversion(Vs64 sizeValue)
    {
    if ((sizeof(Vs64) != sizeof(size_t)) && (sizeValue > V_MAX_S32))
        return true;
    else
        return false;
    }

// static
void VStream::copyMemory(Vu8* toBuffer, const Vu8* fromBuffer, Vs64 numBytesToCopy)
    {
    /*
    The purpose of this function is simply to allow the full 64-bit length
    while remaining compatible with platforms where memcpy() only
    supports a 32-bit length. It's not just a matter of type conversion,
    because if the requested length actually exceeds size_t, then
    we must copy in a loop. (Very unlikely to actually occur in real-world
    scenarios, but we may as well do it correctly.)
    We do make one assumption: that size_t is at least 31 bits, that is
    that it can hold a number up to at least V_MAX_S32.
    If Vs64 and size_t are identical, or we're copying less than 2GB of data,
    then memcpy is fine!
    */
    
//    bool    needSizeConversion = ((sizeof(Vs64) != sizeof(size_t)) && (numBytesToCopy > (Vs64) V_MAX_S32));
    
    if (! VStream::needSizeConversion(numBytesToCopy))
        {
        // Entire copy can occur in a single call to memcpy.
        ::memcpy(toBuffer, fromBuffer, static_cast<size_t> (numBytesToCopy));
        }
    else
        {
        // Need to call memcpy multiple times because numBytesToCopy is too big.
        Vs64    numBytesRemaining = numBytesToCopy;
        size_t    copyChunkSize;
        Vu8*    toPtr = toBuffer;
        Vu8*    fromPtr = const_cast<Vu8*>(fromBuffer);    // we need to move the pointer, w/o altering anything it points to
        
        do
            {
            copyChunkSize = static_cast<size_t> (V_MIN(V_MAX_S32, numBytesRemaining));
            
            ::memcpy(toPtr, fromPtr, copyChunkSize);

            numBytesRemaining -= copyChunkSize;

            } while (numBytesRemaining > 0);
        }
    }

Vu8* VStream::newBuffer(Vs64 bufferSize)
    {
    bool    fits = ((sizeof(Vs64) == sizeof(size_t)) || (bufferSize <= V_MAX_S32));
    
    if (fits)
        return new Vu8[static_cast<size_t> (bufferSize)];
    else
        throw std::bad_alloc();
    }

Vu8* VStream::_getReadIOPtr() const
    {
    // To be overridden by memory-based streams.
    return NULL;
    }

Vu8* VStream::_getWriteIOPtr() const
    {
    // To be overridden by memory-based streams.
    return NULL;
    }

Vs64 VStream::_prepareToRead(Vs64 /*numBytesToRead*/) const
    {
    // To be overridden by memory-based streams.
    return 0;
    }

void VStream::_prepareToWrite(Vs64 /*numBytesToWrite*/)
    {
    // To be overridden by memory-based streams.
    }

void VStream::_finishRead(Vs64 /*numBytesRead*/)
    {
    // To be overridden by memory-based streams.
    }

void VStream::_finishWrite(Vs64 /*numBytesWritten*/)
    {
    // To be overridden by memory-based streams.
    }

