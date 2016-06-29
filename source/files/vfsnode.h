/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
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
class VFSNodeInfo {
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
class VBinaryIOStream;

class VDirectoryIterationCallback {
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
class VFSNode {
    public:

        /**
        Takes a platform-specific directory path and returns
        the normalized form necessary for use with VFSNode. If you
        are given a path from the user or OS that is in the OS path
        format (for example, a DOS path with backslashes), you need to normalize it
        (with slashes as path separators) before supplying it to VFSNode.
        @param    path    a string containing a platform-specific path,
                        which will be modified in place into normalized form
        @return the normalized form of the path
        */
        static VString normalizePath(const VString& path);
        /**
        The reverse of normalizePath -- takes a normalized path and undoes
        the normalization, turning it into a platform-specific path.
        @param    path    a string containing a normalized path,
                        which will be modified in place into platform-specific form
        @return the denormalized form of the path
        */
        static VString denormalizePath(const VString& path);
        /**
        This constant is used internally when constructing or parsing path strings.
        It is the internal (normalized) path separator. It's a char type for speed.
        There is a version typed as a char* for APIs that work better with that.
        */
        static const char PATH_SEPARATOR_CHAR;
        static const char* PATH_SEPARATOR_CHARS;

        /**
        These values identify different known folders whose location you can
        access by calling VFSNode::getKnownDirectoryNode(). These are useful
        as default locations to store or find data in a location that is
        appropriate to the platform.
        */
        typedef enum {
            USER_HOME_DIRECTORY,        ///< The user's home directory.
            LOG_FILES_DIRECTORY,        ///< Where to write log files.
            USER_PREFERENCES_DIRECTORY, ///< Where to store user preferences files.
            CACHED_DATA_DIRECTORY,      ///< Where to store non-critical cached data files.
            APPLICATION_DATA_DIRECTORY, ///< Where to find application data files other than user documents.
            CURRENT_WORKING_DIRECTORY,  ///< The application environment's "current working directory" by path (not as simply ".").
            EXECUTABLE_DIRECTORY        ///< The directory where the app executable lives. (See notes below.)
        } KnownDirectoryIdentifier;
        /**
        Returns a node identifying an identified directory, creating it if it does not exist.(*)
        Below are the platform-specific path examples. Note that these are merely examples, because
        the OS may return something else as appropriate when we call the platform-specific APIs to
        locate the suitable directories.
        (* The user home directory is never created, because it is assumed to exist.)

        Unix: (note that "~" denotes the user's home directory, wherever that may be, and does not actually appear in the path)
            Most of these we would prefer to locate under /var, but it is often not writable.
            USER_HOME_DIRECTORY:        ~ is typically /home/(user)
            LOG_FILES_DIRECTORY:        ~/log/(company)/(app)
            USER_PREFERENCES_DIRECTORY: ~/.(company)/(app)
            CACHED_DATA_DIRECTORY:      ~/cache/(company)/(app)
            APPLICATION_DATA_DIRECTORY: ~/data/(company)/(app)
            CURRENT_WORKING_DIRECTORY:  the full path to the current working directory
            EXECUTABLE_DIRECTORY:       the full path to the directory containing the executable

        Mac OS X: (note that "~" denotes the user's home directory, wherever that may be, and does not actually appear in the path)
            USER_HOME_DIRECTORY:        ~ is typically /Users/(user)
            LOG_FILES_DIRECTORY:        ~/Library/Logs/(company)/(app)
            USER_PREFERENCES_DIRECTORY: ~/Library/Preferences/(company)/(app)
            CACHED_DATA_DIRECTORY:      ~/Library/Caches/(company)/(app)
            APPLICATION_DATA_DIRECTORY: ~/Library/(company)/(app)
            CURRENT_WORKING_DIRECTORY:  the full path to the current working directory
            EXECUTABLE_DIRECTORY:       the full path to the directory containing this app bundle or executable

        iOS: (note that "@" here denotes the app install sandbox directory, and does not actually appear in the path)
            On iOS an app gets installed in its own sandbox at: /var/mobile/Applications/(random serial number for this install)
            USER_HOME_DIRECTORY:        @ (the same dir as the app install sandbox; it is thus different when queried by each installed app)
            LOG_FILES_DIRECTORY:        @/Library/Logs/(company)/(app)
            USER_PREFERENCES_DIRECTORY: @/Library/Preferences/(company)/(app)
            CACHED_DATA_DIRECTORY:      @/Library/Caches/(company)/(app)
            APPLICATION_DATA_DIRECTORY: @/Library/(company)/(app)
            CURRENT_WORKING_DIRECTORY:  / (obviously this is not a writeable directory in this environment)
            EXECUTABLE_DIRECTORY:       @

        Windows XP: (note that the OS can return a different Application Data path, including a different
            drive letter, if so configured; these are just the typical case examples)
            USER_HOME_DIRECTORY:        C:/Documents and Settings/(user)
            LOG_FILES_DIRECTORY:        C:/Documents and Settings/(user)/Application Data/(company)/(app)/Logs
            USER_PREFERENCES_DIRECTORY: C:/Documents and Settings/(user)/Application Data/(company)/(app)/Preferences
            CACHED_DATA_DIRECTORY:      C:/Documents and Settings/(user)/Application Data/(company)/(app)/Caches
            APPLICATION_DATA_DIRECTORY: C:/Documents and Settings/(user)/Application Data/(company)/(app)
            CURRENT_WORKING_DIRECTORY:  the full path to the current working directory
            EXECUTABLE_DIRECTORY:       the full path to the directory containing this app's .exe file

        Windows 7: (note that the OS can return a different Application Data path, including a different
            drive letter, if so configured; these are just the typical case examples; use of the "Roaming"
            folder is merely one type of network configuration)
            USER_HOME_DIRECTORY:        C:/Users/(user)/AppData
            LOG_FILES_DIRECTORY:        C:/Users/(user)/AppData/Roaming/(company)/(app)/Logs
            USER_PREFERENCES_DIRECTORY: C:/Users/(user)/AppData/Roaming/(company)/(app)/Preferences
            CACHED_DATA_DIRECTORY:      C:/Users/(user)/AppData/Roaming/(company)/(app)/Caches
            APPLICATION_DATA_DIRECTORY: C:/Documents and Settings/(user)/Application Data/(company)/(app)
            CURRENT_WORKING_DIRECTORY:  the full path to the current working directory
            EXECUTABLE_DIRECTORY:       the full path to the directory containing this app's .exe file

        When asking for the user home directory, the supplied company and app names are not used.

        @param  id          the directory identifier
        @param  companyName if not empty, a first-level subdirectory used in the hierarchy (not used for USER_HOME_DIRECTORY and CURRENT_WORKING_DIRECTORY)
        @param  appName     if not empty, a second-level subdirectory used in the hierarchy (not used for USER_HOME_DIRECTORY and CURRENT_WORKING_DIRECTORY)
        @return the specified directory node
        @throws may throw VException if there is an error attempting to locate the specified directory
        */
        static VFSNode getKnownDirectoryNode(KnownDirectoryIdentifier id, const VString& companyName, const VString& appName);
        /**
        This convenience function returns the current working directory path node; it is equivalent
        to calling getKnownDirectoryNode with CURRENT_WORKING_DIRECTORY.
        @return the current working directory node, defined by a full path rather than simply "."
        @throws may throw VException if there is an error attempting to locate the specified directory
        */
        static VFSNode getCurrentWorkingDirectory();
        /**
        This convenience function returns the directory path node of the directory containing the
        application executable; it is equivalent to calling getKnownDirectoryNode with EXECUTABLE_DIRECTORY.
        @return the directory node in which the executable is located
        @throws may throw VException if there is an error attempting to locate the specified directory
        */
        static VFSNode getExecutableDirectory();
        /**
        This function returns the file node of the application executable.
        @return the file node representing the application executable
        @throws may throw VException if there is an error attempting to locate the file
        */
        static VFSNode getExecutable();

