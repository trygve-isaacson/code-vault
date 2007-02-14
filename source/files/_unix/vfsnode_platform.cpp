/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vfsnode.h"

#include "vexception.h"
#include "vthread.h"
#include <dirent.h>

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

bool VFSNode::_platform_getNodeInfo(VFSNodeInfo& info) const
    {
    struct stat statData;
    int result = VFSNode::_wrap_stat(mPath, &statData);
    
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
    int result = VFSNode::_wrap_mkdir(mPath, (S_IFDIR | S_IRWXO | S_IRWXG | S_IRWXU));

    if (result != 0)
        throw VException(result, "VFSNode::_platform_createDirectory failed (error %d: %s) for '%s'.", errno, ::strerror(errno), mPath.chars());
    }

bool VFSNode::_platform_removeDirectory() const
    {
    int result = VFSNode::_wrap_rmdir(mPath);
    return (result == 0);
    }

bool VFSNode::_platform_removeFile() const
    {
    int result = VFSNode::_wrap_unlink(mPath);
    return (result == 0);
    }

void VFSNode::_platform_renameNode(const VString& newName) const
    {
    VFSNode    parentNode;
    this->getParentNode(parentNode);
    
    VString    newPath;
    parentNode.getChildPath(newName, newPath);
    
    int result = VFSNode::_wrap_rename(mPath, newPath);
    
    if (result != 0)
        throw VException(result, "VFSNode::_platform_renameNode failed (error %d: %s) renaming '%s' to '%s'.", errno, ::strerror(errno), mPath.chars(), newPath.chars());
    }

// This is the Unix implementation of directory iteration using
// opendir(), readdir(), closedir() functions.

void VFSNode::_platform_directoryIterate(VDirectoryIterationCallback& callback) const
    {
    VString nodeName;

    DIR* dir = ::opendir(mPath);
    
    if (dir == NULL)
        throw VException("VFSNode::_platform_getDirectoryList failed for directory '%s'.", mPath.chars());

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

