/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
*/

/** @file */

#include "vtypes.h"
#include "vtypes_internal.h"

#include "vmutex.h"
#include "vmutexlocker.h"
#include "vstring.h"
#include "vexception.h"

#include <psapi.h> // for GetProcessMemoryInfo used by VgetMemoryUsage

V_STATIC_INIT_TRACE

Vs64 vault::VgetMemoryUsage() {
    PROCESS_MEMORY_COUNTERS info;
    BOOL success = ::GetProcessMemoryInfo(::GetCurrentProcess(), &info, sizeof(info));
    if (success)
        return info.WorkingSetSize;
    else
        return 0;
}

static const Vu8 kDOSLineEnding[2] = { 0x0D, 0x0A };

const Vu8* vault::VgetNativeLineEnding(int& numBytes) {
    numBytes = 2;
    return kDOSLineEnding;
}

// VSystemError ---------------------------------------------------------------
// Platform-specific implementation of VSystemError internal accessors.

// static
int VSystemError::_getSystemErrorCode() {
    return (int) ::GetLastError();
}

// static
int VSystemError::_getSocketErrorCode() {
    return ::WSAGetLastError();;
}

// static
VString VSystemError::_getSystemErrorMessage(int errorCode) {
    LPVOID bufferPtr;
    ::FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        (DWORD) errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR) &bufferPtr,
        0, NULL);

    VString result((char*) bufferPtr);
    ::LocalFree(bufferPtr);
    return result;
}

// static
bool VSystemError::_isLikePosixError(int posixErrorCode) const {
    // We are not POSIX. Perform translations for error codes we know about.
    // The list is endless, but these are the ones we need to check internally.
    switch (posixErrorCode) {
        case EINTR: return mErrorCode == WSAEINTR; break;
        case EBADF: return mErrorCode == WSAEBADF; break;
        case EPIPE: return false; break; // no such thing on Winsock
        default: break;
    }

    // Default to exact value test.
    return (posixErrorCode == mErrorCode);
}

// VPlatformAPI -----------------------------------------------------------------

// static
VString VPlatformAPI::getcwd() {

    wchar_t cwdPath[MAX_PATH];
    wchar_t* cwdResult = ::_wgetcwd(cwdPath, MAX_PATH);
    if (cwdResult == NULL) {
        throw VException(VSystemError(), "Call to _wgetcwd failed.");
    }
    
    return VFSNode::normalizePath(VString(cwdPath));
}

// static
int VPlatformAPI::open(const VString& path, int flags, mode_t mode) {
    return ::_wopen(VFSNode::denormalizePath(path).toUTF16().c_str(), flags, mode);
}

// static
FILE* VPlatformAPI::fopen(const VString& path, const char* mode) {
	return ::_wfopen(VFSNode::denormalizePath(path).toUTF16().c_str(), VString(mode).toUTF16().c_str());
}

// static
int VPlatformAPI::mkdir(const VString& path, mode_t /*mode*/) {
    return ::_wmkdir(VFSNode::denormalizePath(path).toUTF16().c_str());
}

// static
int VPlatformAPI::rmdir(const VString& path) {
    return ::_wrmdir(VFSNode::denormalizePath(path).toUTF16().c_str());
}

// static
int VPlatformAPI::unlink(const VString& path) {
    return ::_wunlink(VFSNode::denormalizePath(path).toUTF16().c_str());
}

// static
int VPlatformAPI::rename(const VString& oldName, const VString& newName) {
    return ::_wrename(VFSNode::denormalizePath(oldName).toUTF16().c_str(), VFSNode::denormalizePath(newName).toUTF16().c_str());
}

// static
int VPlatformAPI::stat(const VString& path, struct stat* buf) {
	// Win32 wide char API macro diversions for stat are a hairball.
	// To work with standard struct stat, we'll need to copy instance variables.
	struct _stat localBuf;
    int result = ::_wstat(VFSNode::denormalizePath(path).toUTF16().c_str(), &localBuf);
	if (result == 0) {
		buf->st_atime = localBuf.st_atime;
        buf->st_dev = localBuf.st_dev;
        buf->st_ino = localBuf.st_ino;
        buf->st_mode = localBuf.st_mode;
        buf->st_nlink = localBuf.st_nlink;
        buf->st_uid = localBuf.st_uid;
        buf->st_gid = localBuf.st_gid;
        buf->st_rdev = localBuf.st_rdev;
        buf->st_size = localBuf.st_size;
        buf->st_atime = localBuf.st_atime;
        buf->st_mtime = localBuf.st_mtime;
        buf->st_ctime = localBuf.st_ctime;
	}

	return result;
}

// miscellaneous --------------------------------------------------------------

static void getCurrentTZ(VString& tz) {
    char* tzEnvString = vault::getenv("TZ");

    if (tzEnvString == NULL)
        tz = VString::EMPTY();
    else
        tz = tzEnvString;
}

static void setCurrentTZ(const VString& tz) {
    VString envString(VSTRING_ARGS("TZ=%s", tz.chars()));

    /*
    The IEEE docs describe putenv()'s strange behavior:
    The (char*) we pass becomes owned by the system, until
    we replace it with another. Unless we keep track of
    each call, each call must result in a small leak. So be it.
    But we must allocate a separate buffer from the envString.
    */
    int bufferLength = 1 + envString.length();
    char* orphanBuffer = new char[bufferLength];
    envString.copyToBuffer(orphanBuffer, bufferLength);
    vault::putenv(orphanBuffer);
}

static VMutex gTimeGMMutex("gTimeGMMutex", true/*suppressLogging*/);

time_t timegm(struct tm* t) {
    VMutexLocker    locker(&gTimeGMMutex, "timegm");
    VString         savedTZ;

    getCurrentTZ(savedTZ);
    setCurrentTZ("UTC");

    time_t result = ::mktime(t);

    setCurrentTZ(savedTZ);

    return result;
}

#ifdef VCOMPILER_CODEWARRIOR
#include "vexception.h"
int vault::open(const char* path, int flags, mode_t mode) {
    throw VException(VSTRING_FORMAT("Error opening '%s': POSIX open() is not supported by CodeWarrior on Windows.", path));
}
FILE* vault::fopen(const char* path, mode_t mode) {
    throw VException(VSTRING_FORMAT("Error opening '%s': POSIX fopen() is not supported by CodeWarrior on Windows.", path));
}
#endif

// VAutoreleasePool is a no-op on Windows.
VAutoreleasePool::VAutoreleasePool() { mPool = NULL; }
void VAutoreleasePool::drain() {}
VAutoreleasePool::~VAutoreleasePool() {}

