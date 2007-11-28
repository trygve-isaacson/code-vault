/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

#ifndef vtypes_platform_h
#define vtypes_platform_h

/** @file */

/*
When compiling for Windows, we have a few differences introduced
because of varying compiler quality of Microsoft Visual C++.

We check here to verify that we are compiling with Microsoft Visual
C++, and specifically which version. This will allow us to set a
couple of preprocessor symbols that we can use here and elsewhere
for conditional compilation.

_MSC_VER is defined by MSVC++. The values I've been able to find
documented are:
1020 - version 4.2 (pretty old)
1200 - version 6 SP4 (a little old now but very widely used)
1300 - version 7 (a.k.a. "2003" or ".NET")
1310 - version 7.1 (patch/update to version 7)
1400 - version 8.0 (a.k.a. "2005")

Versions prior to 7 (1300) are really awful due to their lack of
support for some basic common standard C++ constructs. Therefore,
we define two symbols based on the _MSC_VER value.

VCOMPILER_MSVC - This is to identify that we are using MSVC and
not some other compiler like CodeWarrior X86 or gcc.

VCOMPILER_MSVC_6_CRIPPLED - This is used to
identify that we are using a version earlier than 7 (1300), and
thus need to include hacks to work around the compiler's deficiencies.

Wherever we need to work around a problem in all VC++ versions,
we can just conditionally compile for VCOMPILER_MSVC. Whenever we
need to work around a problem just for VC++ 6 and earlier, we can
conditionally compile for VCOMPILER_MSVC_6_CRIPPLED. Whenever possible,
we do the workarounds just for VCOMPILER_MSVC_6_CRIPPLED, but some
quirks are still present for later compilers (see DEFINE_V_MINMAXABS
below for example).

Thus we don't reference _MSC_VER elsewhere in the Vault code, we just
set up the other symbols here and reference them only where really
needed.
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
    #define VLIBRARY_METROWERKS	/* might want to consider adding a check for using CW but not MSL */
#endif

// The Code Vault does not currently support Unicode strings.
// If we let UNICODE be defined by VC++ it causes problems when
// we call the directory iterator APIs with non-Unicode strings.
#undef UNICODE

#ifdef VCOMPILER_CODEWARRIOR
#include <ctime>
#endif

#ifndef VPLATFORM_CW_FEWER_INCLUDES
#include <sys/types.h>
#include <sys/stat.h>
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
#ifndef VPLATFORM_CW_FEWER_INCLUDES
#include <math.h>
#endif
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#ifdef VPLATFORM_CW_FEWER_INCLUDES
typedef unsigned int off_t;
typedef unsigned char mode_t;
#ifndef VPLATFORM_HAS_BOOLEAN_TYPEDEF
typedef unsigned char bool;
#endif
inline int _mkdir(const char* /*path*/) { return -1; }
inline int stat(const char* /*path*/, struct stat* /*buf*/) { return -1; }
#endif

#ifdef VCOMPILER_CODEWARRIOR
#include <cassert>                              // used by Vassert
#define __assert(cond, file, line) assert(cond) // used by Vassert
#endif

typedef signed long ssize_t;                    // used by _wrap_read, _wrap_write

#ifndef VCOMPILER_MSVC
#include <unistd.h> /* n/a for VC++, needed elsewhere */
#endif

#define VPLATFORM_WIN

#include <winsock2.h>
#include <windows.h>
#include <io.h>
#include <direct.h>
#include <algorithm> // for std::find

// If Windows globally #defines these as preprocessor macros, they cannot
// be used as method names! Get rid of them.
#undef min
#undef max
#undef abs

/*
The VC++ 6.0 compiler is nonstandard in how it defines 64-bit integers,
so we define them specially here.
*/
#ifdef VCOMPILER_MSVC_6_CRIPPLED
typedef LONGLONG  Vs64;
typedef ULONGLONG Vu64;
#endif

/*
The VC++ 6.0 compiler is nonstandard in how it denotes 64-bit constants,
so we use a macro and define it specially here. The standard suffix
is LL for signed and ULL for unsigned. VC++ uses i64 (need to research
signed vs. unsigned).
*/
#ifdef VCOMPILER_MSVC_6_CRIPPLED
#define CONST_S64(s) s##i64
#define CONST_U64(s) s##i64
#endif

/*
The VC++ 6.0 compiler does not support the C++ standard for defining static
class constants such as "const static int kFoo = 1;" so we use a macro and
define it specially here.
*/
#ifdef VCOMPILER_MSVC_6_CRIPPLED
#define CLASS_CONST(type, name, init) enum { name = init }
#endif

/*
The VC++ 6.0 compiler is nonstandard in its lack of support for scoped
variables in for loops. This #define gets around that. This should be
#ifdef'd to the compiler version when they fix this bug. And ideally
this should be #ifdef'd to only be in effect for Microsoft's compiler.
*/
#ifdef VCOMPILER_MSVC_6_CRIPPLED
#define for if(false);else for
#endif

