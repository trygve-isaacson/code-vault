/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vfilewriter_h
#define vfilewriter_h

#include "vfsnode.h"
#include "vmemorystream.h"
#include "vtextiostream.h"
#include "vbinaryiostream.h"

/** @file */

/**
This class simplifies using the VFSNode::safelyOverwriteFile() API, by relieving you of the need
to manually create a temporary buffer stream, etc.
You just instantiate the VFileWriter with the target file node, then write to either a text or binary
stream obtained by one of the getter APIs, and finally 'save' the data. The reason for the explicit
save call is so that nothing is written if an exception is thrown during your serialization, unless
you want to catch it and still write whatever data you did put in the output stream (perhaps after
first manipulating the stream further). Normally you want such exceptions to propagate up and avoid
writing entirely.

This initial version always does your writes to a VMemoryStream and then uses that with the
VFSNode::safelyOverwriteFile() API. It would be good to enhance this with an option (or default
behavior) that actually writes to a temporary file directly, and then swaps it the way that
VFSNode::safelyOverwriteFile() does with the buffered data.

So it can look like this,if you had text data for example:

void Thing::saveMyData() {
    VFileWriter writer(mMyFileNode);
    mMyData.writeToTextStream(writer.getTextOutputStream());
    writer.save();
}

*/
class VFileWriter {

    public:

        /**
        Creates the helper objects, pointing to a specific file node.
        @param  target  the file node that will be written to
        */
        VFileWriter(const VFSNode& target);
        ~VFileWriter() {} // non-virtual, not intended for subclassing
        
        VTextIOStream& getTextOutputStream() { return mTextOutputStream; }
        VBinaryIOStream& getBinaryOutputStream() { return mBinaryOutputStream; }
        
        void save();

    private:
    
        VFSNode             mTarget;
        VMemoryStream       mBuffer;
        VTextIOStream       mTextOutputStream;
        VBinaryIOStream     mBinaryOutputStream;
};

#endif /* vfsnode_h */

