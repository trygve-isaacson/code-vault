/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

/** @file */

#include "vfsnode.h"

#include "vexception.h"
#include "vinstant.h"
#include "vbufferedfilestream.h"
#include "vtextiostream.h"
#include "vbinaryiostream.h"

// VListNodeInfoCallback -----------------------------------------------------

/**
This iteration callback is used to capture a directory's children's names as a list of nodes.
*/
class VFSNodeListCallback : public VDirectoryIterationCallback {
    public:

        VFSNodeListCallback(VFSNodeVector& nodeList) : VDirectoryIterationCallback(), mNodeList(nodeList) {}
        virtual ~VFSNodeListCallback() {}

        virtual bool handleNextNode(const VFSNode& node);

    private:

        // Prevent copy construction and assignment.
        VFSNodeListCallback(const VFSNodeListCallback& other);
        VFSNodeListCallback& operator=(const VFSNodeListCallback& other);

        VFSNodeVector& mNodeList;
};

bool VFSNodeListCallback::handleNextNode(const VFSNode& node) {
    mNodeList.push_back(node);
    return true;
}

// VFSNodeNameCallback -----------------------------------------------------

/**
This iteration callback is used to capture a directory's children's names as a list of strings.
*/
class VFSNodeNameCallback : public VDirectoryIterationCallback {
    public:

        VFSNodeNameCallback(VStringVector& nameList) : VDirectoryIterationCallback(), mNameList(nameList) {}
        virtual ~VFSNodeNameCallback() {}

        virtual bool handleNextNode(const VFSNode& node);

    private:

        // Prevent copy construction and assignment.
        VFSNodeNameCallback(const VFSNodeNameCallback& other);
        VFSNodeNameCallback& operator=(const VFSNodeNameCallback& other);

        VStringVector& mNameList;
};

bool VFSNodeNameCallback::handleNextNode(const VFSNode& node) {
    VString nodeName;
    node.getName(nodeName);
    mNameList.push_back(nodeName);
    return true;
}

// VFSNodeFindCallback -----------------------------------------------------

