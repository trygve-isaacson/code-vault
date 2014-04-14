/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
*/

/** @file */

#include "vclassregistry.h"

#include "vexception.h"

V_STATIC_INIT_TRACE

// VClassFactory -------------------------------------------------------------

VClassFactory::VClassFactory(const VString& classID)
    : mClassID(classID)
    {
}

bool VClassFactory::matchesClassID(const VString& classID) const {
    return mClassID == classID;
}

void VClassFactory::getClassID(VString& classID) const {
    classID = mClassID;
}

// VClassRegistry ------------------------------------------------------------

// static
VClassRegistry* VClassRegistry::gRegistry = NULL;

// static
VClassRegistry* VClassRegistry::registry() {
    static bool initialized = false;    // static, so will be executed 1st time only

    // We're using the "create on first use" idiom here.
    if (! initialized) {
        gRegistry = new VClassRegistry();
        initialized = true;
    }

    return gRegistry;
}

VClassRegistry::VClassRegistry() :
    mFactories() { // -> empty
}

VClassRegistry::~VClassRegistry() {
    for (VSizeType i = 0; i < mFactories.size(); ++i) {
        delete mFactories[i];
    }
}

void* VClassRegistry::instantiateObject(const VString& classID) const {
    const VClassFactory* factory = this->findClassFactory(classID);

    return factory->instantiateObject();
}

void VClassRegistry::registerClass(const VClassFactory* factory) {
    mFactories.push_back(factory);
}

const VClassFactory* VClassRegistry::findClassFactory(const VString& classID) const {
    // FIXME: This would be a lot more efficient using a map instead of a vector.
    for (VSizeType i = 0; i < mFactories.size(); ++i) {
        if (mFactories[i]->matchesClassID(classID)) {
            return mFactories[i];
        }
    }

    throw VStackTraceException(VSTRING_FORMAT("Unable to find class factory for '%s' in class registry.", classID.chars()));
}

