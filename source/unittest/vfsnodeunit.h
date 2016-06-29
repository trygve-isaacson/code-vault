/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

#ifndef vfsnodeunit_h
#define vfsnodeunit_h

/** @file */

#include "vunit.h"
#include "vfsnode.h"
#include "vabstractfilestream.h"

/**
Unit test class for validating VFSNode.
*/
class VFSNodeUnit : public VUnit {
    public:

        /**
        Constructs a unit test object.
        @param    logOnSuccess    true if you want successful tests to be logged
        @param    throwOnError    true if you want an exception thrown for failed tests
        */
        VFSNodeUnit(bool logOnSuccess, bool throwOnError);
        /**
        Destructor.
        */
        virtual ~VFSNodeUnit() {}

        /**
        Executes the unit test.
        */
        virtual void run();

    private:

        void _testTextFileIO(const VString& seriesLabel, VFSNode& node, VAbstractFileStream& fileStream);
        void _testTextFileReadAll(VFSNode& node);
        void _testBinaryFileIO(const VString& seriesLabel, VFSNode& node, VAbstractFileStream& fileStream);
        void _testDirectoryIteration(const VFSNode& dir);
        void _writeKnownDirectoryTestFile(VFSNode::KnownDirectoryIdentifier id, const VString& fileName);
        void _testWindowsDrivePaths(const VString& driveLetter, const VString& childNodeName, bool adornedWithSlash, bool childIsDirectory);

        VStringVector   mTextFileLines;
        VString         mTextFileContents;
};

#endif /* vfsnodeunit_h */
