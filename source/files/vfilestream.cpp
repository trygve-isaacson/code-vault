/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vfilestream.h"

#include "vexception.h"

#define READ_ONLY_MODE                (O_RDONLY | O_BINARY)
#define READWRITE_MODE                (O_RDWR | O_CREAT | O_BINARY)
#define WRITE_CREATE_MODE            (O_WRONLY | O_CREAT | O_BINARY)
#define OPEN_CREATE_PERMISSIONS        (S_IRWXO | S_IRWXG | S_IRWXU)

VFileStream::VFileStream()
    {
    mFile = -1;
    }

VFileStream::VFileStream(const VFSNode& node)
: mNode(node)
    {
    node.getName(mName);
    }

VFileStream::~VFileStream()
    {
    VFileStream::close();
    }

void VFileStream::openReadOnly()
    {
    mFile = VFileStream::threadsafe_open(mNode.path(), READ_ONLY_MODE);
    
    this->throwIfOpenFailed("VFileStream::openReadOnly", mNode.path());
    }

void VFileStream::openReadWrite()
    {
    /*
    The semantics of ::fopen() and ::open() run counter to the normal desire
    to open a file r/w and have it created if it doesn't exist. For example,
    mode "r" does not create the file. Mode "w" does. So instead, we check for
    existence first, and then open it exactly the way we intend.
    */

    mFile = VFileStream::threadsafe_open(mNode.path(), mNode.exists() ? READWRITE_MODE : WRITE_CREATE_MODE);
    
    this->throwIfOpenFailed("VFileStream::openReadWrite", mNode.path());
    }

void VFileStream::openWrite()
    {
    mFile = VFileStream::threadsafe_open(mNode.path(), WRITE_CREATE_MODE);
    
    this->throwIfOpenFailed("VFileStream::openWrite", mNode.path());
    }

void VFileStream::setNode(const VFSNode& node)
    {
    VString    path;
    
    node.getPath(path);
    mNode.setPath(path);

    node.getName(mName);
    }

const VFSNode& VFileStream::getNode() const
    {
    return mNode;
    }


bool VFileStream::isOpen() const
    {
    return (mFile != -1);
    }

void VFileStream::close()
    {
    if (this->isOpen())
        {
        (void) VFileStream::threadsafe_close(mFile);
        mFile = -1;
        }
    }

Vs64 VFileStream::read(Vu8* targetBuffer, Vs64 numBytesToRead)
    {
    /*
    Most of the work here is to deal with the fact that we are providing
    a SInt64 count in our API, but the underlying OS API may only
    allow a size_t count. So we may have to read in a loop until the
    requested number of bytes is read or EOF is reached. In the typical
    case, of course, we only go through the loop once.
    */
    
    bool    needsSizeConversion = VStream::needSizeConversion(numBytesToRead);
    Vs64    numBytesRemaining = numBytesToRead;
    Vs64    numBytesRead = 0;
    size_t    requestCount;
    size_t    actualCount;
    
    // FIXME: If needsSizeConversion is false, just do a read w/o the excess baggage here.
    // Also, we can set it to false if the actual size fits in 31 bits, as in VStream::copyMemory.
    do
        {
        if (needsSizeConversion)
            {
            requestCount = static_cast<size_t> (V_MIN((CONST_S64(0x7FFFFFFF)), numBytesRemaining));
            }
        else
            {
            requestCount = static_cast<size_t> (numBytesRemaining);
            }

        actualCount = static_cast<size_t> (VFileStream::threadsafe_read(mFile, targetBuffer + numBytesRead, requestCount));

        numBytesRead += actualCount;
        numBytesRemaining -= actualCount;

        } while ((actualCount == requestCount) && (numBytesRemaining > 0));

    return numBytesRead;
    }

Vs64 VFileStream::write(const Vu8* buffer, Vs64 numBytesToWrite)
    {
    /*
    Most of the work here is to deal with the fact that we are providing
    a SInt64 count in our API, but the underlying OS API may only
    allow a size_t count. So we may have to write in a loop until the
    requested number of bytes is written. In the typical
    case, of course, we only go through the loop once.
    */
    
    bool    needsSizeConversion = VStream::needSizeConversion(numBytesToWrite);
    Vs64    numBytesRemaining = numBytesToWrite;
    Vs64    numBytesWritten = 0;
    size_t    requestCount;
    size_t    actualCount;
    
    // FIXME: If needsSizeConversion is false, just do a write w/o the excess baggage here.
    // Also, we can set it to false if the actual size fits in 31 bits, as in VStream::copyMemory.
    do
        {
        if (needsSizeConversion)
            {
            requestCount = static_cast<size_t> (V_MIN((static_cast<Vs64> (LONG_MAX)), numBytesRemaining));
            }
        else
            {
            requestCount = static_cast<size_t> (numBytesRemaining);
            }

        actualCount = static_cast<size_t> (VFileStream::threadsafe_write(mFile, buffer + numBytesWritten, requestCount));

        numBytesWritten += actualCount;
        numBytesRemaining -= actualCount;

        } while ((actualCount == requestCount) && (numBytesRemaining > 0));

    if (numBytesWritten != numBytesToWrite)
        {
        VString    path;
        mNode.getPath(path);

        throw VException(errno, "VFileStream::write to '%s' only wrote %lld of %lld requested bytes.", path.chars(), numBytesWritten, numBytesToWrite);
        }

    return numBytesWritten;
    }

