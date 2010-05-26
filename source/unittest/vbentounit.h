/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

#ifndef vbentounit_h
#define vbentounit_h

/** @file */

#include "vunit.h"

class VBentoNode;

/**
Unit test class for validating VBento.
*/
class VBentoUnit : public VUnit
    {
    public:
    
        /**
        Constructs a unit test object.
        @param    logOnSuccess    true if you want successful tests to be logged
        @param    throwOnError    true if you want an exception thrown for failed tests
        */
        VBentoUnit(bool logOnSuccess, bool throwOnError);
        /**
        Destructor.
        */
        virtual ~VBentoUnit() {}
        
        /**
        Executes the unit test.
        */
        virtual void run();

    protected:
    
        /**
        Verifies bento hierarchy contents as previously constructed.
        */
        void _verifyDynamicLengths();
        /**
        Builds some test data to be verified in various ways.
        */
        void _buildTestData(VBentoNode& root);
        /**
        Verifies bento hierarchy contents as previously constructed.
        */
        void _verifyContents(const VBentoNode& node, const VString& labelPrefix);
    };

#endif /* vbentounit_h */
