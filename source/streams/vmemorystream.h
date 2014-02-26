/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
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

VMemoryStream normally allocates its own buffer using new[] rather than
the old C style malloc() function. However, when the caller supplies the buffer
via adoptBuffer() or the constructor that takes a buffer, it indicates how the
buffer was allocated, and VMemoryStream then knows how whether to delete[] or
free() it, and should VMemoryStream need to reallocate it, it will continue with
the same kind of allocation (unless you allow VMemoryStream to adopt a stack-based
buffer, in which case it will reallocate on the heap with new if it needs to
expand; normally you would use a stack-base buffer only in a read-only or other
non-expandable situation). You also indicate in these methods whether
VMemoryStream is adopting (taking ownership of) the buffer; if it is not adopting
the buffer, it cannot reallocate it, and so any write operation that needs to
expand the buffer will cause a VEOFException to be thrown. You can tell
VMemoryStream to relinquish ownership of the buffer by calling orphanBuffer();
this means VMemoryStream will neither reallocate nor delete/free the buffer.
*/
class VMemoryStream : public VStream {
    public:

        static const Vs64 kIncrement2x = 0;             ///< Special constant for resize increment, causes buffer to double when expanded.
        static const Vs64 kDefaultBufferSize = 32768;   ///< The default size of the buffer allocated on construction.
        typedef enum { kAllocatedByOperatorNew, kAllocatedByMalloc, kAllocatedOnStack, kAllocatedUnknown } BufferAllocationType;

        /**
        Constructs the object with a specified buffer size and resizing increment.
        @param    initialBufferSize    the size of the buffer to allocate initially
        @param    resizeIncrement        the "chunk size" in which the buffer will expand when needed
        */
        VMemoryStream(Vs64 initialBufferSize = kDefaultBufferSize, Vs64 resizeIncrement = kIncrement2x);
        /*
        Copy construction and assignment semantics: If the other stream owns its buffer, then
        the copy gets a brand new copied buffer; if the other stream doesn't own its buffer,
        then the copy also refers to the same buffer which it also does not own. When making
        a copy of a buffer, we try to use the same allocation type, but if the original buffer
        is on the stack, the copy is made on the heap.
        @param other the other VMemoryStream that we are initialized from
        */
        VMemoryStream(const VMemoryStream& other);
        /**
        Constructs the object with an existing buffer.
        @param    buffer                the buffer that the VMemoryStream will work on
        @param    allocationType        how the buffer was allocated, so that VMemoryStream knows the
                                        proper way to reallocate it and to delete/free it
        @param    adoptsBuffer          true means VMemoryStream takes ownership of the buffer, and
                                        can reallocate to expand it, and will delete/free it on destruction
        @param    suppliedBufferSize    the size of the supplied buffer
        @param    suppliedEOFOffset     the offset of the end of the "valid" data in the supplied buffer
        @param    resizeIncrement       the "chunk size" in which the buffer will expand when needed
        */
        VMemoryStream(Vu8* buffer, BufferAllocationType allocationType, bool adoptsBuffer, Vs64 suppliedBufferSize, Vs64 suppliedEOFOffset, Vs64 resizeIncrement = kIncrement2x);
        /**
        Destructor.
        */
        virtual ~VMemoryStream();

        /**
        Same semantics as with the copy constructor described above; if the other stream owns its
        buffer, we make a copy that we own; if it's sharing a buffer owned by someone else, then
        we will also share it.
        @param other the other VMemoryStream that we are assigned from
        */
        VMemoryStream& operator=(const VMemoryStream& other);

