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
#include <dirent.h>

#ifdef VPLATFORM_MAC
    #ifndef VPLATFORM_MAC_IOS
        // There is a conflict between standard and OpenTransport definitions such as TCP_NODELAY. We don't use OT nor want it. This
        // only cropped up when turning on precompiled headers. Specifying MAC_OS_X_VERSION_MIN_REQUIRED to MAC_OS_X_VERSION_10_4
        // should suppress the OT includes at the end of OSServices.h, but I can't make that work. These three
        // defines have the same effect. CoreServices.h is the thing that includes OSServices.h which conditionally
        // includes the three OpenTransport*.h files. We need CoreServices.h to use FSFindFolder et al in our directory operations here.
        // We also need Carbon.h for GetCurrentProcess and GetProcessBundleLocation in _platform_getExecutable.
        #define __OPENTRANSPORT__
        #define __OPENTRANSPORTPROVIDERS__
        #define __OPENTRANSPORTPROTOCOL__

        // Workaround problem on iOS SDK. Prevent inclusion of WebServicesCore, which presumes inclusion of CFXMLNode.h which does not exist.
        #define __WEBSERVICESCORE__

        #include <CoreServices/CoreServices.h>
    #endif /* not VPLATFORM_MAC_IOS */
#endif /* VPLATFORM_MAC */

// static
void VFSNode::_platform_normalizePath(VString& /*path*/) {
    // Paths are already in our "normalized" form on Unix.
}

// static
void VFSNode::_platform_denormalizePath(VString& /*path*/) {
    // Paths are already in our "normalized" form on Unix.
}

#ifdef VPLATFORM_MAC

extern VString _V_NSHomeDirectory(); // Implemented in private vtypes_platform.mm Objective-C++ code.

// static
VFSNode VFSNode::_platform_getKnownDirectoryNode(KnownDirectoryIdentifier id, const VString& companyName, const VString& appName) {
    if (id == CURRENT_WORKING_DIRECTORY) {
        char cwdPath[PATH_MAX];
        (void)/*char* pathPtr =*/ vault::getcwd(cwdPath, sizeof(cwdPath));
        VFSNode cwdNode(cwdPath);
        return cwdNode;
    }

    if (id == EXECUTABLE_DIRECTORY) {
        /*
        This depends on the structure of the application or tool.
        If it's an iOS application, it's a bundle where we have:
            /...../wanted-dir/AppName.app/executable
            (2 levels up, wanted-dir is a randomized serial number at some path)
        If it's built as a Mac OS X application bundle we have:
            /...../wanted-dir/AppName.app/Contents/MacOS/executable
            (4 levels up, typically wanted-dir is /Applications if installed, but doesn't have to be)
        If it's built as a simple Unix-y tool we have:
            /...../wanted-dir/executable
            (1 level up, wanted-dir is wherever the tool has been placed)
        */
#ifdef VPLATFORM_MAC_IOS
        const int NUM_LEVELS_UP = 2;
#else
#ifdef VAULT_MACOSX_APP_IS_BUNDLE
        const int NUM_LEVELS_UP = 4;
#else
        const int NUM_LEVELS_UP = 1;
#endif
#endif
        VFSNode node = VFSNode::getExecutable();
        for (int i = 0; i < NUM_LEVELS_UP; ++i) {
            VFSNode parentNode;
            node.getParentNode(parentNode);
            node = parentNode;
        }

        return node;
    }

    VFSNode currentUserFolder(_V_NSHomeDirectory());

    if (id == USER_HOME_DIRECTORY) {
        return currentUserFolder;
    }

    VFSNode libraryFolder;
    currentUserFolder.getChildNode("Library", libraryFolder);
    libraryFolder.mkdir();

    VFSNode subFolder;

    switch (id) {
        case USER_HOME_DIRECTORY:
            // handled earlier; we returned above
            break;

        case LOG_FILES_DIRECTORY:
            libraryFolder.getChildNode("Logs", subFolder);
            break;

        case USER_PREFERENCES_DIRECTORY:
            libraryFolder.getChildNode("Preferences", subFolder);
            break;

        case CACHED_DATA_DIRECTORY:
            libraryFolder.getChildNode("Caches", subFolder);
            break;

        case APPLICATION_DATA_DIRECTORY:
            subFolder = libraryFolder;
            break;

        case CURRENT_WORKING_DIRECTORY:
            // handled earlier; we returned above
            break;

        case EXECUTABLE_DIRECTORY:
            // handled earlier; we returned above
            break;

        default:
            throw VStackTraceException(VSTRING_FORMAT("VFSNode::_platform_getKnownDirectoryNode: Requested invalid directory ID %d.", (int) id));
            break;
    }

    subFolder.mkdir();

    VFSNode companyFolder;
    if (companyName.isEmpty()) {
        companyFolder = subFolder;
    } else {
        subFolder.getChildNode(companyName, companyFolder);
        companyFolder.mkdir();
    }

    VFSNode resultNode;
    if (appName.isEmpty()) {
        resultNode = companyFolder;
    } else {
        companyFolder.getChildNode(appName, resultNode);
        resultNode.mkdir();
    }

    return resultNode;
}

