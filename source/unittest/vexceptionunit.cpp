/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vexceptionunit.h"
#include "vexception.h"

// This little class hierarchy is used to test the VcheckedDynamicCast template function.
class VExceptionUnit_ExampleBase
    {
    public:
        VExceptionUnit_ExampleBase() {}
        virtual void hello() { std::cout << "hello(ExampleBase)" << std::endl; }
    };
class VExceptionUnit_SubclassBranchA : public VExceptionUnit_ExampleBase
    {
    public:
        VExceptionUnit_SubclassBranchA() : VExceptionUnit_ExampleBase() {}
        virtual void hello() { std::cout << "hello(SubclassBranchA)" << std::endl; }
    };
class VExceptionUnit_SubclassBranchB : public VExceptionUnit_ExampleBase
    {
    public:
        VExceptionUnit_SubclassBranchB() : VExceptionUnit_ExampleBase() {}
        virtual void hello() { std::cout << "hello(SubclassBranchB)" << std::endl; }
    };

VExceptionUnit::VExceptionUnit(bool logOnSuccess, bool throwOnError) :
VUnit("VExceptionUnit", logOnSuccess, throwOnError)
    {
    }

void VExceptionUnit::run()
    {
    this->_testConstructors();
    this->_testCatchHierarchy();
    this->_testCheckedDynamicCast();
    this->_testWin32SEH();
    }

void VExceptionUnit::_testConstructors()
    {
    // Note that we must wrap the what() compares here in a VString in order
    // to get VString::operator== involved -- otherwise we are just
    // testing == on two raw char* values (pointers), which are only == if
    // the compiler happens to re-use the same literal const in memory *and*
    // the exception isn't using its internal VString to store the string.

    VException    ex1;
    this->test((ex1.getError() == VException::kGenericError) &&
                VString(ex1.what()).isEmpty(),
                "constructor 1");

    VException    ex2(-2, "ex2");
    this->test((ex2.getError() == -2) &&
                VString(ex2.what()) == "ex2",
                "constructor 2");

    VException    ex3(-3, VString("ex%d", 3));
    this->test((ex3.getError() == -3) &&
                VString(ex3.what()) == "ex3",
                "constructor 3");

    VException    ex4(-4, VString("ex%d", 4));
    this->test((ex4.getError() == -4) &&
                VString(ex4.what()) == "ex4",
                "constructor 4");

    VException    ex5("ex5");
    this->test((ex5.getError() == VException::kGenericError) &&
                VString(ex5.what()) == "ex5",
                "constructor 5");

    VException    ex6(VString("ex%d", 6));
    this->test((ex6.getError() == VException::kGenericError) &&
                VString(ex6.what()) == "ex6",
                "constructor 6");

    VException    ex7(VString("ex%d", 7));
    this->test((ex7.getError() == VException::kGenericError) &&
                VString(ex7.what()) == "ex7",
                "constructor 7");

    VEOFException    exEOF("EOF");
    this->test((exEOF.getError() == VException::kGenericError) &&
                VString(exEOF.what()) == "EOF",
                "EOF Exception constructor");

    VUnimplementedException    exUnimplemented("Unimplemented");
    this->test((exUnimplemented.getError() == VException::kGenericError) &&
                VString(exUnimplemented.what()) == "Unimplemented",
                "Unimplemented Exception constructor");
    }

void VExceptionUnit::_testCatchHierarchy()
    {
    // We can try a few throws to verify type-correctness.
    bool    caughtAsExpected = false;

    try
        {
        throw VException("throw/catch VException");
        }
    catch (const VException& /*ex*/)
        {
        caughtAsExpected = true;
        }
    catch (...) {}
    
    this->test(caughtAsExpected, "throw/catch VException");
    

    try
        {
        caughtAsExpected = false;
        throw VException("throw VException / catch std::exception");
        }
    catch (const std::exception& /*ex*/)
        {
        caughtAsExpected = true;
        }
    catch (...) {}
    
    this->test(caughtAsExpected, "throw VException / catch std::exception");
    

    try
        {
        caughtAsExpected = false;
        throw VEOFException("throw/catch VEOFException");
        }
    catch (const VEOFException& /*ex*/)
        {
        caughtAsExpected = true;
        }
    catch (...) {}
    
    this->test(caughtAsExpected, "throw/catch VEOFException");
    

    try
        {
        caughtAsExpected = false;
        throw VUnimplementedException("throw/catch VUnimplementedException");
        }
    catch (const VUnimplementedException& /*ex*/)
        {
        caughtAsExpected = true;
        }
    catch (...) {}
    
    this->test(caughtAsExpected, "throw/catch VUnimplementedException");
    

    try
        {
        caughtAsExpected = false;
        throw VEOFException("throw VEOFException / catch VException");
        }
    catch (const VException& /*ex*/)
        {
        caughtAsExpected = true;
        }
    catch (...) {}
    
    this->test(caughtAsExpected, "throw VEOFException / catch VException");
    

    try
        {
        caughtAsExpected = false;
        throw VEOFException("throw VEOFException / catch std::exception");
        }
    catch (const std::exception& /*ex*/)
        {
        caughtAsExpected = true;
        }
    catch (...) {}
    
    this->test(caughtAsExpected, "throw VEOFException / catch std::exception");
    }