        // Required VStream method overrides:
        /**
        Attempts to read a specified number of bytes from the stream.
        @param    targetBuffer    the buffer to read into
        @param    numBytesToRead    the number of bytes to read
        @return    the actual number of bytes that could be read
        */
        virtual Vs64 read(Vu8* targetBuffer, Vs64 numBytesToRead);
        /**
        Writes bytes to the stream. Throws a VException if the write extends
        past the end of the buffer but buffer expansion fails.
        @param    buffer            the buffer containing the data
        @param    numBytesToWrite    the number of bytes to write to the stream
        @return the actual number of bytes written
        */
        virtual Vs64 write(const Vu8* buffer, Vs64 numBytesToWrite);
        /**
        Flushes any pending or buffered write data to the stream. Until you
        call flush, you cannot guarantee that your data has actually been
        written to the underlying physical stream.
        */
        virtual void flush();
        /**
        Skips forward in the stream a specified number of bytes.
        @param    numBytesToSkip    the number of bytes to skip
        */
        virtual bool skip(Vs64 numBytesToSkip);
        /**
        Seeks in the stream using Unix seek() semantics.
        @param    offset    the offset, meaning depends on whence value
        @param    whence    SEEK_SET, SEEK_CUR, or SEEK_END
        @return true if the seek was successful
        */
        virtual bool seek(Vs64 offset, int whence);
        /**
        Returns the current offset in the stream. For memory streams, this
        is the same as the i/o offset.
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

        // Methods we define for callers who know what kind of stream they have:
        /**
        Makes the object use a specified buffer instead of the one it is currently using. The
        existing buffer will be deleted/freed if the existing buffer is owned by the VMemoryStream.
        @param    buffer                the buffer that the VMemoryStream will work on
        @param    allocationType        how the buffer was allocated, so that VMemoryStream knows the
                                        proper way to reallocate it and to delete/free it
        @param    adoptsBuffer          true means VMemoryStream takes ownership of the buffer, and
                                        can reallocate to expand it, and will delete/free it on destruction
        @param    suppliedBufferSize    the size of the supplied buffer
        @param    suppliedEOFOffset        the offset of the end of the "valid" data in the supplied buffer
        */
        void adoptBuffer(Vu8* buffer, BufferAllocationType allocationType, bool adoptsBuffer, Vs64 suppliedBufferSize, Vs64 suppliedEOFOffset);
        /**
        Notifies the VMemoryStream that it no longer owns the buffer; it will continue to use the buffer,
        but may not reallocate nor delete/free it.
        */
        void orphanBuffer();
        /**
        Returns the memory buffer pointer.
        @return    the memory buffer pointer
        */
        Vu8* getBuffer() const;
        /**
        Returns the size of the memory buffer.
        @return    the size of the memory buffer
        */
        Vs64 getBufferSize() const;
        /**
        Returns the EOF offset, that is, the length of "valid" data in the buffer.
        @return    the EOF offset
        */
        Vs64 getEOFOffset() const;
        /**
        Sets the EOF offset, constrained to the buffer size. That is, if you
        attempt to set EOF past the end of the buffer, the EOF will be set
        to the end of the buffer.
        @param    eofOffset    the new EOF offset
        */
        void setEOF(Vs64 eofOffset);
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
        virtual void _finishWrite(Vs64 numBytesWritten);

        /** Asserts if any invariant is broken. */
        void _assertInvariant() const;

        Vs64                    mBufferSize;        ///< The physical size of the buffer.
        Vs64                    mIOOffset;          ///< The offset in the buffer of the next read/write.
        Vs64                    mEOFOffset;         ///< The offset in the buffer of the end of the data.
        Vs64                    mResizeIncrement;   ///< The amount to increment when expanding the buffer.
        bool                    mOwnsBuffer;        ///< True means we delete the buffer on destruction; false means it's someone else's responsibility.
        BufferAllocationType    mAllocationType;    ///< Indicates how the buffer was allocated, and thus how it should be deallocated.
        Vu8*                    mBuffer;            ///< The buffer itself.

    private:

        Vu8* _createNewBuffer(Vs64 bufferSize, BufferAllocationType& newAllocationType);
        void _releaseBuffer();
};

/**
VReadOnlyMemoryStream is a convenience class that lets you share a single buffer
between multiple memory streams. To do this, you can instantiate multiple
VReadOnlyMemoryStream objects, supplying them all the same buffer. They will all
prevent writing (throwing an exception for any write method), and will not "own"
the buffer and so will not delete/free it. The various streams can be independently
used to seek and read the stream without affecting the point of view of other
readers. This facility even allows you to supply a pointer to a location inside
an arbitrary actual buffer (heap or stack) because the VReadOnlyMemoryStream object
will never use the pointer for memory managment purposes such as delete or free.
*/
class VReadOnlyMemoryStream : public VMemoryStream {
    public:

        /**
        Constructs the object with an existing buffer.
        @param    buffer                the buffer that the VMemoryStream will work on
        @param    suppliedEOFOffset     the offset of the end of the "valid" data in the supplied buffer
        */
        VReadOnlyMemoryStream(Vu8* buffer, Vs64 suppliedEOFOffset);
        /**
        Copy constructor.
        */
        VReadOnlyMemoryStream(const VReadOnlyMemoryStream& other);
        /**
        Destructor.
        */
        virtual ~VReadOnlyMemoryStream() {}

        /**
        Assignment operator.
        */
        VReadOnlyMemoryStream& operator=(const VReadOnlyMemoryStream& other);

        /**
        Makes the object use a specified buffer instead of the one it is currently using. The
        existing buffer will be deleted/freed if the existing buffer is owned by the VMemoryStream.
        @param    buffer                the buffer that the VMemoryStream will work on
        @param    allocationType        how the buffer was allocated, so that VMemoryStream knows the
                                        proper way to reallocate it and to delete/free it
        @param    suppliedBufferSize    the size of the supplied buffer
        @param    suppliedEOFOffset        the offset of the end of the "valid" data in the supplied buffer
        */
        void adoptBuffer(Vu8* buffer, BufferAllocationType allocationType, Vs64 suppliedBufferSize, Vs64 suppliedEOFOffset);

        /**
        Throws a VEOFException because this class is a read-only stream.
        */
        virtual Vs64 write(const Vu8* buffer, Vs64 numBytesToWrite);
        /**
        Throws a VEOFException because this class is a read-only stream.
        */
        virtual void flush();
};

#endif /* vmemorystream_h */
