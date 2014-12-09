/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

#ifndef viostream_h
#define viostream_h

/** @file */

#include "vtypes.h"

class VStream;

/**
    @defgroup viostream_derived Formatted Streams (upper layer)

    These are the stream classes that you use to read and write formatted
    data -- either text- or binary-oriented data -- regardless of what
    transport it is carried over. When you construct an upper layer stream,
    you supply an instance of a lower layer stream, over which the upper
    layer stream will read and write the formatted data.

    @ingroup vstreams
    @{
*/

/**
VIOStream is an abstract base class from which classes derive that perform
well-typed I/O on raw streams.

This base class merely delegates its methods to the underlying raw stream.
Subclasses provide well-typed read and write APIs that call the base class
methods. So you will typically not instantiate a VIOStream (there is no point
to doing so!) and will instead instantiate a VBinaryIOStream or a
VTextIOStream.

@see VBinaryIOStream
@see VTextIOStream
*/
class VIOStream {
    public:

        /**
        Constructs the object with an underlying raw stream.
        @param    rawStream    the raw stream on which I/O will be performed
        */
        VIOStream(VStream& rawStream);
        /**
        Destructor.
        */
        virtual ~VIOStream() {}

        /**
        Reads a specified number of bytes from the stream, and throws a
        VException if they cannot be read.
        @param    targetBuffer    the buffer to read into
        @param    numBytesToRead    the number of bytes to read
        */
        virtual void readGuaranteed(Vu8* targetBuffer, Vs64 numBytesToRead);
        /**
        Reads one byte from the stream, and throws a VException if it cannot be read.
        @return the byte that was read
        */
        virtual Vu8 readGuaranteedByte();
        /**
        Attempts to read a specified number of bytes from the stream.
        @param    targetBuffer    the buffer to read into
        @param    numBytesToRead    the number of bytes to read
        @return    the actual number of bytes that could be read
        */
        Vs64 read(Vu8* targetBuffer, Vs64 numBytesToRead);

        /**
        Writes bytes to the stream.
        @param    buffer            the buffer containing the data
        @param    numBytesToWrite    the number of bytes to write to the stream
        @return the actual number of bytes written
        */
        Vs64 write(const Vu8* buffer, Vs64 numBytesToWrite);
        /**
        Flushes any pending or buffered write data to the stream. Until you
        call flush, you cannot guarantee that your data has actually been
        written to the underlying physical stream.
        */
        void flush();

        /**
        Skips forward in the stream a specified number of bytes. For memory
        and file streams, this means advancing the i/o offset by the specified
        number of bytes; for socket streams, this means reading and discarding
        the specified number of bytes.
        @param    numBytesToSkip    the number of bytes to skip
        */
        bool skip(Vs64 numBytesToSkip);
        /**
        Seeks in the stream using Unix seek() semantics. VSocketStream has
        some restrictions in the kinds of seek that are allowed; if you
        specify an illegal socket seek operation, a VException is thrown.

        The following table shows the valid seek parameters for the different
        stream types:

        <table>
        <tr>
            <td>&nbsp;</td>
            <td>SEEK_SET</td>
            <td>SEEK_CUR offset>=0</td>
            <td>SEEK_CUR offset<0</td>
            <td>SEEK_END</td>
        </tr>
        <tr>
            <td>VMemoryStream</td>
            <td>yes</td>
            <td>yes</td>
            <td>yes</td>
            <td>yes</td>
        </tr>
        <tr>
            <td>VAbstractFileStream-derived</td>
            <td>yes</td>
            <td>yes</td>
            <td>yes</td>
            <td>yes</td>
        </tr>
        <tr>
            <td>VSocketStream</td>
            <td>no</td>
            <td>yes</td>
            <td>no</td>
            <td>no</td>
        </tr>
        </table>

        @param    offset    the offset, meaning depends on whence value
        @param    whence    SEEK_SET, SEEK_CUR, or SEEK_END
        @return true if the seek was successful
        */
        bool seek(Vs64 offset, int whence);
        /**
        This is a convenience function that means seek(0, SEEK_SET).
        In other words, seek to the start of the stream.
        It has the same restrictions as noted in seek() above.
        @return true if the seek was successful
        */
        bool seek0();
        /**
        Returns the "current" "offset" in the stream. Those scare quotes are
        there because those terms do not quite have consistent or uniform
        meaning and behavior for all stream types, so you need to be a little
        careful in using this feature. For buffered file streams, the current
        offset is simply the i/o mark relative to the start of the file. For
        memory streams, the current offset is also the i/o mark, relative to
        the start of the buffer. But for socket streams and unbuffered file
        streams, there are limitations. Socket streams have no buffer, so
        the current offset is simply the accumulated number of bytes that
        have been read and/or written; when used with a socket stream, this
        function is most useful as a way to determine how much data you have
        read since last looking at the offset, without having to keep track of
        each individual read operation (which you might not be able to).
        Unbuffered file streams have no ability to determine the current i/o
        mark, yet allow seeking; so there is no way for the class to keep
        track of the current offset on its own; so, for unbuffered file streams,
        getIOOffset() throws an exception because it is an illegal operation.
        */
        Vs64 getIOOffset() const;
        /**
        Returns the number of bytes that are available to be read from this
        stream. For file and memory streams, this means the number of bytes
        from the current i/o mark until the end of the file or buffer. For
        socket streams, this means the number of bytes that can be read
        without blocking (that is, the number of bytes that are waiting to
        be read on the socket at this time).
        @return the number of bytes currently available for reading
        */
        Vs64 available() const;

        /**
        Returns a reference to the underlying raw stream; used by the friend
        function streamCopy() so it can call through to the corresponding
        VStream friend function and supply it the raw streams. Also used by
        some debugging functions.
        @return    a reference to the underlying raw stream
        */
        VStream& getRawStream();


        /**
        Compare 2 streams by ascii values. The streams will be restored to their current positions upon
        return.
        @return -1 if streamA < streamB, 0 if streamA == streamB, otherwise returns 1
        */
        static Vs16 streamCompare(VIOStream& streamA, VIOStream& streamB, Vs64 numBytesToCompare);

    protected:

        VStream& mRawStream;    ///< The underlying raw stream.

    private:

        // Prevent copy construction and assignment since there is no provision for sharing a raw stream.
        VIOStream(const VIOStream& other);
        VIOStream& operator=(const VIOStream& other);
};

/** @} */

#endif /* viostream_h */
