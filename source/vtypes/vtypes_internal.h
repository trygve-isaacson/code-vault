/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
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
implementation, while handling thread interruption errors and retrying.
*/
class VFileSystemAPI {
    public:

        // These static functions wrap the raw POSIX functions in order to
        // make them work correctly even if a signal is caught inside the
        // function. These functions can be called from the platform-specific
        // implementation if appropriate.
        static int wrap_mkdir(const char* path, mode_t mode);               ///< Calls POSIX mkdir in a way that is safe even if a signal is caught inside the function.
        static int wrap_rename(const char* oldName, const char* newName);   ///< Calls POSIX rename in a way that is safe even if a signal is caught inside the function.
        static int wrap_stat(const char* path, struct stat* statData);      ///< Calls POSIX stat in a way that is safe even if a signal is caught inside the function.
        static int wrap_unlink(const char* path);                           ///< Calls POSIX unlink in a way that is safe even if a signal is caught inside the function.
        static int wrap_rmdir(const char* path);                            ///< Calls POSIX rmdir in a way that is safe even if a signal is caught inside the function.

        static int      wrap_open(const char* path, int flags);                     ///< Calls POSIX open in a way that is safe even if a signal is caught inside the function.
        static ssize_t  wrap_read(int fd, void* buffer, size_t numBytes);           ///< Calls POSIX read in a way that is safe even if a signal is caught inside the function.
        static ssize_t  wrap_write(int fd, const void* buffer, size_t numBytes);    ///< Calls POSIX write in a way that is safe even if a signal is caught inside the function.
        static off_t    wrap_lseek(int fd, off_t offset, int whence);               ///< Calls POSIX lseek in a way that is safe even if a signal is caught inside the function.
        static int      wrap_close(int fd);                                         ///< Calls POSIX close in a way that is safe even if a signal is caught inside the function.

};

#endif /* vtypes_internal_h */

