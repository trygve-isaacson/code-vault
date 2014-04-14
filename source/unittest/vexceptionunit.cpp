/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
*/

/** @file */

#include "vexceptionunit.h"
#include "vexception.h"

// This little class hierarchy is used to test the VcheckedDynamicCast template function.
class VExceptionUnit_ExampleBase {
    public:
        VExceptionUnit_ExampleBase() {}
        virtual ~VExceptionUnit_ExampleBase() {}
        virtual void hello() { std::cout << "hello(ExampleBase)" << std::endl; }
};
class VExceptionUnit_SubclassBranchA : public VExceptionUnit_ExampleBase {
    public:
        VExceptionUnit_SubclassBranchA() : VExceptionUnit_ExampleBase() {}
        virtual ~VExceptionUnit_SubclassBranchA() {}
        virtual void hello() { std::cout << "hello(SubclassBranchA)" << std::endl; }
};
class VExceptionUnit_SubclassBranchB : public VExceptionUnit_ExampleBase {
    public:
        VExceptionUnit_SubclassBranchB() : VExceptionUnit_ExampleBase() {}
        virtual ~VExceptionUnit_SubclassBranchB() {}
        virtual void hello() { std::cout << "hello(SubclassBranchB)" << std::endl; }
};

VExceptionUnit::VExceptionUnit(bool logOnSuccess, bool throwOnError) :
    VUnit("VExceptionUnit", logOnSuccess, throwOnError) {
}

void VExceptionUnit::run() {
    this->_testConstructors();
    this->_testCatchHierarchy();
    this->_testCheckedDynamicCast();
}

void VExceptionUnit::_testConstructors() {
    // Note that we must wrap the what() compares here in a VString in order
    // to get VString::operator== involved -- otherwise we are just
    // testing == on two raw char* values (pointers), which are only == if
    // the compiler happens to re-use the same literal const in memory *and*
    // the exception isn't using its internal VString to store the string.

    VException    ex1;
    VUNIT_ASSERT_TRUE_LABELED((ex1.getError() == VException::kGenericError) &&
               VString(ex1.what()).isEmpty(),
               "constructor 1");

    VException    ex2(-2, "ex2");
    VUNIT_ASSERT_TRUE_LABELED((ex2.getError() == -2) &&
               VString(ex2.what()) == "ex2",
               "constructor 2");

    VException    ex3(-3, VSTRING_FORMAT("ex%d", 3));
    VUNIT_ASSERT_TRUE_LABELED((ex3.getError() == -3) &&
               VString(ex3.what()) == "ex3",
               "constructor 3");

    VException    ex4(-4, VSTRING_FORMAT("ex%d", 4));
    VUNIT_ASSERT_TRUE_LABELED((ex4.getError() == -4) &&
               VString(ex4.what()) == "ex4",
               "constructor 4");

    VException    ex5("ex5");
    VUNIT_ASSERT_TRUE_LABELED((ex5.getError() == VException::kGenericError) &&
               VString(ex5.what()) == "ex5",
               "constructor 5");

    VException    ex6(VSTRING_FORMAT("ex%d", 6));
    VUNIT_ASSERT_TRUE_LABELED((ex6.getError() == VException::kGenericError) &&
               VString(ex6.what()) == "ex6",
               "constructor 6");

    VException    ex7(VSTRING_FORMAT("ex%d", 7));
    VUNIT_ASSERT_TRUE_LABELED((ex7.getError() == VException::kGenericError) &&
               VString(ex7.what()) == "ex7",
               "constructor 7");

    VEOFException    exEOF("EOF");
    VUNIT_ASSERT_TRUE_LABELED((exEOF.getError() == VException::kGenericError) &&
               VString(exEOF.what()) == "EOF",
               "EOF Exception constructor");

    VUnimplementedException    exUnimplemented("Unimplemented");
    VUNIT_ASSERT_TRUE_LABELED((exUnimplemented.getError() == VException::kGenericError) &&
               VString(exUnimplemented.what()).startsWith("Unimplemented"),
               "Unimplemented Exception constructor");
}

void VExceptionUnit::_testCatchHierarchy() {
    // We can try a few throws to verify type-correctness.

    try {
        throw VException("throw/catch VException");
        VUNIT_ASSERT_FAILURE("throw/catch VException");
    } catch (const VException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("throw/catch VException");
    } catch (...) {
        VUNIT_ASSERT_FAILURE("throw/catch VException");
    }

    try {
        throw VException("throw VException / catch std::exception");
        VUNIT_ASSERT_FAILURE("throw VException / catch std::exception");
    } catch (const std::exception& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("throw VException / catch std::exception");
    } catch (...) {
        VUNIT_ASSERT_FAILURE("throw VException / catch std::exception");
    }

    try {
        throw VEOFException("throw/catch VEOFException");
        VUNIT_ASSERT_FAILURE("throw/catch VEOFException");
    } catch (const VEOFException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("throw/catch VEOFException");
    } catch (...) {
        VUNIT_ASSERT_FAILURE("throw/catch VEOFException");
    }

    try {
        throw VUnimplementedException("throw/catch VUnimplementedException");
        VUNIT_ASSERT_FAILURE("throw/catch VUnimplementedException");
    } catch (const VUnimplementedException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("throw/catch VUnimplementedException");
    } catch (...) {
        VUNIT_ASSERT_FAILURE("throw/catch VUnimplementedException");
    }

    try {
        throw VEOFException("throw VEOFException / catch VException");
        VUNIT_ASSERT_FAILURE("throw VEOFException / catch VException");
    } catch (const VException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("throw VEOFException / catch VException");
    } catch (...) {
        VUNIT_ASSERT_FAILURE("throw VEOFException / catch VException");
    }

    try {
        throw VEOFException("throw VEOFException / catch std::exception");
        VUNIT_ASSERT_FAILURE("throw VEOFException / catch std::exception");
    } catch (const std::exception& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("throw VEOFException / catch std::exception");
    } catch (...) {
        VUNIT_ASSERT_FAILURE("throw VEOFException / catch std::exception");
    }

}

