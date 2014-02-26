/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

/** @file */

/**
This is where we implement all of the Objective-C++ code that we rely on. Although we could partition
this out by subdirectory/module, there is so little of it that it seems cleaner to put it what little
of it exists all in one place.
*/

#include "vtypes.h"

#include "vstring.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSPathUtilities.h> // for NSHomeDirectory()

// VAutoreleasePool -----------------------------------------------------------

VAutoreleasePool::VAutoreleasePool() {
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    mPool = pool;
}

void VAutoreleasePool::drain() {
    NSAutoreleasePool* pool = static_cast<NSAutoreleasePool*>(mPool);
    [pool drain];
}

VAutoreleasePool::~VAutoreleasePool() {
    this->drain();
    mPool = nil;
}

// _V_NSHomeDirectory ---------------------------------------------------------

/*
This function is not declared extern in a header file because it's private for
our platform-specific implementation code. We don't even define it in our
platform headers because we don't want to make them require inclusion of vstring.h.
This is only referenced by the Mac OS X version of VFSNode::_platform_getKnownDirectoryNode(),
so it is manually declared extern there to match this.
*/
extern VString _V_NSHomeDirectory();
VString _V_NSHomeDirectory() {
    NSString* currentUserHomePath = NSHomeDirectory(); // Note: autoreleased. Caller needs an autorelease pool context, e.g. VAutoreleasePool.
    VString path((CFStringRef) currentUserHomePath);
    return path;
}
