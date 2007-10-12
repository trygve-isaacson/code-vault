/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

#ifndef vtypes_platform_h
#define vtypes_platform_h

/** @file */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <fcntl.h>

#define _BSD_WCHAR_T_DEFINED_
#ifdef _lint
    typedef _BSD_WCHAR_T_ wchar_t;
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <limits.h>

#ifdef HAVE_MALLOC_H
    #include <malloc.h>
#endif

#include <unistd.h>

#define VPLATFORM_MAC

// Note: we can also check for __GNUC__

#ifdef __MWERKS__
    #define VCOMPILER_CODEWARRIOR
    #define VLIBRARY_METROWERKS    /* might want to consider adding a check for using CW but not MSL */
#endif

#ifndef VLIBRARY_METROWERKS
    #define V_HAVE_REENTRANT_TIME // we can and should use the _r versions of time.h calls
#endif

#ifndef VCOMPILER_CODEWARRIOR
    #ifdef VAULT_CORE_FOUNDATION_SUPPORT
        #include <CoreFoundation/CFString.h>
    #endif
#endif

#define O_BINARY 0x8000 ///< Macro to define O_BINARY mode, which is not in the standard headers.

#undef FD_ZERO
#define FD_ZERO(p) memset(p, 0, sizeof(*(p)))

/*
The proprocessor macros __BIG_ENDIAN__ and __LITTLE_ENDIAN__ are defined by GCC,
which allows us to get the byte ordering right. In order to rely less on the
existence of these on early versions of GCC, let's just check for the newer
condition, __LITTLE_ENDIAN__ Mac on Intel.
*/
#if __LITTLE_ENDIAN__
	#define VBYTESWAP_NEEDED

	#define V_BYTESWAP_HTONS_GET(x)			vault::VbyteSwap16((Vu16) x)
	#define V_BYTESWAP_NTOHS_GET(x)			vault::VbyteSwap16((Vu16) x)
	#define V_BYTESWAP_HTONS_IN_PLACE(x)	((x) = (vault::VbyteSwap16((Vu16) x)))
	#define V_BYTESWAP_NTOHS_IN_PLACE(x)	((x) = (vault::VbyteSwap16((Vu16) x)))

	#define V_BYTESWAP_HTONL_GET(x)			vault::VbyteSwap32((Vu32) x)
	#define V_BYTESWAP_NTOHL_GET(x)			vault::VbyteSwap32((Vu32) x)
	#define V_BYTESWAP_HTONL_IN_PLACE(x)	((x) = (vault::VbyteSwap32((Vu32) x)))
	#define V_BYTESWAP_NTOHL_IN_PLACE(x)	((x) = (vault::VbyteSwap32((Vu32) x)))

	#define V_BYTESWAP_HTON64_GET(x)		vault::VbyteSwap64((Vu64) x)
	#define V_BYTESWAP_NTOH64_GET(x)		vault::VbyteSwap64((Vu64) x)
	#define V_BYTESWAP_HTON64_IN_PLACE(x)	((x) = (vault::VbyteSwap64((Vu64) x)))
	#define V_BYTESWAP_NTOH64_IN_PLACE(x)	((x) = (vault::VbyteSwap64((Vu64) x)))

	#define V_BYTESWAP_HTONF_GET(x)			vault::VbyteSwapFloat((VFloat) x)
	#define V_BYTESWAP_NTOHF_GET(x)			vault::VbyteSwapFloat((VFloat) x)
	#define V_BYTESWAP_HTONF_IN_PLACE(x)	((x) = (vault::VbyteSwapFloat((VFloat) x)))
	#define V_BYTESWAP_NTOHF_IN_PLACE(x)	((x) = (vault::VbyteSwapFloat((VFloat) x)))

	#define V_BYTESWAP_HTOND_GET(x)			vault::VbyteSwapDouble((VDouble) x)
	#define V_BYTESWAP_NTOHD_GET(x)			vault::VbyteSwapDouble((VDouble) x)
	#define V_BYTESWAP_HTOND_IN_PLACE(x)	((x) = (vault::VbyteSwapDouble((VDouble) x)))
	#define V_BYTESWAP_NTOHD_IN_PLACE(x)	((x) = (vault::VbyteSwapDouble((VDouble) x)))