void VExceptionUnit::_testCheckedDynamicCast() {
    // Tests to verify proper function of VcheckedDynamicCast and its macros.
    VExceptionUnit_ExampleBase* branchB_asBase = new VExceptionUnit_SubclassBranchB();
    VExceptionUnit_ExampleBase* base = new VExceptionUnit_ExampleBase();
    VExceptionUnit_SubclassBranchA* branchA = new VExceptionUnit_SubclassBranchA();
    VExceptionUnit_SubclassBranchB* branchB = new VExceptionUnit_SubclassBranchB();
    VExceptionUnit_ExampleBase* nullPtr = NULL;

    VUNIT_ASSERT_NOT_NULL_LABELED(V_CHECKED_DYNAMIC_CAST(VExceptionUnit_SubclassBranchB*, branchB_asBase), "V_CHECKED_DYNAMIC_CAST normal dynamic cast");
    VUNIT_ASSERT_NOT_NULL_LABELED(V_CHECKED_DYNAMIC_CAST(VExceptionUnit_ExampleBase*, base), "V_CHECKED_DYNAMIC_CAST base -> base");
    VUNIT_ASSERT_NOT_NULL_LABELED(V_CHECKED_DYNAMIC_CAST(VExceptionUnit_ExampleBase*, branchA), "V_CHECKED_DYNAMIC_CAST subclass -> base");
    VUNIT_ASSERT_NULL_LABELED(V_CHECKED_DYNAMIC_CAST(VExceptionUnit_SubclassBranchA*, base), "V_CHECKED_DYNAMIC_CAST base -> subclass => null");
    VUNIT_ASSERT_NULL_LABELED(V_CHECKED_DYNAMIC_CAST(VExceptionUnit_SubclassBranchA*, branchB), "V_CHECKED_DYNAMIC_CAST subclass A -> subclass B => null");
    VUNIT_ASSERT_NULL_LABELED(V_CHECKED_DYNAMIC_CAST(VExceptionUnit_ExampleBase*, nullPtr), "V_CHECKED_DYNAMIC_CAST null => null");
#ifdef VPLATFORM_WIN /* The following junk can't sneak past the compiler on Mac or Linux GCC. */
    VUNIT_ASSERT_NULL_LABELED(V_CHECKED_DYNAMIC_CAST(VExceptionUnit_ExampleBase*, (VExceptionUnit*)this), "V_CHECKED_DYNAMIC_CAST other hierarchy => null");
#endif

    // Make sure the function is callable directly without the macro.
    VExceptionUnit_SubclassBranchA* nullResult = VcheckedDynamicCast<VExceptionUnit_SubclassBranchA*>(branchB_asBase, __FILE__, __LINE__, true, true, true);
    VUNIT_ASSERT_NULL_LABELED(nullResult, "VcheckedDynamicCast subclass A -> subclass B => null");
    VExceptionUnit_SubclassBranchB* nonNullResult = VcheckedDynamicCast<VExceptionUnit_SubclassBranchB*>(branchB_asBase, __FILE__, __LINE__, true, true, true);
    VUNIT_ASSERT_NOT_NULL_LABELED(nonNullResult, "VcheckedDynamicCast normal dynamic cast");

    /* The following behavior is VC++-specific. Some (but not all) garbage pointer values are detected
    by the dynamic_cast implementation and throw a __non_rtti_object (derived from std::exception)
    complaining about lack of RTTI data (at the dereferenced location, one presumes). I tried the pointer
    value 0x12345678 and got that behavior, so I am testing it here. So far, I don't know what the
    limitations are of dynamic_cast's abilities to detect error and throw an exception; and it obviously
    depends on the platform. This ability is really what we are trying to exploit in VcheckedDynamicCast().
    */
#ifdef VPLATFORM_WIN
    try {
        VExceptionUnit_ExampleBase* garbage = (VExceptionUnit_ExampleBase*) 0x12345678; // This value causes exception when casting on Windows.
        VExceptionUnit_ExampleBase* result;

        // This should return null and eat the expected exception.
        VLOGGER_INFO(VSTRING_FORMAT("Note: You may see a stack crawl for a bad dynamic cast originating at %s line %d after this line in the log. This is expected test output.", __FILE__, (__LINE__ + 1)));
        result = V_CHECKED_DYNAMIC_CAST_NOTHROW(VExceptionUnit_SubclassBranchA*, garbage);
        VUNIT_ASSERT_NULL_LABELED(result, "V_CHECKED_DYNAMIC_CAST_NOTHROW particular garbage => null");

        // This should throw an exception, so we should land in the catch block.
        VLOGGER_INFO(VSTRING_FORMAT("Note: You may see a stack crawl for a bad dynamic cast originating at %s line %d after this line in the log. This is expected test output.", __FILE__, (__LINE__ + 1)));
        result = V_CHECKED_DYNAMIC_CAST(VExceptionUnit_SubclassBranchA*, garbage);
        VUNIT_ASSERT_FAILURE("V_CHECKED_DYNAMIC_CAST particular garbage => throws exception");
    } catch (const VException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("V_CHECKED_DYNAMIC_CAST particular garbage => throws exception");
    }

#endif /* VPLATFORM_WIN */

    delete branchB_asBase;
    delete base;
    delete branchA;
    delete branchB;
}
