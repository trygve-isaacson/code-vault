/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

/** @file */

#include "vfsnode.h"
#include "vtypes_internal.h"

#include "vexception.h"
#include "vthread.h"
#ifdef VCOMPILER_MSVC
    #pragma warning(disable: 6387)  // the library file doesn't past muster
#endif
#include <shlobj.h>
#ifdef VCOMPILER_MSVC
    #pragma warning(default: 6387)
#endif

/*
Note: We explicitly reference several the "A"-suffix data type and APIs here, because
we are using a non-Unicode path string, and even if this version of Windows supports
and is configured for Unicode, we must reference the non-Unicode APIs here. Examples:
    ::SHGetFolderPathA()
    ::GetModuleFileNameA()
    ::GetFileAttributesA()
    ::FindFirstFileA()
    ::FindNextFileA()
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

// static
void VFSNode::_platform_normalizePath(VString& path) {
    // For the moment, we get almost all the functionality we need by
    // simply converting backslash to slash if we are compiled for Windows.
    // Mac OS X support is free since it's Unix. Mac OS 9 support would
    // have to deal with ':'. DOS drive letters and Mac OS 9 root/relative
    // paths would complicate things a little for things like getParentPath.
    path.replace("\\", "/");
}

// static
void VFSNode::_platform_denormalizePath(VString& path) {
    // See comments above.
    path.replace("/", "\\");
}

// static
VFSNode VFSNode::_platform_getKnownDirectoryNode(KnownDirectoryIdentifier id, const VString& companyName, const VString& appName) {
    if (id == CURRENT_WORKING_DIRECTORY) {
        char cwdPath[MAX_PATH];
        (void)/*char* pathPtr =*/ vault::getcwd(cwdPath, sizeof(cwdPath));
        VString cwdPathString(cwdPath);
        VFSNode::normalizePath(cwdPathString);
        VFSNode cwdNode(cwdPathString);
        return cwdNode;
    }

    if (id == EXECUTABLE_DIRECTORY) {
        VFSNode executable = VFSNode::getExecutable();
        VFSNode executableDirectory;
        executable.getParentNode(executableDirectory);
        return executableDirectory;
    }

    char pathBuffer[MAX_PATH];
    HRESULT result = ::SHGetFolderPathA(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, static_cast<LPSTR>(pathBuffer));

    if (result != S_OK) {
        throw VStackTraceException(VSTRING_FORMAT("VFSNode::_platform_getKnownDirectoryNode: Unable to find current user Application Data folder. Error code %d.", (int) result));
    }

    VString path(pathBuffer);
    VFSNode::_platform_normalizePath(path);
    VFSNode appDataFolder(path);

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
    VString exePath;
    exePath.preflight(_MAX_PATH);   // preallocate buffer space
    DWORD result = ::GetModuleFileNameA(HMODULE(NULL), LPSTR(exePath.buffer()), DWORD(_MAX_PATH));

    if (result == 0)
        throw VStackTraceException(VSTRING_FORMAT("VFSNode::_platform_getExecutable: Unable to determine exe path. Error %d.", (int) ::GetLastError()));

    exePath.postflight(result); // result is actual length of returned string data
    VFSNode::normalizePath(exePath); // must supply normalized form to VFSNode below
    VFSNode exeNode(exePath);
    return exeNode;
}

bool VFSNode::_platform_getNodeInfo(VFSNodeInfo& info) const {
    struct stat statData;
    int result = VFileSystemAPI::wrap_stat(mPath, &statData);

    if (result >= 0) {
        info.mCreationDate = CONST_S64(1000) * static_cast<Vs64>(statData.st_ctime);
        info.mModificationDate = CONST_S64(1000) * static_cast<Vs64>(statData.st_mtime);
        info.mFileSize = statData.st_size;
        info.mIsFile = ((GetFileAttributesA(mPath) & FILE_ATTRIBUTE_DIRECTORY) == 0);
        info.mIsDirectory = ((GetFileAttributesA(mPath) & FILE_ATTRIBUTE_DIRECTORY) != 0);
        info.mErrNo = 0;
    } else {
        info.mErrNo = errno;
    }

    return (result >= 0);
}

void VFSNode::_platform_createDirectory() const {
    int result = VFileSystemAPI::wrap_mkdir(mPath, (S_IFDIR | S_IRWXO | S_IRWXG | S_IRWXU));

    if (result != 0)
        throw VException(result, VSTRING_FORMAT("VFSNode::_platform_createDirectory failed (error %d: %s) for '%s'.", errno, ::strerror(errno), mPath.chars()));
}

bool VFSNode::_platform_removeDirectory() const {
    int result = VFileSystemAPI::wrap_rmdir(mPath);
    return (result == 0);
}

bool VFSNode::_platform_removeFile() const {
    int result = VFileSystemAPI::wrap_unlink(mPath);
    return (result == 0);
}

void VFSNode::_platform_renameNode(const VString& newPath) const {
    int result = VFileSystemAPI::wrap_rename(mPath, newPath);

    if (result != 0)
        throw VException(result, VSTRING_FORMAT("VFSNode::_platform_renameNode failed (error %d: %s) renaming '%s' to '%s'.", errno, ::strerror(errno), mPath.chars(), newPath.chars()));
}

// This is the Windows implementation of directory iteration using
// FindFirstFile(), FindNextFile(), FindClose() functions.

void VFSNode::_platform_directoryIterate(VDirectoryIterationCallback& callback) const {
    // FIXME: This code has not been made UNICODE/DBCS compatible.
    // The problem areas are in the strings, and for now we just
    // brute-force cast so that the compiler is happy.

    VString nodeName;
    VString searchPath(VSTRING_ARGS("%s/*", mPath.chars()));

    VFSNode::denormalizePath(searchPath);    // make it have DOS syntax

    WIN32_FIND_DATAA data;
    HANDLE dir = ::FindFirstFileA((LPCSTR) searchPath.chars(), &data);

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

            nodeName = (char*) data.cFileName;

            // Skip current and parent pseudo-entries. Otherwise client must
            // know too much detail in order to avoid traversal problems.
            if ((nodeName != ".") &&
                    (nodeName != "..")) {
                VFSNode childNode;
                this->getChildNode(nodeName, childNode);
                keepGoing = callback.handleNextNode(childNode);
            }

        } while (keepGoing && ::FindNextFileA(dir, &data)); // see Unicode comment above
    } catch (...) {
        ::FindClose(dir);
        throw;
    }

    ::FindClose(dir);
}

