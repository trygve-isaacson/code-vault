/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

/** @file */

#include "vclassregistryunit.h"
#include "vclassregistry.h"
#include "vexception.h"

class ADynamicClass {
    public:

        ADynamicClass() : mTestValue(42) {}
        ~ADynamicClass() {}

        int mTestValue;
};

DEFINE_CLASSFACTORY(ADynamicClass, FactoryFor_ADynamicClass);
DECLARE_CLASSFACTORY(ADynamicClass, FactoryFor_ADynamicClass);

VClassRegistryUnit::VClassRegistryUnit(bool logOnSuccess, bool throwOnError) :
    VUnit("VClassRegistryUnit", logOnSuccess, throwOnError) {
}

void VClassRegistryUnit::run() {
    ADynamicClass x;    /// Let's make sure linker doesn't dead-strip its code.

    // Verify that we can properly get a factory object.
    ADynamicClass* dynamicObject =
        static_cast<ADynamicClass*>(VClassRegistry::registry()->instantiateObject(
                                        "ADynamicClass"));

    VUNIT_ASSERT_NOT_NULL_LABELED(dynamicObject, "class registry 1");
    VUNIT_ASSERT_EQUAL_LABELED(dynamicObject->mTestValue, 42, "class registry 2");
    delete dynamicObject;

    // Verify that specifying a bogus class name results in an exception.
    try {
        dynamicObject =
            static_cast<ADynamicClass*>(VClassRegistry::registry()->instantiateObject(
                                            "ABogusClassThatDoesNotExist"));
        VUNIT_ASSERT_FAILURE("class registry 3");
    } catch (const VException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("class registry 3");
    }

}

