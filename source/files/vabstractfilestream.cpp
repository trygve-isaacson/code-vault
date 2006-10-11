/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vabstractfilestream.h"

#include "vexception.h"

VAbstractFileStream::VAbstractFileStream()
// mNode constructs to an empty path
    {
    }

VAbstractFileStream::VAbstractFileStream(const VFSNode& node) :
mNode(node)
    {
    node.getName(mName); // set the stream name for debugging use
    }

void VAbstractFileStream::setNode(const VFSNode& node)
    {
    mNode = node;
    node.getName(mName);
    }

const VFSNode& VAbstractFileStream::getNode() const
    {
    return mNode;
    }

void VAbstractFileStream::_throwIfOpenFailed(const VString& failedMethod, const VString& path)
    {
    if (! this->isOpen())
        throw VException(errno, "%s failed to open '%s'. Error %d (%s).", failedMethod.chars(), path.chars(), errno, ::strerror(errno));
    }

