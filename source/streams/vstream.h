/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vstream_h
#define vstream_h

/** @file */

#include "vstring.h"

/**

    @defgroup vstreams Vault Streams

    <h3>Vault Streams Architecture</h3>

    The stream facilities provided by the Vault are extremely easy to use,
    and provide a clean and simple architecture for stream i/o.
    There are two layers in the stream architecture. Usually you will
    talk to the upper layer, even if you have to instantiate lowerx
    layer objects to set up the stream.

    <h4>Lower Layer: Raw Streams</h4>

    The lower layer concerns the transport over which the stream is
    carried. The Vault supplies implementations for files, sockets,
    and memory. The base class is VStream, and its primary function is to read
    and write bytes, to seek (as allowed), and to flush. The concrete subclasses
    provide the actual i/o implementation that works over the particular
    transport. Data at this stream layer is untyped -- it's all just
    bytes. You'll instantiate either a VBufferedFileStream, VDirectIOFileStream,
    VSocketStream, or VMemoryStream, either directly or indirectly. In
    addition, the class VWriteBufferedStream lets you buffer writes to a
    stream such as VSocketStream that doesn't buffer data on its own.
    VMemoryStream uses a memory buffer to hold the stream data; you don't have
    to worry about writing past the end of the buffer: it expands as
    necessary. Regarding VBufferedFileStream vs. VDirectIOFileStream, you should
    generally use VBufferedFileStream, unless you need the particular
    behavior of VDirectIOFileStream, because VBufferedFileStream uses the platform
    APIs that give better file i/o performance.

    <h4>Upper Layer: Formatted Streams</h4>

    The upper layer is the one you will most often use directly. This
    layer concerns the format of the data stream, independent of the
    lower layer transport that the stream is carried over. The Vault
    supplies implementations for binary i/o and text/line-based i/o.
    You'll instantiate either a VBinaryIOStream or VTextIOStream and
    associate with a lower layer stream object (discussed above), and
    then you'll use the upper layer stream object's APIs for reading
    and writing data in the appropriate format. For example, with
    binary i/o, you read and write typed data such as: integers of
    particular sizes; floating-point values; booleans; strings; time
    values (instants). In contrast, with text i/o, you read and write
    lines of text as strings; the text i/o class manages the details
    of how text file line endings differ across platforms, and the fact
    that you might be reading a file on one platform that was created
    on a different platform.

    <h4>Stream Copying</h4>

    Copying between streams is made trivial by the overloaded streamCopy
    methods of VStream and VIOStream. These methods allow you to copy
    data from one stream to another regardless of the type of stream, and
    regardless of whether you have a reference to a lower layer VStream
    object or an upper layer VIOStream object. The implementation ensures
    that no extra copying is performed -- if you are doing i/o where one
    stream (source or destination) is a file or socket, and the other
    stream is a memory buffer, the copying is done directly to/from the
    buffer with zero overhead. Yet you can supply a pair of non-memory-based
    streams (file and socket) and a temporary buffer will be used to
    perform the copy without requiring you to deal with the details (although
    you may specify the buffer size explicitly if the default size is not
    ideal).

    The semantics of stream copying are simple: the source is read
    and the destination is written. So the stream position moves as
    you'd expect. If you need to copy without performing the entire
    copy in a single blocking function call, you can use the
    helper class VStreamCopier, which allows you to copy in chunks and
    thus be aware of the progress of the copy; an example of where this
    is useful is if you are copying a large amount of data and need to
    keep some UI feedback updated as the copy progresses.

    <h3>Summary</h3>

    - You'll use a class derived from VStream to specify what kind of
    transport the data is carried on: VBufferedFileStream, VSocketStream, or
    VMemoryStream. (Or in certain circumstances, VDirectIOFileStream or
    VWriteBufferedStream.)

    - You'll most directly use a class derived from VIOStream to read
    and write your data. If you are
    working with text, use VTextIOStream. If you are working with binary
    data, use VBinaryIOStream. When you instantiate the object, you'll supply
    it the lower level stream object to use as a transport.

*/

