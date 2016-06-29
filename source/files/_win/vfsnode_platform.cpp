/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

/** @file */

#include "vfsnode.h"
#include "vtypes_internal.h"

#include "vexception.h"
#include "vthread.h"
#ifdef VCOMPILER_MSVC
    #pragma warning(disable: 6387)  // the following system header emits warnings
#endif
#include <shlobj.h>
#ifdef VCOMPILER_MSVC
    #pragma warning(default: 6387)
#endif

/*
Notes on use of Win32 "wide" APIs:

We define APIs in VFileSystem for common functions across platforms, and these
take VStrings and call through to VPlatformAPI which can do any necessary conversion.

But there are few Win32 APIs we call here that are not exposed in a cross-platform way
in VFileSystem because they are not general-purpose. So we must do the necessary
conversions here, in this manner:

Win32 has three flavors of each string-based API.
The single-byte char, or "ANSI" versions, have an "A" suffix.
  For example: FindFirstFileA(const char* path, WIN32_FIND_DATAA* data)
  (WIN32_FIND_DATAA has instance variables containing char)
  These use ANSI C char strings that must be in the "current" code page.
  They are not compatible with arbitrary string data; only the current code page.
The two-byte char, or "wide" versions, have a "W" suffix.
  For example: FindFirstFileW(const wchar_t* path, WIN32_FIND_DATAW* data)
  (WIN32_FIND_DATAW has instance variables containing wchar_t)
  These use UTF-16 wchar_t strings, so they take Unicode in UTF-16 format.
  They will work with any UTF-16 character, regardless of the current code page.
A version of each API without any suffix at all is macro-based and defined
as the "A" version if UNICODE is not defined, or the "W" version if UNICODE is defined.
  For example: FindFirstFile() is defined as either FindFirstFileA() or FindFirstFileW()

We call the "W" versions directly, since our paths are all VString objects internally,
which store Unicode in UTF-8 format, and we can easily convert to/from UTF-16 when we call
the Win32 APIs here.
For APIs where we pass the string, we just convert on-the-fly as the parameter:
  SetterW(s.toUTF16().c_str());
For APIs where we get a string back, we pass a wchar_t buffer and then construct a
VString from it afterwards:
  wchar_t buf[LENGTH];
  GetterW(buf);
  VString s(buf);
*/

/* Note: according to Microsoft KB article 177506, only the following characters
are valid in file and folder names on their operating system. I should provide
some way of filtering out bad stuff for the particular platform.

- letters
- numbers
^   Accent circumflex (caret)
&   Ampersand
'   Apostrophe (single quotation mark)
@   At sign
{   Brace left
}   Brace right
[   Bracket opening
]   Bracket closing
,   Comma
$   Dollar sign
=   Equal sign
!   Exclamation point
-   Hyphen
#   Number sign
(   Parenthesis opening
)   Parenthesis closing
%   Percent
.   Period
+   Plus
~   Tilde
_   Underscore
*/

static const char _DOS_DRIVE_LETTER_SUFFIX_CHAR = ':';
static const char _DOS_PATH_SEPARATOR_CHAR = '\\';
static const char* _DOS_PATH_SEPARATOR_CHARS = "\\"; // The first backslash is a compiler escape for the second one. The string is length 1.

static bool _pathIsDriveLetterVolume(const VString& path) {
    // Example: "C:"
    if ((path.length() == 2) && (path[1] == _DOS_DRIVE_LETTER_SUFFIX_CHAR)) {
        return true;
    }

    // Example: "C:/"
    if ((path.length() == 3) && (path[1] == _DOS_DRIVE_LETTER_SUFFIX_CHAR) && (path[2] == VFSNode::PATH_SEPARATOR_CHAR)) {
        return true;
    }

    return false;
}

