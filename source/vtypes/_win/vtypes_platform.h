/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

#ifndef vtypes_platform_h
#define vtypes_platform_h

/** @file */

/*
First, define the 2 fundamental properties:
- platform
- compiler
*/
// Define the platform.
#define VPLATFORM_WIN

// Detect and define the compiler.
// We also need to detect the Metrowerks MSL library for some differences elsewhere.

/*
Visual C++ version 6 and earlier have awful standard C++ compliance,
so for those crippled compilers we have do a few workarounds for basic
C++ work.

_MSC_VER is defined by MSVC++. The values I've been able to find
documented are:
1020 - version 4.2 (pretty old)
1200 - version 6 SP4 (a little old now but very widely used)
1300 - version 7 (a.k.a. "2003" or ".NET")
1310 - version 7.1 (patch/update to version 7)
1400 - version 8.0 (a.k.a. "2005")
1500 - version 9.0 (a.k.a. "2008")
1600 - version 10.0 (a.k.a. "2010")

Symbols we define conditionally:

VCOMPILER_CODEWARRIOR - Defined if the compiler is CodeWarrior.

VCOMPILER_MSVC - Defined if the compiler is Visual C++.

VCOMPILER_MSVC_6_CRIPPLED - Defined if the compiler is Visual C++
and the version is 6 or earlier, based on _MSC_VER. Workarounds
become required.
*/

#ifdef _MSC_VER
    #define VCOMPILER_MSVC

    #if _MSC_VER < 1300
        #define VCOMPILER_MSVC_6_CRIPPLED
    #endif

    // For the moment, turn off the new 8.0 library deprecation stuff.
    // Later, we can change the code to conditionally use the newer
    // function names, depending on the compiler version.
    #if _MSC_VER >= 1400
        #define _CRT_SECURE_NO_DEPRECATE
    #endif

#endif

#ifdef __MWERKS__
    #define VCOMPILER_CODEWARRIOR
    #define VLIBRARY_METROWERKS    /* might want to consider adding a check for using CW but not MSL */
#endif

/*
Second, include the user-editable header file that configures
the desired Vault features/behavior. It can decide based on
the fundamental properties defined above.
*/
#include "vconfigure.h"

/*
Finally, proceed with everything else.
*/

// Boost seems to require being included first.
// If using shared_ptr, must include before our vtypes.h redefines new.
#ifdef VAULT_BOOST_STRING_FORMATTING_SUPPORT
    #define V_INCLUDE_BOOST_CORE
#endif
#ifdef VAULT_BOOST_SHARED_PTR_INCLUDE
    #define V_INCLUDE_BOOST_CORE
#endif
#ifdef V_INCLUDE_BOOST_CORE
    // Prevent spurious VC8 warnings about "deprecated"/"unsafe" boost std c++ lib use.
    #if _MSC_VER >= 1400
        #pragma warning(push)
        #pragma warning(disable : 4996)
    #endif
#endif

#ifdef VAULT_BOOST_STRING_FORMATTING_SUPPORT
    #include <boost/format.hpp>
#endif
#ifdef VAULT_BOOST_SHARED_PTR_INCLUDE
    #include <boost/shared_ptr.hpp>
    #include <boost/weak_ptr.hpp>
    #include <boost/enable_shared_from_this.hpp>
#endif

#ifdef V_INCLUDE_BOOST_CORE
    #if _MSC_VER >= 1400
        #pragma warning(pop)
    #endif
#endif

// Set our standard symbol indicating a 32/64-bit compile.
// If you need to set it manually in vconfigure.h, that will be respected.
// You really should write code that works in either mode, but if you need to check, use this.
#if !defined(VCOMPILER_32BIT) && !defined(VCOMPILER_64BIT)
    #if (_WIN32 || _WIN64)
        #if _WIN64
            #define VCOMPILER_64BIT
        #else
            #define VCOMPILER_32BIT
        #endif
    #else
        #define VCOMPILER_32BIT /* something to fall back on for unknown compilers, may need more */
    #endif
#endif

// Only VC 2010 and later have stdint.h, so if necessary, define ourself what vtypes.h relies on from there.
#if _MSC_VER >= 1600
	#include <stdint.h>
#else
    typedef signed char         int8_t;
    typedef unsigned char       uint8_t;
    typedef short               int16_t;
    typedef unsigned short      uint16_t;
    typedef int                 int32_t;
    typedef unsigned int        uint32_t;
    typedef signed long long    int64_t;
    typedef unsigned long long  uint64_t;

    #define INT8_MAX         127
    #define INT16_MAX        32767
    #define INT32_MAX        2147483647
    #define INT64_MAX        9223372036854775807LL

    #define INT8_MIN          -128
    #define INT16_MIN         -32768
       /*
          Note:  the literal "most negative int" cannot be written in C --
          the rules in the standard (section 6.4.4.1 in C99) will give it
          an unsigned type, so INT32_MIN (and the most negative member of
          any larger signed type) must be written via a constant expression.
       */
    #define INT32_MIN        (-INT32_MAX-1)
    #define INT64_MIN        (-INT64_MAX-1)

    #define UINT8_MAX         255
    #define UINT16_MAX        65535
    #define UINT32_MAX        4294967295U
    #define UINT64_MAX        18446744073709551615ULL

    #ifndef SIZE_MAX
        #ifdef _WIN64
            #define SIZE_MAX    UINT64_MAX
        #else
            #define SIZE_MAX    UINT32_MAX
        #endif
    #endif

