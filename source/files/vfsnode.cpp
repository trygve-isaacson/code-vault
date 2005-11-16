/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vfsnode.h"

#include "vexception.h"
#include "vinstant.h"

#ifndef VPLATFORM_WIN

#include <dirent.h>

// static
void VFSNode::normalizePath(VString& /*path*/) {}

// static
void VFSNode::denormalizePath(VString& /*path*/) {}

#else  /* VPLATFORM_WIN */

// static
void VFSNode::normalizePath(VString& path)
    {
    // For the moment, we get almost all the functionality we need by
    // simply converting backslash to slash if we are compiled for Windows.
    // Mac OS X support is free since it's Unix. Mac OS 9 support would
    // have to deal with ':'. DOS drive letters and Mac OS 9 root/relative
    // paths would complicate things a little for things like getParentPath.
    path.replace("\\", "/");
    }

// static
void VFSNode::denormalizePath(VString& path)
    {
    // See comments above.
    path.replace("/", "\\");
    }

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

#endif /* VPLATFORM_WIN */

VFSNode::VFSNode()
    {
    }

VFSNode::VFSNode(const VString& inPath)
    {
    mPath = inPath;
    }

void VFSNode::setPath(const VString& inPath)
    {
    mPath = inPath;
    }

void VFSNode::getPath(VString& outPath) const
    {
    outPath = mPath;
    }

const VString& VFSNode::path() const
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
    
    if (! parentNode.path().isEmpty())    // root or parent of supplied path must be assumed to exist
        parentNode.mkdirs();
    
    // Create this directory.
    this->mkdir();
    }

