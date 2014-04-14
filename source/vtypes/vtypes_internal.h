/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
*/

#ifndef vtypes_internal_h
#define vtypes_internal_h

#include "vtypes.h"

#include "vtypes_internal_platform.h"
#include "vassert.h" // needed by many internal .cpp files

#define READ_ONLY_MODE          (O_RDONLY | O_BINARY)
#define READWRITE_MODE          (O_RDWR | O_CREAT | O_BINARY)
#define WRITE_CREATE_MODE       (O_WRONLY | O_CREAT | O_TRUNC | O_BINARY) // Added O_TRUNC which should zero the file upon creation/opening
#define OPEN_CREATE_PERMISSIONS (S_IRWXO | S_IRWXG | S_IRWXU)

/**
This helper class is used internally to wrap POSIX file APIs needed by our low-level
implementation, while handling thread interruption errors and retrying. We also redefine
the path parameters as VString instead of const char*, so that we can pass them through
to VPlatformAPI low level functions (on Windows these need to denormalize the path string
and widen it to UTF-16 for use with Win32 wide string APIs that can handle Unicode; contrast
with Unix/Mac which can take our existing UTF-8 char strings).
*/
class VFileSystem {
    public:
    
        static int      mkdir(const VString& path, mode_t mode);                            ///< Calls POSIX mkdir in a way that is safe even if a signal is caught inside the function.
        static int      rename(const VString& oldName, const VString& newName);             ///< Calls POSIX rename in a way that is safe even if a signal is caught inside the function.
        static int      stat(const VString& path, struct stat* statData);                   ///< Calls POSIX stat in a way that is safe even if a signal is caught inside the function.
        static int      unlink(const VString& path);                                        ///< Calls POSIX unlink in a way that is safe even if a signal is caught inside the function.
        static int      rmdir(const VString& path);                                         ///< Calls POSIX rmdir in a way that is safe even if a signal is caught inside the function.

        static int      open(const VString& path, int flags);                               ///< Calls POSIX open in a way that is safe even if a signal is caught inside the function.
        static ssize_t  read(int fd, void* buffer, size_t numBytes);                        ///< Calls POSIX read in a way that is safe even if a signal is caught inside the function.
        static ssize_t  write(int fd, const void* buffer, size_t numBytes);                 ///< Calls POSIX write in a way that is safe even if a signal is caught inside the function.
        static off_t    lseek(int fd, off_t offset, int whence);                            ///< Calls POSIX lseek in a way that is safe even if a signal is caught inside the function.
        static int      close(int fd);                                                      ///< Calls POSIX close in a way that is safe even if a signal is caught inside the function.

        static FILE*    fopen(const VString& nativePath, const char* mode);                 ///< Calls POSIX fopen in a way that is safe even if a signal is caught inside the function.
        static int      fclose(FILE* f);                                                    ///< Calls POSIX fclose in a way that is safe even if a signal is caught inside the function.
        static size_t   fread(void* buffer, size_t size, size_t numItems, FILE* f);         ///< Calls POSIX fread in a way that is safe even if a signal is caught inside the function.
        static size_t   fwrite(const void* buffer, size_t size, size_t numItems, FILE* f);  ///< Calls POSIX fwrite in a way that is safe even if a signal is caught inside the function.
        static int      fseek(FILE* f, long int offset, int whence);                        ///< Calls POSIX fseek in a way that is safe even if a signal is caught inside the function.
        static int      fflush(FILE* f);                                                    ///< Calls POSIX fflush in a way that is safe even if a signal is caught inside the function.
        static long int ftell(FILE* f);                                                     ///< Calls POSIX ftell in a way that is safe even if a signal is caught inside the function.

    private:
        VFileSystem(); // static functions only; not constructable
};

/**
VPlatformAPI defines the lower level APIs for interfacing directly with the platform's
file system API which deal in paths. These APIs take UTF-8 formatted VString paths.
The implementation is platform-specific:
- Unix and Mac can just call the matching vault:: namespace function.
- Windows must denormalize the path and then widen it to UTF-16 to pass to the corresponding Win32 "wide" API.
These functions are to be called by the corresponding VFileSystem functions.
*/
class VPlatformAPI {
    public:
    
        static VString  getcwd();
        static int      open(const VString& path, int flags, mode_t mode);
        static FILE*    fopen(const VString& path, const char* mode);
        static int      mkdir(const VString& path, mode_t mode);
        static int      rmdir(const VString& path);
        static int      unlink(const VString& path);
        static int      rename(const VString& oldName, const VString& newName);
        static int      stat(const VString& path, struct stat* buf);

    private:
        VPlatformAPI(); // static functions only; not constructable
};

#endif /* vtypes_internal_h */

