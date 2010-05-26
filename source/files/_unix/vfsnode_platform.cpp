/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vfsnode.h"
#include "vtypes_internal.h"

#include "vexception.h"
#include "vthread.h"
#include <dirent.h>

#ifdef VPLATFORM_MAC
    // There is a conflict between standard and OpenTransport definitions such as TCP_NODELAY. We don't use OT nor want it. This
    // only cropped up when turning on precompiled headers. Specifying MAC_OS_X_VERSION_MIN_REQUIRED to MAC_OS_X_VERSION_10_4
    // should suppress the OT includes at the end of OSServices.h, but I can't make that work. These three
    // defines have the same effect. CoreServices.h is the thing that includes OSServices.h which conditionall
    // includes the three OpenTransport*.h files. We need CoreServices.h to use FSFindFolder et al in our directory operations here.
    // We also need Carbon.h for GetCurrentProcess and GetProcessBundleLocation in _platform_getExecutable.
    #define __OPENTRANSPORT__
    #define __OPENTRANSPORTPROVIDERS__
    #define __OPENTRANSPORTPROTOCOL__
    #include <CoreServices/CoreServices.h>
#endif

// static
void VFSNode::_platform_normalizePath(VString& /*path*/)
    {
    // Paths are already in our "normalized" form on Unix.
    }

// static
void VFSNode::_platform_denormalizePath(VString& /*path*/)
    {
    // Paths are already in our "normalized" form on Unix.
    }

#ifdef VPLATFORM_MAC
// static
VFSNode VFSNode::_platform_getKnownDirectoryNode(KnownDirectoryIdentifier id, const VString& companyName, const VString& appName)
    {
    if (id == CURRENT_WORKING_DIRECTORY)
        {
        char cwdPath[PATH_MAX];
        (void)/*char* pathPtr =*/ vault::getcwd(cwdPath, sizeof(cwdPath));
        VFSNode cwdNode(cwdPath);
        return cwdNode;
        }

    if (id == EXECUTABLE_DIRECTORY)
        {
        return VFSNode::getExecutableDirectory();
        }

    FSRef ref;
    OSErr err = ::FSFindFolder(kOnAppropriateDisk, kCurrentUserFolderType, kDontCreateFolder, &ref);

    if (err != noErr)
        throw VException(VString("VFSNode::_platform_getKnownDirectoryNode: Unable to find current user folder. Error code %d.", (int) err));

    UInt8 pathBuffer[PATH_MAX];
    OSStatus status = ::FSRefMakePath(&ref, pathBuffer, PATH_MAX);

    if (status != noErr)
        throw VException(VString("VFSNode::_platform_getKnownDirectoryNode: Unable to build path to current user folder. Error code %d.", (int) status));

    VFSNode currentUserFolder(reinterpret_cast<char*>(pathBuffer));

    if (id == USER_HOME_DIRECTORY)
        return currentUserFolder;

    VFSNode libraryFolder;
    currentUserFolder.getChildNode("Library", libraryFolder);
    libraryFolder.mkdir();

    VFSNode subFolder;

    switch (id)
        {
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
            throw VException(VString("VFSNode::_platform_getKnownDirectoryNode: Requested invalid directory ID %d.", (int) id));
            break;
        }

    subFolder.mkdir();

    VFSNode companyFolder;
    if (companyName.isEmpty())
        {
        companyFolder = subFolder;
        }
    else
        {
        subFolder.getChildNode(companyName, companyFolder);
        companyFolder.mkdir();
        }

    VFSNode resultNode;
    if (appName.isEmpty())
        {
        resultNode = companyFolder;
        }
    else
        {
        companyFolder.getChildNode(appName, resultNode);
        resultNode.mkdir();
        }

    return resultNode;
    }

#include <mach-o/dyld.h> // for _NSGetExecutablePath()

// static
VFSNode VFSNode::_platform_getExecutable()
    {
    uint32_t bufsize = 3000; // can in theory be bigger than MAXPATH=1024
    VString executablePath;
    executablePath.preflight((int) bufsize);
    int result = _NSGetExecutablePath(executablePath.buffer(), &bufsize);
    if (result == -1)
        throw VException(VString("VFSNode::_platform_getExecutable: Failed to get path. _NSGetExecutablePath returned %d.", result));

    executablePath.postflight((int) bufsize);

    // todo: could then convert to a "real path" in case returned path has sym links
    VFSNode::normalizePath(executablePath); // must supply normalized form to VFSNode below
    return VFSNode(executablePath);
    }
    
#else /* end of Mac OS X implementation of VFSNode::_platform_getKnownDirectoryNode and VFSNode::_platform_getExecutable */
/* The generic Unix implementation of VFSNode::_platform_getKnownDirectoryNode and VFSNode::_platform_getExecutable follows */