// Allows us to declare our static streamCopy functions here.
class VIOStream;

/**
    @defgroup vstream_derived Raw Streams (lower layer)

    These are the stream classes that you use to specify what sort of
    physical transport the stream data is carried over -- file, socket,
    or memory. Usually, you'll use an upper layer stream object to
    format the data, rather than directly perform i/o on the lower
    layer stream.

    @ingroup vstreams
    @{
*/

/**
VStream is an abstract base class that defines a stream-oriented i/o API.

You will generally use VSocketStream for socket i/o, VBufferedFileStream for file
i/o, and VMemoryStream for memory i/o. VStream also defines and implements a
static function streamCopy() for efficiently copying data between any two
streams, no matter their types; this is useful for doing zero-copy file and
socket i/o, or buffering such i/o, without having to specialize the code.

However, it is generally better to use one of the VIOStream-derived classes
to do your actual i/o and avoid calling VStream methods directly. The
VIOStream classes take a VStream in their constructors and use that stream,
letting you do more well-typed i/o, as well as the same operations provided
here. For binary data you will use VBinaryIOStream, and for text data (such
as a text file or a line-oriented wire protocol) you will use VTextIOStream.

There are two ways to read raw bytes. The readGuaranteed() method will throw
an exception if the stream does not contain the requested number of bytes. The
read() method will return the actual number of bytes that were read.
readGuaranteed() is implemented by calling read() and throwing if it returns
an unsatisfactory number of bytes.

You can also write raw bytes. After you have completed your sequence of
writes, you should call flush to allow special classes to clean up or write
whatever they may have buffered.

Depending on the actual type of stream, you may be able to seek around
in the stream. Here are some examples of which streams allow what kind
of seeks. Really, the only limitations are obvious: on socket streams you
cannot seek anywhere but forward relative to the current position. If
you need to do some kind of pseudo-random-access i/o on a socket stream,
then you'll just need to buffer it (using a VMemoryStream would make this
very easy).

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

If you just want to skip over some bytes while reading, use the skip()
method, which is equivalent to SEEK_CUR forward, and works on all
streams. Unfortunately, because of how the different underlying native
implementations differ, you can only get a boolean return value
indicating whether you could succesfully seek to the desired location.

@see    VBinaryIOStream
@see    VTextIOStream
@see    VMemoryStream
@see    VAbstractFileStream
@see    VBufferedFileStream
@see    VDirectIOFileStream
@see    VSocketStream
*/
class VStream {
    public:

        /**
        Default constructor.
        */
        VStream();
        /**
        Constructor that gives the stream a name for debugging purposes.
        */
        VStream(const VString& name);
        /**
        Destructor.
        */
        virtual ~VStream() {}

        /**
        Reads a specified number of bytes from the stream, and throws a
        VException if they cannot be read.
        @param    targetBuffer    the buffer to read into
        @param    numBytesToRead    the number of bytes to read
        */
        void readGuaranteed(Vu8* targetBuffer, Vs64 numBytesToRead);
        /**
        Reads one byte from the stream, and throws a VException if it cannot be read.
        @return the byte that was read
        */
        Vu8 readGuaranteedByte();
        /**
        Attempts to read a specified number of bytes from the stream.
        @param    targetBuffer    the buffer to read into
        @param    numBytesToRead    the number of bytes to read
        @return    the actual number of bytes that could be read
        */
        virtual Vs64 read(Vu8* targetBuffer, Vs64 numBytesToRead) = 0;

        /**
        Writes bytes to the stream.
        @param    buffer            the buffer containing the data
        @param    numBytesToWrite    the number of bytes to write to the stream
        @return the actual number of bytes written
        */
        virtual Vs64 write(const Vu8* buffer, Vs64 numBytesToWrite) = 0;
        /**
        Flushes any pending or buffered write data to the stream. Until you
        call flush, you cannot guarantee that your data has actually been
        written to the underlying physical stream.
        */
        virtual void flush() = 0;

