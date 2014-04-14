/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
*/

/** @file */

#include "vtypes.h"
#include "vtypes_internal.h"
#include "vstring.h"
#include "vexception.h"

#include <iostream> // for namespace std
#include <assert.h>

// Still to be determined is what constant value/type should be used here when
// performing 64-bit VC++ compilation. Until then, the optional "/Wp64" option
// in the 32-bit VC++ compiler ("Detect 64-bit Portability Issues") will emit
// a warning #4312 so we want to disable that for this line of code. The warning
// is because we are using a 32-bit constant 0xFEEEFEEE. Simply making it a
// 64-bit constant is not portable either because that overflows in 32 bits.
#ifdef VCOMPILER_MSVC
    #pragma warning(disable: 6001)  // VS2010 static analysis is confused by our byte swapping functions.
    #pragma warning(disable: 4312)
#endif
const void* const VCPP_DEBUG_BAD_POINTER_VALUE = reinterpret_cast<const void*>(0xFEEEFEEE);
#ifdef VCOMPILER_MSVC
    #pragma warning(default: 4312)
#endif

Vu16 vault::VbyteSwap16(Vu16 a16BitValue) {
    Vu16    original = a16BitValue;
    Vu16    swapped;
    Vu8*    originalBytes = reinterpret_cast<Vu8*>(&original);
    Vu8*    swappedBytes = reinterpret_cast<Vu8*>(&swapped);

    swappedBytes[0] = originalBytes[1];
    swappedBytes[1] = originalBytes[0];

    return swapped;
}

Vu32 vault::VbyteSwap32(Vu32 a32BitValue) {
    Vu32    original = a32BitValue;
    Vu32    swapped;
    Vu8*    originalBytes = reinterpret_cast<Vu8*>(&original);
    Vu8*    swappedBytes = reinterpret_cast<Vu8*>(&swapped);

    swappedBytes[0] = originalBytes[3];
    swappedBytes[1] = originalBytes[2];
    swappedBytes[2] = originalBytes[1];
    swappedBytes[3] = originalBytes[0];

    return swapped;
}

Vu64 vault::VbyteSwap64(Vu64 a64BitValue) {
    Vu64    original = a64BitValue;
    Vu64    swapped;
    Vu8*    originalBytes = reinterpret_cast<Vu8*>(&original);
    Vu8*    swappedBytes = reinterpret_cast<Vu8*>(&swapped);

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

VFloat vault::VbyteSwapFloat(VFloat a32BitValue) {
    /*
    The key here is avoid allowing the compiler to do any
    conversion of the float to int, which would cause truncation
    of the fractional value. That's what happens if you use
    VbyteSwap32 because the float gets converted to an int.
    */
    VFloat  original = a32BitValue;
    VFloat  swapped;
    Vu8*    originalBytes = reinterpret_cast<Vu8*>(&original);
    Vu8*    swappedBytes = reinterpret_cast<Vu8*>(&swapped);

    swappedBytes[0] = originalBytes[3];
    swappedBytes[1] = originalBytes[2];
    swappedBytes[2] = originalBytes[1];
    swappedBytes[3] = originalBytes[0];

    return swapped;
}

VDouble vault::VbyteSwapDouble(VDouble a64BitValue) {
    /*
    The key here is avoid allowing the compiler to do any
    conversion of the double to Vs64, which would cause truncation
    of the fractional value. That's what happens if you use
    VbyteSwap64 because the double gets converted to a Vs64.
    */
    VDouble original = a64BitValue;
    VDouble swapped;
    Vu8*    originalBytes = reinterpret_cast<Vu8*>(&original);
    Vu8*    swappedBytes = reinterpret_cast<Vu8*>(&swapped);

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

/*
We don't conditionally compile this according to V_DEBUG_STATIC_INITIALIZATION_TRACE;
rather, we always compile it, so that you have the option of turning it on per-file
instead of all-or-nothing. We let the linker decide whether anyone calls it.
*/
int Vtrace(const char* fileName, int lineNumber) {
    std::cout << "Static Initialization @ " << fileName << ":" << lineNumber << std::endl;
    return 0;
}

