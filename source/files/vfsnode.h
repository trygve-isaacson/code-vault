/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

#ifndef vfsnode_h
#define vfsnode_h

/** @file */

#include "vstring.h"

/**

    @defgroup vfilesystem Vault File System Access

    <h3>Overview</h3>
    
    The Vault uses the term "node" to mean either a file or directory
    within the file system. The class VFSNode is what you use to
    specify or identify a particular node in the file system. Operations
    that are carried out on nodes without requiring i/o on file contents,
    are defined as methods of VFSNode; you invoke methods on the VFSNode
    object whose path represents the file or directory you want to act on.
    
    A VFSNode can represent a file or directory that does not currently
    exist. Naturally, that's how you would create a new directory: you
    would create a VFSNode to point to the path of the directory you
    want to create, and then you would call its mkdir() method. The
    mkdirs() method will ensure that any non-existent intermediate
    directories are created along the way, if you are creating a deep
    directory whose ancestory directory hierarchy does not yet exist.
    You can remove a directory or file by calling its rm() method.
    
    Similarly, to create a new file, you'd make a VFSNode for the location
    where you want to create it, and then you'd use a VBufferedFileStream
    to create the file and write to it.
    
    You can test the existence of a file or directory by calling exists().
    If you need to know the difference between a file and a directory,
    you can call isDirectory() and isFile(), both of which return false
    if the node is of the other type or simply does not exist at all.
    
    You can also use VFSNode to traverse the directory hierarchy, walking
    down to subdirectories and files by calling getChildNode() or
    getChildPath(), and walking up to a parent directory by calling
    getParentNode() or getParentPath(). In each case you are invoking the
    method on a VFSNode that represents the node whose child or parent you
    wish to locate.
    
    You can get a directory node's list of files and subdirectories by 
    calling list().
    
    To perform i/o on a file, you will typically pass the VFSNode object
    that represents the file to a VBufferedFileStream object's constructor
    or setNode() method. Then you can call that object's methods to open
    the file stream in read-only, read-write, or write/create mode. From
    there you'll typically use a VBinaryIOStream or VTextIOStream object
    to actually format the file data correct (or to read it correctly).
    
*/
    
/**
    @ingroup vfilesystem
*/

/**
VFSNodeInfo holds file system information about a node, and
is used internally by VFSNode and its platform-specific helper
functions. We define the time fields using a raw Vs64 value
(millisecond offset from 1970 UTC) rather than a VInstant
object so that there is zero overhead in constructing one of
these.
*/
class VFSNodeInfo
    {
    public:
    
        VFSNodeInfo();
        ~VFSNodeInfo() {}

        Vs64    mCreationDate;      // a VInstant offset value
        Vs64    mModificationDate;  // a VInstant offset value
        VFSize  mFileSize;
        bool    mIsFile;
        bool    mIsDirectory;
        int     mErrNo; // the value of errno if call failed, 0 otherwise
    };


class VInstant;
class VFSNode;

class VDirectoryIterationCallback
    {
    public:
    
        VDirectoryIterationCallback() {}
        virtual ~VDirectoryIterationCallback() {}
        
        /**
        A callback implementation needs to implement this method; it should
        return false if no further iteration is desired, true if iteration
        should continue.
        */
        virtual bool handleNextNode(const VFSNode& node) = 0;
    };

/**
VFSNodeVector is simply a vector of VFSNode objects. Note that the vector
elements are objects, not pointers to objects.
*/
typedef std::vector<VFSNode> VFSNodeVector;

