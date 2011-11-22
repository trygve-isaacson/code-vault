/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

#ifndef vtypes_internal_platform_h
#define vtypes_internal_platform_h

/** @file */

/*
These are the Windows-specific includes and definitions needed
to compile the Vault code itself. Code that includes Vault headers
does not need to be exposed to any of this.
*/

#ifdef VCOMPILER_CODEWARRIOR
#include <unistd.h>
#include <cassert> // used by Vassert
#define __assert(cond, file, line) assert(cond) // used by Vassert
#endif

#include <sys/types.h>
#include <sys/stat.h>

#ifdef VCOMPILER_MSVC
#include <fcntl.h>  // open(), file mode constants
#include <time.h>   // mktime() (for CodeWarrior, time.h must be included by vtypes_platform.h)
#endif

#include <errno.h>  // errno, error constants
#include <io.h>     // _read(), etc.
#include <direct.h> // _mkdir(), _rmdir()
#include <assert.h> // assert()

#undef FD_ZERO
#define FD_ZERO(p) memset(p, 0, sizeof(*(p)))

typedef signed long ssize_t; // used by i/o APIs defined below

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
inline char* getcwd(char* buf, size_t size) { return ::_getcwd(buf, static_cast<int>(size)); }
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

#else
// VC++ versions prior to 8.0; a few of the function names start with an underscore.

inline int putenv(char* env) { return ::putenv(env); }
inline char* getenv(const char* name) { return ::getenv(name); }
inline char* getcwd(char* buf, size_t size) { return ::_getcwd(buf, size); }
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
    va_list args;
    va_start(args, format);
    int result = ::snprintf(buffer, length, format, args);
    va_end(args);
    return result;
    }

#endif /* Compiler-specific inline core functions. */

}

#endif /* vtypes_internal_platform_h */
