/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vfsnode.h"

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
void VFSNode::_platform_normalizePath(VString& path)
    {
    // For the moment, we get almost all the functionality we need by
    // simply converting backslash to slash if we are compiled for Windows.
    // Mac OS X support is free since it's Unix. Mac OS 9 support would
    // have to deal with ':'. DOS drive letters and Mac OS 9 root/relative
    // paths would complicate things a little for things like getParentPath.
    path.replace("\\", "/");
    }

// static
void VFSNode::_platform_denormalizePath(VString& path)
    {
    // See comments above.
    path.replace("/", "\\");
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
        info.mIsFile = ((GetFileAttributes(mPath) & FILE_ATTRIBUTE_DIRECTORY) == 0);
        info.mIsDirectory = ((GetFileAttributes(mPath) & FILE_ATTRIBUTE_DIRECTORY) != 0);
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

// This is the Windows implementation of directory iteration using
// FindFirstFile(), FindNextFile(), FindClose() functions.

void VFSNode::_platform_getDirectoryList(VStringVector& childNames, VFSNodeVector& childNodes, bool useNames, bool useNodes) const
    {
    // FIXME: This code has not been made UNICODE/DBCS compatible.
    // The problem areas are in the strings, and for now we just
    // brute-force cast so that the compiler is happy.
    
    VString childPath;
    VString nodeName;
    VFSNode childNode;
    VString searchPath("%s/*", mPath.chars());
    
    VFSNode::denormalizePath(searchPath);    // make it have DOS syntax

    //Initialize data structs
    childNames.clear();
    childNodes.clear();

    WIN32_FIND_DATA data;
//  HANDLE dir = ::FindFirstFile((const unsigned short*) searchPath.chars(), &data); // FIXME: error in VC++7 (OK for CW?)
// Actually the cast here depends on the flags for wchar support?
    HANDLE dir = ::FindFirstFile((LPCTSTR) searchPath.chars(), &data);

    if (dir == INVALID_HANDLE_VALUE)
        {
        DWORD error = ::GetLastError();
        
        if (error == ERROR_NO_MORE_FILES)
            return;
            
        throw VException("VFSNode::_platform_getDirectoryList failed (error %d) for directory '%s'.", error , searchPath.chars());
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

