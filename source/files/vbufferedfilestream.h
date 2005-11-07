/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

#ifndef vbufferedfilestream_h
#define vbufferedfilestream_h

/** @file */

#include "vstream.h"

#include "vfsnode.h"

/**
    @ingroup vstream_derived vfilesystem
*/

/**
VBufferedFileStream is a concrete VStream class that implements stream i/o
on a file, using buffered i/o APIs (e.g., fopen/fclose/fread/fwrite).

For unbuffered i/o (usually not as good for performance, but occasionally
useful due to lack of buffering), use VFileStream.

@see VFileStream
*/
class VBufferedFileStream : public VStream
    {
    public:
    
        /**
        Constructs an undefined stream (you will have to set it up
        with a subsequent call to setNode()).
        */
        VBufferedFileStream();
        /**
        Constructs a stream with a node.
        @param    node    the node representing the file
        */
        VBufferedFileStream(const VFSNode& node);
        /**
        Constructs a stream with an already-open standard POSIX file handle.
        @param    f    the POSIX file handle
        */
        VBufferedFileStream(FILE* f);
        /**
        Destructor, closes the stream if it is open.
        */
        virtual ~VBufferedFileStream();

        /**
        Opens the stream read-only. Throws a VException if it cannot be opened.
        */
        void    openReadOnly();
        /**
        Opens the stream read-write, creating the file if it does not exist.
        Throws a VException if it cannot be opened.
        */
        void    openReadWrite();
        /**
        Opens the stream for write, creating the file if it does not exist.
        Throws a VException if it cannot be opened.
        */
        void    openWrite();
        
        /**
        Sets the file node, so that you can use the empty constructor, and
        call this method after construction (but before opening the file).
        @param    node    the node representing the file
        */
        void    setNode(const VFSNode& node);
        /**
        Returns the file node.
        */
        const VFSNode& getNode() const;
        /**
        Sets the file to an already-open standard POSIX file handle, so that
        you can use the empty constructor, and call this method after
        construction; don't call an open method if you use this.
        @param    f    the POSIX file handle
        */
        void    setFile(FILE* f);
        /**
        Returns true if the stream is open.
        */
        bool    isOpen() const;
        /**
        Closes the stream.
        */
        void    close();

        // Required VStream method overrides:
        /**
        Attempts to read a specified number of bytes from the stream.
        @param    targetBuffer    the buffer to read into
        @param    numBytesToRead    the number of bytes to read
        @return    the actual number of bytes that could be read
        */
        virtual Vs64    read(Vu8* targetBuffer, Vs64 numBytesToRead);
        /**
        Writes bytes to the stream.
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
        Skips forward in the stream a specified number of bytes. For memory
        and file streams, this means advancing the i/o offset by the specified
        number of bytes; for socket streams, this means reading and discarding
        the specified number of bytes.
        @param    numBytesToSkip    the number of bytes to skip
        */
        virtual bool    skip(Vs64 numBytesToSkip);
        /**
        Seeks in the stream using Unix seek() semantics.
        @param    offset    the offset, meaning depends on whence value
        @param    whence    SEEK_SET, SEEK_CUR, or SEEK_END
        @return true if the seek was successful
        */
        virtual bool    seek(Vs64 inOffset, int whence);
        /**
        Returns the current offset in the stream. For file streams, this
        is the same as the i/o mark.
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
    
    protected:
    
        /**
        Called by the open methods after they attempt to open the stream;
        throws a VException if the stream is not open.
        @param    failedMethod    the name of the method that was originally
                                called; used to build the error message
        @param    path            path of file we failed to open; used to build
                                the error message
        */
        void    throwIfOpenFailed(const VString& failedMethod, const VString& path) const;

        static FILE*    threadsafe_fopen(const char* nativePath, const char* mode);                    ///< Calls POSIX fopen in a way that is safe even if interrupted by another thread.
        static int        threadsafe_fclose(FILE* f);                                                        ///< Calls POSIX fclose in a way that is safe even if interrupted by another thread.
        static size_t    threadsafe_fread(void* buffer, size_t size, size_t nItems, FILE* f);            ///< Calls POSIX fread in a way that is safe even if interrupted by another thread.
        static size_t    threadsafe_fwrite(const void* buffer, size_t size, size_t nItems, FILE* f);    ///< Calls POSIX fwrite in a way that is safe even if interrupted by another thread.
        static int        threadsafe_fseek(FILE* f, long int inOffset, int whence);                        ///< Calls POSIX fseek in a way that is safe even if interrupted by another thread.
        static int        threadsafe_fflush(FILE* f);                                                        ///< Calls POSIX fflush in a way that is safe even if interrupted by another thread.
        static long int    threadsafe_ftell(FILE* f);                                                    ///< Calls POSIX ftell in a way that is safe even if interrupted by another thread.
    
        VFSNode    mNode;                ///< The node representing the file.
        FILE*    mFile;                ///< The Unix API file handle.
        bool    mCloseOnDestruct;    ///< True if we'll close on destruct, set false on setFile.
    };

#endif /* vbufferedfilestream_h */

