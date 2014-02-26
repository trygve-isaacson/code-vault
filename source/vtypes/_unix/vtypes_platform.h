/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
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
#define VPLATFORM_UNIX

// Maybe there's a cleaner way to do this; but we need to distinguish HP-UX below.
#ifdef _HPUX_SOURCE
    #define VPLATFORM_UNIX_HPUX
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
    #include <boost/format.hpp>
    #include <boost/shared_ptr.hpp>
    #include <boost/weak_ptr.hpp>
    #include <boost/enable_shared_from_this.hpp>
    // Unfortunately GCC does not have a save/restore diagnostic state pragma.
#endif

#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <algorithm>
#include <limits.h> // LONG_MAX
#include <memory> // auto_ptr
#include <typeinfo> // std::bad_cast, etc.

#ifdef VPLATFORM_UNIX_HPUX
    // On HPUX, std::abs(float) is not defined. For now, we can revert to macro style.
    // FIXME: do this more selectively rather than all-or-nothing below; just do V_FABS
    // rather than all 4 macros.
    #define DEFINE_V_MINMAXABS
#endif

#define V_HAVE_REENTRANT_TIME    // we can and should use the _r versions of time.h calls

// Set our standard symbol indicating a 32/64-bit compile.
// If you need to set it manually in vconfigure.h, that will be respected.
// You really should write code that works in either mode, but if you need to check, use this.
#if !defined(VCOMPILER_32BIT) && !defined(VCOMPILER_64BIT)
    #if __GNUC__
        #if __x86_64__ || __ppc64__
            #define VCOMPILER_64BIT
        #else
            #define VCOMPILER_32BIT
        #endif
    #else
        #define VCOMPILER_32BIT /* something to fall back on for unknown compilers, may need more */
    #endif
#endif

// For Linux at least, we need endian.h explicitly, and the __USE_BSD symbol gets the endian
// symbol BYTE_ORDER defined as appropriate for the processor architecture.
#define USE_BSD
#include <endian.h>

/* In case BYTE_ORDER stuff from <endian.h> is not defined, we'll
   define the environment as big-endian. */
#ifndef BYTE_ORDER
    #define LITTLE_ENDIAN   1234    /* LSB first: i386, vax */
    #define BIG_ENDIAN      4321    /* MSB first: 68000, ibm, net */
    #define PDP_ENDIAN      3412    /* LSB first in word, MSW first in long */
    #define BYTE_ORDER      BIG_ENDIAN
#endif

/*
We define our own platform-neutral byte-swapping macros and functions, because
the platform headers are way too inconsistent about what they support and how.

If the host system uses little endian byte layout, our byte swapping macros
actually call the swapping functions. If not, the macros do nothing.
*/

#if BYTE_ORDER == LITTLE_ENDIAN
    #define VBYTESWAP_NEEDED // This means "host order" and "network order" differ.
#endif

/*
min/max/abs/fabs are inconsistently defined and supported, so we have our
own portable versions.
*/
#ifdef DEFINE_V_MINMAXABS

    #define V_MIN(a, b) ((a) > (b) ? (b) : (a)) ///< Macro for getting min of compatible values when standard functions / templates are not available.
    #define V_MAX(a, b) ((a) > (b) ? (a) : (b)) ///< Macro for getting max of compatible values when standard functions / templates are not available.
    #define V_ABS(a) ((a) < 0 ? (-(a)) : (a))   ///< Macro for getting abs of an integer value when standard functions / templates are not available.
    #define V_FABS(a) ((a) < 0 ? (-(a)) : (a))  ///< Macro for getting abs of a floating point value when standard functions / templates are not available.

#else

    #define V_MIN(a, b) std::min(a, b)  ///< Macro for getting min of compatible values using standard function template.
    #define V_MAX(a, b) std::max(a, b)  ///< Macro for getting max of compatible values using standard function template.
    #define V_ABS(a) std::abs(a)        ///< Macro for getting abs of an integer value using standard function template.
    #define V_FABS(a) fabs(a)      ///< Macro for getting abs of a floating point value using standard function template.

#endif /* DEFINE_V_MINMAXABS */

// vsnprintf(NULL, 0, . . .) behavior generally conforms to IEEE 1003.1,
// but not on HP-UX.
#ifndef VPLATFORM_UNIX_HPUX
    #define V_EFFICIENT_SPRINTF
#endif

#endif /* vtypes_platform_h */
