/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

#ifndef vfsnodeunit_h
#define vfsnodeunit_h

/** @file */

#include "vunit.h"
#include "vfsnode.h"

/**
Unit test class for validating VFSNode.
*/
class VFSNodeUnit : public VUnit
    {
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
    
        void _testTextFileIO(const VString& seriesLabel, VFSNode& node, bool buffered);
        void _testBinaryFileIO(const VString& seriesLabel, VFSNode& node, bool buffered);
    };

#endif /* vfsnodeunit_h */
