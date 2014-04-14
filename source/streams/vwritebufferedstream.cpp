/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
*/

/** @file */

#include "vwritebufferedstream.h"

#include "vexception.h"

VWriteBufferedStream::VWriteBufferedStream(VStream& rawStream, Vs64 initialBufferSize, Vs64 resizeIncrement)
    : VMemoryStream(initialBufferSize, resizeIncrement)
    , mRawStream(rawStream)
    {
}

Vs64 VWriteBufferedStream::read(Vu8* /*targetBuffer*/, Vs64 /*numBytesToRead*/) {
    throw VUnimplementedException("VWriteBufferedStream::read: Read is not permitted on buffered write stream.");
}

void VWriteBufferedStream::flush() {
    // Flush the complete contents of our buffer to the raw stream.
    this->seek(SEEK_SET, 0);    // set our i/o offset back to 0
    (void) VStream::streamCopy(*this, mRawStream, this->getEOFOffset());

    // Reset ourself to be "empty" and at i/o offset 0.
    this->seek(SEEK_SET, 0);
    this->setEOF(0);
}

bool VWriteBufferedStream::skip(Vs64 /*numBytesToSkip*/) {
    throw VUnimplementedException("VWriteBufferedStream::skip: Skip is not permitted on buffered write stream.");
}

