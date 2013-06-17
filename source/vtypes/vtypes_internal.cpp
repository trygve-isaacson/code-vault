/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

#include "vtypes_internal.h"

#include "vfsnode.h"

// This is a useful place to put a breakpoint when things aren't going as planned.
static void _debugCheck(bool success) {
    if (! success) {
        VSystemError e;
    }
}

// static
int VFileSystemAPI::wrap_mkdir(const char* path, mode_t mode) {
    int     result = 0;
    bool    done = false;

    while (! done) {
        result = vault::mkdir(path, mode);

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
int VFileSystemAPI::wrap_rename(const char* oldName, const char* newName) {
    int     result = 0;
    bool    done = false;

    while (! done) {
        result = vault::rename(oldName, newName);

        if ((result == 0) || (errno != EINTR))
            done = true;
    }

    _debugCheck(result == 0);

    return result;
}

// static
int VFileSystemAPI::wrap_stat(const char* path, struct stat* buf) {
    int     result = 0;
    bool    done = false;

    while (! done) {
        result = vault::stat(path, buf);

        if ((result == 0) || (errno != EINTR))
            done = true;
    }

    _debugCheck(result == 0);

    return result;
}

// static
int VFileSystemAPI::wrap_unlink(const char* path) {
    int     result = 0;
    bool    done = false;

    while (! done) {
        result = vault::unlink(path);

        if ((result == 0) || (errno != EINTR))
            done = true;
    }

    _debugCheck(result == 0);

    return result;
}

// static
int VFileSystemAPI::wrap_rmdir(const char* path) {
    int     result = 0;
    bool    done = false;

    while (! done) {
        result = vault::rmdir(path);

        if ((result == 0) || (errno != EINTR))
            done = true;
    }

    _debugCheck(result == 0);

    return result;
}

// static
int VFileSystemAPI::wrap_open(const char* path, int flags) {
    if ((path == NULL) || (path[0] == 0))
        return -1;

    int     fd = -1;
    bool    done = false;

    while (! done) {
        if (flags == WRITE_CREATE_MODE)
            fd = vault::open(path, WRITE_CREATE_MODE, OPEN_CREATE_PERMISSIONS);
        else
            fd = vault::open(path, flags, 0);

        if ((fd != -1) || (errno != EINTR))
            done = true;
    }

    _debugCheck(fd != -1);

    return fd;
}

// static
ssize_t VFileSystemAPI::wrap_read(int fd, void* buffer, size_t numBytes) {
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
ssize_t VFileSystemAPI::wrap_write(int fd, const void* buffer, size_t numBytes) {
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
off_t VFileSystemAPI::wrap_lseek(int fd, off_t offset, int whence) {
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
int VFileSystemAPI::wrap_close(int fd) {
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

