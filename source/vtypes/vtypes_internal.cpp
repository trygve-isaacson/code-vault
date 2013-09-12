/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#include "vtypes_internal.h"

#include "vfsnode.h"
#include "vexception.h"

// This is a useful place to put a breakpoint when things aren't going as planned.
static void _debugCheck(bool success) {
    if (! success) {
        VSystemError e;
    }
}

// static
int VFileSystem::mkdir(const VString& path, mode_t mode) {
    int     result = 0;
    bool    done = false;

    while (! done) {
        result = VPlatformAPI::mkdir(path, mode);

        if ((result == 0) || (errno != EINTR))
            done = true;
    }

    /*
    If two threads are competing to create the same directory, even if they
    both check for its existence, they might end up trying to create it at
    the same time -- one of them will get result -1 with errno == EEXIST.
    The best thing for our interface is to have mkdir succeed if the directory
    already exists.
    */

    if ((result == -1) && (errno == EEXIST)) {
        // Call stat to determine whether the existent node is a directory.
        // If it is, then we "succeeded" in creating it.
        VFSNode node(path);
        if (node.isDirectory())
            result = 0;
    }

    _debugCheck(result == 0);

    return result;
}

// static
int VFileSystem::rename(const VString& oldName, const VString& newName) {
    int     result = 0;
    bool    done = false;

    while (! done) {
        result = VPlatformAPI::rename(oldName, newName);

        if ((result == 0) || (errno != EINTR))
            done = true;
    }

    _debugCheck(result == 0);

    return result;
}

// static
int VFileSystem::stat(const VString& path, struct stat* buf) {
    int     result = 0;
    bool    done = false;

    while (! done) {
        result = VPlatformAPI::stat(path, buf);

        if ((result == 0) || (errno != EINTR))
            done = true;
    }

    _debugCheck(result == 0);

    return result;
}

// static
int VFileSystem::unlink(const VString& path) {
    int     result = 0;
    bool    done = false;

    while (! done) {
        result = VPlatformAPI::unlink(path);

        if ((result == 0) || (errno != EINTR))
            done = true;
    }

    _debugCheck(result == 0);

    return result;
}

// static
int VFileSystem::rmdir(const VString& path) {
    int     result = 0;
    bool    done = false;

    while (! done) {
        result = VPlatformAPI::rmdir(path);

        if ((result == 0) || (errno != EINTR))
            done = true;
    }

    _debugCheck(result == 0);

    return result;
}

// static
int VFileSystem::open(const VString& path, int flags) {
    if (path.isEmpty())
        return -1;

    int     fd = -1;
    bool    done = false;

    while (! done) {
        if (flags == WRITE_CREATE_MODE)
            fd = VPlatformAPI::open(path, WRITE_CREATE_MODE, OPEN_CREATE_PERMISSIONS);
        else
            fd = VPlatformAPI::open(path, flags, 0);

        if ((fd != -1) || (errno != EINTR))
            done = true;
    }

    _debugCheck(fd != -1);

    return fd;
}

// static
ssize_t VFileSystem::read(int fd, void* buffer, size_t numBytes) {
    ssize_t result = 0;
    bool    done = false;

    while (! done) {
        result = vault::read(fd, buffer, numBytes);

        if ((result != (ssize_t) - 1) || (errno != EINTR))
            done = true;
    }

    _debugCheck(result != -1);

    return result;
}

// static
ssize_t VFileSystem::write(int fd, const void* buffer, size_t numBytes) {
    ssize_t result = 0;
    bool    done = false;

    while (! done) {
        result = vault::write(fd, buffer, numBytes);

        if ((result != (ssize_t) - 1) || (errno != EINTR))
            done = true;
    }

    _debugCheck(result != -1);

    return result;
}

// static
off_t VFileSystem::lseek(int fd, off_t offset, int whence) {
    off_t   result = 0;
    bool    done = false;

    while (! done) {
        result = vault::lseek(fd, offset, whence);

        if ((result != (off_t) - 1) || (errno != EINTR))
            done = true;
    }

    _debugCheck(result != (off_t) - 1);

    return result;
}

// static
int VFileSystem::close(int fd) {
    int     result = 0;
    bool    done = false;

    while (! done) {
        result = vault::close(fd);

        if ((result == 0) || (errno != EINTR))
            done = true;
    }

    _debugCheck(result != -1);

    return result;
}

// static
FILE* VFileSystem::fopen(const VString& nativePath, const char* mode) {
    if (nativePath.isEmpty())
        return NULL;

    FILE*   f = NULL;
    bool    done = false;

    while (! done) {
        f = VPlatformAPI::fopen(nativePath, mode);

        if ((f != NULL) || (errno != EINTR)) {
            done = true;
        }
    }

    _debugCheck(f != NULL);

    return f;
}

// static
int VFileSystem::fclose(FILE* f) {
    if (f == NULL)
        return EOF;

    int     result = 0;
    bool    done = false;

    while (! done) {
        result = ::fclose(f);

        if ((result == 0) || (errno != EINTR)) {
            done = true;
        }
    }

    _debugCheck(result == 0);

    return result;
}

// static
size_t VFileSystem::fread(void* buffer, size_t size, size_t numItems, FILE* f) {
    if ((buffer == NULL) || (f == NULL)) {
        return 0;
    }

    size_t  result = 0;
    bool    done = false;

    while (! done) {
        result = ::fread(buffer, size, numItems, f);

        if ((result != numItems) && (ferror(f) != 0) && (errno == EINTR)) {
            done = false;
        } else {
            done = true;
        }
    }

    _debugCheck(result == numItems);

    return result;
}

// static
size_t VFileSystem::fwrite(const void* buffer, size_t size, size_t numItems, FILE* f) {
    size_t  result = 0L;
    bool    done = false;

    if ((buffer == NULL) || (f == NULL)) {
        return 0L;
    }

    while (! done) {
        result = ::fwrite(buffer, size, numItems, f);

        if ((result != numItems) && (ferror(f) != 0) && (errno == EINTR)) {
            done = false;
        } else {
            done = true;
        }
    }

    _debugCheck(result == numItems);

    return result;
}

// static
int VFileSystem::fseek(FILE* f, long int offset, int whence) {
    int     result = 0;
    bool    done = false;

    if (f == NULL) {
        return EOF;
    }

    while (! done) {
        result = ::fseek(f, offset, whence);

        if ((result != -1) || (errno != EINTR)) {
            done = true;
        }
    }

    _debugCheck(result == 0);

    return result;
}

// static
int VFileSystem::fflush(FILE* f) {
    int     result = 0;
    bool    done = false;

    if (f == NULL) {
        return EOF;
    }

    while (! done) {
        result = ::fflush(f);

        if ((result == 0) || (errno != EINTR)) {
            done = true;
        }
    }
    _debugCheck(result == 0);

    return result;
}

// static
long int VFileSystem::ftell(FILE* f) {
    long int    result = 0;
    bool        done = false;

    if (f == NULL) {
        return 0;
    }

    while (! done) {
        result = ::ftell(f);

        if ((result >= 0) || (errno != EINTR)) {
            done = true;
        }
    }

    _debugCheck(result != -1);

    return result;
}