        /**
        This function safely overwrites an existing file using a temporary file, to ensure that the original
        file is intact if the write fails. Specifically, the sequence is:
        1. create a temporary file next to the target file (named uniquely via current timestamp string)
        2. write to the temporary file
        3. delete (or rename if keeping) the target file (it's OK if it doesn't exist)
        4. rename the temporary file to the target file's name
        If steps 1, 2, or 3 fails, the original remains and the temporary is deleted.
        If there is a failure, a VException is thrown.
        The temporary file is initially named "<timestamp>_tmp_<originalfilename>", before being renamed
        to the original file name.
        If keepOld is specified, the original file is not deleted, but rather is renamed to the following
        file name: "<timestamp>_ver_<originalfilename>"
        @param  target      the file node to be overwritten (if it exists)
        @param  dataLength  the length of the data to be written
        @param  dataStream  the stream to be written to the file
        @param  keepOld     if true, the original file is not deleted, but rather renamed to a variant of the temporary file name
        */
        static void safelyOverwriteFile(const VFSNode& target, Vs64 dataLength, VBinaryIOStream& dataStream, bool keepOld=false);

        /**
        Constructs an undefined VFSNode object (you will have to set its path
        with a subsequent call to setPath()).
        */
        VFSNode();
        /**
        Copy constructor.
        */
        VFSNode(const VFSNode& rhs);
        /**
        Constructs a VFSNode with a path.
        @param    path    the path of the file or directory
        */
        VFSNode(const VString& path);
        /**
        Constructs a VFSNode with a parent directory node and a child directory or file name within it.
        @param    directory    the parent directory of the node
        @param    childName    the name of the subdirectory or file within the parent
        */
        VFSNode(const VFSNode& directory, const VString& childName);
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
        Alternate convenience function for getName(). May require an
        extra string copy.
        @return    the node name
        */
        VString getName() const;
        /**
        Specifies the name of the node, without changing the parent path
        nor actually renaming anything on disk (as one of the renameXXX()
        methods would do).
        @param    name    the name of the file or directory
        */
        void setName(const VString& name);
        /**
        Gets the path of the node's parent.
        @param    parentPath    the string to set
        */
        void getParentPath(VString& parentPath) const;
        /**
        Alternate convenience function for getParentPath(). May require an
        extra string copy.
        @return    the parent node's path
        */
        VString getParentPath() const;
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
        Alternate convenience function for getParentPath(). May require an
        extra string copy.
        @param    childName    the name of the child file or directory
        @return   the child node's path
        */
        VString getChildPath(const VString& childName) const;
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
        node at the old path, even though it is no longer present if the
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
        Renames the node by specifying a node whose path to use; this could
        include changing its directory location. This function does NOT update
        this VFSNode's path property (it continues to describe the node at the
        old path, even though it is no longer present if the rename succeeded).
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

