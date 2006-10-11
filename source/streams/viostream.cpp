/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vexception.h"
#include "viostream.h"
#include "vstream.h"

VIOStream::VIOStream(VStream& rawStream) :
mRawStream(rawStream)
    {
    }

void VIOStream::readGuaranteed(Vu8* targetBuffer, Vs64 numBytesToRead)
    {
    mRawStream.readGuaranteed(targetBuffer, numBytesToRead);
    }

Vs64 VIOStream::read(Vu8* targetBuffer, Vs64 numBytesToRead)
    {
    return mRawStream.read(targetBuffer, numBytesToRead);
    }

Vs64 VIOStream::write(const Vu8* buffer, Vs64 numBytesToWrite)
    {
    return mRawStream.write(buffer, numBytesToWrite);
    }

void VIOStream::flush()
    {
    mRawStream.flush();
    }

bool VIOStream::skip(Vs64 numBytesToSkip)
    {
    return mRawStream.skip(numBytesToSkip);
    }

bool VIOStream::seek(Vs64 inOffset, int whence)
    {
    return mRawStream.seek(inOffset, whence);
    }

Vs64 VIOStream::offset() const
    {
    return mRawStream.offset();
    }

Vs64 VIOStream::available() const
    {
    return mRawStream.available();
    }

VStream& VIOStream::rawStream()
    {
    return mRawStream;
    }

// static
Vs64 /*VIOStream::*/streamCopy(VIOStream& fromStream, VIOStream& toStream, Vs64 numBytesToCopy, Vs64 tempBufferSize)
    {
    return /*VStream::*/streamCopy(fromStream.rawStream(), toStream.rawStream(), numBytesToCopy, tempBufferSize);
    }

// static
Vs64 /*VIOStream::*/streamCopy(VIOStream& fromStream, VStream& toStream, Vs64 numBytesToCopy, Vs64 tempBufferSize)
    {
    return /*VStream::*/streamCopy(fromStream.rawStream(), toStream, numBytesToCopy, tempBufferSize);
    }

// static
Vs64 /*VIOStream::*/streamCopy(VStream& fromStream, VIOStream& toStream, Vs64 numBytesToCopy, Vs64 tempBufferSize)
    {
    return /*VStream::*/streamCopy(fromStream, toStream.rawStream(), numBytesToCopy, tempBufferSize);
    }

// static
Vs16 VIOStream::streamCompare(VIOStream& streamA, VIOStream& streamB, Vs64 numBytesToCompare)
    {
    Vs64 offsetA = streamA.offset();
    Vs64 offsetB = streamB.offset();
    
    Vs16 compareResult = 0;        // Assume equal until we know better
    while (numBytesToCompare-- > 0)
        {
            Vu8 byteA, byteB;
            bool aEnded = false, bEnded = false;
            
            try
                {
                streamA.read(&byteA, CONST_S64(1));
                }
            catch (const VEOFException& /*ex*/)
                {
                aEnded = true;
                }
                
            try
                {
                streamB.read(&byteB, CONST_S64(1));
                }
            catch (const VEOFException& /*ex*/)
                {
                bEnded = true;
                }
                
            if (aEnded && bEnded)
                break;
            else if (aEnded)
                {
                compareResult = -1;
                break;
                }
            else if (bEnded)
                {
                compareResult = 1;
                break;
                }
                
            if (byteA < byteB)
                {
                compareResult = -1;
                break;
                }
            else if (byteA > byteB)
                {
                compareResult = 1;
                break;
                }
        }
    
    streamA.seek(offsetA, SEEK_SET);
    streamB.seek(offsetB, SEEK_SET);
    
    return compareResult;
    }