        /**
        Skips forward in the stream a specified number of bytes. For memory
        and file streams, this means advancing the i/o offset by the specified
        number of bytes; for socket streams, this means reading and discarding
        the specified number of bytes.
        @param    numBytesToSkip    the number of bytes to skip
        */
        virtual bool skip(Vs64 numBytesToSkip) = 0;
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
        virtual bool seek(Vs64 offset, int whence) = 0;
        /**
        This is a convenience function that means seek(0, SEEK_SET).
        In other words, seek to the start of the stream.
        It has the same restrictions as noted in seek() above.
        @return true if the seek was successful
        */
        bool seek0() { return this->seek(0, SEEK_SET); }
        /**
        Returns the "current" "offset" in the stream. Those scare quotes are
        there because those terms do not quite have consistent or uniform
        meaning and behavior for all stream types, so you need to be a little
        careful in using this feature. For file streams, the current offset
        is simply the i/o mark relative to the start of the file. For memory
        streams, the current offset is also the i/o mark, relative to the start
        of the buffer. But for socket streams, which have no buffer,
        the current offset is simply the accumulated number of bytes that
        have been read; when used with a socket stream, this
        function is most useful as a way to determine how much data you have
        read since last looking at the offset, without having to keep track of
        each individual read operation (which you might not be able to).
        @return the current offset
        */
        virtual Vs64 getIOOffset() const = 0;
        /**
        Returns the number of bytes that are available to be read from this
        stream. For file and memory streams, this means the number of bytes
        from the current i/o mark until the end of the file or buffer. For
        socket streams, this means the number of bytes that can be read
        without blocking (that is, the number of bytes that are waiting to
        be read on the socket at this time).
        @return the number of bytes currently available for reading
        */
        virtual Vs64 available() const = 0;

        /**
        Efficiently copies bytes from one stream to another, no matter which
        concrete stream types are being used. Some examples of using it
        include reading a file into memory (fromStream is VAbstractFileStream-derived,
        toStream is a VMemoryStream), writing from memory to a socket
        (fromStream is a VMemoryStream, toStream is a VSocketStream), and
        transferring a file to a socket (fromStream is VAbstractFileStream-derived,
        toStream is a VSocketStream).

        If either of the streams is a VMemoryStream, the copy is made
        directly with no extra copying. If neither stream is a VMemoryStream,
        a temporary buffer is used to transfer the data with just a single
        copy.

        Of course, this method does not actually know the stream classes,
        but simply asks the to and from streams about their capabilities.

        @param    fromStream    the source stream that is read
        @param    toStream    the target stream that is written
        @param    numBytesToCopy    the number of bytes read from fromStream and write to toStream
        @param    tempBufferSize    the size of temporary buffer to create, if one is needed
        @return the actual number of bytes copied
        */
        static Vs64 streamCopy(VStream& fromStream, VStream& toStream, Vs64 numBytesToCopy, Vs64 tempBufferSize = 16384);
        static Vs64 streamCopy(VIOStream& fromStream, VIOStream& toStream, Vs64 numBytesToCopy, Vs64 tempBufferSize = 16384);
        static Vs64 streamCopy(VIOStream& fromStream, VStream& toStream, Vs64 numBytesToCopy, Vs64 tempBufferSize = 16384);
        static Vs64 streamCopy(VStream& fromStream, VIOStream& toStream, Vs64 numBytesToCopy, Vs64 tempBufferSize = 16384);

        friend class VWriteBufferedStream;

        /**
        Returns the name of the stream that it was given when constructed.
        The name is just an arbitrary string that can be useful when debugging
        to distinguish between different streams' logging output.
        @return the stream name
        */
        const VString& getName() const { return mName; }
        /**
        Sets the stream name.
        The name is just an arbitrary string that can be useful when debugging
        to distinguish between different streams' logging output.
        @param name the name with which to label the stream
        */
        void setName(const VString& name) { mName = name; }