// static
VString VFSNode::_platform_normalizePath(const VString& path) {
    // For the moment, we get almost all the functionality we need by
    // simply converting backslash to slash if we are compiled for Windows.
    // Mac OS X support is free since it's Unix. Mac OS 9 support would
    // have to deal with ':'. DOS drive letters and Mac OS 9 root/relative
    // paths would complicate things a little for things like getParentPath.
    VString normalized(path);
    normalized.replace(_DOS_PATH_SEPARATOR_CHARS, PATH_SEPARATOR_CHARS);
    return normalized;
}

// static
VString VFSNode::_platform_denormalizePath(const VString& path) {
    // See comments above.
    VString denormalized(path);
    denormalized.replace(PATH_SEPARATOR_CHARS, _DOS_PATH_SEPARATOR_CHARS);
    return denormalized;
}

// static
VFSNode VFSNode::_platform_getKnownDirectoryNode(KnownDirectoryIdentifier id, const VString& companyName, const VString& appName) {
    if (id == CURRENT_WORKING_DIRECTORY) {
        return VFSNode(VPlatformAPI::getcwd());
    }

    if (id == EXECUTABLE_DIRECTORY) {
        VFSNode executable = VFSNode::getExecutable();
        VFSNode executableDirectory;
        executable.getParentNode(executableDirectory);
        return executableDirectory;
    }

    wchar_t pathBuffer[MAX_PATH];
    HRESULT gfpResult = ::SHGetFolderPathW(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, pathBuffer);

    if (gfpResult != S_OK) {
        throw VStackTraceException(VSTRING_FORMAT("VFSNode::_platform_getKnownDirectoryNode: Unable to find current user Application Data folder. Error code %d.", (int) gfpResult));
    }

    VString path(pathBuffer);
    VFSNode appDataFolder(VFSNode::normalizePath(path));

    // User's folder is one level up from user's Application Data folder.
    VFSNode currentUserFolder;
    appDataFolder.getParentNode(currentUserFolder);

    if (id == USER_HOME_DIRECTORY) {
        return currentUserFolder;
    }

    VFSNode companyFolder;
    if (companyName.isEmpty()) {
        companyFolder = appDataFolder;
    } else {
        appDataFolder.getChildNode(companyName, companyFolder);
        companyFolder.mkdir();
    }

    VFSNode appFolder;
    if (appName.isEmpty()) {
        appFolder = companyFolder;
    } else {
        companyFolder.getChildNode(appName, appFolder);
        appFolder.mkdir();
    }

    VFSNode resultNode;

    switch (id) {
        case USER_HOME_DIRECTORY:
            // handled earlier; we returned above
            break;

        case LOG_FILES_DIRECTORY:
            appFolder.getChildNode("Logs", resultNode);
            break;

        case USER_PREFERENCES_DIRECTORY:
            appFolder.getChildNode("Preferences", resultNode);
            break;

        case CACHED_DATA_DIRECTORY:
            appFolder.getChildNode("Caches", resultNode);
            break;

        case APPLICATION_DATA_DIRECTORY:
            resultNode = appFolder;
            break;

        case EXECUTABLE_DIRECTORY:
            // handled earlier; we returned above
            break;

        default:
            throw VStackTraceException(VSTRING_FORMAT("VFSNode::_platform_getKnownDirectoryNode: Requested invalid directory ID %d.", (int) id));
            break;
    }

    resultNode.mkdir();

    return resultNode;
}

// static
VFSNode VFSNode::_platform_getExecutable() {
    wchar_t exePathBuffer[MAX_PATH];
    DWORD result = ::GetModuleFileNameW(HMODULE(NULL), exePathBuffer, DWORD(MAX_PATH));

    if (result == 0)
        throw VStackTraceException(VSystemError(), "VFSNode::_platform_getExecutable: Unable to determine exe path.");

    VString exePath(exePathBuffer);
    VFSNode exeNode(VFSNode::normalizePath(exePath));
    return exeNode;
}