/**
This directory iterator is used by VFSNode::find() to search for a node with
a specified name.
*/
class VFSNodeFindCallback : public VDirectoryIterationCallback {
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

VFSNodeFindCallback::VFSNodeFindCallback(const VString& nameToMatch)
    : VDirectoryIterationCallback()
    , mFound(false)
    , mNameToMatchLowerCase(nameToMatch)
    , mMatchedNode()
    {
    mNameToMatchLowerCase.toLowerCase();
}

bool VFSNodeFindCallback::handleNextNode(const VFSNode& node) {
    VString nodeNameLowerCase;
    node.getName(nodeNameLowerCase);
    nodeNameLowerCase.toLowerCase();

    if (nodeNameLowerCase == mNameToMatchLowerCase) {
        mFound = true;
        mMatchedNode = node;
        return false; // we found a match, so stop looking
    }

    return true;
}

// VFSNode -------------------------------------------------------------------

const char VFSNode::PATH_SEPARATOR_CHAR = '/';
const char* VFSNode::PATH_SEPARATOR_CHARS = "/";

// static
VString VFSNode::normalizePath(const VString& path) {
    return VFSNode::_platform_normalizePath(path);
}

// static
VString VFSNode::denormalizePath(const VString& path) {
    return VFSNode::_platform_denormalizePath(path);
}

// static
VFSNode VFSNode::getKnownDirectoryNode(KnownDirectoryIdentifier id, const VString& companyName, const VString& appName) {
    return VFSNode::_platform_getKnownDirectoryNode(id, companyName, appName);
}

// static
VFSNode VFSNode::getCurrentWorkingDirectory() {
    return VFSNode::getKnownDirectoryNode(CURRENT_WORKING_DIRECTORY, VString::EMPTY(), VString::EMPTY());
}

// static
VFSNode VFSNode::getExecutableDirectory() {
    return VFSNode::getKnownDirectoryNode(EXECUTABLE_DIRECTORY, VString::EMPTY(), VString::EMPTY());
}

// static
VFSNode VFSNode::getExecutable() {
    return VFSNode::_platform_getExecutable();
}

static VInstantFormatter VFSNODE_SAFE_FILE_NAME_INSTANT_FORMATTER("yMMddHHmmssSSS");

// static
void VFSNode::safelyOverwriteFile(const VFSNode& target, Vs64 dataLength, VBinaryIOStream& dataStream, bool keepOld) {
    bool success = true;
    VString errorMessage;

    VString targetFileName = target.getName();

    VInstant now;
    VString temporaryFileName = now.getLocalString(VFSNODE_SAFE_FILE_NAME_INSTANT_FORMATTER) + "_tmp_" + targetFileName;
    VString keptFileName = now.getLocalString(VFSNODE_SAFE_FILE_NAME_INSTANT_FORMATTER) + "_ver_" + targetFileName;

    VFSNode directoryNode;
    target.getParentNode(directoryNode);
    VFSNode originalTargetNode(target);
    VFSNode temporaryFileNode(directoryNode, temporaryFileName);
    VFSNode keptFileNode(directoryNode, keptFileName);

    // Create and write to the temp file within a scope block to ensure file is closed when scope is exited.
    /* stream scope */ {
        VBufferedFileStream tempFileStream(temporaryFileNode);
        VBinaryIOStream tempOutputStream(tempFileStream);

        try {
            tempFileStream.openWrite();
        } catch (const VException& ex) {
            success = false;
            errorMessage = VSTRING_FORMAT("Unable to open temporary file '%s': %s", target.getPath().chars(), ex.what());
        }

        if (success) {
            try {
                VStream::streamCopy(dataStream, tempOutputStream, dataLength);
                tempOutputStream.flush();
            } catch (const VException& ex) {
                success = false;
                errorMessage = VSTRING_FORMAT("Unable to write to temporary file '%s': %s", target.getPath().chars(), ex.what());
            }
        }
    }

    /*
    If we succeeded, delete or rename the original file, and rename the temporary file to the original location.
    If we failed, delete the temporary file.
    Do this itself in separate phases, so that if the delete/rename fails, we still delete the temporary file.
    */
    // 1. Remove target. (It might not exist yet.)
    if (success && target.exists()) {
    
        if (keepOld) {
        
            try {
                target.renameToNode(keptFileNode);
            } catch (const VException& ex) {
                success = false;
                errorMessage = VSTRING_FORMAT("Failed renaming '%s' to '%s': %s", target.getPath().chars(), keptFileNode.getPath().chars(), ex.what());
            }

        } else {

            if (! target.rm()) {
                success = false;
                errorMessage = VSTRING_FORMAT("Unable to remove target file '%s'.", target.getPath().chars());
            }

        }
    
    }

    // 2. Rename temporary to (original) target.
    if (success) {
        try {
            temporaryFileNode.renameToNode(originalTargetNode);
        } catch (const VException& ex) {
            success = false;
            errorMessage = VSTRING_FORMAT("Failed renaming '%s' to '%s': %s", temporaryFileNode.getPath().chars(), originalTargetNode.getPath().chars(), ex.what());
        }
    }

    // 3. Remove temporary if unsuccessful.
    if (! success) {
        if (! temporaryFileNode.rm()) {
            errorMessage += VSTRING_FORMAT(" Removal of temporary file '%s' failed.", temporaryFileNode.getPath().chars());
        }
    }

    // If we failed, throw an exception with the error message we built wherever we encountered errors.
    if (! success) {
        throw VException(errorMessage);
    }
}

// static
void VFSNode::copyFile(const VFSNode& source, const VFSNode& dest) {
    VBufferedFileStream fs(source);
    fs.openReadOnly();
    VBinaryIOStream in(fs);
    VFSNode::safelyOverwriteFile(dest, source.size(), in);
}

// Helper class used by VFSNode::copyDirectory as callback. Copies the supplied node.
// If the node is a file, it is copied. If the node is a directory and recursion is on, the directory is copied.
class VFSNodeCopyDirectoryCallback : public VDirectoryIterationCallback {
    public:
        VFSNodeCopyDirectoryCallback(const VFSNode& destDir, bool recursive) : mDestDir(destDir), mRecursive(recursive) {}
        virtual ~VFSNodeCopyDirectoryCallback() {}
        virtual bool handleNextNode(const VFSNode& node);
    private:
        VFSNode mDestDir;
        bool mRecursive;
};

bool VFSNodeCopyDirectoryCallback::handleNextNode(const VFSNode& source) {
    VFSNode dest(mDestDir, source.getName());
    if (source.isFile()) {
        VFSNode::copyFile(source, dest);
    } else if (mRecursive) {
        VFSNode::copyDirectory(source, dest, mRecursive);
    }
    
    return true;
}

// static
void VFSNode::copyDirectory(const VFSNode& source, const VFSNode& dest, bool recursive) {
    if (recursive) {
        VString sourcePathWithTrailingSeparator = source.getPath() + (source.getPath().endsWith(PATH_SEPARATOR_CHAR) ? "" : PATH_SEPARATOR_CHARS);
        if (dest.getPath().startsWith(sourcePathWithTrailingSeparator)) {
            throw VException(VSTRING_FORMAT("Attempt to recursively copy '%s' into '%s'.", source.getPath().chars(), dest.getPath().chars()));
        }
    }

    if (!dest.exists()) {
        dest.mkdirs();
    }
    
    VFSNodeCopyDirectoryCallback copyDirectoryCallback(dest, recursive);
    source.iterate(copyDirectoryCallback);
}

VFSNode::VFSNode()
    : mPath()
    {
}

VFSNode::VFSNode(const VFSNode& node)
    : mPath(node.mPath)
    {
}

VFSNode::VFSNode(const VString& path)
    : mPath(path)
    {

    if (path.isEmpty()) {
        mPath = ".";
    }
}

VFSNode::VFSNode(const VFSNode& directory, const VString& childName)
    : mPath()
    {
    directory.getChildNode(childName, *this);
}

VFSNode& VFSNode::operator=(const VFSNode& other) {
    mPath = other.mPath;
    return *this;
}

void VFSNode::setPath(const VString& path) {
    if (path.isEmpty())
        mPath = ".";
    else
        mPath = path;
}

void VFSNode::getPath(VString& path) const {
    path = mPath;
}

const VString& VFSNode::getPath() const {
    return mPath;
}

static int _lastNonTrailingIndexOfPathSeparator(const VString& s, int& lengthWithoutTrailingSeparator) {
    bool hasTrailingPathSeparator = s.endsWith(VFSNode::PATH_SEPARATOR_CHAR);
    if (!hasTrailingPathSeparator) {
        lengthWithoutTrailingSeparator = s.length();
        return s.lastIndexOf(VFSNode::PATH_SEPARATOR_CHAR);
    }

    VString stripped;
    s.getSubstring(stripped, s.begin(), s.end() - 1);
    lengthWithoutTrailingSeparator = stripped.length();
    return stripped.lastIndexOf(VFSNode::PATH_SEPARATOR_CHAR);
}

void VFSNode::getName(VString& name) const {
    int lengthWithoutTrailingSeparator;
    int lastSeparatorIndex = _lastNonTrailingIndexOfPathSeparator(mPath, lengthWithoutTrailingSeparator);
    // The following works even if lastIndexOf returns -1 "not found",
    // because we add 1 to get the correct startIndex parameter.
    name.copyFromBuffer(mPath.chars(), lastSeparatorIndex + 1, lengthWithoutTrailingSeparator);
    return;
}

VString VFSNode::getName() const {
    VString name;
    this->getName(name);
    return name;
}

void VFSNode::setName(const VString& name) {
    VFSNode parentNode;
    this->getParentNode(parentNode);

    VString newPath = parentNode.getChildPath(name);
    this->setPath(newPath);
}

void VFSNode::getParentPath(VString& parentPath) const {
    int lengthWithoutTrailingSeparator;
    int lastSeparatorIndex = _lastNonTrailingIndexOfPathSeparator(mPath, lengthWithoutTrailingSeparator);
    parentPath.copyFromBuffer(mPath.chars(), 0, lastSeparatorIndex);
    return;
}

VString VFSNode::getParentPath() const {
    VString parentPath;
    this->getParentPath(parentPath);
    return parentPath;
}

void VFSNode::getParentNode(VFSNode& parent) const {
    VString parentPath;

    this->getParentPath(parentPath);
    parent.setPath(parentPath);
}

void VFSNode::getChildPath(const VString& childName, VString& childPath) const {
    // TODO: Should we throw an exception if childName is empty? It would generate a nonsensical childPath.
    childPath.format("%s%s%s", mPath.chars(),
        (mPath.endsWith(PATH_SEPARATOR_CHAR) ? "" : PATH_SEPARATOR_CHARS), // don't add another slash if already trailing
        childName.chars());
}

VString VFSNode::getChildPath(const VString& childName) const {
    VString childPath;
    this->getChildPath(childName, childPath);
    return childPath;
}

void VFSNode::getChildNode(const VString& childName, VFSNode& child) const {
    VString childPath;

    this->getChildPath(childName, childPath);
    child.setPath(childPath);
}

void VFSNode::mkdirs() const {
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

void VFSNode::mkdir() const {
    this->_platform_createDirectory();
}

bool VFSNode::rm() const {
    /*
    This could be optimized for Mac APIs which do a fast delete of
    the directory and its contents in one swipe. The following way
    is required on Unix file systems and is slower because we must
    delete a directory's contents before deleting it.
    */
    if (! this->exists()) {
        return false;
    }

    bool success = true;
    bool isDir = this->isDirectory();

    if (isDir) {
        success = this->rmDirContents();
    }

    if (success) {
        if (isDir) {
            success = this->_platform_removeDirectory();
        } else {
            success = this->_platform_removeFile();
        }
    }

    return success;
}

bool VFSNode::rmDirContents() const {
    bool            allSucceeded = true;
    VFSNodeVector   children;

    this->list(children);

    for (VSizeType i = 0; i < children.size(); ++i) {
        allSucceeded = allSucceeded && children[i].rm();
    }

    return allSucceeded;
}

void VFSNode::renameToPath(const VString& newPath) const {
    this->_platform_renameNode(newPath);
}

void VFSNode::renameToName(const VString& newName) const {
    VFSNode destinationNode; // not used
    this->renameToName(newName, destinationNode);
}

void VFSNode::renameToName(const VString& newName, VFSNode& nodeToUpdate) const {
    VFSNode parentNode;
    this->getParentNode(parentNode);

    VString newPath;
    parentNode.getChildPath(newName, newPath);

    this->_platform_renameNode(newPath);

    nodeToUpdate.setPath(newPath);    // it IS allowed for nodeToUpdate to be this
}

void VFSNode::renameToNode(const VFSNode& newNode) const {
    VString newPath;
    newNode.getPath(newPath);

    this->_platform_renameNode(newPath);
}

void VFSNode::list(VStringVector& children) const {
    VFSNodeNameCallback callback(children);
    this->_platform_directoryIterate(callback);
}

void VFSNode::list(VFSNodeVector& children) const {
    VFSNodeListCallback callback(children);
    this->_platform_directoryIterate(callback);
}

void VFSNode::iterate(VDirectoryIterationCallback& callback) const {
    this->_platform_directoryIterate(callback);
}

bool VFSNode::find(const VString& name, VFSNode& node) const {
    VFSNodeFindCallback callback(name);
    this->_platform_directoryIterate(callback);

    bool found = callback.found();
    if (found) {
        callback.getMatchedNode(node);
    }

    return found;
}

void VFSNode::readAll(VString& s, bool includeLineEndings) {
    VBufferedFileStream fs(*this);
    fs.openReadOnly();
    VTextIOStream in(fs);
    in.readAll(s, includeLineEndings);
}

void VFSNode::readAll(VStringVector& lines) {
    VBufferedFileStream fs(*this);
    fs.openReadOnly();
    VTextIOStream in(fs);
    in.readAll(lines);
}

bool VFSNode::exists() const {
    VFSNodeInfo info;
    return this->_platform_getNodeInfo(info); // only the function result is needed
}

// static
VString VFSNode::readTextFile(const VString& path, bool includeLineEndings) {
    VFSNode node(path);
    VString text;
    node.readAll(text, includeLineEndings);
    return text;
}

// static
void VFSNode::readTextFile(const VString& path, VStringVector& lines) {
    VFSNode node(path);
    node.readAll(lines);
}

VInstant VFSNode::creationDate() const {
    VFSNodeInfo info;
    bool nodeExists = this->_platform_getNodeInfo(info);

    if (!nodeExists)
        throw VException(VSystemError(info.mErrNo), VSTRING_FORMAT("VFSNode::creationDate failed for '%s'.", mPath.chars()));

    return VInstant::instantFromRawValue(info.mCreationDate);
}

VInstant VFSNode::modificationDate() const {
    VFSNodeInfo info;
    bool nodeExists = this->_platform_getNodeInfo(info);

    if (!nodeExists)
        throw VException(VSystemError(info.mErrNo), VSTRING_FORMAT("VFSNode::modificationDate failed for '%s'.", mPath.chars()));

    return VInstant::instantFromRawValue(info.mModificationDate);
}

VFSize VFSNode::size() const {
    VFSNodeInfo info;
    bool nodeExists = this->_platform_getNodeInfo(info);

    if (!nodeExists)
        throw VException(VSystemError(info.mErrNo), VSTRING_FORMAT("VFSNode::size failed for '%s'.", mPath.chars()));

    return info.mFileSize;
}

bool VFSNode::isFile() const {
    VFSNodeInfo info;
    bool nodeExists = this->_platform_getNodeInfo(info);

    return nodeExists && info.mIsFile;
}

bool VFSNode::isDirectory() const {
    VFSNodeInfo info;
    bool nodeExists = this->_platform_getNodeInfo(info);

    return nodeExists && info.mIsDirectory;
}

// VFSNodeInfo ---------------------------------------------------------------

VFSNodeInfo::VFSNodeInfo()
    : mCreationDate(0)
    , mModificationDate(0)
    , mFileSize(0)
    , mIsFile(false)
    , mIsDirectory(false)
    , mErrNo(0)
    {
}

