/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vfilewriter.h"

// VFileWriter -------------------------------------------------

VFileWriter::VFileWriter(const VFSNode& target)
    : mTarget(target)
    , mBuffer()
    , mTextOutputStream(mBuffer)
    , mBinaryOutputStream(mBuffer)
    {
}

void VFileWriter::save() {
    mBuffer.seek0();
    VBinaryIOStream bufferStream(mBuffer);
    
    VFSNode::safelyOverwriteFile(mTarget, mBuffer.getEOFOffset(), bufferStream);
}