#include <mach-o/dyld.h> // for _NSGetExecutablePath()

// static
VFSNode VFSNode::_platform_getExecutable() {
    uint32_t bufsize = 3000; // can in theory be bigger than MAXPATH=1024
    VString executablePath;
    executablePath.preflight((int) bufsize);
    char* buffer = executablePath.buffer();
    int result = _NSGetExecutablePath(buffer, &bufsize);
    if (result == -1) {
        throw VStackTraceException(VSTRING_FORMAT("VFSNode::_platform_getExecutable: Failed to get path. _NSGetExecutablePath returned %d.", result));
    }

    executablePath.postflight((int) ::strlen(buffer));

    // todo: could then convert to a "real path" in case returned path has sym links
    VFSNode::normalizePath(executablePath); // must supply normalized form to VFSNode below
    return VFSNode(executablePath);
}

#else /* end of Mac OS X implementation of VFSNode::_platform_getKnownDirectoryNode and VFSNode::_platform_getExecutable */

/* The generic Unix implementation of VFSNode::_platform_getKnownDirectoryNode and VFSNode::_platform_getExecutable follows */

#include <pwd.h>

// static
VFSNode VFSNode::_platform_getKnownDirectoryNode(KnownDirectoryIdentifier id, const VString& companyName, const VString& appName) {
    if (id == CURRENT_WORKING_DIRECTORY) {
        char cwdPath[PATH_MAX];
        (void)/*char* pathPtr =*/ vault::getcwd(cwdPath, sizeof(cwdPath));
        VFSNode cwdNode(cwdPath);
        return cwdNode;
    }

    if (id == EXECUTABLE_DIRECTORY) {
        VFSNode executable = VFSNode::getExecutable();
        VFSNode executableDirectory;
        executable.getParentNode(executableDirectory);
        return executableDirectory;
    }

    struct passwd* pwInfo = ::getpwuid(::getuid()); // Get info about the current user.
    if (pwInfo == NULL) {
        throw VStackTraceException(errno, VSTRING_FORMAT("VFSNode::_platform_getKnownDirectoryNode failed to get current user info from getpwuid() (error %d: %s)", errno, (errno == 0 ? "No such user" : ::strerror(errno))));
    }

    const VString homePath(pwInfo->pw_dir);

    if (id == USER_HOME_DIRECTORY) {
        return VFSNode(homePath);
    }

    VString basePath;
    VString companyFolderName(companyName);

    switch (id) {
        case USER_HOME_DIRECTORY:
            // handled earlier; we returned above
            break;

        case LOG_FILES_DIRECTORY:
            basePath = homePath + "/log";
            break;

        case USER_PREFERENCES_DIRECTORY:
            basePath = homePath;
            if (companyName.isNotEmpty()) {
                companyFolderName.format(".%s", companyName.chars());
            }
            break;

        case CACHED_DATA_DIRECTORY:
            basePath = homePath + "/cache";
            break;

        case APPLICATION_DATA_DIRECTORY:
            basePath = homePath + "/data";
            break;

        case CURRENT_WORKING_DIRECTORY:
            // handled earlier; we returned above
            break;

        case EXECUTABLE_DIRECTORY:
            // handled earlier; we returned above
            break;

        default:
            throw VStackTraceException(VSTRING_FORMAT("VFSNode::_platform_getKnownDirectoryNode: Requested invalid directory ID %d.", (int) id));
            break;
    }

    VFSNode baseDir(basePath);
    baseDir.mkdir();

    VFSNode companyFolder;
    if (companyFolderName.isEmpty()) {
        companyFolder = baseDir;
    } else {
        baseDir.getChildNode(companyFolderName, companyFolder);
        companyFolder.mkdir();
    }

    VFSNode resultNode;
    if (appName.isEmpty()) {
        resultNode = companyFolder;
    } else {
        companyFolder.getChildNode(appName, resultNode);
        resultNode.mkdir();
    }

    return resultNode;
}

