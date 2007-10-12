/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vfsnode.h"

#include "vexception.h"
#include "vinstant.h"

// VListNodeInfoCallback -----------------------------------------------------

class VFSNodeListCallback : public VDirectoryIterationCallback
    {
    public:

        VFSNodeListCallback(VFSNodeVector& nodeList) : VDirectoryIterationCallback(), mNodeList(nodeList) {}
        virtual ~VFSNodeListCallback() {}

        virtual bool handleNextNode(const VFSNode& node);

    private:

        VFSNodeVector& mNodeList;
    };

bool VFSNodeListCallback::handleNextNode(const VFSNode& node)
    {
    mNodeList.push_back(node);
    return true;
    }

// VFSNodeNameCallback -----------------------------------------------------

class VFSNodeNameCallback : public VDirectoryIterationCallback
    {
    public:

        VFSNodeNameCallback(VStringVector& nameList) : VDirectoryIterationCallback(), mNameList(nameList) {}
        virtual ~VFSNodeNameCallback() {}

        virtual bool handleNextNode(const VFSNode& node);

    private:

        VStringVector& mNameList;
    };

bool VFSNodeNameCallback::handleNextNode(const VFSNode& node)
    {
    VString nodeName;
    node.getName(nodeName);
    mNameList.push_back(nodeName);
    return true;
    }

// VFSNodeFindCallback -----------------------------------------------------

class VFSNodeFindCallback : public VDirectoryIterationCallback
    {
    public:

        VFSNodeFindCallback(const VString& nameToMatch);
        virtual ~VFSNodeFindCallback() {}

        virtual bool handleNextNode(const VFSNode& node);

        bool found() const { return mFound; }
        void getMatchedNode(VFSNode& node) const { node = mMatchedNode; }

    private:

        bool    mFound;
        VString mNameToMatchLowerCase;
        VFSNode mMatchedNode;
    };

VFSNodeFindCallback::VFSNodeFindCallback(const VString& nameToMatch) :
VDirectoryIterationCallback(),
mFound(false),
mNameToMatchLowerCase(nameToMatch)
    {
    mNameToMatchLowerCase.toLowerCase();
    }

bool VFSNodeFindCallback::handleNextNode(const VFSNode& node)
    {
    VString nodeNameLowerCase;
    node.getName(nodeNameLowerCase);
    nodeNameLowerCase.toLowerCase();

    if (nodeNameLowerCase == mNameToMatchLowerCase)
        {
        mFound = true;
        mMatchedNode = node;
        return false; // we found a match, so stop looking
        }

    return true;
    }

// VFSNode -------------------------------------------------------------------

// static
void VFSNode::normalizePath(VString& path)
    {
    VFSNode::_platform_normalizePath(path);
    }

// static
void VFSNode::denormalizePath(VString& path)
    {
    VFSNode::_platform_denormalizePath(path);
    }

VFSNode::VFSNode()
    {
    }

VFSNode::VFSNode(const VFSNode& node) :
mPath(node.mPath)
    {
    }

VFSNode::VFSNode(const VString& path) :
mPath(path)
    {
    if (path.isEmpty())
        mPath = ".";
    }

VFSNode& VFSNode::operator=(const VFSNode& other)
    {
    mPath = other.mPath;
    return *this;
    }

void VFSNode::setPath(const VString& path)
    {
    if (path.isEmpty())
        mPath = ".";
    else
        mPath = path;
    }

void VFSNode::getPath(VString& path) const
    {
    path = mPath;
    }

const VString& VFSNode::getPath() const
    {
    return mPath;
    }

void VFSNode::getName(VString& name) const
    {
    // The following works even if lastIndexOf returns -1 "not found",
    // because we add 1 to get the correct startIndex parameter.
    name.copyFromBuffer(mPath.chars(), mPath.lastIndexOf('/') + 1, mPath.length());
    }

void VFSNode::getParentPath(VString& parentPath) const
    {
    parentPath.copyFromBuffer(mPath.chars(), 0, mPath.lastIndexOf('/'));
    }

void VFSNode::getParentNode(VFSNode& parent) const
    {
    VString    parentPath;

    this->getParentPath(parentPath);
    parent.setPath(parentPath);
    }

void VFSNode::getChildPath(const VString& childName, VString& childPath) const
    {
    childPath.format("%s/%s", mPath.chars(), childName.chars());
    }

void VFSNode::getChildNode(const VString& childName, VFSNode& child) const
    {
    VString    childPath;

    this->getChildPath(childName, childPath);
    child.setPath(childPath);
    }

void VFSNode::mkdirs() const
    {
    // If this directory already exists, we are done.
    if (this->exists())
        return;

    // Create the parent directory (and its parents etc.) if necessary.
    VFSNode    parentNode;
    this->getParentNode(parentNode);

    if (! parentNode.getPath().isEmpty())    // root or parent of supplied path must be assumed to exist
        parentNode.mkdirs();

    // Create this directory specifically.
    this->mkdir();
    }

void VFSNode::mkdir() const
    {
    this->_platform_createDirectory();
    }

bool VFSNode::rm() const
    {
    /*
    This could be optimized for Mac APIs which do a fast delete of
    the directory and its contents in one swipe. The following way
    is required on Unix file systems and is slower because we must
    delete a directory's contents before deleting it.
    */
    if (! this->exists())
        return false;

    bool    success = true;
    bool    isDir = this->isDirectory();

    if (isDir)
        success = this->rmDirContents();

    if (success)
        {
        if (isDir)
            success = this->_platform_removeDirectory();
        else
            success = this->_platform_removeFile();
        }

    return success;
    }