        /**
        This convenience function does the following in a single call:
        - Open the file read-only.
        - Create a text input stream.
        - Read the entire file as text into the supplied string.
        - Close the file.
        Note that the default value for includeLineEndings is the opposite
        of VTextIOStream::readLine(), because here you probably want the whole
        file with lines separated, whereas when you read one line you probably
        don't want the end of line characters.
        @param    s                    a VString to append to
        @param    includeLineEndings   true if you want the line ending character(s)
                                        to be included in the string that is returned
        @throws VException if the file cannot be opened or read
        */
        void readAll(VString& s, bool includeLineEndings = true);
        /**
        This convenience function is like the other readAll, but it returns the
        file's contents as a vector of strings rather than a single giant string.
        The lines do not have line ending characters at the end.
        @param  lines   a vector of strings; lines are appended to this vector
        @throws VException if the file cannot be opened or read
        */
        void readAll(VStringVector& lines);
        /**
        The static version of readAll(VString&), for more concise calling code, at the expense
        of a likely string copy to return by value.
        @param  path                the path of the VFSNode from which to read
        @param  includeLineEndings  true if you want the line ending character(s)
                                        to be included in the string that is returned
        @throws VException if the file cannot be opened or read
        */
        static VString readTextFile(const VString& path, bool includeLineEndings = true);
        /**
        The static version of readAll(VStringVector&), for more concise calling code.
        @param  path    the path of the VFSNode from which to read
        @param  lines   a vector of strings; lines are appended to this vector
        @throws VException if the file cannot be opened or read
        */
        static void readTextFile(const VString& path, VStringVector& lines);

        // The < operator is needed for to support STL sort(); others provided for completeness.
        // The purpose here is to allow a sorting a directory listing by name,
        // after calling VFSNode::list(). This is reasonable for many cases, but
        // note that the comparision is ultimately performed by strcmp().
        friend inline bool operator< (const VFSNode& lhs, const VFSNode& rhs);
        friend inline bool operator<=(const VFSNode& lhs, const VFSNode& rhs);
        friend inline bool operator>=(const VFSNode& lhs, const VFSNode& rhs);
        friend inline bool operator> (const VFSNode& lhs, const VFSNode& rhs);

    private:

        // These are the key functions that typically peculiar to each platform,
        // and are implemented in the platform-specific vfsnode_platform.cpp
        // files.

        /**
        Converts a path string from the platform native form into "normalized"
        form. The normalized form uses forward slash to separate segments
        in the path. On Unix, there's nothing to do because it is the normalized
        form, but on Windows, we must convert backslash to slash, etc.
        @param path the path string to be modified
        */
        static VString _platform_normalizePath(const VString& path);
        /**
        Converts a path string from "normalized" form into the platform native
        form.
        @param path the path string to be modified
        */
        static VString _platform_denormalizePath(const VString& path);
        /**
        Returns a node identifying an identified directory, creating it if it does not exist.
        @param  id          the directory identifier
        @param  companyName if not empty, a first-level subdirectory used in the hierarchy
        @param  appName     if not empty, a second-level subdirectory used in the hierarchy
        @throws may throw VException if there is an error attempting to locate the file
        */
        static VFSNode _platform_getKnownDirectoryNode(KnownDirectoryIdentifier id, const VString& companyName, const VString& appName);
        /**
        This function returns the file node of the application executable.
        @return the file node representing the application executable
        @throws may throw VException if there is an error attempting to locate the file
        */
        static VFSNode _platform_getExecutable();

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
        @param newPath the new path of the node (this can allow moving
                    the node to a different directory, if the OS allows it)
        */
        void _platform_renameNode(const VString& newPath) const;

        /**
        Iterates over this directory, calling the supplied callback
        object for each node found. If the nodes "." or ".." are
        present in the platform's iteration function, they are
        omitted (not passed to the callback).
        @param callback the callback object to notify of each node we
            iterate over
        */
        void _platform_directoryIterate(VDirectoryIterationCallback& callback) const;

        VString mPath;  ///< The node's path.

};

inline bool operator< (const VFSNode& lhs, const VFSNode& rhs) { return lhs.mPath < rhs.mPath; }
inline bool operator<=(const VFSNode& lhs, const VFSNode& rhs) { return !operator>(lhs, rhs); }
inline bool operator>=(const VFSNode& lhs, const VFSNode& rhs) { return !operator<(lhs, rhs); }
inline bool operator> (const VFSNode& lhs, const VFSNode& rhs) { return  operator<(rhs, lhs); }

#endif /* vfsnode_h */

