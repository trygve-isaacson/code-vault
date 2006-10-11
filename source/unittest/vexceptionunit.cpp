/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vexceptionunit.h"
#include "vexception.h"

VExceptionUnit::VExceptionUnit(bool logOnSuccess, bool throwOnError)
: VUnit("VExceptionUnit", logOnSuccess, throwOnError)
    {
    }

void VExceptionUnit::run()
    {
    // About all we can unit test here is the contents of a VException object.
    
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

    VException    ex3(-3, "ex%d", 3);
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

    VException    ex6("ex%d", 6);
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

    // We can try a few throws to verify type-correctness.
    bool    caughtAsExpected;


    try
        {
        caughtAsExpected = false;
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