bool VFSNode::rmDirContents() const
    {
    bool            allSucceeded = true;
    VFSNodeVector    children;

    this->list(children);

    for (VSizeType i = 0; i < children.size(); ++i)
        {
        allSucceeded = allSucceeded && children[i].rm();
        }

    return allSucceeded;
    }

void VFSNode::renameToPath(const VString& newPath) const
    {
    VFSNode newNode(newPath);
    VString newName;

    newNode.getName(newName);

    this->_platform_renameNode(newName);
    }

void VFSNode::renameToName(const VString& newName) const
    {
    this->_platform_renameNode(newName);
    }

void VFSNode::renameToName(const VString& newName, VFSNode& nodeToUpdate) const
    {
    this->_platform_renameNode(newName);

    VFSNode    parentNode;
    this->getParentNode(parentNode);

    VString    newPath;
    parentNode.getChildPath(newName, newPath);

    nodeToUpdate.setPath(newPath);    // it IS allowed for nodeToUpdate to be this
    }

void VFSNode::renameToNode(const VFSNode& newNode) const
    {
    VString    newPath;

    newNode.getPath(newPath);

    this->renameToPath(newPath);
    }

void VFSNode::list(VStringVector& children) const
    {
    VFSNodeNameCallback callback(children);
    this->_platform_directoryIterate(callback);
    }

void VFSNode::list(VFSNodeVector& children) const
    {
    VFSNodeListCallback callback(children);
    this->_platform_directoryIterate(callback);
    }

void VFSNode::iterate(VDirectoryIterationCallback& callback) const
    {
    this->_platform_directoryIterate(callback);
    }

bool VFSNode::find(const VString& name, VFSNode& node) const
    {
    VFSNodeFindCallback callback(name);
    this->_platform_directoryIterate(callback);

    bool found = callback.found();
    if (found)
        callback.getMatchedNode(node);

    return found;
    }

bool VFSNode::exists() const
    {
    VFSNodeInfo info;
    return this->_platform_getNodeInfo(info); // only the function result is needed
    }

VInstant VFSNode::creationDate() const
    {
    VFSNodeInfo info;
    bool exists = this->_platform_getNodeInfo(info);

    if (!exists)
        throw VException(info.mErrNo, "VFSNode::creationDate failed (error %d: %s) for '%s'.", info.mErrNo, ::strerror(info.mErrNo), mPath.chars());

    return VInstant::instantFromRawValue(info.mCreationDate);
    }

VInstant VFSNode::modificationDate() const
    {
    VFSNodeInfo info;
    bool exists = this->_platform_getNodeInfo(info);

    if (!exists)
        throw VException(info.mErrNo, "VFSNode::modificationDate failed (error %d: %s) for '%s'.", info.mErrNo, ::strerror(info.mErrNo), mPath.chars());

    return VInstant::instantFromRawValue(info.mModificationDate);
    }

VFSize VFSNode::size() const
    {
    VFSNodeInfo info;
    bool exists = this->_platform_getNodeInfo(info);

    if (!exists)
        throw VException(info.mErrNo, "VFSNode::size failed (error %d: %s) for '%s'.", info.mErrNo, ::strerror(info.mErrNo), mPath.chars());

    return info.mFileSize;
    }

bool VFSNode::isFile() const
    {
    VFSNodeInfo info;
    bool exists = this->_platform_getNodeInfo(info);

    return exists && info.mIsFile;
    }

bool VFSNode::isDirectory() const
    {
    VFSNodeInfo info;
    bool exists = this->_platform_getNodeInfo(info);

    return exists && info.mIsDirectory;
    }

// This is a useful place to put a breakpoint when things aren't going as planned.
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
int VFSNode::_wrap_mkdir(const char* path, mode_t mode)
    {
    int        result;
    bool    done = false;

    while (! done)
        {
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

    if ((result == -1) && (errno == EEXIST))
        {
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
int VFSNode::_wrap_rename(const char* oldName, const char* newName)
    {
    int        result;
    bool    done = false;

    while (! done)
        {
        result = vault::rename(oldName, newName);

        if ((result == 0) || (errno != EINTR))
            done = true;
        }

    _debugCheck(result == 0);

    return result;
    }

// static
int VFSNode::_wrap_stat(const char* path, struct stat* buf)
    {
    int        result;
    bool    done = false;

    while (! done)
        {
        result = vault::stat(path, buf);

        if ((result == 0) || (errno != EINTR))
            done = true;
        }

    _debugCheck(result == 0);

    return result;
    }

// static
int VFSNode::_wrap_unlink(const char* path)
    {
    int        result;
    bool    done = false;

    while (! done)
        {
        result = vault::unlink(path);

        if ((result == 0) || (errno != EINTR))
            done = true;
        }

    _debugCheck(result == 0);

    return result;
    }

// static
int VFSNode::_wrap_rmdir(const char* path)
    {
    int        result;
    bool    done = false;

    while (! done)
        {
        result = vault::rmdir(path);

        if ((result == 0) || (errno != EINTR))
            done = true;
        }

    _debugCheck(result == 0);

    return result;
    }

// VFSNodeInfo ---------------------------------------------------------------

VFSNodeInfo::VFSNodeInfo() :
mCreationDate(0),
mModificationDate(0),
mFileSize(0),
mIsFile(false),
mIsDirectory(false),
mErrNo(0)
    {
    }