bool VFSNode::_platform_getNodeInfo(VFSNodeInfo& info) const {
    
    // _wstat does not work on volumes; if we know the path is a drive letter, use alternate API to check just the existence
    if (_pathIsDriveLetterVolume(mPath)) {
        // GetVolumeInformationW() requires trailing backslash. Add if not present.
        std::wstring denormalizedWidePath = VFSNode::denormalizePath(mPath).toUTF16();
        if (!mPath.endsWith(PATH_SEPARATOR_CHAR)) {
            denormalizedWidePath += _DOS_PATH_SEPARATOR_CHAR;
        }
        bool exists = ::GetVolumeInformationW(denormalizedWidePath.c_str(), NULL, 0, NULL, NULL, NULL, NULL, 0) ? true : false; // BOOL to bool conversion
        info.mIsDirectory = exists;
        return exists;
    }

    struct stat statData;
    int result = VFileSystem::stat(mPath, &statData);

    if (result >= 0) {
        info.mCreationDate = CONST_S64(1000) * static_cast<Vs64>(statData.st_ctime);
        info.mModificationDate = CONST_S64(1000) * static_cast<Vs64>(statData.st_mtime);
        info.mFileSize = statData.st_size;
        DWORD attributes = ::GetFileAttributesW(VFSNode::denormalizePath(mPath).toUTF16().c_str());
        info.mIsFile = ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0);
        info.mIsDirectory = ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
        info.mErrNo = 0;
    } else {
        info.mErrNo = errno;
    }

    return (result >= 0);
}

void VFSNode::_platform_createDirectory() const {
    int result = VFileSystem::mkdir(mPath, (S_IFDIR | S_IRWXO | S_IRWXG | S_IRWXU));

    if (result != 0)
        throw VException(VSystemError(), VSTRING_FORMAT("VFSNode::_platform_createDirectory failed with result %d for '%s'.", result, mPath.chars()));
}

bool VFSNode::_platform_removeDirectory() const {
    int result = VFileSystem::rmdir(mPath);
    return (result == 0);
}

bool VFSNode::_platform_removeFile() const {
    int result = VFileSystem::unlink(mPath);
    return (result == 0);
}

void VFSNode::_platform_renameNode(const VString& newPath) const {
    int result = VFileSystem::rename(mPath, newPath);

    if (result != 0)
        throw VException(VSystemError(), VSTRING_FORMAT("VFSNode::_platform_renameNode failed with result %d renaming '%s' to '%s'.", result, mPath.chars(), newPath.chars()));
}

// This is the Windows implementation of directory iteration using
// FindFirstFile(), FindNextFile(), FindClose() functions.

void VFSNode::_platform_directoryIterate(VDirectoryIterationCallback& callback) const {
    VString searchPath(VFSNode::denormalizePath(VSTRING_FORMAT("%s/*", mPath.chars())));    // Supply DOS path syntax to Win32 API
    WIN32_FIND_DATAW data;
    HANDLE dir = ::FindFirstFileW(searchPath.toUTF16().c_str(), &data);

    if (dir == INVALID_HANDLE_VALUE) {
        DWORD error = ::GetLastError();

        if (error == ERROR_NO_MORE_FILES) {
            return;
        }

        throw VException(VSTRING_FORMAT("VFSNode::_platform_getDirectoryList failed (error %d) for directory '%s'.", error , searchPath.chars()));
    }

    try {
        bool keepGoing = true;

        do {
            VThread::yield(); // be nice if we're iterating over a huge directory

            VString nodeName = data.cFileName; // assign VString from wide char string

            // Skip current and parent pseudo-entries. Otherwise client must
            // know too much detail in order to avoid traversal problems.
            if ((nodeName != ".") &&
                    (nodeName != "..")) {
                VFSNode childNode;
                this->getChildNode(nodeName, childNode);
                keepGoing = callback.handleNextNode(childNode);
            }

        } while (keepGoing && ::FindNextFileW(dir, &data));
    } catch (...) {
        ::FindClose(dir);
        throw;
    }

    ::FindClose(dir);
}