// Assume Linux; conditionalize others:
#ifdef VPLATFORM_UNIX_BSD
static const VString PROCESS_LINKPATH("/proc/curproc/file");
#else
static const VString PROCESS_LINKPATH("/proc/self/exe");
#endif

// static
VFSNode VFSNode::_platform_getExecutable() {
    const int PATH_BUFFER_SIZE = 1024;
    VString executablePath;
    executablePath.preflight(PATH_BUFFER_SIZE);
    ssize_t len = ::readlink(PROCESS_LINKPATH, executablePath.buffer(), PATH_BUFFER_SIZE - 1);
    if (len == -1) {
        throw VStackTraceException(VSTRING_FORMAT("VFSNode::_platform_getExecutable: Unable to determine executable path. Error %d (%s)", (int) errno, ::strerror(errno)));
    }

    executablePath.postflight(len);
    VFSNode::normalizePath(executablePath); // must supply normalized form to VFSNode below
    return VFSNode(executablePath);
}

#endif /* end of generic Unix implementation of VFSNode::_platform_getKnownDirectoryNode and VFSNode::_platform_getExecutable */

bool VFSNode::_platform_getNodeInfo(VFSNodeInfo& info) const {
    struct stat statData;
    int result = VFileSystemAPI::wrap_stat(mPath, &statData);

    if (result >= 0) {
        info.mCreationDate = CONST_S64(1000) * static_cast<Vs64>(statData.st_ctime);
        info.mModificationDate = CONST_S64(1000) * static_cast<Vs64>(statData.st_mtime);
        info.mFileSize = statData.st_size;
        info.mIsFile = (! S_ISDIR(statData.st_mode)) && (! S_ISLNK(statData.st_mode));
        info.mIsDirectory = (S_ISDIR(statData.st_mode)) || (S_ISLNK(statData.st_mode));
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

// This is the Unix implementation of directory iteration using
// opendir(), readdir(), closedir() functions.

void VFSNode::_platform_directoryIterate(VDirectoryIterationCallback& callback) const {
    VString nodeName;

    DIR* dir = ::opendir(mPath);

    if (dir == NULL) {
        throw VException(VSTRING_FORMAT("VFSNode::_platform_getDirectoryList failed for directory '%s'.", mPath.chars()));
    }

    try {
        bool keepGoing = true;
        struct dirent* entry = ::readdir(dir);

        while (keepGoing && (entry != NULL)) {
            VThread::yield(); // be nice if we're iterating over a huge directory

            nodeName.copyFromCString(entry->d_name);

            // Skip current and parent pseudo-entries. Otherwise client must
            // know too much detail in order to avoid traversal problems.
            if ((nodeName != ".") &&
                    (nodeName != "..")) {
                VFSNode childNode;
                this->getChildNode(nodeName, childNode);
                keepGoing = callback.handleNextNode(childNode);
            }

            entry = ::readdir(dir);
        }
    } catch (...) {
        ::closedir(dir);
        throw;
    }

    ::closedir(dir);
}

