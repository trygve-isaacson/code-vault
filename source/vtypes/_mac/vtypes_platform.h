/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
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
#define VPLATFORM_MAC

// We now make use of CF features in the Mac platform specific code all the time.
#define VAULT_CORE_FOUNDATION_SUPPORT

// Detect and define the compiler.
// Note: we can also check for __GNUC__
// We also need to detect the Metrowerks MSL library for some differences elsewhere
//   (below and in vinstant_platform.cpp).
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
#ifdef VAULT_BOOST_STRING_FORMATTING_SUPPORT
    #include <boost/format.hpp>
    // Unfortunately GCC does not have a save/restore diagnostic state pragma.
#endif

#ifndef VLIBRARY_METROWERKS
    #define V_HAVE_REENTRANT_TIME // we can and should use the _r versions of time.h calls
#endif

// Minimal includes needed to compile against the Vault header files.

#ifndef VCOMPILER_CODEWARRIOR
    #ifdef VAULT_CORE_FOUNDATION_SUPPORT
        #include <CoreFoundation/CFString.h>
    #endif
#endif

// Platform-specific definitions for types, byte order, min/max/abs/fabs, etc.

/*
The proprocessor macros __BIG_ENDIAN__ and __LITTLE_ENDIAN__ are defined by GCC,
which allows us to get the byte ordering right. In order to rely less on the
existence of these on early versions of GCC, let's just check for the newer
condition, __LITTLE_ENDIAN__ Mac on Intel.
*/
#if __LITTLE_ENDIAN__
    #define VBYTESWAP_NEEDED // This means "host order" and "network order" differ.
#endif

/*
The following conditional instructions are for compatibility when compiling using
CodeWarrior 8.3 with the Mac OS X 10.4 (Tiger) developer tools. There are some symbol
conflicts between the MSL headers and the BSD headers. This checks for the environment,
and then does what's necessary to resolve the symbol conflicts. Basically, we pull in
the BSD math.h header, and then undef a few symbols that MSL re-defines in a harmlessly
compatible way.

A separate issue with CW 8.3 and Tiger BSD headers is that the Tiger headers have added
an extraneous token after four #endif directives, and CW 8.3 calls it a syntax error.
This requires fixing the BSD headers in 4 places, by removing or commenting out the
extraneous token following the #endif directive. Here are the lines of code to fix:
/usr/include/netinet6/in6.h:307,338,347
/usr/include/netinet/in.h:498

Next fix required, if console_OS_X.c is part of the CW project, comment out:
{CodeWarrior}MSL/MSL_C/MSL_MacOS/Src/console_OS_X.c:9
The include of size_t.h causes a problem and removing it solves it.

Note that when we check for MAC_OS_X_VERSION_10_4 below, we're really checking to see
if we are building on 10.4 or later -- the 10.3 header only defines up to the 10.3
symbol.
*/

#include <AvailabilityMacros.h> /* so we can test MAC_OS_X_VERSION_xxxx values' presence */

#ifdef VLIBRARY_METROWERKS
    #ifdef MAC_OS_X_VERSION_10_4
        #define __FP__
        #include <math.h>
        #undef FP_NAN
        #undef FP_INFINITE
        #undef FP_ZERO
        #undef FP_NORMAL
        #undef FP_SUBNORMAL
        #undef HUGE_VALF
        #undef HUGE_VALL
    #endif /* MAC_OS_X_VERSION_10_4 */
#endif /* VLIBRARY_METROWERKS */

/*
The definition of V_ABS and V_FABS needs to vary quite a bit between CodeWarrior
(using Metrowerks Standard Library), GCC 3.x (on Mac OS X 10.3), and GCC 4.0 (on
Mac OS X 10.4), and even on CodeWarrior between Mac OS X 10.3 and 10.4, because
of which headers dominate.
*/

// min and max work the same everywhere
#define V_MIN(a, b) std::min(a, b)  ///< Macro for getting min of compatible values using standard function template.
#define V_MAX(a, b) std::max(a, b)  ///< Macro for getting max of compatible values using standard function template.

#ifdef VLIBRARY_METROWERKS

    // Metrowerks cannot access abs under 10.4, and in any case does not put abs or fabs in std namespace.

    #ifdef MAC_OS_X_VERSION_10_4
        #define V_ABS(a) ((a) < 0 ? (-(a)) : (a))   ///< Macro for getting abs of an integer value when standard functions / templates are not available.
    #else
        #define V_ABS(a) abs(a)                     ///< Macro for getting abs of an integer value using standard function.
    #endif

    #define V_FABS(a) abs(a)                        ///< Macro for getting abs of a floating point value using standard function.

#else

    // GCC can access std::abs under 10.3 and 10.4, but fabs is strangely inconsistent.

    #define V_ABS(a) std::abs(a)                    ///< Macro for getting abs of an integer value using standard function template.

    #ifdef MAC_OS_X_VERSION_10_4
        #include <math.h>
        #define V_FABS(a) fabs(a)                   ///< Macro for getting abs of a floating point value using standard function.
    #else
        #define V_FABS(a) std::fabs(a)              ///< Macro for getting abs of a floating point value using standard function template.
    #endif

#endif /* VLIBRARY_METROWERKS */

// vsnprintf(NULL, 0, . . .) behavior conforms to IEEE 1003.1 on CW and
// for GCC/10.4; I have not verified this on GCC/10.3 yet.
#define V_EFFICIENT_SPRINTF

// We treat IOS as essentially a subset of Mac OS X. This test must come after including AvailabilityMacros.h above.
#if defined(TARGET_OS_IPHONE) && (TARGET_OS_IPHONE == 1)
    #define VPLATFORM_MAC_IOS
#endif

#ifdef MAC_OS_X_VERSION_10_6 // SPI not available in pthread.h prior to the 10.6 headers.
    #ifndef VPLATFORM_MAC_IOS // SPI not available in iOS dev kit as of iOS 4 SDK
        #define VTHREAD_PTHREAD_SETNAME_SUPPORTED
    #endif
#endif

#endif /* vtypes_platform_h */