/*
VC++ up to and including 7.1 does not like using std::min(), std::max(), std::abs()
so our convenience macros V_MIN, V_MAX, and V_ABS need to be implemented as
old-fashioned C preprocessor macros rather than standard C++ library inlines.
*/
#ifdef VCOMPILER_MSVC
#define DEFINE_V_MINMAXABS 1
#endif

/*
Perhaps we can rely on a Win32 header to determine how these macros should
map as we do in the Unix header file, but for now let's just use X86 behavior
and assume we don't compile for Windows on processors that are not using
Intel byte order.
*/

#define VBYTESWAP_NEEDED

//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_HTONS_GET(x)         vault::VbyteSwap16((Vu16) x)
//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_NTOHS_GET(x)         vault::VbyteSwap16((Vu16) x)
//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_HTONS_IN_PLACE(x)    ((x) = (vault::VbyteSwap16((Vu16) x)))
//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_NTOHS_IN_PLACE(x)    ((x) = (vault::VbyteSwap16((Vu16) x)))

//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_HTONL_GET(x)         vault::VbyteSwap32((Vu32) x)
//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_NTOHL_GET(x)         vault::VbyteSwap32((Vu32) x)
//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_HTONL_IN_PLACE(x)    ((x) = (vault::VbyteSwap32((Vu32) x)))
//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_NTOHL_IN_PLACE(x)    ((x) = (vault::VbyteSwap32((Vu32) x)))

//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_HTON64_GET(x)        vault::VbyteSwap64((Vu64) x)
//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_NTOH64_GET(x)        vault::VbyteSwap64((Vu64) x)
//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_HTON64_IN_PLACE(x)   ((x) = (vault::VbyteSwap64((Vu64) x)))
//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_NTOH64_IN_PLACE(x)   ((x) = (vault::VbyteSwap64((Vu64) x)))

//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_HTONF_GET(x)         vault::VbyteSwapFloat((VFloat) x)
//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_NTOHF_GET(x)         vault::VbyteSwapFloat((VFloat) x)
//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_HTONF_IN_PLACE(x)    ((x) = (vault::VbyteSwapFloat((VFloat) x)))
//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_NTOHF_IN_PLACE(x)    ((x) = (vault::VbyteSwapFloat((VFloat) x)))

//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_HTOND_GET(x)			vault::VbyteSwapDouble((VDouble) x)
//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_NTOHD_GET(x)			vault::VbyteSwapDouble((VDouble) x)
//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_HTOND_IN_PLACE(x)	((x) = (vault::VbyteSwapDouble((VDouble) x)))
//lint -e961 "Violates MISRA Advisory Rule 44, redundant explicit casting"
#define V_BYTESWAP_NTOHD_IN_PLACE(x)	((x) = (vault::VbyteSwapDouble((VDouble) x)))

/* #define S_ISLNK(x) ... Would need equivalent for stat.mode on Win32, see VFSNode::isDirectory() */
#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND

#ifdef VCOMPILER_MSVC
    typedef int mode_t;
    #define S_IRWXO    _S_IREAD | _S_IWRITE
    #define S_IRWXG    _S_IREAD | _S_IWRITE
    #define S_IRWXU    _S_IREAD | _S_IWRITE
#endif

// On VC++ but not CodeWarrior, we implement snapshot using _ftime64(), which is UTC-based.
#ifdef VCOMPILER_MSVC
#define V_INSTANT_SNAPSHOT_IS_UTC    // platform_snapshot() gives us a UTC time suitable for platform_now()
#endif

// We have to implement timegm() because there's no equivalent Win32 function.
extern time_t timegm(struct tm* t);

// The WinSock types fail to define in_addr_t, so to avoid making VSocketBase
// contain conditional code for address resolution, we just define it here.
typedef unsigned long in_addr_t;

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