void VFSNode::mkdir() const
    {
    int result = VFSNode::threadsafe_mkdir(mPath, (S_IFDIR | S_IRWXO | S_IRWXG | S_IRWXU));

    /*
    If two threads are competing to create the same directory, even if they
    both check for its existence, they might end up trying to create it at
    the same time -- one of them will get result -1 with errno == EEXIST.
    The best thing for our interface is to have mkdir succeed if the directory
    already exists.
    */
    
    if ((result == -1) && (errno == EEXIST) && this->isDirectory())
        result = 0;

    if (result != 0)
        throw VException(result, "VFSNode::mkdir failed (error %d: %s) for '%s'.", errno, ::strerror(errno), mPath.chars());
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
        int result;
        
        if (isDir)
            result = VFSNode::threadsafe_rmdir(mPath);
        else
            result = VFSNode::threadsafe_unlink(mPath);

        success = (result == 0);
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

bool VFSNode::exists() const
    {
    struct stat    statData;
    
    return this->stat(statData);
    }

bool VFSNode::isDirectory() const
    {
    struct stat    statData;

    bool doesExist = this->stat(statData);

#ifndef VPLATFORM_WIN
    return (doesExist
        && ((S_ISDIR(statData.st_mode)) || (S_ISLNK(statData.st_mode))));
#else
    return (doesExist
        && ((GetFileAttributes(mPath) & FILE_ATTRIBUTE_DIRECTORY) != 0));
#endif
    }

bool VFSNode::isFile() const
    {
    struct stat    statData;

    bool doesExist = this->stat(statData);

#ifndef VPLATFORM_WIN
    return (doesExist
        && (! S_ISDIR(statData.st_mode))
        && (! S_ISLNK(statData.st_mode)));
#else
    return (doesExist
        && ((GetFileAttributes(mPath) & FILE_ATTRIBUTE_DIRECTORY) == 0));
#endif
    }

void VFSNode::renameToPath(const VString& newPath) const
    {
    int    result = VFSNode::threadsafe_rename(mPath, newPath);
    
    if (result != 0)
        throw VException(result, "VFSNode::rename failed (error %d: %s) renaming '%s' to '%s'.", errno, ::strerror(errno), mPath.chars(), newPath.chars());
    }

void VFSNode::renameToName(const VString& newName) const
    {
    VFSNode    parentNode;
    this->getParentNode(parentNode);
    
    VString    newPath;
    parentNode.getChildPath(newName, newPath);
    
    this->renameToPath(newPath);
    }

void VFSNode::renameToName(const VString& newName, VFSNode& nodeToUpdate) const
    {
    VFSNode    parentNode;
    this->getParentNode(parentNode);
    
    VString    newPath;
    parentNode.getChildPath(newName, newPath);
    
    this->renameToPath(newPath);
    nodeToUpdate.setPath(newPath);    // it IS allowed for nodeToUpdate to be this
    }

void VFSNode::renameToNode(const VFSNode& newNode) const
    {
    VString    newPath;

    newNode.getPath(newPath);
    
    this->renameToPath(newPath);
    }

bool VFSNode::stat(struct stat& statData) const
    {
    int result = VFSNode::threadsafe_stat(mPath, &statData);
    return (result >= 0);
    }

void VFSNode::list(VStringVector& children) const
    {
    VFSNodeVector    dummy;
    
    // Call the implementation with parameters to indicate we are using names, not nodes.
    this->listImplementation(children, dummy, true, false);
    }

void VFSNode::list(VFSNodeVector& children) const
    {
    VStringVector    dummy;
    
    // Call the implementation with parameters to indicate we are using nodes, not names.
    this->listImplementation(dummy, children, false, true);
    }

VFSize VFSNode::size() const
    {
    struct stat    statData;
    
    int result = VFSNode::threadsafe_stat(mPath, &statData);

    if (result >= 0)
        {
        return statData.st_size;
        }
    else
        {
        throw VException(result, "VFSNode::size failed (error %d: %s) for '%s'.", errno, ::strerror(errno), mPath.chars());
        }
    }

VInstant VFSNode::modificationDate() const
    {
    struct stat    statData;
    
    int result = VFSNode::threadsafe_stat(mPath, &statData);

    if (result >= 0)
        {
        return VInstant(statData.st_mtime);
        }
    else
        {
        throw VException(result, "VFSNode::modificationDate failed (error %d: %s) for '%s'.", errno, ::strerror(errno), mPath.chars());
        }
    }

VInstant VFSNode::creationDate() const
    {
    struct stat    statData;
    
    int result = VFSNode::threadsafe_stat(mPath, &statData);

    if (result >= 0)
        {
        return VInstant(statData.st_ctime);
        }
    else
        {
        throw VException(result, "VFSNode::creationDate failed (error %d: %s) for '%s'.", errno, ::strerror(errno), mPath.chars());
        }
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
int VFSNode::threadsafe_mkdir(const char* inPath, mode_t mode)
    {
    int        result;
    bool    done = false;
    
    while (! done)
        {
#ifdef VPLATFORM_WIN
        mode = 0;    // avoid compiler warning for unused argument
        result = ::mkdir(inPath);
#else
        result = ::mkdir(inPath, mode);
#endif
        
        if ((result == 0) || (errno != EINTR))
            done = true;
        }
    
    _debugCheck(result == 0);

    return result;
    }

// static
int VFSNode::threadsafe_rename(const char* oldName, const char* newName)
    {
    int        result;
    bool    done = false;

    while (! done)
        {
        result = ::rename(oldName, newName);
        
        if ((result == 0) || (errno != EINTR))
            done = true;
        }
    
    _debugCheck(result == 0);

    return result;
    }

// static
int VFSNode::threadsafe_stat(const char* inPath, struct stat* buf)
    {
    int        result;
    bool    done = false;

    while (! done)
        {
        result = ::stat(inPath, buf);
        
        if ((result == 0) || (errno != EINTR))
            done = true;
        }
    
    _debugCheck(result == 0);

    return result;
    }

// static
int VFSNode::threadsafe_unlink(const char* inPath)
    {
    int        result;
    bool    done = false;

    while (! done)
        {
        result = ::unlink(inPath);
        
        if ((result == 0) || (errno != EINTR))
            done = true;
        }
    
    _debugCheck(result == 0);

    return result;
    }

// static
int VFSNode::threadsafe_rmdir(const char* inPath)
    {
    int        result;
    bool    done = false;

    while (! done)
        {
        result = ::rmdir(inPath);
        
        if ((result == 0) || (errno != EINTR))
            done = true;
        }
    
    _debugCheck(result == 0);

    return result;
    }

#ifndef VPLATFORM_WIN

// This is the Unix implementation using opendir(), readdir(), closedir() functions.

void VFSNode::listImplementation(VStringVector& childNames, VFSNodeVector& childNodes, bool useNames, bool useNodes) const
    {
    VString    childPath;
    VString    nodeName;
    VFSNode childNode;

    DIR*    dir = ::opendir(mPath);
    
    if (dir == NULL)
        throw VException("VFSNode::list failed for '%s'.", mPath.chars());

    try
        {
        struct dirent*    entry = ::readdir(dir);
        
        while (entry != NULL)
            {
            nodeName = entry->d_name;
            
            // Skip current and parent pseudo-entries. Otherwise client must
            // know too much detail in order to avoid traversal problems.
            if ((nodeName != ".") &&
                (nodeName != ".."))
                {
                if (useNames)
                    {
                    childNames.push_back(nodeName);
                    }

                if (useNodes)
                    {
                    this->getChildPath(nodeName, childPath);
                    childNode.setPath(childPath);
                    childNodes.push_back(childNode);
                    }

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

#else /* VPLATFORM_WIN */

// This is the Windows implementation using FindFirstFile(), FindNextFile(), FindClose() functions.

void VFSNode::listImplementation(VStringVector& childNames, VFSNodeVector& childNodes, bool useNames, bool useNodes) const
    {
    // FIXME: This code has not been made UNICODE/DBCS compatible.
    // The problem areas are in the strings, and for now we just
    // brute-force cast so that the compiler is happy.
    
    //SN 10/8/04 : CR 22227 & CR 22266 - moved childpath at begining of function to avoid crash
    VString    childPath;
    //SN 10/11/04 : CR 22227 & CR 22266 - moved nodeName at begining of function to avoid crash
    VString    nodeName;
    VFSNode childNode;
    VString    searchPath("%s/*", mPath.chars());
    
    VFSNode::denormalizePath(searchPath);    // make it have DOS syntax

    //Initialize data structs
    childNames.clear();
    childNodes.clear();

    WIN32_FIND_DATA    data;
//    HANDLE            dir = ::FindFirstFile((const unsigned short*) searchPath.chars(), &data); // FIXME: error in VC++7 (OK for CW?)
// Actually the cast here depends on the flags for wchar support?
    HANDLE            dir = ::FindFirstFile((LPCTSTR) searchPath.chars(), &data);

    if (dir == INVALID_HANDLE_VALUE)
        {
        DWORD    error = ::GetLastError();
        
        if (error == ERROR_NO_MORE_FILES)
            return;
            
        throw VException("VFSNode::list failed (error %d) for '%s'.", error , searchPath.chars());
        }
    
    try
        {
        do
            {
            nodeName = (char*) data.cFileName;

            // Skip current and parent pseudo-entries. Otherwise client must
            // know too much detail in order to avoid traversal problems.
            if ((nodeName != ".") &&
                (nodeName != ".."))
                {
                if (useNames)
                    {
                    childNames.push_back(nodeName);
                    }

                if (useNodes)
                    {
                    this->getChildPath(nodeName, childPath);
                    childNode.setPath(childPath);
                    childNodes.push_back(childNode);
                    }
                }
            
            } while (::FindNextFile(dir, &data));
        }
    catch (...)
        {
        ::FindClose(dir);
        throw;
        }

    ::FindClose(dir);
    }

#endif /* VPLATFORM_WIN */
