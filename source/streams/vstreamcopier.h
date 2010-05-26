/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

#ifndef vstreamcopier_h
#define vstreamcopier_h

/** @file */

#include "viostream.h"

/**
    @defgroup vstream_util Stream Utilities
    
    These are the utility classes related to streams.
    
    @ingroup vstreams
    @{
*/
    
/**
VStreamCopier is a helper class for certain kinds of stream copy operations.

In many cases you can just call streamCopy(), which is defined in vstream.h
and viostream.h for use with any pair of VStream or VIOStream objects. It is
a trivially easy way to perform stream-to-stream copies.

VStreamCopier allows you to do a bulk stream-to-stream copy operation, but unlike
streamCopy(), which does it in a single call, VStreamCopier lets you perform
the copy iteratively, in chunks. This is useful if you need to monitor the
progress of a large copy. An example is when you are providing user feedback
and want to update the progress. If you were to just use streamCopy() in a
single call, you'd not be able to see the progress until the copy is complete.

You can supply any pair of streams in the constructor, either VStream-based
or VIOStream-based, in any combination. You also supply the chunk size.

Then you repeatedly call copyChunk() in a while loop until it returns false.
A return value of true indicates that the copy has not run out of data yet,
although it's possible that the next copy will discover that there are no
more bytes left to copy (EOF), but that's not an error.
To check the actual byte count progress, call numBytesCopied().

@see    VIOStream
@see    VStream
*/
class VStreamCopier
    {
    public:
    
        /**
        Default constructor. You need to call init() later if you use
        this constructor.
        */
        VStreamCopier();
        /**
        Constructor that takes two VStream objects.
        @param chunkSize    the number of bytes to copy in each chunk
        @param from            the stream to copy from
        @param to            the stream to copy to
        */
        VStreamCopier(int chunkSize, VStream* from, VStream* to);
        /**
        Constructor that takes two VIOStream objects.
        @param chunkSize    the number of bytes to copy in each chunk
        @param from            the stream to copy from
        @param to            the stream to copy to
        */
        VStreamCopier(int chunkSize, VIOStream* from, VIOStream* to);
        /**
        Constructor that takes a VStream and a VIOStream.
        @param chunkSize    the number of bytes to copy in each chunk
        @param from            the stream to copy from
        @param to            the stream to copy to
        */
        VStreamCopier(int chunkSize, VStream* from, VIOStream* to);
        /**
        Constructor that takes a VIOStream and a VStream.
        @param chunkSize    the number of bytes to copy in each chunk
        @param from            the stream to copy from
        @param to            the stream to copy to
        */
        VStreamCopier(int chunkSize, VIOStream* from, VStream* to);
        /**
        Destructor.
        */
        virtual ~VStreamCopier() {}
        
        /**
        Init function that takes two VStream objects.
        @param chunkSize    the number of bytes to copy in each chunk
        @param from            the stream to copy from
        @param to            the stream to copy to
        */
        void init(int chunkSize, VStream* from, VStream* to);
        /**
        Init function that takes two VIOStream objects.
        @param chunkSize    the number of bytes to copy in each chunk
        @param from            the stream to copy from
        @param to            the stream to copy to
        */
        void init(int chunkSize, VIOStream* from, VIOStream* to);
        /**
        Init function that takes a VStream and a VIOStream.
        @param chunkSize    the number of bytes to copy in each chunk
        @param from            the stream to copy from
        @param to            the stream to copy to
        */
        void init(int chunkSize, VStream* from, VIOStream* to);
        /**
        Init function that takes a VIOStream and a VStream.
        @param chunkSize    the number of bytes to copy in each chunk
        @param from            the stream to copy from
        @param to            the stream to copy to
        */
        void init(int chunkSize, VIOStream* from, VStream* to);
        
        /**
        Copies a chunk between the streams, and returns false if less than a
        complete chunk was available to copy. That is, true indicates that
        you should call this function again, and false indicates that the
        copy operation is complete.
        @return true if there is more data to copy
        */
        bool copyChunk();
        /**
        Returns the number of bytes copied so far.
        @return the number of bytes copied so far
        */
        Vs64 numBytesCopied() const;

    private:

        VStreamCopier(const VStreamCopier&); // not copyable
        VStreamCopier& operator=(const VStreamCopier&); // not assignable
        
        int         mChunkSize;         ///< The number of bytes to copy per chunk.
        VStream*    mFrom;              ///< The underlying stream we are copying from.
        VStream*    mTo;                ///< The underlying stream we are copying to.
        Vs64        mNumBytesCopied;    ///< The total number of bytes copied so far.
    };

/** @} */

#endif /* vstreamcopier_h */
