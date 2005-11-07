/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vbufferedfilestream.h"

#include "vexception.h"

VBufferedFileStream::VBufferedFileStream()
    {
    mFile = NULL;
    mCloseOnDestruct = true;
    }

VBufferedFileStream::VBufferedFileStream(const VFSNode& node)
: mNode(node)
    {
    mFile = NULL;
    mCloseOnDestruct = true;
    node.getName(mName);
    }

VBufferedFileStream::VBufferedFileStream(FILE* f)
    {
    mFile = f;
    mCloseOnDestruct = false;
    }

VBufferedFileStream::~VBufferedFileStream()
    {
    if (mCloseOnDestruct)
        VBufferedFileStream::close();
        
    mFile = NULL;
    }

void VBufferedFileStream::openReadOnly()
    {
    mFile = VBufferedFileStream::threadsafe_fopen(mNode.path(), "rb");
    
    this->throwIfOpenFailed("VBufferedFileStream::openReadOnly", mNode.path());
    }

void VBufferedFileStream::openReadWrite()
    {
    /*
    The semantics of ::fopen() and ::open() run counter to the normal desire
    to open a file r/w and have it created if it doesn't exist. For example,
    mode "r" does not create the file. Mode "w" does. So instead, we check for
    existence first, and then open it exactly the way we intend.
    */

    mFile = VBufferedFileStream::threadsafe_fopen(mNode.path(), mNode.exists() ? "r+b" : "w+b");
    
    this->throwIfOpenFailed("VBufferedFileStream::openReadWrite", mNode.path());
    }

void VBufferedFileStream::openWrite()
    {
    mFile = VBufferedFileStream::threadsafe_fopen(mNode.path(), "wb");
    
    this->throwIfOpenFailed("VBufferedFileStream::openWrite", mNode.path());
    }

void VBufferedFileStream::setNode(const VFSNode& node)
    {
    VString    path;
    
    node.getPath(path);
    mNode.setPath(path);

    node.getName(mName);
    }

const VFSNode& VBufferedFileStream::getNode() const
    {
    return mNode;
    }

void VBufferedFileStream::setFile(FILE* f)
    {
    mFile = f;
    mCloseOnDestruct = false;
    }

bool VBufferedFileStream::isOpen() const
    {
    return (mFile != NULL);
    }

void VBufferedFileStream::close()
    {
    if (this->isOpen())
        {
        (void) VBufferedFileStream::threadsafe_fclose(mFile);
        mFile = NULL;
        }
    }

Vs64 VBufferedFileStream::read(Vu8* targetBuffer, Vs64 numBytesToRead)
    {
    /*
    Most of the work here is to deal with the fact that we are providing
    a SInt64 count in our API, but the underlying OS API may only
    allow a size_t count. So we may have to read in a loop until the
    requested number of bytes is read or EOF is reached. In the typical
    case, of course, we only go through the loop once.
    */
    
    bool    needsSizeConversion = (sizeof(Vs64) != sizeof(size_t));
    Vs64    numBytesRemaining = numBytesToRead;
    Vs64    numBytesRead = 0;
    size_t    requestCount;
    size_t    actualCount;
    
    do
        {
        if (needsSizeConversion)
            {
            requestCount = static_cast<size_t> (V_MIN(((Vs64) (0x7FFFFFFF)), numBytesRemaining));
            }
        else
            {
            requestCount = static_cast<size_t> (numBytesRemaining);
            }

        actualCount = VBufferedFileStream::threadsafe_fread(targetBuffer + numBytesRead, 1, requestCount, mFile);

        numBytesRead += actualCount;
        numBytesRemaining -= actualCount;

        } while ((actualCount == requestCount) && (numBytesRemaining > 0));

    return numBytesRead;
    }

Vs64 VBufferedFileStream::write(const Vu8* buffer, Vs64 numBytesToWrite)
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

        actualCount = VBufferedFileStream::threadsafe_fwrite(buffer + numBytesWritten, 1, requestCount, mFile);

        numBytesWritten += actualCount;
        numBytesRemaining -= actualCount;

        } while ((actualCount == requestCount) && (numBytesRemaining > 0));

    if (numBytesWritten != numBytesToWrite)
        {
        VString    path;
        mNode.getPath(path);

        throw VException(errno, "VBufferedFileStream::write to '%s' only wrote %lld of %lld requested bytes.", path.chars(), numBytesWritten, numBytesToWrite);
        }

    return numBytesWritten;
    }

