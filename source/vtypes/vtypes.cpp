/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vtypes.h"

#include <iostream> // for namespace std

Vu16 VbyteSwap16(Vu16 a16BitValue)
    {
    Vu16    original = a16BitValue;
    Vu16    swapped;
    Vu8*    originalBytes = reinterpret_cast<Vu8*> (&original);
    Vu8*    swappedBytes = reinterpret_cast<Vu8*> (&swapped);
    
    swappedBytes[0] = originalBytes[1];
    swappedBytes[1] = originalBytes[0];
    
    return swapped;
    }

Vu32 VbyteSwap32(Vu32 a32BitValue)
    {
    Vu32    original = a32BitValue;
    Vu32    swapped;
    Vu8*    originalBytes = reinterpret_cast<Vu8*> (&original);
    Vu8*    swappedBytes = reinterpret_cast<Vu8*> (&swapped);
    
    swappedBytes[0] = originalBytes[3];
    swappedBytes[1] = originalBytes[2];
    swappedBytes[2] = originalBytes[1];
    swappedBytes[3] = originalBytes[0];
    
    return swapped;
    }

Vu64 VbyteSwap64(Vu64 a64BitValue)
    {
    Vu64    original = a64BitValue;
    Vu64    swapped;
    Vu8*    originalBytes = reinterpret_cast<Vu8*> (&original);
    Vu8*    swappedBytes = reinterpret_cast<Vu8*> (&swapped);
    
    swappedBytes[0] = originalBytes[7];
    swappedBytes[1] = originalBytes[6];
    swappedBytes[2] = originalBytes[5];
    swappedBytes[3] = originalBytes[4];
    swappedBytes[4] = originalBytes[3];
    swappedBytes[5] = originalBytes[2];
    swappedBytes[6] = originalBytes[1];
    swappedBytes[7] = originalBytes[0];
    
    return swapped;
    }

VFloat VbyteSwapFloat(VFloat a32BitValue)
    {
    /*
    The key here is avoid allowing the compiler to do any
    conversion of the float to int, which would cause truncation
    of the fractional value. That's what happens if you use
    VbyteSwap32 because the float gets converted to an int.
    */
    VFloat    original = a32BitValue;
    VFloat    swapped;
    Vu8*    originalBytes = reinterpret_cast<Vu8*> (&original);
    Vu8*    swappedBytes = reinterpret_cast<Vu8*> (&swapped);
    
    swappedBytes[0] = originalBytes[3];
    swappedBytes[1] = originalBytes[2];
    swappedBytes[2] = originalBytes[1];
    swappedBytes[3] = originalBytes[0];
    
    return swapped;
    }

//include "vlogger.h"

void Vassert(bool expression, const char* file, int line)
    {
    if (! expression)
        {
        // Place a breakpoint inside here to stop on an assertion failure.
        try
            {
            std::cout << "Assertion Failure at " << file << " line " << line << std::endl;
// FIXME: If we get here while logging, this could deadlock on the VLogger mutex. If we want
// to log the assertion safely, we need to ensure we're not being called from VLogger.
//            VLogger::getDefaultLogger()->log(VLogger::kFatal, file, line, "Assertion Failure.");
            }
        catch (...) {}
        
#ifndef VCOMPILER_MSVC
        //lint -e421 "Caution -- function 'abort(void)' is considered dangerous [MISRA Rule 126]"
        __assert(false, file, line);
#else
        assert(expression);
#endif
        }
    }

/*
We don't conditionally compile this according to V_DEBUG_STATIC_INITIALIZATION_TRACE;
rather, we always compile it, so that you have the option of turning it on per-file
instead of all-or-nothing. We let the linker decide whether anyone calls it.
*/

int Vtrace(const char* fileName, int lineNumber)
    {
    std::cout << "Static Initialization @ " << fileName << ":" << lineNumber << std::endl;
    return 0;
    }

