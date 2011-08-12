/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vclassregistryunit.h"
#include "vclassregistry.h"
#include "vexception.h"

class ADynamicClass
    {
    public:
    
        ADynamicClass() : mTestValue(42) {}
        ~ADynamicClass() {}
        
        int mTestValue;
    };

DEFINE_CLASSFACTORY(ADynamicClass, FactoryFor_ADynamicClass);
DECLARE_CLASSFACTORY(ADynamicClass, FactoryFor_ADynamicClass);

VClassRegistryUnit::VClassRegistryUnit(bool logOnSuccess, bool throwOnError) :
VUnit("VClassRegistryUnit", logOnSuccess, throwOnError)
    {
    }

void VClassRegistryUnit::run()
    {
    ADynamicClass x;    /// Let's make sure linker doesn't dead-strip its code.
    
    // Verify that we can properly get a factory object.
    ADynamicClass* dynamicObject =
        static_cast<ADynamicClass*>(VClassRegistry::registry()->instantiateObject(
        "ADynamicClass"));

    this->test(dynamicObject != NULL, "class registry 1");
    this->test(dynamicObject->mTestValue == 42, "class registry 2");
    delete dynamicObject;

    // Verify that specifying a bogus class name results in an exception.
    bool caughtException = false;
    try
        {
        dynamicObject =
            static_cast<ADynamicClass*>(VClassRegistry::registry()->instantiateObject(
            "ABogusClassThatDoesNotExist"));
        }
    catch (const VException& /*ex*/)
        {
        caughtException = true;
        }

    this->test(caughtException, "class registry 3");

    }