#endif

#define V_SIZE_T_IS_UNSIGNED_INT /* In MS headers, size_t is typedef of unsigned int. This define is used to avoid defining fn overloads that conflict. */

// Minimal includes needed to compile against the Vault header files.

#include <winsock2.h>
#include <windows.h>
#include <algorithm> // std::find
#include <math.h>

#ifdef VCOMPILER_CODEWARRIOR
    #include <time.h>
    #include <stdio.h>
#endif

// Platform-specific definitions for types, byte order, min/max/abs/fabs, etc.

// If Windows globally #defines these as preprocessor macros, they cannot
// be used as method names! Get rid of them.
#undef min
#undef max
#undef abs

/*
Here are the workarounds we need to define if we're compiling under
the crippled VC++ 6 compiler.
*/
#ifdef VCOMPILER_MSVC_6_CRIPPLED
    // 64-bit integer definitions are non-standard. Should be signed/unsigned long long.
    typedef LONGLONG  Vs64;
    typedef ULONGLONG Vu64;
    // 64-bit constants definitions are non-standard. Should be LL and ULL.
    #define CONST_S64(s) s##i64
    #define CONST_U64(s) s##i64
    // Static constants are not supported, so this is a way to create them.
    #define CLASS_CONST(type, name, init) enum { name = init }
    // Scoped variables do not work. This works around that.
    #define for if(false);else for
#endif

/*
Since Windows currently only runs on X86 (well, little-endian) processors, we
can simply define the byte-swapping macros to be unconditionally enabled. If
Windows ever runs on a big-endian processor, these macros can just be conditionally
defined as they are on Unix and Mac, based on the processor type.
*/
#define VBYTESWAP_NEEDED // This means "host order" and "network order" differ.

/*
VC++ up to and including 7.1 does not like using std::min(), std::max(), std::abs()
so our convenience macros V_MIN, V_MAX, and V_ABS need to be implemented as
old-fashioned C preprocessor macros rather than standard C++ library inlines.
*/
#ifdef VCOMPILER_MSVC
    #if _MSC_VER < 1600 // VC++ 10.0 (2010) (_MSC_VER = 1600) has std::min/max/abs/fabs available; not sure about 1400/1500.
        #define DEFINE_V_MINMAXABS 1
    #endif
#endif

#ifdef DEFINE_V_MINMAXABS

    #define V_MIN(a, b) ((a) > (b) ? (b) : (a)) ///< Macro for getting min of compatible values when standard functions / templates are not available.
    #define V_MAX(a, b) ((a) > (b) ? (a) : (b)) ///< Macro for getting max of compatible values when standard functions / templates are not available.
    #define V_ABS(a) ((a) < 0 ? (-(a)) : (a))   ///< Macro for getting abs of an integer value when standard functions / templates are not available.
    #define V_FABS(a) ((a) < 0 ? (-(a)) : (a))  ///< Macro for getting abs of a floating point value when standard functions / templates are not available.

#else

    #define V_MIN(a, b) std::min(a, b) ///< Macro for getting min of compatible values using standard function template.
    #define V_MAX(a, b) std::max(a, b) ///< Macro for getting max of compatible values using standard function template.
    #define V_ABS(a) std::abs(a)       ///< Macro for getting abs of an integer value using standard function template.
    #define V_FABS(a) std::fabs(a)     ///< Macro for getting abs of a floating point value using standard function template.

#endif /* DEFINE_V_MINMAXABS */

// vsnprintf(NULL, 0, . . .) behavior conforms to IEEE 1003.1 on CW/Win and VC++ 8
// (may need to set conditionally for older versions of VC++).
#define V_EFFICIENT_SPRINTF

// Suppress incorrect warnings from Level 4 VC++ warning level. (/W4 option)
#ifdef VCOMPILER_MSVC
    // VLOGGER_xxx macros properly use "do ... while(false)" but this emits warning 4127.
    #pragma warning(disable: 4127)
#endif

#ifdef VCOMPILER_MSVC
    #ifdef VAULT_WIN32_STRUCTURED_EXCEPTION_TRANSLATION_SUPPORT
        #define V_TRANSLATE_WIN32_STRUCTURED_EXCEPTIONS // This is the private symbol we actually used. Depends on user setting, platform, and compiler.
    #endif
#endif

#endif /* vtypes_platform_h */
