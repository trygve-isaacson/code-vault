/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

#ifndef vbufferedfilestream_h
#define vbufferedfilestream_h

/** @file */

#include "vabstractfilestream.h"

#include "vfsnode.h"

/**
    @ingroup vstream_derived vfilesystem
*/

/**
VBufferedFileStream is a concrete VStream class that implements stream i/o
on a file, using buffered i/o APIs (e.g., fopen/fclose/fread/fwrite).

For unbuffered i/o (usually not as good for performance, but occasionally
useful due to lack of buffering), use VDirectIOFileStream.

@see VStream
@see VAbstractFileStream
@see VDirectIOFileStream
*/
class VBufferedFileStream : public VAbstractFileStream
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
        @param    f                 the POSIX file handle
        @param    closeOnDestruct   true if you want the FILE automatically
                                    closed when this object is destructed,
                                    false if the caller still owns that
                                    responsibility
        */
        VBufferedFileStream(FILE* f, bool closeOnDestruct);
        /**
        Destructor, closes the stream if it is open, unless you supplied
        a POSIX FILE handle and specified closeOnDestruct = FALSE.
        */
        virtual ~VBufferedFileStream();
        
        /**
        Sets the file to an already-open standard POSIX file handle, so that
        you can use the empty constructor, and call this method after
        construction; don't call an open method if you use this.
        @param    f                 the POSIX file handle
        @param    closeOnDestruct   true if you want the FILE automatically
                                    closed when this object is destructed,
                                    false if the caller still owns that
                                    responsibility
        */
        void setFile(FILE* f, bool closeOnDestruct);

        // Implementation of VAbstractFileStream -----------------------------

        /**
        Opens the file read-only. Throws a VException if it cannot be opened.
        */
        virtual void openReadOnly();
        /**
        Opens the file read-write, creating the file if it does not exist.
        Throws a VException if it cannot be opened.
        */
        virtual void openReadWrite();
        /**
        Opens the file for writing, creating the file if it does not exist.
        Throws a VException if it cannot be opened.
        */
        virtual void openWrite();
        /**
        Returns true if the file stream is open.
        */
        virtual bool isOpen() const;
        /**
        Closes the file stream.
        */
        virtual void close();

        // Implementation of VStream -----------------------------------------

        /**
        Attempts to read a specified number of bytes from the stream.
        @param    targetBuffer    the buffer to read into
        @param    numBytesToRead    the number of bytes to read
        @return    the actual number of bytes that could be read
        */
        virtual Vs64 read(Vu8* targetBuffer, Vs64 numBytesToRead);
        /**
        Writes bytes to the stream.
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
        Skips forward in the stream a specified number of bytes. For memory
        and file streams, this means advancing the i/o offset by the specified
        number of bytes; for socket streams, this means reading and discarding
        the specified number of bytes.
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
        Returns the current offset in the stream. For file streams, this
        is the same as the i/o mark.
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
    
        // Prevent copy construction and assignment, since there is no provision for sharing the mFile pointer.
        VBufferedFileStream(const VBufferedFileStream& other);
        VBufferedFileStream& operator=(const VBufferedFileStream& other);

        /*
        These wrapper functions handle signal interruptions that may occur on
        some Unix systems, and they also provide a common breakpoint location
        for debugging i/o errors.
        */
        static FILE*    _wrap_fopen(const char* nativePath, const char* mode);                      ///< Calls POSIX fopen in a way that is safe even if a signal is caught inside the function.
        static int      _wrap_fclose(FILE* f);                                                      ///< Calls POSIX fclose in a way that is safe even if a signal is caught inside the function.
        static size_t   _wrap_fread(void* buffer, size_t size, size_t numItems, FILE* f);           ///< Calls POSIX fread in a way that is safe even if a signal is caught inside the function.
        static size_t   _wrap_fwrite(const void* buffer, size_t size, size_t numItems, FILE* f);    ///< Calls POSIX fwrite in a way that is safe even if a signal is caught inside the function.
        static int      _wrap_fseek(FILE* f, long int offset, int whence);                          ///< Calls POSIX fseek in a way that is safe even if a signal is caught inside the function.
        static int      _wrap_fflush(FILE* f);                                                      ///< Calls POSIX fflush in a way that is safe even if a signal is caught inside the function.
        static long int _wrap_ftell(FILE* f);                                                       ///< Calls POSIX ftell in a way that is safe even if a signal is caught inside the function.
    
        FILE*   mFile;              ///< The Unix API file handle.
        bool    mCloseOnDestruct;   ///< True if we'll close on destruct, set false on setFile.
    
    };

#endif /* vbufferedfilestream_h */