/**
A VFSNode represents a file or directory in the file system, whether it
actually exists or not, and provides some methods for operating on it.
*/
class VFSNode
    {
    public:
    
        /**
        Takes a platform-specific directory path and modifies it to be
        int the normalized form necessary for use with VFSNode. If you
        are given a path from the user or OS that is in the OS path
        format (for example, a DOS path), you need to normalize it
        before supplying it to VFSNode. Note that this facility does
        not allow DOS style "drive letters" but is designed to handle
        relative DOS paths.
        @param    path    a string containing a platform-specific path,
                        which will be modified in place into normalized form
        */
        static void normalizePath(VString& path);
        /**
        The reverse of normalizePath -- takes a normalized path and undoes
        the normalization, turning it into a platform-specific path.
        */
        static void denormalizePath(VString& path);
    
        /**
        Constructs an undefined VFSNode object (you will have to set its path
        with a subsequent call to setPath()).
        */
        VFSNode();
        /**
        Constructs a VFSNode with a path.
        @param    path    the path of the file or directory
        */
        VFSNode(const VString& path);
        /**
        Destructor.
        */
        virtual ~VFSNode() {}

        /**
        Assignment operator.
        @param  other   the node to copy
        */
        VFSNode& operator=(const VFSNode& other);
        
        /**
        Specifies the path of the node.
        @param    path    the path of the file or directory
        */
        void setPath(const VString& path);
        /**
        Gets the path of the node.
        @param    path    the string to set
        */
        void getPath(VString& path) const;
        /**
        Returns a reference to the node's path.
        @return    the path
        */
        const VString& getPath() const;
        /**
        Returns the node's name, without any directory path information.
        That is, the file or directory name, only.
        @param    name    the string to set to the file or node name
        */
        void getName(VString& name) const;
        
        /**
        Gets the path of the node's parent.
        @param    parentPath    the string to set
        */
        void getParentPath(VString& parentPath) const;
        /**
        Gets a VFSNode of the node's parent.
        @param    parent    the object to set
        */
        void getParentNode(VFSNode& parent) const;
        /**
        Gets the path of a child of the node (the node must be a directory).
        @param    childName    the name of the child file or directory
        @param    childPath    the string to set
        */
        void getChildPath(const VString& childName, VString& childPath) const;
        /**
        Gets the node of a child of the node (the node must be a directory).
        @param    childName    the name of the child file or directory
        @param    child        the node to set
        */
        void getChildNode(const VString& childName, VFSNode& child) const;
        
        /**
        Creates the directory the node represents, and all non-existent
        directories above it.
        */
        void mkdirs() const;
        /**
        Creates the directory the node represents.
        */
        void mkdir() const;
        /**
        Deletes the node; if it is a directory its contents are deleted first.
        @return    true if the deletion was successful; false if the node itself,
                or any contained nodes could not be deleted
        */
        bool rm() const;
        /**
        Deletes the contents of the directory node (the node must be a
        directory).
        @return    true if the deletion was successful; false if one or more
                contained nodes could not be deleted
        */
        bool rmDirContents() const;
        /**
        Returns true if the node (file or directory) currently exists.
        @return true if the node exists, false if not
        */
        bool exists() const;
        /**
        Returns the node's creation date.
        @return    the node's creation date
        */
        VInstant creationDate() const;
        /**
        Returns the node's modification date.
        @return    the node's modification date
        */
        VInstant modificationDate() const;
        /**
        Returns the file node's size (must be a file node). Throws a
        VException if the node is not a file or does not exist.
        @return    the file size
        */
        VFSize size() const;
        /**
        Returns true if the node is a file.
        @return true if the node is a file, false if not
        */
        bool isFile() const;
        /**
        Returns true if the node is a directory.
        @return true if the node is a directory, false if not
        */
        bool isDirectory() const;

        /**
        Renames the node by specifying its new path; this could include
        changing its directory location. This function does NOT update
        this VFSNode's path property (it continues to describe the
        node at the old path, even though it is not present if the
        rename succeeded).
        @param    newPath    the new path to rename to
        */
        void renameToPath(const VString& newPath) const;
        /**
        Renames the node by specifying its new name; this means just
        changing the leaf file or dir name, without changing its containing
        path. That is, this function does not move the file or dir to
        another directory. This function does NOT update
        this VFSNode's path property (it continues to describe the
        node at the old path, even though it is not present if the
        rename succeeded).
        @param    newName    the new name to rename to
        */
        void renameToName(const VString& newName) const;
        /**
        Renames the node by specifying its new name; this means just
        changing the leaf file or dir name, without changing its containing
        path. That is, this function does not move the file or dir to
        another directory. This function does NOT update
        this VFSNode's path property (it continues to describe the
        node at the old path, even though it is not present if the
        rename succeeded). However, it updates the supplied nodeToUpdate,
        which could be this node if you pass *this for the parameter.
        @param    newName            the new name to rename to
        @param    nodeToUpdate    a VFSNode whose path to rename
        */
        void renameToName(const VString& newName, VFSNode& nodeToUpdate) const;
        /**
        Renames the node by specifying a node whose path to use. This
        function does NOT update this VFSNode's path property (it continues
        to describe the node at the old path, even though it is not present
        if the rename succeeded).
        @param    newNode    a node whose path is the path to rename the node to
        */
        void renameToNode(const VFSNode& newNode) const;
        
        /**
        Returns a vector of strings containing the names of the node's
        children (the node must be a directory).
        @param    children    the vector of strings to be filled in
        */
        void list(VStringVector& children) const;
        /**
        Returns a vector of nodes, one for each of the node's
        children (the node must be a directory).
        @param    children    the vector of node objects to be filled in
        */
        void list(VFSNodeVector& children) const;
        /**
        Iterates over the directory's nodes, calling the supplied callback
        object for each one (the "." and ".." are omitted). The callback
        can process each node as needed, and can halt the iteration at any
        time.
        @param    callback    the object that will be called for each node
                                iterated over
        */
        void iterate(VDirectoryIterationCallback& callback) const;
        /**
        Iterates over the directory until it finds the specified child node using
        a case-insensitive match on the node names. This is useful if you need
        to open a file but don't know what case it is in due to cross-platform
        naming issues. If you know the case, you should just open the file rather
        than trying to find it first before opening.
        @param name the name to match (matching is not case-sensitive)
        @param node the node to be returned indicating the actual name in its
            particular case; node is unmodified if there was no match
        @return true if a match was found and node contains the match
        */
        bool find(const VString& name, VFSNode& node) const;
        
    private:
    
        // These are the key functions that typically peculiar to each platform,
        // and are implemented in the platform-specific vfsnode_platform.cpp
        // files.

        /**
        Converts a path string from the platform native form into "normalized"
        form. The normalized form uses forward slash '/' to separate segments
        in the path. On Unix, there's nothing to do because it is the normalized
        form, but on Windows, we must convert backslash to slash, etc.
        @param path the path string to be modified
        */
        static void _platform_normalizePath(VString& path);
        /**
        Converts a path string from "normalized" form into the platform native
        form.
        @param path the path string to be modified
        */
        static void _platform_denormalizePath(VString& path);
        /**
        Fills out a VFSNodeInfo structure to describe this node as it actually
        exists in the file system. If the node does not exist or if the attempt
        to obtain its state fails, the result is false, info.mErrNo may contain
        a system error number, and all other fields are unmodified.
        @param info the structure to fill out
        @return false if the call failed or the node does not exist, true if
                the call succeeded and the node exists
        */
        bool _platform_getNodeInfo(VFSNodeInfo& info) const;
        /**
        Creates a directory at this node location.
        @throw VException if the operation fails
        */
        void _platform_createDirectory() const;
        /**
        Removes the directory at this node location.
        @return false if unable to remove the directory (this may include the
                case where the directory does not exist, which is a benign
                error and is why we return a boolean you can ignore rather
                than throwing an exception)
        */
        bool _platform_removeDirectory() const;
        /**
        Removes the file at this node location.
        @return false if unable to remove the file (this may include the
                case where the file does not exist, which is a benign
                error and is why we return a boolean you can ignore rather
                than throwing an exception)
        */
        bool _platform_removeFile() const;
        /**
        Renames the file or directory at this node location.
        @param newName the new name of the node (just the name, does not
                include the path or parent directory's information)
        */
        void _platform_renameNode(const VString& newName) const;

        /**
        Iterates over this directory, calling the supplied callback
        object for each node found. If the nodes "." or ".." are
        present in the platform's iteration function, they are
        omitted (not passed to the callback).
        @param callback the callback object to notify of each node we
            iterate over
        */
        void _platform_directoryIterate(VDirectoryIterationCallback& callback) const;
    
        // These static functions wrap the raw POSIX functions in order to
        // make them work correctly even if a signal is caught inside the
        // function. These functions can be called from the platform-specific
        // implementation if appropriate.
        static int _wrap_mkdir(const char* path, mode_t mode);             ///< Calls POSIX mkdir in a way that is safe even if a signal is caught inside the function.
        static int _wrap_rename(const char* oldName, const char* newName); ///< Calls POSIX rename in a way that is safe even if a signal is caught inside the function.
        static int _wrap_stat(const char* path, struct stat* statData);    ///< Calls POSIX stat in a way that is safe even if a signal is caught inside the function.
        static int _wrap_unlink(const char* path);                         ///< Calls POSIX unlink in a way that is safe even if a signal is caught inside the function.
        static int _wrap_rmdir(const char* path);                          ///< Calls POSIX rmdir in a way that is safe even if a signal is caught inside the function.
        
        VString    mPath;    ///< The node's path.

    };

#endif /* vfsnode_h */

