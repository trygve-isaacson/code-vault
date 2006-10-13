/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

#ifndef vtypes_platform_h
#define vtypes_platform_h

/** @file */

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <fcntl.h>

#ifdef HAVE_MALLOC_H
    #include <malloc.h>
#endif

#include <unistd.h>

// Solaris-specific includes.
#ifdef sun
    #include <strings.h>
    #include <netinet/in.h>
    #include <inttypes.h>
#endif

#define VPLATFORM_UNIX

#ifdef _HPUX_SOURCE
    #define VPLATFORM_UNIX_HPUX
#endif

#ifdef VPLATFORM_UNIX_HPUX
    #include <math.h>

    // On HPUX, std::abs(float) is not defined. For now, we can revert to macro style.
    // FIXME: do this more selectively rather than all-or-nothing below; just do V_FABS
    // rather than all 4 macros.
    #define DEFINE_V_MINMAXABS
#endif

#ifdef DEFINE_V_MINMAXABS

    #define V_MIN(a, b) ((a) > (b) ? (b) : (a)) ///< Macro for getting min of compatible values when standard functions / templates are not available.
    #define V_MAX(a, b) ((a) > (b) ? (a) : (b)) ///< Macro for getting max of compatible values when standard functions / templates are not available.
    #define V_ABS(a) ((a) < 0 ? (-(a)) : (a))   ///< Macro for getting abs of an integer value when standard functions / templates are not available.
    #define V_FABS(a) ((a) < 0 ? (-(a)) : (a))  ///< Macro for getting abs of a floating point value when standard functions / templates are not available.

#else

    #define V_MIN(a, b) std::min(a, b)  ///< Macro for getting min of compatible values using standard function template.
    #define V_MAX(a, b) std::max(a, b)  ///< Macro for getting max of compatible values using standard function template.
    #define V_ABS(a) std::abs(a)        ///< Macro for getting abs of an integer value using standard function template.
    #define V_FABS(a) std::fabs(a)      ///< Macro for getting abs of a floating point value using standard function template.

#endif /* DEFINE_V_MINMAXABS */

#define V_HAVE_REENTRANT_TIME    // we can and should use the _r versions of time.h calls

// For Linux, there is no O_BINARY mask, so define to zero so it will do nothing.
#define O_BINARY 0x0000 ///< Macro to define O_BINARY mode, which is not in the standard headers.

/*
Some Unix platforms do not define the byte order macros and definitions
that are defined in BSD's <machine/endian.h>, so we define them here in
that case.
*/

/* In case BYTE_ORDER stuff from <machine/endian.h> is not defined, we'll
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
    #define VBYTESWAP_NEEDED

    #define V_BYTESWAP_HTONS_GET(x)         vault::VbyteSwap16((Vu16) x)
    #define V_BYTESWAP_NTOHS_GET(x)         vault::VbyteSwap16((Vu16) x)
    #define V_BYTESWAP_HTONS_IN_PLACE(x)    ((x) = (vault::VbyteSwap16((Vu16) x)))
    #define V_BYTESWAP_NTOHS_IN_PLACE(x)    ((x) = (vault::VbyteSwap16((Vu16) x)))

    #define V_BYTESWAP_HTONL_GET(x)         vault::VbyteSwap32((Vu32) x)
    #define V_BYTESWAP_NTOHL_GET(x)         vault::VbyteSwap32((Vu32) x)
    #define V_BYTESWAP_HTONL_IN_PLACE(x)    ((x) = (vault::VbyteSwap32((Vu32) x)))
    #define V_BYTESWAP_NTOHL_IN_PLACE(x)    ((x) = (vault::VbyteSwap32((Vu32) x)))

    #define V_BYTESWAP_HTON64_GET(x)        vault::VbyteSwap64((Vu64) x)
    #define V_BYTESWAP_NTOH64_GET(x)        vault::VbyteSwap64((Vu64) x)
    #define V_BYTESWAP_HTON64_IN_PLACE(x)   ((x) = (vault::VbyteSwap64((Vu64) x)))
    #define V_BYTESWAP_NTOH64_IN_PLACE(x)   ((x) = (vault::VbyteSwap64((Vu64) x)))

    #define V_BYTESWAP_HTONF_GET(x)         vault::VbyteSwapFloat((VFloat) x)
    #define V_BYTESWAP_NTOHF_GET(x)         vault::VbyteSwapFloat((VFloat) x)
    #define V_BYTESWAP_HTONF_IN_PLACE(x)    ((x) = (vault::VbyteSwapFloat((VFloat) x)))
    #define V_BYTESWAP_NTOHF_IN_PLACE(x)    ((x) = (vault::VbyteSwapFloat((VFloat) x)))

#else

    #define V_BYTESWAP_HTONS_GET(x)         (x)
    #define V_BYTESWAP_NTOHS_GET(x)         (x)
    #define V_BYTESWAP_HTONS_IN_PLACE(x)    (x)
    #define V_BYTESWAP_NTOHS_IN_PLACE(x)    (x)

    #define V_BYTESWAP_HTONL_GET(x)         (x)
    #define V_BYTESWAP_NTOHL_GET(x)         (x)
    #define V_BYTESWAP_HTONL_IN_PLACE(x)    (x)
    #define V_BYTESWAP_NTOHL_IN_PLACE(x)    (x)

    #define V_BYTESWAP_HTON64_GET(x)        (x)
    #define V_BYTESWAP_NTOH64_GET(x)        (x)
    #define V_BYTESWAP_HTON64_IN_PLACE(x)   (x)
    #define V_BYTESWAP_NTOH64_IN_PLACE(x)   (x)

    #define V_BYTESWAP_HTONF_GET(x)         (x)
    #define V_BYTESWAP_NTOHF_GET(x)         (x)
    #define V_BYTESWAP_HTONF_IN_PLACE(x)    (x)
    #define V_BYTESWAP_NTOHF_IN_PLACE(x)    (x)

#endif

// vsnprintf(NULL, 0, . . .) behavior generally conforms to IEEE 1003.1,
// but not on HP-UX.
#ifndef VPLATFORM_UNIX_HPUX
    #define V_EFFICIENT_SPRINTF
#endif

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
inline int rename(const char* oldName, const char* newName) { return ::rename(oldName, newName); }
inline int stat(const char* path, struct stat* buf) { return ::stat(path, buf); }
inline int vsnprintf(char* buffer, size_t length, const char* format, va_list args) { return ::vsnprintf(buffer, length, format, args); }

inline int snprintf(char* buffer, size_t length, const char* format, ...)
    {
    va_list	args;
    va_start(args, format);
    int result = ::vsnprintf(buffer, length, format, args);
    va_end(args);
    return result;
    }

} // namespace vault

#endif /* vtypes_platform_h */
