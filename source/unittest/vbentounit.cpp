/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vbentounit.h"
#include "vbento.h"
#include "vexception.h"

VBentoUnit::VBentoUnit(bool logOnSuccess, bool throwOnError)
: VUnit("VBentoUnit", logOnSuccess, throwOnError)
    {
    }

void VBentoUnit::run()
    {
    // We'll just populate a Bento container with every type, and then
    // retrieve them and validate.
    
    VBentoNode    root("root");
    
    root.addS8Value("s8", 100);
    root.addU8Value("u8", 200);
    root.addS16Value("s16", 300);
    root.addU16Value("u16", 400);
    root.addS32Value("s32", 500);
    root.addU32Value("u32", 600);
    root.addS64Value("s64", CONST_S64(700));
    root.addU64Value("u64", CONST_U64(800));
    root.addBoolValue("bool", true);
    root.addStringValue("vstr", "bento unit test");
    
    VBentoNode*    childNode = new VBentoNode("child");
    childNode->addS32Value("ch32", 900);
    root.addChildNode(childNode);
    
    // We'll call the throwing getters. We test for the correct result,
    // but also know that if the value is not found, it'll throw.
    try
        {
        this->test(root.getS8Value("s8") == 100, "s8");
        this->test(root.getU8Value("u8") == 200, "u8");
        this->test(root.getS16Value("s16") == 300, "s16");
        this->test(root.getU16Value("u16") == 400, "u16");
        this->test(root.getS32Value("s32") == 500, "s32");
        this->test(root.getU32Value("u32") == 600, "u32");
        this->test(root.getS64Value("s64") == CONST_S64(700), "s64");
        this->test(root.getU64Value("u64") == CONST_U64(800), "u64");
        this->test(root.getBoolValue("bool") == true, "bool");
        VString* s = root.getStringValue("vstr");
        this->test(*s == "bento unit test", "vstr");
        delete s;    // it was allocated for us in the form of get() we used
        const VBentoNode* child = root.findNode("child");
        this->test(child != NULL, "child");
        if (child != NULL)    // in case we aren't aborting on earlier failures
            this->test(child->getS32Value("ch32") == 900, "ch32");
        }
    catch (const VException& ex)
        {
        this->test(false, VString("VBentoUnit threw an exception: %s", ex.what()));
        }

    }