        /**
        Returns true if the specified size value requires conversion given the compiler's
        definition of size_t and the actual value given. If size_t is 64 bits, or if the
        given value fits in 32 bits, then no conversion is necessary, and an operation
        that takes a size_t can be called with a simple cast; otherwise the caller needs
        to perform the operation in a loop, processing 32 bits worth of data at a time.
        This is used internally by various Code Vault stream and i/o functions that
        provide a consistent 64-bit wide size parameter interface, but need to work on
        OS's and compilers that might only provide 32 bits of size for certain functions such as
        read, write, copy, etc.
        @param    sizeValue    the value that is being used to describe a size
        @return true if the size won't fit in a size_t
        */
        static bool needSizeConversion(Vs64 sizeValue);
        /**
        A direct replacement for memcpy() that allows for 64-bit lengths that we
        use everywhere, and deals with situations
        where the native sizes used by memcpy() are smaller. This could even have
        optimized versions per platform if the platform supports a native API that
        is better than memcpy().
        */
        static void copyMemory(Vu8* toBuffer, const Vu8* fromBuffer, Vs64 numBytesToCopy);
        /**
        The compiler- and size-safe way to do new[bufferSize] when the buffer size
        is a Vs64. If size_t is 64 bits then it just does new. Otherwise, if the
        requested buffer size fits in 32 bits the it also does new. Otherwise,
        (size_t is 32 bits but requested size needs 64 bits) then it throws a
        std::bad_alloc just like new would if it ran out of memory.
        @param    bufferSize    the requested buffer size
        @return a pointer to the newly allocated buffer, allocated with new[bufferSize]
        */
        static Vu8* newNewBuffer(Vs64 bufferSize);
        /**
        This is provided for consistency with newNewBuffer() when a buffer needs to
        be allocated with malloc() instead of operator new.
        */
        static Vu8* mallocNewBuffer(Vs64 bufferSize);

    protected:

        /*
        These methods are ONLY overridden by buffer-based subclasses,
        for example VMemoryStream. They are called by the friend function
        streamCopy() so that it can efficiently copy data directly to/from
        streams that have data buffers (namely, VMemoryStream).
        */

        /**
        Returns a pointer to the current read i/o position in the stream's buffer,
        or NULL if the stream does not have a buffer or support direct copying
        from that buffer (for example, a file or socket stream).
        @return    the i/o buffer pointer, or NULL
        */
        virtual Vu8* _getReadIOPtr() const;
        /**
        Returns a pointer to the current write i/o position in the stream's buffer,
        or NULL if the stream does not have a buffer or support direct copying
        to that buffer (for example, a file or socket stream).
        @return    the i/o buffer pointer, or NULL
        */
        virtual Vu8* _getWriteIOPtr() const;
        /**
        Returns the number of bytes available for reading from the stream's
        buffer, or zero by default for streams without buffers.
        @param    numBytesToRead    the number of bytes that will be read
        @return    the number of bytes available to read, or zero
        */
        virtual Vs64 _prepareToRead(Vs64 numBytesToRead) const;
        /**
        Preflights the stream's buffer so that it can have the specified
        number of bytes written to it subsequently. Throws a VException
        if the buffer cannot be expanded to accomodate the data.
        @param    numBytesToWrite    the number of bytes that will be written
        */
        virtual void _prepareToWrite(Vs64 numBytesToWrite);
        /**
        Postflights a copy by advancing the i/o offset to reflect
        the specified number of bytes having just been read.
        @param    numBytesRead    the number of bytes that were previously read
        */
        virtual void _finishRead(Vs64 numBytesRead);
        /**
        Postflights a copy by advancing the i/o offset to reflect
        the specified number of bytes having just been written.
        @param    numBytesWritten    the number of bytes that were previously written
        */
        virtual void    _finishWrite(Vs64 numBytesWritten);

        VString mName; ///< A name for use when debugging stream.
};

/** @} */

#endif /* vstream_h */
