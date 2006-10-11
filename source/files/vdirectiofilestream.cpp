/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vdirectiofilestream.h"

#include "vexception.h"

#define READ_ONLY_MODE          (O_RDONLY | O_BINARY)
#define READWRITE_MODE          (O_RDWR | O_CREAT | O_BINARY)
#define WRITE_CREATE_MODE       (O_WRONLY | O_CREAT | O_TRUNC | O_BINARY) // Added O_TRUNC which should zero the file upon creation/opening
#define OPEN_CREATE_PERMISSIONS (S_IRWXO | S_IRWXG | S_IRWXU)

VDirectIOFileStream::VDirectIOFileStream() :
VAbstractFileStream(),
mFile(-1),
mCloseOnDestruct(true)
    {
    }

VDirectIOFileStream::VDirectIOFileStream(const VFSNode& node) :
VAbstractFileStream(node),
mFile(-1),
mCloseOnDestruct(true)
    {
    }

VDirectIOFileStream::VDirectIOFileStream(int fd, bool closeOnDestruct) :
VAbstractFileStream(),
mFile(fd),
mCloseOnDestruct(closeOnDestruct)
    {
    }

VDirectIOFileStream::~VDirectIOFileStream()
    {
    if (mCloseOnDestruct)
        VDirectIOFileStream::close();
    }

void VDirectIOFileStream::setFile(int fd, bool closeOnDestruct)
    {
    mFile = fd;
    mCloseOnDestruct = closeOnDestruct;
    }

void VDirectIOFileStream::openReadOnly()
    {
    mFile = VDirectIOFileStream::_wrap_open(mNode.getPath(), READ_ONLY_MODE);
    
    this->_throwIfOpenFailed("VDirectIOFileStream::openReadOnly", mNode.getPath());
    }

void VDirectIOFileStream::openReadWrite()
    {
    /*
    The semantics of ::fopen() and ::open() run counter to the normal desire
    to open a file r/w and have it created if it doesn't exist. For example,
    mode "r" does not create the file. Mode "w" does. So instead, we check for
    existence first, and then open it exactly the way we intend.
    */

    mFile = VDirectIOFileStream::_wrap_open(mNode.getPath(), mNode.exists() ? READWRITE_MODE : WRITE_CREATE_MODE);
    
    this->_throwIfOpenFailed("VDirectIOFileStream::openReadWrite", mNode.getPath());
    }

void VDirectIOFileStream::openWrite()
    {
    mFile = VDirectIOFileStream::_wrap_open(mNode.getPath(), WRITE_CREATE_MODE);
    
    this->_throwIfOpenFailed("VDirectIOFileStream::openWrite", mNode.getPath());
    }

bool VDirectIOFileStream::isOpen() const
    {
    return (mFile != -1);
    }

void VDirectIOFileStream::close()
    {
    if (this->isOpen())
        {
        (void) VDirectIOFileStream::_wrap_close(mFile);
        mFile = -1;
        }
    }

Vs64 VDirectIOFileStream::read(Vu8* targetBuffer, Vs64 numBytesToRead)
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

        actualCount = static_cast<size_t> (VDirectIOFileStream::_wrap_read(mFile, targetBuffer + numBytesRead, requestCount));

        numBytesRead += actualCount;
        numBytesRemaining -= actualCount;

        } while ((actualCount == requestCount) && (numBytesRemaining > 0));

    return numBytesRead;
    }

Vs64 VDirectIOFileStream::write(const Vu8* buffer, Vs64 numBytesToWrite)
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

        actualCount = static_cast<size_t> (VDirectIOFileStream::_wrap_write(mFile, buffer + numBytesWritten, requestCount));

        numBytesWritten += actualCount;
        numBytesRemaining -= actualCount;

        } while ((actualCount == requestCount) && (numBytesRemaining > 0));

    if (numBytesWritten != numBytesToWrite)
        {
        VString    path;
        mNode.getPath(path);

        throw VException(errno, "VDirectIOFileStream::write to '%s' only wrote %lld of %lld requested bytes.", path.chars(), numBytesWritten, numBytesToWrite);
        }

    return numBytesWritten;
    }

void VDirectIOFileStream::flush()
    {
    // Unbuffered file writes have no flush mechanism since they are not buffered.
    }

bool VDirectIOFileStream::skip(Vs64 numBytesToSkip)
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
    // FIXME: This calls VDirectIOFileStream::seek below, which is not as savvy as this--should it be?
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

bool VDirectIOFileStream::seek(Vs64 inOffset, int whence)
    {
    // FIXME: need to deal with Vs64-to-off_t conversion
    return (VDirectIOFileStream::_wrap_lseek(mFile, static_cast<off_t> (inOffset), whence) == 0);
    }

Vs64 VDirectIOFileStream::offset() const
    {
    return VDirectIOFileStream::_wrap_lseek(mFile, static_cast<off_t> (0), SEEK_CUR);
    }

Vs64 VDirectIOFileStream::available() const
    {
    Vs64    currentOffset = this->offset();
    Vs64    eofOffset;
    
    const_cast<VDirectIOFileStream*>(this)->seek(0, SEEK_END);
    eofOffset = this->offset();
    const_cast<VDirectIOFileStream*>(this)->seek(currentOffset, SEEK_SET);    // restore original position
    
    return eofOffset - currentOffset;
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
int VDirectIOFileStream::_wrap_open(const char* path, int flags)
    {
    if ((path == NULL) || (path[0] == 0))
        return -1;

    int        fd = -1;
    bool    done = false;

    while (! done)
        {
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
ssize_t VDirectIOFileStream::_wrap_read(int fd, void* buffer, size_t numBytes)
    {
    ssize_t    result;
    bool    done = false;

    while (! done)
        {
        result = vault::read(fd, buffer, numBytes);
        
        if ((result != (ssize_t) -1) || (errno != EINTR))
            done = true;
        }

    _debugCheck(result != -1);
    
    return result;
    }

// static
ssize_t VDirectIOFileStream::_wrap_write(int fd, const void* buffer, size_t numBytes)
    {
    ssize_t    result;
    bool    done = false;

    while (! done)
        {
        result = vault::write(fd, buffer, numBytes);
        
        if ((result != (ssize_t) -1) || (errno != EINTR))
            done = true;
        }

    _debugCheck(result != -1);
    
    return result;
    }

// static
off_t VDirectIOFileStream::_wrap_lseek(int fd, off_t inOffset, int whence)
    {
    off_t    result;
    bool    done = false;

    while (! done)
        {
        result = vault::lseek(fd, inOffset, whence);
        
        if ((result != (off_t) -1) || (errno != EINTR))
            done = true;
        }

    _debugCheck(result != (off_t) -1);
    
    return result;
    }

// static
int VDirectIOFileStream::_wrap_close(int fd)
    {
    int        result;
    bool    done = false;

    while (! done)
        {
        result = vault::close(fd);
        
        if ((result == 0) || (errno != EINTR))
            done = true;
        }

    _debugCheck(result != -1);
    
    return result;
    }

