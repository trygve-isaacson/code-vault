/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

#ifndef vstreamsunit_h
#define vstreamsunit_h

/** @file */

#include "vunit.h"

/**
Unit test class for validating several of the VStream-related classes.
*/
class VStreamsUnit : public VUnit {
    public:

        /**
        Constructs a unit test object.
        @param    logOnSuccess    true if you want successful tests to be logged
        @param    throwOnError    true if you want an exception thrown for failed tests
        */
        VStreamsUnit(bool logOnSuccess, bool throwOnError);
        /**
        Destructor.
        */
        virtual ~VStreamsUnit() {}

        /**
        Executes the unit test.
        */
        virtual void run();

    private:

        void _testWriteBufferedStream();
        void _testStreamCopier();
        void _testBufferOwnership();
        void _testReadOnlyStream();
        void _testOverloadedStreamCopyAPIs();
        void _testStreamTailer();
};

#endif /* vstreamsunit_h */
