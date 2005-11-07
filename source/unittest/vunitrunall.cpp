/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vunitrunall.h"

#include "vlogger.h"
#include "vinstant.h"

#include "vbentounit.h"
#include "vbinaryiounit.h"
#include "vcharunit.h"
#include "vclassregistryunit.h"
#include "vexceptionunit.h"
#include "vfsnodeunit.h"
#include "vhexunit.h"
#include "vinstantunit.h"
#include "vstreamsunit.h"
#include "vstringunit.h"
#include "vtextiostream.h"
#include "vthreadsunit.h"

#define UNIT_TEST(classname) \
    {  \
    classname u(logOnSuccess, throwOnError); \
    VUnit::runUnit(u, xmlOutputStream); \
    success = success && u.success(); \
    numSuccessfulTests += u.getNumSuccessfulTests(); \
    numFailedTests += u.getNumFailedTests(); \
    }

void runAllVUnitTests(bool logToFile, bool logOnSuccess, bool throwOnError, bool& success, int& numSuccessfulTests, int& numFailedTests, VTextIOStream* xmlOutputStream)
    {
    numSuccessfulTests = 0;
    numFailedTests = 0;
    
    if (logToFile)    // if not, it'll use default cout logger
        {
        VInstant    now;
        VString        nowString;
        
        now.getLocalString(nowString);
        // avoid file names with trouble characters in them - space, colon, slash are likely in date/time strings
        (void) nowString.replace(" ", "_");    
        (void) nowString.replace(":", "_");    
        (void) nowString.replace("/", "_");    
        
        VString    filePath("unit_%s.txt", nowString.chars());

        VLogger::installLogger(new VFileLogger(VLogger::kDebug, "VUnit", filePath));
        }

    success = true;
    
    // Use the macro above to declare each unit test and run it in a single line of boilerplate.

    UNIT_TEST(VBentoUnit)
    UNIT_TEST(VBinaryIOUnit)
    UNIT_TEST(VCharUnit)
    UNIT_TEST(VClassRegistryUnit)
    UNIT_TEST(VExceptionUnit)
    UNIT_TEST(VFSNodeUnit)
    UNIT_TEST(VHexUnit)
    UNIT_TEST(VInstantUnit)
    UNIT_TEST(VStreamsUnit)
    UNIT_TEST(VStringUnit)
    UNIT_TEST(VThreadsUnit)

    }

