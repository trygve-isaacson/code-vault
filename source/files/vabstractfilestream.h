/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vabstractfilestream_h
#define vabstractfilestream_h

/** @file */

#include "vstream.h"

#include "vfsnode.h"

/**
    @ingroup vstream_derived vfilesystem
*/

/**
VAbstractFileStream is a VStream subclass that implements the common
part of the buffered and unbuffered file concrete subclasses, and
defines the pure virtual API that they must implement.

@see VBufferedFileStream
@see VDirectIOFileStream
*/
class VAbstractFileStream : public VStream {
    public:

        /**
        Constructs an undefined stream (you will have to set it up
        with a subsequent call to setNode() or a method specific
        to the subclass).
        */
        VAbstractFileStream();
        /**
        Constructs a stream with a node.
        @param    node    the node representing the file
        */
        VAbstractFileStream(const VFSNode& node);
        /**
        Destructor.
        */
        virtual ~VAbstractFileStream() {}

        /**
        Sets the file node, so that you can use the empty constructor, and
        call this method after construction (but before opening the file).
        @param    node    the node representing the file
        */
        void setNode(const VFSNode& node);
        /**
        Returns the file node.
        */
        const VFSNode& getNode() const;

        /**
        Opens the file read-only. Throws a VException if it cannot be opened.
        */
        virtual void openReadOnly() = 0;
        /**
        Opens the file read-write, creating the file if it does not exist.
        Throws a VException if it cannot be opened.
        */
        virtual void openReadWrite() = 0;
        /**
        Opens the file for writing, creating the file if it does not exist.
        Throws a VException if it cannot be opened.
        */
        virtual void openWrite() = 0;
        /**
        Returns true if the file stream is open.
        */
        virtual bool isOpen() const = 0;
        /**
        Closes the file stream.
        */
        virtual void close() = 0;

        // In addition to the pure virtual methods defined above, the
        // concrete subclass must also implement the stream methods
        // defined by VStream (read, write, flush, skip, seek,
        // offset, available).

    protected:

        /**
        Called by the open methods after they attempt to open the stream;
        throws a VException if the stream is not open.
        @param    failedMethod    the name of the method that was originally
                                called; used to build the error message
        @param    path            path of file we failed to open; used to build
                                the error message
        */
        void _throwIfOpenFailed(const VString& failedMethod, const VString& path);

        VFSNode mNode;  ///< The node representing the file.

};

#endif /* vabstractfilestream_h */

