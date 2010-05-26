/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vabstractfilestream.h"
#include "vtypes_internal_platform.h"

#include "vexception.h"

VAbstractFileStream::VAbstractFileStream() :
mNode() // -> empty path
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
        throw VException(errno, VString("%s failed to open '%s'. Error %d (%s).", failedMethod.chars(), path.chars(), errno, ::strerror(errno)));
    }

