/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

#ifndef vwritebufferedstream_h
#define vwritebufferedstream_h

/** @file */

#include "vmemorystream.h"

/**
    @ingroup vstream_derived
*/

/**
VWriteBufferedStream is a helper class that buffers writes until a flush
is issued; reads are not allowed. You may seek() within the written data
until it is flushed; once a chunk of data is flushed, the buffered part
of the stream is reset to empty, so you can't seek in it. The idea is that
you may do a series of writes and seeks, followed by a flush, which appends
all pending data to the underlying raw stream.

Thus you must instantiate a raw stream and supply it to the VWriteBufferedStream
so it has an actual stream to do i/o on. You will then typically instantiate
a VIOStream-derived object for i/o, and construct it with the VWriteBufferedStream.

@see    VBinaryIOStream
@see    VTextIOStream
@see    VMemoryStream
@see    VFileStream
@see    VSocketStream
*/
class VWriteBufferedStream : public VMemoryStream
    {
    public:
    
        /**
        Constructor.
        */
        VWriteBufferedStream(VStream& rawStream, Vs64 initialBufferSize=VMemoryStream::kDefaultBufferSize, Vs64 resizeIncrement=VMemoryStream::kIncrement2x);
        /**
        Destructor.
        */
        virtual ~VWriteBufferedStream() {}
        
        /**
        Overrides VMemoryStream::read; throws a VException.
        @param    targetBuffer    the buffer to read into
        @param    numBytesToRead    the number of bytes to read
        @return    the actual number of bytes that could be read
        */
        virtual Vs64    read(Vu8* targetBuffer, Vs64 numBytesToRead);
        
        /**
        Overrides VMemoryStream::flush in order to copy the buffered data
        to the raw stream.
        */
        virtual void    flush();
        
        /**
        Overrides VMemoryStream::skip; throws a VException.
        @param    numBytesToSkip    the number of bytes to skip
        */
        virtual bool    skip(Vs64 numBytesToSkip);
        
        // FIXME: need to define the semantics of tell() and seek() for
        // this subclass. For the moment, we'll do like seek/skip and
        // call through to the raw stream, though I'm not sure it should
        // really be like that.
        
    private:
        
        VStream&        mRawStream;        ///< The raw stream we eventually flush to.
    };

#endif /* vwritebufferedstream_h */