/*
These are the custom uniform definitions of system-level functions that behave
slightly differently on each compiler/library/OS platform. These are declared
in each platform's header file in a way that works with that platform.
*/
namespace vault {

// For a few of these functions, the VC++ version does not have the same parameter list
// as the normal POSIX function, either in number of parameters or their types.

#ifdef VCOMPILER_CODEWARRIOR

inline int putenv(char* env) { return ::_putenv(env); }
inline char* getenv(const char* name) { return ::getenv(name); }
inline int strcasecmp(const char* s1, const char* s2) { return ::_stricmp(s1, s2); }
inline int strncasecmp(const char* s1, const char* s2, size_t length) { return ::_strnicmp(s1, s2, length); }
inline int vsnprintf(char* buffer, size_t length, const char* format, va_list args) { return ::vsnprintf(buffer, length, format, args); }
inline ssize_t read(int fd, void* buffer, size_t numBytes) { return ::_read(fd, buffer, static_cast<unsigned int>(numBytes)); }
inline ssize_t write(int fd, const void* buffer, size_t numBytes) { return ::_write(fd, buffer, static_cast<unsigned int>(numBytes)); }
inline off_t lseek(int fd, off_t offset, int whence) { return ::_lseek(fd, offset, whence); }
inline int close(int fd) { return ::_close(fd); }
inline int mkdir(const char* path, mode_t /*mode*/) { return ::_mkdir(path); }
inline int rename(const char* oldName, const char* newName) { return ::rename(oldName, newName); }
inline int stat(const char* path, struct stat* buf) { return ::stat(path, buf); }
inline int unlink(const char* path) { return ::_unlink(path); }
inline int rmdir(const char* path) { return ::_rmdir(path); }

// The POSIX open() API (or _open() on newer VC++ runtimes) is not available on
// CW+Win32. It's painful to code around, so we just throw an exception if anyone
// calls it. The only user in Vault is VDirectIOFileStream, which is almost never
// used because VBufferedFileStream is almost always better.
#define O_RDONLY 0
#define O_WRONLY 0
#define O_BINARY 0
#define O_RDWR 0
#define O_CREAT 0
#define O_TRUNC 0
extern int open(const char* path, int flags, mode_t mode); // Per comments above: this always throws.

#elif _MSC_VER >= 1400
// VC++ 8.0 makes many of these function names start with an underscore.

inline int putenv(char* env) { return ::_putenv(env); }
inline char* getenv(const char* name) { return ::getenv(name); }
inline ssize_t read(int fd, void* buffer, size_t numBytes) { return ::_read(fd, buffer, static_cast<unsigned int>(numBytes)); }
inline ssize_t write(int fd, const void* buffer, size_t numBytes) { return ::_write(fd, buffer, static_cast<unsigned int>(numBytes)); }
inline off_t lseek(int fd, off_t offset, int whence) { return ::_lseek(fd, offset, whence); }
inline int open(const char* path, int flags, mode_t mode) { return ::_open(path, flags, mode); }
inline int close(int fd) { return ::_close(fd); }
inline int mkdir(const char* path, mode_t /*mode*/) { return ::_mkdir(path); }
inline int rmdir(const char* path) { return ::_rmdir(path); }
inline int unlink(const char* path) { return ::_unlink(path); }
inline int rename(const char* oldName, const char* newName) { return ::rename(oldName, newName); }
inline int stat(const char* path, struct stat* buf) { return ::stat(path, buf); }
inline int strcasecmp(const char* s1, const char* s2) { return ::_stricmp(s1, s2); }
inline int strncasecmp(const char* s1, const char* s2, size_t length) { return ::_strnicmp(s1, s2, length); }
inline int vsnprintf(char* buffer, size_t length, const char* format, va_list args) { return ::_vsnprintf(buffer, length, format, args); }

inline int snprintf(char* buffer, size_t length, const char* format, ...)
    {
    va_list	args;
    va_start(args, format);
    int result = ::_vsnprintf(buffer, length, format, args);
    va_end(args);
    return result;
    }

#else
// VC++ versions prior to 8.0; a few of the function names start with an underscore.

inline int putenv(char* env) { return ::putenv(env); }
inline char* getenv(const char* name) { return ::getenv(name); }
inline ssize_t read(int fd, void* buffer, size_t numBytes) { return ::read(fd, buffer, static_cast<unsigned int>(numBytes)); }
inline ssize_t write(int fd, const void* buffer, size_t numBytes) { return ::write(fd, buffer, static_cast<unsigned int>(numBytes)); }
inline off_t lseek(int fd, off_t offset, int whence) { return ::lseek(fd, offset, whence); }
inline int open(const char* path, int flags, mode_t mode) { return ::_open(path, flags, mode); }
inline int close(int fd) { return ::close(fd); }
inline int mkdir(const char* path, mode_t /*mode*/) { return ::mkdir(path); }
inline int rmdir(const char* path) { return ::_rmdir(path); }
inline int unlink(const char* path) { return ::unlink(path); }
inline int rename(const char* oldName, const char* newName) { return ::rename(oldName, newName); }
inline int stat(const char* path, struct stat* buf) { return ::stat(path, buf); }
inline int strcasecmp(const char* s1, const char* s2) { return ::_stricmp(s1, s2); }
inline int strncasecmp(const char* s1, const char* s2, size_t length) { return ::_strnicmp(s1, s2, length); }
inline int vsnprintf(char* buffer, size_t length, const char* format, va_list args) { return ::_vsnprintf(buffer, length, format, args); }

inline int snprintf(char* buffer, size_t length, const char* format, ...)
    {
    va_list	args;
    va_start(args, format);
    int result = ::snprintf(buffer, length, format, args);
    va_end(args);
    return result;
    }

#endif

}

#endif /* vtypes_platform_h */
