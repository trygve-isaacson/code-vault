/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

#ifndef vmemorystream_h
#define vmemorystream_h

/** @file */

#include "vstream.h"

/**
    @ingroup vstream_derived
*/

/**
VMemoryStream is a concrete subclass of VStream, that provides stream i/o
to a buffer in memory; during writes the buffer will expand automatically
as necessary.

Normally VMemoryStream allocates its own buffer; however, you may supply it
with a buffer you have already allocated, though if VMemoryStream needs to
expand the buffer to accomodate a write, it will delete your buffer and
allocate a new one.

You specify a resize increment by which the buffer will be expanded when it
needs expansion. There is a special constant, VMemoryStream::kIncrement2x,
which makes the buffer expand by doubling in size each time. This is often
the most efficient way to deal with streams whose size is not known in
advance, because it allows the buffer to grow in a way that slows down the
number of re-allocations as more expansion is needed. kIncrement2x is the
default value of the resize increment.

As noted in the method documentation, VMemoryStream allocates the buffer
using "new Vu8[size]" rather than the old C style "malloc(size)", and so it
assumes that if you give it a buffer, it has been allocated that way as well.
If you really need to give it a malloc'ed buffer, you must ensure that
the VMemoryStream will not try to delete that buffer: this means that you
must not cause expansion, and that before destruction you must give it a
a buffer created with new, or NULL, calling adoptBuffer(). It would not be
difficult to modify this class, or subclass it, to allow malloc'ed buffers (it
would just need to have a flag indicating such buffers, and always delete/free
accordingly). I have chosen not to do that, in order to keep this code
clean and pure C++ stylistically.
*/
class VMemoryStream : public VStream
    {
    public:
    
        CLASS_CONST(Vs64, kIncrement2x, 0);                ///< Special constant for resize increment, causes buffer to double when expanded.
        CLASS_CONST(Vs64, kDefaultBufferSize, 32768);    ///< The default size of the buffer allocated on construction.
    
        /**
        Constructs the object with a specified buffer size and resizing increment.
        @param    initialBufferSize    the size of the buffer to allocate initially
        @param    resizeIncrement        the "chunk size" in which the buffer will expand when needed
        */
        VMemoryStream(Vs64 initialBufferSize=kDefaultBufferSize, Vs64 resizeIncrement=kIncrement2x);
        /**
        Constructs the object with an existing buffer.
        @param    buffer    the buffer that the VMemoryStream will take ownership of; Note: must be
                        allocated with "new Vu8[size]", not malloc(), because it will be deleted
                        with "delete [] x", not free(). It is possible to use a malloc'ed buffer
                        only if you carefully use the adoptBuffer() method to assign a different
                        buffer (or NULL) to the object before it is destructed or needs to expand
                        the buffer.
        @param    inBufferSize    the size of the supplied buffer
        @param    inEOFOffset        the offset of the end of the "valid" data in the supplied buffer
        @param    resizeIncrement    the "chunk size" in which the buffer will expand when needed
        */
        VMemoryStream(Vu8* buffer, Vs64 inBufferSize, Vs64 inEOFOffset, Vs64 resizeIncrement=kIncrement2x);
        /**
        Destructor.
        */
        virtual ~VMemoryStream();
        
        // Required VStream method overrides:
        /**
        Attempts to read a specified number of bytes from the stream.
        @param    targetBuffer    the buffer to read into
        @param    numBytesToRead    the number of bytes to read
        @return    the actual number of bytes that could be read
        */
        virtual Vs64    read(Vu8* targetBuffer, Vs64 numBytesToRead);
        /**
        Writes bytes to the stream. Throws a VException if the write extends
        past the end of the buffer but buffer expansion fails.
        @param    buffer            the buffer containing the data
        @param    numBytesToWrite    the number of bytes to write to the stream
        @return the actual number of bytes written
        */
        virtual Vs64    write(const Vu8* buffer, Vs64 numBytesToWrite);
        /**
        Flushes any pending or buffered write data to the stream. Until you
        call flush, you cannot guarantee that your data has actually been
        written to the underlying physical stream.
        */
        virtual void    flush();
        /**
        Skips forward in the stream a specified number of bytes.
        @param    numBytesToSkip    the number of bytes to skip
        */
        virtual bool    skip(Vs64 numBytesToSkip);
        /**
        Seeks in the stream using Unix seek() semantics.
        @param    inOffset    the offset, meaning depends on whence value
        @param    whence    SEEK_SET, SEEK_CUR, or SEEK_END
        @return true if the seek was successful
        */
        virtual bool    seek(Vs64 inOffset, int whence);
        /**
        Returns the current offset in the stream. For memory streams, this
        is the same as the i/o offset.
        @return the current offset
        */
        virtual Vs64    offset() const;
        /**
        Returns the number of bytes that are available to be read from this
        stream. For file and memory streams, this means the number of bytes
        from the current i/o mark until the end of the file or buffer. For
        socket streams, this means the number of bytes that can be read
        without blocking (that is, the number of bytes that are waiting to
        be read on the socket at this time).
        @return the number of bytes currently available for reading
        */
        virtual Vs64    available() const;

        // Methods we define for callers who know what kind of stream they have:
        /**
        Makes the object use a specified buffer instead of the one it is currently
        using.
        @param    buffer    the buffer that the VMemoryStream will take ownership of; Note: must be
                        allocated with "new Vu8[size]", not malloc(), because it will be deleted
                        with "delete [] x", not free(). It is possible to use a malloc'ed buffer
                        only if you carefully use the adoptBuffer() method to assign a different
                        buffer (or NULL) to the object before it is destructed or needs to expand
                        the buffer.
        @param    inBufferSize    the size of the supplied buffer
        @param    inEOFOffset        the offset of the end of the "valid" data in the supplied buffer
        @param    deleteOldBuffer    true if the object should "delete [] x" its existing buffer,
                                false if you want it orphaned (for example, if you retain a
                                pointer to it and will manage it yourself)
        */
        void    adoptBuffer(Vu8* buffer, Vs64 inBufferSize, Vs64 inEOFOffset, bool deleteOldBuffer=true);
        /**
        Returns the memory buffer pointer.
        @return    the memory buffer pointer
        */
        Vu8*    getBuffer() const;
        /**
        Returns the size of the memory buffer.
        @return    the size of the memory buffer
        */
        Vs64    bufferSize() const;
        /**
        Returns the EOF offset, that is, the length of "valid" data in the buffer.
        @return    the EOF offset
        */
        Vs64    eofOffset() const;
        /**
        Returns the I/O offset, that is, the position where the next read or write
        will occur. This offset moves forward during reads, writes, and skips, and moves in
        various ways during seeks.
        @return    the I/O offset
        */
        Vs64    ioOffset() const;
        /**
        Sets the EOF offset, constrained to the buffer size. That is, if you
        attempt to set EOF past the end of the buffer, the EOF will be set
        to the end of the buffer.
        @param    offset    the new EOF offset
        */
        void    setEOF(Vs64 inOffset);
        /**
        Returns true if the two memory streams contain exactly the same number
        of bytes (that is, they have the same EOF offsets) and those bytes
        contain identical values.
        @param    m1    a memory stream
        @param    m2    another memory stream
        @return true if the streams's lengths and data are the same
        */
        friend bool operator==(const VMemoryStream& m1, const VMemoryStream& m2);

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
        virtual Vu8*    getReadIOPtr() const;
        /**
        Returns a pointer to the current write i/o position in the stream's buffer,
        or NULL if the stream does not have a buffer or support direct copying
        to that buffer (for example, a file or socket stream).
        @return    the i/o buffer pointer, or NULL
        */
        virtual Vu8*    getWriteIOPtr() const;
        /**
        Returns the number of bytes available for reading from the stream's
        buffer, or zero by default for streams without buffers.
        @param    numBytesToRead    the number of bytes that will be read
        @return    the number of bytes available to read, or zero
        */
        virtual Vs64    prepareToRead(Vs64 numBytesToRead) const;
        /**
        Preflights the stream's buffer so that it can have the specified
        number of bytes written to it subsequently. Throws a VException
        if the buffer cannot be expanded to accomodate the data.
        @param    numBytesToWrite    the number of bytes that will be written
        */
        virtual void    prepareToWrite(Vs64 numBytesToWrite);
        /**
        Postflights a copy by advancing the i/o offset to reflect
        the specified number of bytes having just been read.
        @param    numBytesRead    the number of bytes that were previously read
        */
        virtual void    finishRead(Vs64 numBytesRead);
        /**
        Postflights a copy by advancing the i/o offset to reflect
        the specified number of bytes having just been written.
        @param    numBytesWritten    the number of bytes that were previously written
        */
        virtual void    finishWrite(Vs64 numBytesWritten);
        
        /** Asserts if any invariant is broken. */
        void assertInvariant() const;
    
        Vs64    mBufferSize;        ///< The physical size of the buffer.
        Vs64    mIOOffset;            ///< The offset in the buffer of the next read/write.
        Vs64    mEOFOffset;            ///< The offset in the buffer of the end of the data.
        Vs64    mResizeIncrement;    ///< The amount to increment when expanding the buffer.
        Vu8*    mBuffer;            ///< The buffer itself.
    };

#endif /* vmemorystream_h */
