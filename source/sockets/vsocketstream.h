/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

#ifndef vsocketstream_h
#define vsocketstream_h

/** @file */

#include "vstream.h"

class VSocket;

/**
    @ingroup vstream_derived vsocket
*/

/**
VSocketStream is a concrete VStream class that knows how to do i/o on
a VSocket.

It is recommended to use a VIOStream object rather than read/write on
a VSocketStream directly.

@see    VIOStream
@see    VBinaryIOStream
@see    VTextIOStream
*/
class VSocketStream : public VStream {
    public:

        /**
        Empty constructor for use with a subsequent call to setSocket().
        @param    name    an arbitrary name given to this stream
        */
        VSocketStream(const VString& name);
        /**
        Constructs a VSocketStream to use a specified VSocket.
        @param    socket    the socket to do i/o on
        @param    name    an arbitrary name given to this stream
        */
        VSocketStream(VSocket* socket, const VString& name);
        /**
        Copy constructor. Both streams will share the same socket unless a
        call to setSocket() is subsequently made on one of them.
        */
        VSocketStream(const VSocketStream& other);
        /**
        Destructor.
        */
        virtual ~VSocketStream() {}

        /**
        Assignment operator. Both streams will share the same socket unless a
        call to setSocket() is subsequently made on one of them.
        */
        VSocketStream& operator=(const VSocketStream& other);

        /**
        Returns a pointer to the VSocket object used by this VSocketStream.
        @return    the VSocket this stream is doing i/o on
        */
        VSocket* getSocket() const;
        /**
        Assigns a new VSocket object for this stream to do i/o on.
        @param    socket    the stream to do subsequent i/o on
        */
        void setSocket(VSocket* socket);

        // Required VStream method overrides:

        /**
        Reads bytes from the stream into a buffer.
        @param    targetBuffer    the buffer to read into
        @param    numBytesToRead    the number of bytes to read
        @return    the number of bytes actually read
        */
        virtual Vs64 read(Vu8* targetBuffer, Vs64 numBytesToRead);
        /**
        Writes bytes from a buffer into the stream.
        @param    buffer            the buffer to read from
        @param    numBytesToWrite    the number of bytes to write
        @return    the number of bytes actually written
        */
        virtual Vs64 write(const Vu8* buffer, Vs64 numBytesToWrite);
        /**
        Flushes any pending write data to the underlying stream.
        */
        virtual void flush();
        /**
        Skips over a number of bytes in the input (read) stream.
        @param    numBytesToSkip    the number of bytes to skip over
        */
        virtual bool skip(Vs64 numBytesToSkip);
        /**
        Seeks in the stream; note that VSocketStream only supports seeking
        forward (offset >= 0, whence = SEEK_CUR) and will throw a VException
        if the caller requests an illegal seek operation.

        @param  offset  the offset (meaning depends on whence param)
        @param  whence  SEEK_SET, SEEK_CUR, or SEEK_END
        @return true if the seek was successful
        */
        virtual bool seek(Vs64 offset, int whence);
        /**
        Returns the current offset in the stream. For file streams, this
        returns the number of accumulated bytes read and/or written.
        @return the current offset
        */
        virtual Vs64 getIOOffset() const;
        /**
        Returns the number of bytes that are available to be read from this
        stream. For file and memory streams, this means the number of bytes
        from the current i/o mark until the end of the file or buffer. For
        socket streams, this means the number of bytes that can be read
        without blocking (that is, the number of bytes that are waiting to
        be read on the socket at this time).
        @return the number of bytes currently available for reading
        */
        virtual Vs64 available() const;

    private:

        VSocket* mSocket;   ///< The socket on which this stream does its i/o.
};

#endif /* vsocketstream_h */