#else

    #define V_BYTESWAP_HTONS_GET(x)			/*lint -save -e522*/ (x) /*lint -restore */
    #define V_BYTESWAP_NTOHS_GET(x)			/*lint -save -e522*/ (x) /*lint -restore */
    #define V_BYTESWAP_HTONS_IN_PLACE(x)	/*lint -save -e522*/ (x) /*lint -restore */
    #define V_BYTESWAP_NTOHS_IN_PLACE(x)	/*lint -save -e522*/ (x) /*lint -restore */

    #define V_BYTESWAP_HTONL_GET(x)			/*lint -save -e522*/ (x) /*lint -restore */
    #define V_BYTESWAP_NTOHL_GET(x)			/*lint -save -e522*/ (x) /*lint -restore */
    #define V_BYTESWAP_HTONL_IN_PLACE(x)	/*lint -save -e522*/ (x) /*lint -restore */
    #define V_BYTESWAP_NTOHL_IN_PLACE(x)	/*lint -save -e522*/ (x) /*lint -restore */

    #define V_BYTESWAP_HTON64_GET(x)		/*lint -save -e522*/ (x) /*lint -restore */
    #define V_BYTESWAP_NTOH64_GET(x)		/*lint -save -e522*/ (x) /*lint -restore */
    #define V_BYTESWAP_HTON64_IN_PLACE(x)	/*lint -save -e522*/ (x) /*lint -restore */
    #define V_BYTESWAP_NTOH64_IN_PLACE(x)	/*lint -save -e522*/ (x) /*lint -restore */

    #define V_BYTESWAP_HTONF_GET(x)			/*lint -save -e522*/ (x) /*lint -restore */
    #define V_BYTESWAP_NTOHF_GET(x)			/*lint -save -e522*/ (x) /*lint -restore */
    #define V_BYTESWAP_HTONF_IN_PLACE(x)	/*lint -save -e522*/ (x) /*lint -restore */
    #define V_BYTESWAP_NTOHF_IN_PLACE(x)	/*lint -save -e522*/ (x) /*lint -restore */

    #define V_BYTESWAP_HTOND_GET(x)			/*lint -save -e522*/ (x) /*lint -restore */
    #define V_BYTESWAP_NTOHD_GET(x)			/*lint -save -e522*/ (x) /*lint -restore */
    #define V_BYTESWAP_HTOND_IN_PLACE(x)	/*lint -save -e522*/ (x) /*lint -restore */
    #define V_BYTESWAP_NTOHD_IN_PLACE(x)	/*lint -save -e522*/ (x) /*lint -restore */

#endif

// Both CodeWarrior and GCC (4.0 at least) provide gettimeofday(), which uses UTC-based values.
#define V_INSTANT_SNAPSHOT_IS_UTC    // platform_snapshot() gives us a UTC time suitable for platform_now()

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

/*
These are the custom uniform definitions of system-level functions that behave
slightly differently on each compiler/library/OS platform. These are declared
in each platform's header file in a way that works with that platform.
*/
namespace vault {

inline int putenv(char* env) { return ::putenv(env); }
inline char* getenv(const char* name) { return ::getenv(name); }
inline ssize_t read(int fd, void* buffer, size_t numBytes) { return ::read(fd, buffer, numBytes); }
inline ssize_t write(int fd, const void* buffer, size_t numBytes) { return ::write(fd, buffer, numBytes); }
inline off_t lseek(int fd, off_t offset, int whence) { return ::lseek(fd, offset, whence); }
inline int open(const char* path, int flags, mode_t mode) { return ::open(path, flags, mode); }
inline int close(int fd) { return ::close(fd); }
inline int mkdir(const char* path, mode_t mode) { return ::mkdir(path, mode); }
inline int rmdir(const char* path) { return ::rmdir(path); }
inline int unlink(const char* path) { return ::unlink(path); }
inline int rename(const char* oldName, const char* newName) { return ::rename(oldName, newName); }
inline int stat(const char* path, struct stat* buf) { return ::stat(path, buf); }
inline int strcasecmp(const char* s1, const char* s2) { return ::strcasecmp(s1, s2); }
inline int strncasecmp(const char* s1, const char* s2, size_t length) { return ::strncasecmp(s1, s2, length); }
inline int vsnprintf(char* buffer, size_t length, const char* format, va_list args) { return ::vsnprintf(buffer, length, format, args); }

inline int snprintf(char* buffer, size_t length, const char* format, ...)
    {
    va_list	args;
    va_start(args, format);
    int result = ::vsnprintf(buffer, length, format, args);
    va_end(args);
    return result;
    }

}

#endif /* vtypes_platform_h */
