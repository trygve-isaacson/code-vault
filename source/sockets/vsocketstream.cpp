/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocketstream.h"

#include "vsocket.h"
#include "vexception.h"

VSocketStream::VSocketStream(const VString& name)
: VStream(name)
    {
    }

VSocketStream::VSocketStream(VSocket* socket, const VString& name)
: VStream(name)
    {
    mSocket = socket;
    }

VSocketStream::~VSocketStream()
    {
    }

VSocket* VSocketStream::getSocket() const
    {
    return mSocket;
    }

void VSocketStream::setSocket(VSocket* socket)
    {
    mSocket = socket;
    }

Vs64 VSocketStream::read(Vu8* targetBuffer, Vs64 numBytesToRead)
    {
    return mSocket->read(targetBuffer, static_cast<int> (numBytesToRead));
    }

Vs64 VSocketStream::write(const Vu8* buffer, Vs64 numBytesToWrite)
    {
    return mSocket->write(buffer, static_cast<int> (numBytesToWrite));
    }

void VSocketStream::flush()
    {
    mSocket->flush();
    }

bool VSocketStream::skip(Vs64 numBytesToSkip)
    {
    /*
    It seems like we could be a little more efficient either by
    reading in larger blocks if the skip amount is big enough.
    For now, just read individual bytes (it's probably being
    buffered anyway).
    */
    Vu8    aByte;
    
    for (Vs64 i = 0; i < numBytesToSkip; ++i)
        this->read(&aByte, 1);
        
    return true;
    }

bool VSocketStream::seek(Vs64 offset, int whence)
    {
    if ((whence != SEEK_CUR) || (offset < 0))
        throw VException("VSocketStream::seek received unsupported seek type.");

    return this->skip(offset);
    }

Vs64 VSocketStream::offset() const
    {
    return mSocket->numBytesRead();
    }

Vs64 VSocketStream::available() const
    {
    return mSocket->available();
    }