// static
VFSNode VFSNode::_platform_getKnownDirectoryNode(KnownDirectoryIdentifier id, const VString& companyName, const VString& appName)
    {
    if (id == CURRENT_WORKING_DIRECTORY)
        {
        char cwdPath[PATH_MAX];
        (void)/*char* pathPtr =*/ vault::getcwd(cwdPath, sizeof(cwdPath));
        VFSNode cwdNode(cwdPath);
        return cwdNode;
        }

    if (id == EXECUTABLE_DIRECTORY)
        {
        return VFSNode::getExecutableDirectory();
        }

    // todo: use getenv() or other API to get suitable path that "~" represents (the user's home dir path)
    const VString homePath("~");

    if (id == USER_HOME_DIRECTORY)
        return VFSNode(homePath);

    VString basePath;
    VString companyFolderName(companyName);

    switch (id)
        {
        case USER_HOME_DIRECTORY:
            // handled earlier; we returned above
            break;

        case LOG_FILES_DIRECTORY:
            basePath = "/var/log";
            break;

        case USER_PREFERENCES_DIRECTORY:
            basePath = homePath;
            if (companyName.isNotEmpty())
                companyFolderName = '.' + companyName;
            break;

        case CACHED_DATA_DIRECTORY:
            basePath = "/var/run";
            break;

        case APPLICATION_DATA_DIRECTORY:
            basePath = "/etc";
            break;

        case CURRENT_WORKING_DIRECTORY:
            // handled earlier; we returned above
            break;

        case EXECUTABLE_DIRECTORY:
            // handled earlier; we returned above
            break;

        default:
            throw VException(VString("VFSNode::_platform_getKnownDirectoryNode: Requested invalid directory ID %d.", (int) id));
            break;
        }

    VFSNode baseDir(basePath);
    baseDir.mkdir();

    VFSNode companyFolder;
    if (companyFolderName.isEmpty())
        {
        companyFolder = baseDir;
        }
    else
        {
        baseDir.getChildNode(companyFolderName, companyFolder);
        companyFolder.mkdir();
        }

    VFSNode resultNode;
    if (appName.isEmpty())
        {
        resultNode = companyFolder;
        }
    else
        {
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
VFSNode VFSNode::_platform_getExecutable()
    {
    const int PATH_BUFFER_SIZE = 1024;
    VString executablePath;
    executablePath.preflight(PATH_BUFFER_SIZE);
    ssize_t len = ::readlink(PROCESS_LINKPATH, executablePath.buffer(), PATH_BUFFER_SIZE-1);
    if (len == -1)
        throw VException(VString("VFSNode::_platform_getExecutable: Unable to determine executable path. Error %d (%s)", (int) errno, ::strerror(errno)));
    executablePath.postflight(len);
    VFSNode::normalizePath(executablePath); // must supply normalized form to VFSNode below
    return VFSNode(executablePath);
    }

#endif /* end of generic Unix implementation of VFSNode::_platform_getKnownDirectoryNode and VFSNode::_platform_getExecutable */

bool VFSNode::_platform_getNodeInfo(VFSNodeInfo& info) const
    {
    struct stat statData;
    int result = VFileSystemAPI::wrap_stat(mPath, &statData);

    if (result >= 0)
        {
        info.mCreationDate = CONST_S64(1000) * static_cast<Vs64>(statData.st_ctime);
        info.mModificationDate = CONST_S64(1000) * static_cast<Vs64>(statData.st_mtime);
        info.mFileSize = statData.st_size;
        info.mIsFile = (! S_ISDIR(statData.st_mode)) && (! S_ISLNK(statData.st_mode));
        info.mIsDirectory = (S_ISDIR(statData.st_mode)) || (S_ISLNK(statData.st_mode));
        info.mErrNo = 0;
        }
    else
        {
        info.mErrNo = errno;
        }

    return (result >= 0);
    }

void VFSNode::_platform_createDirectory() const
    {
    int result = VFileSystemAPI::wrap_mkdir(mPath, (S_IFDIR | S_IRWXO | S_IRWXG | S_IRWXU));

    if (result != 0)
        throw VException(result, VString("VFSNode::_platform_createDirectory failed (error %d: %s) for '%s'.", errno, ::strerror(errno), mPath.chars()));
    }

bool VFSNode::_platform_removeDirectory() const
    {
    int result = VFileSystemAPI::wrap_rmdir(mPath);
    return (result == 0);
    }

bool VFSNode::_platform_removeFile() const
    {
    int result = VFileSystemAPI::wrap_unlink(mPath);
    return (result == 0);
    }

void VFSNode::_platform_renameNode(const VString& newPath) const
    {
    int result = VFileSystemAPI::wrap_rename(mPath, newPath);

    if (result != 0)
        throw VException(result, VString("VFSNode::_platform_renameNode failed (error %d: %s) renaming '%s' to '%s'.", errno, ::strerror(errno), mPath.chars(), newPath.chars()));
    }

// This is the Unix implementation of directory iteration using
// opendir(), readdir(), closedir() functions.

void VFSNode::_platform_directoryIterate(VDirectoryIterationCallback& callback) const
    {
    VString nodeName;

    DIR* dir = ::opendir(mPath);

    if (dir == NULL)
        throw VException(VString("VFSNode::_platform_getDirectoryList failed for directory '%s'.", mPath.chars()));

    try
        {
        bool keepGoing = true;
        struct dirent* entry = ::readdir(dir);

        while (keepGoing && (entry != NULL))
            {
            VThread::yield(); // be nice if we're iterating over a huge directory

            nodeName = entry->d_name;

            // Skip current and parent pseudo-entries. Otherwise client must
            // know too much detail in order to avoid traversal problems.
            if ((nodeName != ".") &&
                (nodeName != ".."))
                {
                VFSNode childNode;
                this->getChildNode(nodeName, childNode);
                keepGoing = callback.handleNextNode(childNode);
                }

            entry = ::readdir(dir);
            }
        }
    catch (...)
        {
        ::closedir(dir);
        throw;
        }

    ::closedir(dir);
    }

