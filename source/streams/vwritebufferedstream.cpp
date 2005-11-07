/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vwritebufferedstream.h"

#include "vexception.h"

VWriteBufferedStream::VWriteBufferedStream(VStream& rawStream, Vs64 initialBufferSize, Vs64 resizeIncrement)
: VMemoryStream(initialBufferSize, resizeIncrement), mRawStream(rawStream)
    {
    }

Vs64 VWriteBufferedStream::read(Vu8* /*targetBuffer*/, Vs64 /*numBytesToRead*/)
    {
    throw VException("VWriteBufferedStream::read: Read is not permitted on buffered write stream.");

    return CONST_S64(0);
    }

void VWriteBufferedStream::flush()
    {
    // Flush the complete contents of our buffer to the raw stream.
    this->seek(SEEK_SET, 0);    // set our i/o offset back to 0
    ::streamCopy(*this, mRawStream, this->eofOffset());
    
    // Reset ourself to be "empty" and at i/o offset 0.
    this->seek(SEEK_SET, 0);
    this->setEOF(0);
    }

bool VWriteBufferedStream::skip(Vs64 /*numBytesToSkip*/)
    {
    throw VException("VWriteBufferedStream::skip: Skip is not permitted on buffered write stream.");

    return false;
    }

