/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

/** @file */

#include "vabstractfilestream.h"
#include "vtypes_internal_platform.h"

#include "vexception.h"

VAbstractFileStream::VAbstractFileStream()
    : mNode() // -> empty path
    {
}

VAbstractFileStream::VAbstractFileStream(const VFSNode& node)
    : mNode(node)
    {
    node.getName(mName); // set the stream name for debugging use
}

void VAbstractFileStream::setNode(const VFSNode& node) {
    mNode = node;
    node.getName(mName);
}

const VFSNode& VAbstractFileStream::getNode() const {
    return mNode;
}

void VAbstractFileStream::_throwIfOpenFailed(const VString& failedMethod, const VString& path) {
    if (! this->isOpen()) {
        throw VException(VSystemError(), VSTRING_FORMAT("%s failed to open '%s'.", failedMethod.chars(), path.chars()));
    }
}

