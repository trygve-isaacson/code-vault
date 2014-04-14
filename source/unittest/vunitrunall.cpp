/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

/** @file */

#include "vunitrunall.h"

#include "vlogger.h"
#include "vinstant.h"

#include "vassertunit.h"
#include "vbentounit.h"
#include "vbinaryiounit.h"
#include "vcharunit.h"
#include "vclassregistryunit.h"
#include "vexceptionunit.h"
#include "vfsnodeunit.h"
#include "vgeometryunit.h"
#include "vcolorunit.h"
#include "vhexunit.h"
#include "vinstantunit.h"
#include "vplatformunit.h"
#include "vstreamsunit.h"
#include "vstringunit.h"
#include "vtextiostream.h"
#include "vthreadsunit.h"
#include "vmessageunit.h"
#include "vloggerunit.h"

#define UNIT_TEST(classname) \
    {  \
    classname u(logOnSuccess, throwOnError); \
    try \
        { \
        VUnit::runUnit(u, writers); \
        success = success && u.success(); \
        numSuccessfulTests += u.getNumSuccessfulTests(); \
        numFailedTests += u.getNumFailedTests(); \
        } \
    catch (...) \
        { \
        success = false; \
        numSuccessfulTests += u.getNumSuccessfulTests(); \
        numFailedTests += u.getNumFailedTests(); \
        if (throwOnError) \
            throw; \
        } \
    }

void runAllVUnitTests(bool logOnSuccess, bool throwOnError, bool& success, int& numSuccessfulTests, int& numFailedTests, VUnitOutputWriterList* writers) {
    numSuccessfulTests = 0;
    numFailedTests = 0;
    success = true;

    // Use the macro above to declare each unit test and run it in a single line of boilerplate.
    UNIT_TEST(VPlatformUnit)
    UNIT_TEST(VAssertUnit)
    UNIT_TEST(VBentoUnit)
    UNIT_TEST(VBinaryIOUnit)
    UNIT_TEST(VCharUnit)
    UNIT_TEST(VClassRegistryUnit)
    UNIT_TEST(VExceptionUnit)
    UNIT_TEST(VFSNodeUnit)
    UNIT_TEST(VGeometryUnit)
    UNIT_TEST(VColorUnit)
    UNIT_TEST(VHexUnit)
    UNIT_TEST(VInstantUnit)
    UNIT_TEST(VStreamsUnit)
    UNIT_TEST(VStringUnit)
    UNIT_TEST(VThreadsUnit)
    UNIT_TEST(VMessageUnit)
    UNIT_TEST(VLoggerUnit)
}