void VExceptionUnit::_testCheckedDynamicCast()
    {
    // Tests to verify proper function of VcheckedDynamicCast and its macros.
    VExceptionUnit_ExampleBase* branchB_asBase = new VExceptionUnit_SubclassBranchB();
    VExceptionUnit_ExampleBase* base = new VExceptionUnit_ExampleBase();
    VExceptionUnit_SubclassBranchA* branchA = new VExceptionUnit_SubclassBranchA();
    VExceptionUnit_SubclassBranchB* branchB = new VExceptionUnit_SubclassBranchB();
    VExceptionUnit_ExampleBase* nullPtr = NULL;

    this->test(V_CHECKED_DYNAMIC_CAST(VExceptionUnit_SubclassBranchB*, branchB_asBase) != NULL, "V_CHECKED_DYNAMIC_CAST normal dynamic cast");
    this->test(V_CHECKED_DYNAMIC_CAST(VExceptionUnit_ExampleBase*, base) != NULL, "V_CHECKED_DYNAMIC_CAST base -> base");
    this->test(V_CHECKED_DYNAMIC_CAST(VExceptionUnit_ExampleBase*, branchA) != NULL, "V_CHECKED_DYNAMIC_CAST subclass -> base");
    this->test(V_CHECKED_DYNAMIC_CAST(VExceptionUnit_SubclassBranchA*, base) == NULL, "V_CHECKED_DYNAMIC_CAST base -> subclass => null");
    this->test(V_CHECKED_DYNAMIC_CAST(VExceptionUnit_SubclassBranchA*, branchB) == NULL, "V_CHECKED_DYNAMIC_CAST subclass A -> subclass B => null");
    this->test(V_CHECKED_DYNAMIC_CAST(VExceptionUnit_ExampleBase*, nullPtr) == NULL, "V_CHECKED_DYNAMIC_CAST null => null");
    this->test(V_CHECKED_DYNAMIC_CAST(VExceptionUnit_ExampleBase*, this) == NULL, "V_CHECKED_DYNAMIC_CAST other hierarchy => null");

    // Make sure the function is callable directly without the macro.
    VExceptionUnit_SubclassBranchA* nullResult = VcheckedDynamicCast<VExceptionUnit_SubclassBranchA*>(branchB_asBase, __FILE__, __LINE__, true, true, true);
    this->test(nullResult == NULL, "VcheckedDynamicCast subclass A -> subclass B => null");
    VExceptionUnit_SubclassBranchB* nonNullResult = VcheckedDynamicCast<VExceptionUnit_SubclassBranchB*>(branchB_asBase, __FILE__, __LINE__, true, true, true);
    this->test(nonNullResult != NULL, "VcheckedDynamicCast normal dynamic cast");

    /* The following behavior is VC++-specific. Some (but not all) garbage pointer values are detected
    by the dynamic_cast implementation and throw a __non_rtti_object (derived from std::exception)
    complaining about lack of RTTI data (at the dereferenced location, one presumes). I tried the pointer
    value 0x12345678 and got that behavior, so I am testing it here. So far, I don't know what the
    limitations are of dynamic_cast's abilities to detect error and throw an exception; and it obviously
    depends on the platform. This ability is really what we are trying to exploit in VcheckedDynamicCast().
    */
#ifdef VPLATFORM_WIN    
    try
        {
        VExceptionUnit_ExampleBase* garbage = (VExceptionUnit_ExampleBase*) 0x12345678; // This value causes exception when casting on Windows.
        VExceptionUnit_ExampleBase* result;
        
        // This should return null and eat the expected exception.
        VLOGGER_INFO(VString("Note: You may see a stack crawl for a bad dynamic cast originating at %s line %d after this line in the log. This is expected test output.", __FILE__, (__LINE__ + 1)));
        result = V_CHECKED_DYNAMIC_CAST_NOTHROW(VExceptionUnit_SubclassBranchA*, garbage);
        this->test(result == NULL, "V_CHECKED_DYNAMIC_CAST_NOTHROW particular garbage => null");

        // This should throw an exception, so we should land in the catch block.
        VLOGGER_INFO(VString("Note: You may see a stack crawl for a bad dynamic cast originating at %s line %d after this line in the log. This is expected test output.", __FILE__, (__LINE__ + 1)));
        result = V_CHECKED_DYNAMIC_CAST(VExceptionUnit_SubclassBranchA*, garbage);
        this->test(false, "V_CHECKED_DYNAMIC_CAST particular garbage => throws exception");
        }
    catch (const VException& /*ex*/)
        {
        this->test(true, "V_CHECKED_DYNAMIC_CAST particular garbage => throws exception");
        }
    
#endif /* VPLATFORM_WIN */

    delete branchB_asBase;
    delete base;
    delete branchA;
    delete branchB;
    }

