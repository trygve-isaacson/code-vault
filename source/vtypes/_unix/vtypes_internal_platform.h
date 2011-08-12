/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

#ifndef vtypes_internal_platform_h
#define vtypes_internal_platform_h

/** @file */

/*
These are the Unix-specific includes and definitions needed
to compile the Vault code itself. Code that includes Vault headers
does not need to be exposed to any of this.
*/

#include <fcntl.h> // open(), file mode constants
#include <sys/stat.h> // mkdir(), stat()
#include <errno.h> // errno, error constants
#include <assert.h> // assert()

// Solaris-specific includes.
#ifdef sun
    #include <strings.h>
    #include <netinet/in.h>
    #include <inttypes.h>
#endif

#define O_BINARY 0x8000 ///< Macro to define O_BINARY mode, which is not in the standard headers.

#undef FD_ZERO
#define FD_ZERO(p) memset(p, 0, sizeof(*(p)))

// Both CodeWarrior and GCC (4.0 at least) provide gettimeofday(), which uses UTC-based values.
#define V_INSTANT_SNAPSHOT_IS_UTC    // platform_snapshot() gives us a UTC time suitable for platform_now()

/*
These are the custom uniform definitions of system-level functions that behave
slightly differently on each compiler/library/OS platform. These are declared
in each platform's header file in a way that works with that platform.
*/
namespace vault {

inline int putenv(char* env) { return ::putenv(env); }
inline char* getenv(const char* name) { return ::getenv(name); }
inline char* getcwd(char* buf, size_t size) { return ::getcwd(buf, size); }
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
}

#endif /* vtypes_internal_platform_h */