void VBufferedFileStream::flush()
    {
    int    result = VBufferedFileStream::threadsafe_fflush(mFile);
    
    if (result != 0)
        {
        VString    path;
        mNode.getPath(path);

        throw VException(result, "VBufferedFileStream::flush to '%s' failed with errno=%d.", path.chars(), errno);
        }
    }

bool VBufferedFileStream::skip(Vs64 numBytesToSkip)
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

        success = this->seek(numBytesToSkip, SEEK_CUR);
        
        numBytesRemaining -= requestCount;

        } while (success && (numBytesRemaining > 0));

    return success;
    }

bool VBufferedFileStream::seek(Vs64 inOffset, int whence)
    {
    // FIXME: need to deal with Vs64-to-long conversion
    return (VBufferedFileStream::threadsafe_fseek(mFile, static_cast<long> (inOffset), whence) == 0);
    }

Vs64 VBufferedFileStream::offset() const
    {
    return VBufferedFileStream::threadsafe_ftell(mFile);
    }

Vs64 VBufferedFileStream::available() const
    {
    Vs64    currentOffset = this->offset();
    Vs64    eofOffset;
    
    const_cast<VBufferedFileStream*>(this)->seek(0, SEEK_END);
    eofOffset = this->offset();
    const_cast<VBufferedFileStream*>(this)->seek(currentOffset, SEEK_SET);    // restore original position
    
    return eofOffset - currentOffset;
    }

void VBufferedFileStream::throwIfOpenFailed(const VString& failedMethod, const VString& path) const
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
FILE* VBufferedFileStream::threadsafe_fopen(const char* nativePath, const char* mode)
    {
    if ((nativePath == NULL) || (nativePath[0] == 0))
        return NULL;

    FILE*    f = NULL;
    bool    done = false;

    while (! done)
        {
        f = ::fopen(nativePath, mode);
        
        if ((f != NULL) || (errno != EINTR))
            done = true;
        }
    
    _debugCheck(f != NULL);
        
    return f;
    }

// static
int VBufferedFileStream::threadsafe_fclose(FILE* f)
    {
    if (f == NULL)
        return EOF;

    int        result;
    bool    done = false;

        
    while (! done)
        {
        result = ::fclose(f);
        
        if ((result == 0) || (errno != EINTR))
            done = true;
        }

    _debugCheck(result == 0);
    
    return result;
    }

// static
size_t VBufferedFileStream::threadsafe_fread(void* buffer, size_t size, size_t nItems, FILE* f)
    {
    if ((buffer == NULL) || (f == NULL))
        return 0;

    size_t    result;
    bool    done = false;

    while (! done)
        {
        result = ::fread(buffer, size, nItems, f);
        
        if ((result != nItems) && (ferror(f) != 0) && (errno == EINTR))
            done = false;
        else
            done = true;
        }

    _debugCheck(result == nItems);
    
    return result;
    }

// static
size_t VBufferedFileStream::threadsafe_fwrite(const void* buffer, size_t size, size_t nItems, FILE* f)
    {
    size_t    result;
    bool    done = false;

    // 04.11.15 JHR #23892 Added f == NULL check
    if ((buffer == NULL) || (f == NULL))
        return 0L;
        
    while (! done)
        {
        result = ::fwrite(buffer, size, nItems, f);
        
        if ((result != nItems) && (ferror(f) != 0) && (errno == EINTR))
            done = false;
        else
            done = true;
        }

    _debugCheck(result == nItems);
    
    return result;
    }

// static
int VBufferedFileStream::threadsafe_fseek(FILE* f, long int inOffset, int whence)
    {
    int        result;
    bool    done = false;

    if (f == NULL)
        return EOF;

    while (! done)
        {
        result = ::fseek(f, inOffset, whence);
        
        if ((result != -1) || (errno != EINTR))
            done = true;
        }

    _debugCheck(result == 0);

    return result;
    }

// static
int VBufferedFileStream::threadsafe_fflush(FILE* f)
    {
    int        result;
    bool    done = false;

    if (f == NULL)
        return EOF;

    while (! done)
        {
        result = ::fflush(f);
        
        if ((result == 0) || (errno != EINTR))
            done = true;
        }
    _debugCheck(result == 0);
    
    return result;
    }

// static
long int VBufferedFileStream::threadsafe_ftell(FILE* f)
    {
    long int    result;
    bool        done = false;

    if (f == NULL)
        return 0;

    while (! done)
        {
        result = ::ftell(f);
        
        if ((result >= 0) || (errno != EINTR))
            done = true;
        }

    _debugCheck(result != -1);
    
    return result;
    }