void VExceptionUnit::_testWin32SEH()
    {
#ifdef V_TRANSLATE_WIN32_STRUCTURED_EXCEPTIONS

    if (!VException::isWin32SEHandlerEnabled())
        return;

    bool caughtException = false;
    // See if we can correctly use Win32 exception translation.
    VString exceptionStack;
    try
        {
        caughtException = false;
        int* nullptr = NULL;
        *nullptr = 0; // kerblammo!
        }
    catch (const VException& ex)
        {
        caughtException = true;
        exceptionStack = ex.what();
        }

    this->test(caughtException, "Caught exception when writing to address 0. If caught, stack follows");
    if (caughtException)
        this->logStatus(exceptionStack);

    // 2010.03.21 JHR ARGO-24630 Unit test for access violations
    //   Tests are from Sparcs: testAccessViolationCatches
    try
        {
        caughtException = false;
        VString* nullPointer = NULL;
        nullPointer->format("will %s first", "crash");
        }
    catch (const VException& ex)
        {
        caughtException = true;
        exceptionStack = ex.what();
        }

    this->test(caughtException, "Caught exception when calling bad object vtable. If caught, stack follows");
    if (caughtException)
        this->logStatus(exceptionStack);

    // Really hard thing: method call from valid object pointer of totally wrong kind.
    try
        {
        caughtException = false;
        VString* vstrPointer = NULL;
        VString myString("aVString");
        VException* badPointer = NULL;

        vstrPointer = &myString;
        badPointer = (VException *)vstrPointer;
        myString = badPointer->what();
        }
    catch (const VException& ex)
        {
        caughtException = true;
        exceptionStack = ex.what();
        }

    this->test(caughtException, "Caught exception when calling wrong method call. If caught, stack follows");
    if (caughtException)
        this->logStatus(exceptionStack);

    try
        {
        caughtException = false;
        int* badPtr = (int*)0xDEADDEAD;
        *badPtr = 0; // kerblammo!
        }
    catch (const VException& ex)
        {
        caughtException = true;
        exceptionStack = ex.what();
        }

    this->test(caughtException, "Caught exception when writing to address 0xDEADDEAD. If caught, stack follows");
    if (caughtException)
        this->logStatus(exceptionStack);

    try
        {
        caughtException = false;
        int x = 0;
        int y = 5;
        int z = y / x;
        if (z > 4) {}
        }
    catch (const VException& ex)
        {
        caughtException = true;
        exceptionStack = ex.what();
        }

    this->test(caughtException, "Caught exception when dividing int by zero. If caught, stack follows");
    if (caughtException)
        this->logStatus(exceptionStack);

    try
        {
        caughtException = false;
        VDouble x = 0.0;
        VDouble y = 5.0;
        VDouble z = y / x;
        if (z > 4.0) {}
        }
    catch (const VException& ex)
        {
        caughtException = true;
        exceptionStack = ex.what();
        }

    this->test(caughtException, "Caught exception when dividing double by zero. If caught, stack follows");
    if (caughtException)
        this->logStatus(exceptionStack);

    try
        {
        caughtException = false;
        Vu8 buffer[8];
        Vs32* p = (Vs32*) &buffer[1];
        *p = 42;
        }
    catch (const VException& ex)
        {
        caughtException = true;
        exceptionStack = ex.what();
        }

    this->test(caughtException, "Caught exception when writing to misaligned address (32 bit write on odd byte boundary). If caught, stack follows");
    if (caughtException)
        this->logStatus(exceptionStack);

    try
        {
        caughtException = false;
        int things[4];
        things[20] = 42;
        }
    catch (const VException& ex)
        {
        caughtException = true;
        exceptionStack = ex.what();
        }

    this->test(caughtException, "Caught exception when writing to out of bounds stack array element. If caught, stack follows");
    if (caughtException)
        this->logStatus(exceptionStack);

    try
        {
        caughtException = false;
        int things[4];
        things[-4] = 42;
        }
    catch (const VException& ex)
        {
        caughtException = true;
        exceptionStack = ex.what();
        }

    this->test(caughtException, "Caught exception when writing to negative index stack array element. If caught, stack follows");
    if (caughtException)
        this->logStatus(exceptionStack);

#endif
    }