void VFileStream::flush()
    {
    // Unbuffered file writes have no flush mechanism since they are not buffered.
    }

bool VFileStream::skip(Vs64 numBytesToSkip)
    {
    /*
    Most of the work here is to deal with the fact that we are providing
    a SInt64 count in our API, but the underlying OS API may only
    allow a long count. So we may have to seek in a loop until the
    requested offset is seeked. In the typical case, of course, we only go
    through the loop once.
    */
    
    bool    success;
    bool    needsSizeConversion = VStream::needSizeConversion(numBytesToSkip);
    Vs64    numBytesRemaining = numBytesToSkip;
    Vs64    requestCount;
    
    // FIXME: If needsSizeConversion is false, just do a seek w/o the excess baggage here.
    // Also, we can set it to false if the actual size fits in 31 bits, as in VStream::copyMemory.
    // FIXME: This calls VFileStream::seek below, which is not as savvy as this--should it be?
    do
        {
        if (needsSizeConversion)
            {
            requestCount = V_MIN((static_cast<Vs64> (LONG_MAX)), numBytesRemaining);
            }
        else
            {
            requestCount = numBytesRemaining;
            }

        success = this->seek(requestCount, SEEK_CUR);
        
        numBytesRemaining -= requestCount;

        } while (success && (numBytesRemaining > 0));

    return success;
    }

bool VFileStream::seek(Vs64 inOffset, int whence)
    {
    // FIXME: need to deal with Vs64-to-off_t conversion
    return (VFileStream::threadsafe_lseek(mFile, static_cast<off_t> (inOffset), whence) == 0);
    }

Vs64 VFileStream::offset() const
    {
    return VFileStream::threadsafe_lseek(mFile, static_cast<off_t> (0), SEEK_CUR);
    }

Vs64 VFileStream::available() const
    {
    Vs64    currentOffset = this->offset();
    Vs64    eofOffset;
    
    const_cast<VFileStream*>(this)->seek(0, SEEK_END);
    eofOffset = this->offset();
    const_cast<VFileStream*>(this)->seek(currentOffset, SEEK_SET);    // restore original position
    
    return eofOffset - currentOffset;
    }

void VFileStream::throwIfOpenFailed(const VString& failedMethod, const VString& path)
    {
    if (! this->isOpen())
        throw VException(errno, "%s failed to open '%s'. Error %d (%s).", failedMethod.chars(), path.chars(), errno, ::strerror(errno));
    }

// This a useful place to put a breakpoint when things aren't going as planned.
static void _debugCheck(bool success)
    {
    if (! success)
        {
        int        e = errno;
        char*    s = ::strerror(e);
        s = NULL;    // avoid compiler warning for unused variable s
        }
    }

// static
int VFileStream::threadsafe_open(const char* path, int flags)
    {
    if ((path == NULL) || (path[0] == 0))
        return -1;

    int        fd = -1;
    bool    done = false;

    while (! done)
        {
        if (flags == WRITE_CREATE_MODE)
            fd = ::open(path, WRITE_CREATE_MODE, OPEN_CREATE_PERMISSIONS);
        else
            fd = ::open(path, flags);
        
        if ((fd != -1) || (errno != EINTR))
            done = true;
        }

    _debugCheck(fd != -1);
        
    return fd;
    }

// static
ssize_t VFileStream::threadsafe_read(int fd, void* buffer, size_t numBytes)
    {
    ssize_t    result;
    bool    done = false;

    while (! done)
        {
#ifdef VPLATFORM_WIN
        result = ::read(fd, buffer, (unsigned int) numBytes);
#else
        result = ::read(fd, buffer, numBytes);
#endif
        
        if ((result != (ssize_t) -1) || (errno != EINTR))
            done = true;
        }

    _debugCheck(result != -1);
    
    return result;
    }

// static
ssize_t VFileStream::threadsafe_write(int fd, const void* buffer, size_t numBytes)
    {
    ssize_t    result;
    bool    done = false;

    while (! done)
        {
#ifdef VPLATFORM_WIN
        result = ::write(fd, buffer, (unsigned int) numBytes);
#else
        result = ::write(fd, buffer, numBytes);
#endif
        
        if ((result != (ssize_t) -1) || (errno != EINTR))
            done = true;
        }

    _debugCheck(result != -1);
    
    return result;
    }

// static
off_t VFileStream::threadsafe_lseek(int fd, off_t inOffset, int whence)
    {
    off_t    result;
    bool    done = false;

    while (! done)
        {
        result = ::lseek(fd, inOffset, whence);
        
        if ((result != (off_t) -1) || (errno != EINTR))
            done = true;
        }

    _debugCheck(result != (off_t) -1);
    
    return result;
    }

// static
int VFileStream::threadsafe_close(int fd)
    {
    int        result;
    bool    done = false;

    while (! done)
        {
        result = ::close(fd);
        
        if ((result == 0) || (errno != EINTR))
            done = true;
        }

    _debugCheck(result != -1);
    
    return result;
    }

