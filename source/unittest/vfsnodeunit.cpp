/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

/** @file */

#include "vfsnodeunit.h"

#include "vbinaryiostream.h"
#include "vbufferedfilestream.h"
#include "vdirectiofilestream.h"
#include "vexception.h"
#include "vtextiostream.h"

// VFSNodeIterateTestCallback -----------------------------------------------------

class VFSNodeIterateTestCallback : public VDirectoryIterationCallback {
    public:

        VFSNodeIterateTestCallback() : mNodeNames() {}
        virtual ~VFSNodeIterateTestCallback() {}

        virtual bool handleNextNode(const VFSNode& node);

        VStringVector mNodeNames;
};

// VFSNodeUnit -------------------------------------------------------------

VFSNodeUnit::VFSNodeUnit(bool logOnSuccess, bool throwOnError) :
    VUnit("VFSNodeUnit", logOnSuccess, throwOnError) {
    mTextFileLines.push_back("This is line 1.");
    mTextFileLines.push_back("This is the second line.");
    mTextFileLines.push_back("This is the third and final line.");

    for (VStringVector::const_iterator i = mTextFileLines.begin(); i != mTextFileLines.end(); ++i) {
        mTextFileContents += (*i);
        mTextFileContents += VString::NATIVE_LINE_ENDING();
    }
}

void VFSNodeUnit::run() {
    // Note that we also do testing of streams and file i/o here.

    this->logStatus(VSTRING_FORMAT("getExecutable: '%s'", VFSNode::getExecutable().getPath().chars()));
    this->logStatus(VSTRING_FORMAT("getExecutableDirectory: '%s'", VFSNode::getExecutableDirectory().getPath().chars()));
    this->logStatus(VSTRING_FORMAT("USER_HOME_DIRECTORY: '%s'", VFSNode::getKnownDirectoryNode(VFSNode::USER_HOME_DIRECTORY, "com", "app").getPath().chars()));
    this->logStatus(VSTRING_FORMAT("LOG_FILES_DIRECTORY: '%s'", VFSNode::getKnownDirectoryNode(VFSNode::LOG_FILES_DIRECTORY, "com", "app").getPath().chars()));
    this->logStatus(VSTRING_FORMAT("USER_PREFERENCES_DIRECTORY: '%s'", VFSNode::getKnownDirectoryNode(VFSNode::USER_PREFERENCES_DIRECTORY, "com", "app").getPath().chars()));
    this->logStatus(VSTRING_FORMAT("CACHED_DATA_DIRECTORY: '%s'", VFSNode::getKnownDirectoryNode(VFSNode::CACHED_DATA_DIRECTORY, "com", "app").getPath().chars()));
    this->logStatus(VSTRING_FORMAT("APPLICATION_DATA_DIRECTORY: '%s'", VFSNode::getKnownDirectoryNode(VFSNode::APPLICATION_DATA_DIRECTORY, "com", "app").getPath().chars()));
    this->logStatus(VSTRING_FORMAT("CURRENT_WORKING_DIRECTORY: '%s'", VFSNode::getKnownDirectoryNode(VFSNode::CURRENT_WORKING_DIRECTORY, "com", "app").getPath().chars()));
    this->logStatus(VSTRING_FORMAT("EXECUTABLE_DIRECTORY: '%s'", VFSNode::getKnownDirectoryNode(VFSNode::EXECUTABLE_DIRECTORY, "com", "app").getPath().chars()));

    VFSNode tempDir = VFSNode::getKnownDirectoryNode(VFSNode::CACHED_DATA_DIRECTORY, "vault", "unittest");
    VString tempDirPath = tempDir.getPath();

    VFSNode testDirRoot(tempDir, "vfsnodetest_temp");
    (void) testDirRoot.rm();

    VFSNode testDirDeep(tempDir, "vfsnodetest_temp/one/two/three");
    VUNIT_ASSERT_FALSE_LABELED(testDirDeep.exists(), "initial state 1");
    testDirDeep.mkdirs();
    VUNIT_ASSERT_TRUE_LABELED(testDirDeep.exists(), "deep mkdirs");

    VFSNode testDirDeeper(testDirDeep, "four");
    VUNIT_ASSERT_FALSE_LABELED(testDirDeeper.exists(), "initial state 2");
    testDirDeeper.mkdirs();
    VUNIT_ASSERT_TRUE_LABELED(testDirDeeper.exists(), "one-deep mkdirs");

    // Now that we have created a deep directory structure, let's do some
    // file i/o streams stuff here.

    VFSNode testTextFileNode(testDirDeeper, "test_text_file.txt");

    VBufferedFileStream btfs(testTextFileNode);
    this->_testTextFileIO("starting Buffered Text IO tests", testTextFileNode, btfs);
    (void) testTextFileNode.rm();
    VUNIT_ASSERT_FALSE_LABELED(testTextFileNode.exists(), "buffered text file removed");

    VDirectIOFileStream dtfs(testTextFileNode);
    this->_testTextFileIO("starting Unbuffered Text IO tests", testTextFileNode, dtfs);
    this->_testTextFileReadAll(testTextFileNode);
    (void) testTextFileNode.rm();
    VUNIT_ASSERT_FALSE_LABELED(testTextFileNode.exists(), "unbuffered text file removed");

    VFSNode testBinaryFileNode(testDirDeeper, "test_binary_file");

    VBufferedFileStream bbfs(testBinaryFileNode);
    this->_testBinaryFileIO("starting Buffered Binary IO tests", testBinaryFileNode, bbfs);
    (void) testBinaryFileNode.rm();
    VUNIT_ASSERT_FALSE_LABELED(testBinaryFileNode.exists(), "buffered binary file removed");

    VDirectIOFileStream dbfs(testBinaryFileNode);
    this->_testBinaryFileIO("starting Unbuffered Binary IO tests", testBinaryFileNode, dbfs);
    (void) testBinaryFileNode.rm();
    VUNIT_ASSERT_FALSE_LABELED(testBinaryFileNode.exists(), "unbuffered binary file removed");

    this->_testDirectoryIteration(testDirDeeper);

    // Next, test all flavors of renaming operations.
    VFSNode copyTest1(tempDir, "vfsnodetest_temp/one/two/test1.txt");
    VBufferedFileStream sourceFileStream(copyTest1);
    sourceFileStream.openWrite();
    VTextIOStream sourceOut(sourceFileStream);
    sourceOut.writeLine("line 1");
    sourceOut.writeLine("line 2");
    sourceOut.flush();
    sourceFileStream.close();
    VUNIT_ASSERT_TRUE_LABELED(copyTest1.exists(), "test1 exists");

    VFSNode copyTest2(tempDir, "vfsnodetest_temp/one/two/test2.txt");
    copyTest1.renameToName("test2.txt");
    VUNIT_ASSERT_FALSE_LABELED(copyTest1.exists(), "test1 was renamed");
    VUNIT_ASSERT_TRUE_LABELED(copyTest2.exists(), "test2 exists");

    VFSNode copyTest3;
    copyTest2.renameToName("test3.txt", copyTest3);
    VUNIT_ASSERT_FALSE_LABELED(copyTest2.exists(), "test2 was renamed");
    VUNIT_ASSERT_TRUE_LABELED(copyTest3.getPath() == tempDirPath + "/vfsnodetest_temp/one/two/test3.txt" && copyTest3.exists(), "test3 exists");

    VFSNode copyTest4(tempDir, "vfsnodetest_temp/one/two/three/test4.txt");
    copyTest3.renameToNode(copyTest4);
    VUNIT_ASSERT_FALSE_LABELED(copyTest3.exists(), "test3 was moved and renamed");
    VUNIT_ASSERT_TRUE_LABELED(copyTest4.exists(), "test4 exists");

    copyTest4.renameToPath(tempDirPath + "/vfsnodetest_temp/one/two/test5.txt");
    VFSNode copyTest5(tempDir, "vfsnodetest_temp/one/two/test5.txt");
    VUNIT_ASSERT_FALSE_LABELED(copyTest4.exists(), "test4 was moved and renamed");
    VUNIT_ASSERT_TRUE_LABELED(copyTest5.exists(), "test5 exists");

    copyTest5.renameToName("test5.txt"); // should throw

    // Clean up our litter.
    (void) copyTest5.rm();

    // Done with exercising file i/o and streams and directory stuff. Clean up our litter.

    VString deepPath;
    testDirDeeper.getParentPath(deepPath);
    VUNIT_ASSERT_EQUAL_LABELED(deepPath, tempDirPath + "/vfsnodetest_temp/one/two/three", "get parent path");

    VString nodeName;
    testDirDeeper.getName(nodeName);
    VUNIT_ASSERT_EQUAL_LABELED(nodeName, "four", "get deep node name");

    VFSNode shallowNode("shallow");
    shallowNode.getName(nodeName);
    VUNIT_ASSERT_EQUAL_LABELED(nodeName, "shallow", "get shallow node name");

    (void) testDirRoot.rm();
    VUNIT_ASSERT_FALSE_LABELED(testDirRoot.exists(), "rm tree");

    // Test some of the path string manipulation.

    VString testPath3("one/two/three");
    VFSNode testPath3Node(testPath3);

    VString testPath2;
    testPath3Node.getParentPath(testPath2);
    VFSNode testPath2Node(testPath2);
    VUNIT_ASSERT_EQUAL_LABELED(testPath2, "one/two", "parent of level 3 path");

    VString testPath1;
    testPath2Node.getParentPath(testPath1);
    VFSNode testPath1Node(testPath1);
    VUNIT_ASSERT_EQUAL_LABELED(testPath1, "one", "parent of level 2 path");

    VString testPath0;
    testPath1Node.getParentPath(testPath0);
    VFSNode testPath0Node(testPath0);
    VUNIT_ASSERT_EQUAL_LABELED(testPath0, "", "parent of level 1 path");

    // Test oddities with DOS driver letters and mapped drives. Trailing slash on drive letter may or may be present.
#ifdef VPLATFORM_WIN
    // These tests assume that C: and C:Windows exist; some installations may use a different drive letter, in which case skip the test.
    const VString DRIVE_LETTER("C");
    const VString CHILD_NODE_NAME("Windows");
    if (VFSNode(VSTRING_FORMAT("%s:/%s", DRIVE_LETTER.chars(), CHILD_NODE_NAME.chars())).exists()) {
        this->_testWindowsDrivePaths(DRIVE_LETTER, CHILD_NODE_NAME, false, true);
        this->_testWindowsDrivePaths(DRIVE_LETTER, CHILD_NODE_NAME, true, true);
    }
#endif

    // Test assignment operator.
    VFSNode someNode("a/b/c/d");
    VFSNode copiedNode;
    copiedNode = someNode;
    VUNIT_ASSERT_EQUAL_LABELED(copiedNode.getPath(), "a/b/c/d", "assignment operator");

    /*
        Uncomment if you want to exercise this code. It's commented out for now because by its nature it
        will litter several directories with its output. (It tests the APIs that locate the various
        platform-dependent directories where log files, preference files, etc. should be written.
        So for now I've chosen not to exercise this code in this unit test.

        // Test known directory location lookup. Just write a file to each directory;
        // user will have to visually check that it was put in the right place.
        this->_writeKnownDirectoryTestFile(VFSNode::USER_HOME_DIRECTORY, "unittest-user");
        this->_writeKnownDirectoryTestFile(VFSNode::LOG_FILES_DIRECTORY, "unittest-logs");
        this->_writeKnownDirectoryTestFile(VFSNode::USER_PREFERENCES_DIRECTORY, "unittest-prefs");
        this->_writeKnownDirectoryTestFile(VFSNode::CACHED_DATA_DIRECTORY, "unittest-cache");
        this->_writeKnownDirectoryTestFile(VFSNode::APPLICATION_DATA_DIRECTORY, "unittest-appdata");
        this->_writeKnownDirectoryTestFile(VFSNode::CURRENT_WORKING_DIRECTORY, "unittest-cwd");
    */
}

void VFSNodeUnit::_testTextFileIO(const VString& seriesLabel, VFSNode& node, VAbstractFileStream& fileStream) {
    // This output line is just to mark which kind of file i/o we're doing:
    this->logStatus(seriesLabel);

    VTextIOStream io(fileStream);
    fileStream.openWrite();

    for (VStringVector::const_iterator i = mTextFileLines.begin(); i != mTextFileLines.end(); ++i)
        io.writeLine(*i);

    io.flush();

    fileStream.close();

    VUNIT_ASSERT_TRUE_LABELED(node.size() != 0, "non-empty file");
    VUNIT_ASSERT_TRUE_LABELED(node.isFile(), "is file");
    VUNIT_ASSERT_FALSE_LABELED(node.isDirectory(), "is not directory");

    fileStream.openReadOnly();

    VString line;
    int lineNumber = 1;
    for (VStringVector::const_iterator i = mTextFileLines.begin(); i != mTextFileLines.end(); ++i) {
        io.readLine(line);
        VUNIT_ASSERT_EQUAL_LABELED(line, *i, VSTRING_FORMAT("Line %d match", lineNumber));
        ++lineNumber;
    }

    try {
        // We should not be able to read any more lines.
        io.readLine(line);
        // If we get here, there's junk past the proper end of the file.
        VUNIT_ASSERT_FAILURE("EOF mark position");
    } catch (const VEOFException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("EOF mark position");
    } catch (...) {
        VUNIT_ASSERT_FAILURE("EOF mark position (unexpected exception type)");
    }

    fileStream.close();
}

void VFSNodeUnit::_testTextFileReadAll(VFSNode& node) {
    VString contents;
    node.readAll(contents);
    VUNIT_ASSERT_EQUAL_LABELED(contents, mTextFileContents, "VFSNode::readAll() to VString");

    VStringVector lines;
    node.readAll(lines);
    VUNIT_ASSERT_EQUAL_LABELED((int) lines.size(), (int) mTextFileLines.size(), "VFSNode::readAll() to VStringVector, count");

    int lineNumber = 1;
    for (size_t i = 0; i < lines.size(); ++i) {
        VUNIT_ASSERT_EQUAL_LABELED(lines[i], mTextFileLines[i], VSTRING_FORMAT("VFSNode::readAll() to VStringVector, line %d match", lineNumber));
        ++lineNumber;
    }
}

void VFSNodeUnit::_testBinaryFileIO(const VString& seriesLabel, VFSNode& node, VAbstractFileStream& fileStream) {
    // This output line is just to mark which kind of file i/o we're doing:
    this->logStatus(seriesLabel);

    VBinaryIOStream io(fileStream);

    fileStream.openWrite();

    io.writeS8(1);
    io.writeU8(2);
    io.writeS16(3);
    io.writeU16(4);
    io.writeS32(5L);
    io.writeU32(6L);
    io.writeS64(CONST_S64(7));
    io.writeU64(CONST_U64(8));
    io.writeFloat(9.9f);
    io.writeDouble(3.1415926);
    io.writeBool(true);
    io.writeString("hello");
    io.flush();

    fileStream.close();

    VUNIT_ASSERT_TRUE_LABELED(node.size() != 0, "non-empty file");
    VUNIT_ASSERT_TRUE_LABELED(node.isFile(), "is file");
    VUNIT_ASSERT_FALSE_LABELED(node.isDirectory(), "is not directory");

    fileStream.openReadOnly();

    VUNIT_ASSERT_TRUE_LABELED(io.readS8() == 1, "S8 match");
    VUNIT_ASSERT_TRUE_LABELED(io.readU8() == 2, "U8 match");
    VUNIT_ASSERT_TRUE_LABELED(io.readS16() == 3, "S16 match");
    VUNIT_ASSERT_TRUE_LABELED(io.readU16() == 4, "U16 match");
    VUNIT_ASSERT_TRUE_LABELED(io.readS32() == 5L, "S32 match");
    VUNIT_ASSERT_TRUE_LABELED(io.readU32() == 6L, "U32 match");
    VUNIT_ASSERT_TRUE_LABELED(io.readS64() == CONST_S64(7), "S64 match");
    VUNIT_ASSERT_TRUE_LABELED(io.readU64() == CONST_U64(8), "U64 match");
    VUNIT_ASSERT_TRUE_LABELED(io.readFloat() == 9.9f, "Float match");
    VUNIT_ASSERT_TRUE_LABELED(io.readDouble() == 3.1415926, "Double match");
    VUNIT_ASSERT_TRUE_LABELED(io.readBool() == true, "Bool match");
    VUNIT_ASSERT_TRUE_LABELED(io.readString() == "hello", "String match");
    try {
        // We should not be able to read any more data.
        (void) io.readU8();
        // If we get here, there's junk past the proper end of the file.
        VUNIT_ASSERT_FAILURE("EOF mark position");
    } catch (const VEOFException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("EOF mark position");
    } catch (...) {
        VUNIT_ASSERT_FAILURE("EOF mark position (unexpected exception type)");
    }

    fileStream.close();
}

void VFSNodeUnit::_testDirectoryIteration(const VFSNode& dir) {
    const int NUM_FILES_TO_CREATE = 5;
    const int NUM_FILES_TO_CHECK = NUM_FILES_TO_CREATE + 3; // we'll verify we don't have these extras

    // Test directory listing, iteration, find.
    // Create 5 files in the deep directory, then test that we can find them.
    for (int i = 0; i < NUM_FILES_TO_CREATE; ++i) {
        VString testIterFileName(VSTRING_ARGS("iter_test_%d.txt", i));
        VFSNode testIterFileNode;
        dir.getChildNode(testIterFileName, testIterFileNode);
        VBufferedFileStream testIterStream(testIterFileNode);
        testIterStream.openWrite();
        VTextIOStream out(testIterStream);
        out.writeLine(testIterFileName);
    }

    {
        // find() test
        VFSNode testIterNode;
        for (int i = 0; i < NUM_FILES_TO_CHECK; ++i) {
            VString testIterFileName(VSTRING_ARGS("iter_test_%d.txt", i));
            if (i < NUM_FILES_TO_CREATE)
                VUNIT_ASSERT_TRUE_LABELED(dir.find(testIterFileName, testIterNode), VSTRING_FORMAT("find() found #%d", i)); // this file should exist
            else
                VUNIT_ASSERT_FALSE_LABELED(dir.find(testIterFileName, testIterNode), VSTRING_FORMAT("find() did not find #%d", i)); // this file should not exist
        }
    }

    // There is no guarantee that the list() and iterate() methods will return the directory
    // listing in any particular order. We either need to sort them ourself, or verify without
    // regard to order. Here, we'll sort the returned string list, since we know that our
    // file names are sortable because they are single-digit-number-based strings, e.g. "iter_test_3.txt".
    // Note that the result of sorting is ultimately dependent on strcmp().

    {
        // list() names test
        int index = 0;
        VStringVector fileNames;
        dir.list(fileNames);
        std::sort(fileNames.begin(), fileNames.end());
        VUNIT_ASSERT_EQUAL_LABELED(static_cast<int>(fileNames.size()), NUM_FILES_TO_CREATE, "list names size");
        for (VStringVector::const_iterator i = fileNames.begin(); i != fileNames.end(); ++i, ++index) {
            VString testIterFileName(VSTRING_ARGS("iter_test_%d.txt", index));
            VUNIT_ASSERT_EQUAL_LABELED(*i, testIterFileName, VSTRING_FORMAT("list names #%d", index));
        }
    }

    {
        // list() nodes test
        int index = 0;
        VFSNodeVector fileNodes;
        dir.list(fileNodes);
        std::sort(fileNodes.begin(), fileNodes.end());
        VUNIT_ASSERT_EQUAL_LABELED(static_cast<int>(fileNodes.size()), NUM_FILES_TO_CREATE, "list nodes size");
        for (VFSNodeVector::const_iterator i = fileNodes.begin(); i != fileNodes.end(); ++i, ++index) {
            VString testIterFileName(VSTRING_ARGS("iter_test_%d.txt", index));
            VString nodeFileName;
            i->getName(nodeFileName);
            VUNIT_ASSERT_EQUAL_LABELED(nodeFileName, testIterFileName, VSTRING_FORMAT("list nodes #%d", index));
        }
    }

    {
        // iterate() test
        int index = 0;
        VFSNodeIterateTestCallback callback;
        dir.iterate(callback);
        std::sort(callback.mNodeNames.begin(), callback.mNodeNames.end());
        VUNIT_ASSERT_EQUAL_LABELED(static_cast<int>(callback.mNodeNames.size()), NUM_FILES_TO_CREATE, "iterate size");
        for (VStringVector::const_iterator i = callback.mNodeNames.begin(); i != callback.mNodeNames.end(); ++i, ++index) {
            VString testIterFileName(VSTRING_ARGS("iter_test_%d.txt", index));
            VUNIT_ASSERT_EQUAL_LABELED(*i, testIterFileName, VSTRING_FORMAT("iterate nodes #%d", index));
        }
    }

}

bool VFSNodeIterateTestCallback::handleNextNode(const VFSNode& node) {
    VString nodeName;
    node.getName(nodeName);
    mNodeNames.push_back(nodeName);
    return true;
}

void VFSNodeUnit::_writeKnownDirectoryTestFile(VFSNode::KnownDirectoryIdentifier id, const VString& fileName) {
    VFSNode folder = VFSNode::getKnownDirectoryNode(id, "BombayDigital", "unittest-temp");

    VFSNode fileNode;
    folder.getChildNode(fileName, fileNode);

    VBufferedFileStream fs(fileNode);
    fs.openWrite();

    VTextIOStream out(fs);

    VInstant now;
    out.writeLine(now.getLocalString());
    out.flush();

    this->logStatus(VSTRING_FORMAT("Wrote to file '%s'.", fileNode.getPath().chars()));
}

void VFSNodeUnit::_testWindowsDrivePaths(const VString& driveLetter, const VString& childNodeName, bool adornedWithSlash, bool childIsDirectory) {
    VString driveLetterNodeName(VSTRING_ARGS("%s:", driveLetter.chars()));
    VString adornedDriveLetterPath(VSTRING_ARGS("%s:%s", driveLetter.chars(), (adornedWithSlash ? VFSNode::PATH_SEPARATOR_CHARS : "")));

    VFSNode driveLetterNode(adornedDriveLetterPath);
    this->logStatus(VSTRING_FORMAT("Testing '%s'...", adornedDriveLetterPath.chars()));
    VUNIT_ASSERT_EQUAL_LABELED(driveLetterNode.getName(), driveLetterNodeName, "drive letter node name");
    VUNIT_ASSERT_EQUAL_LABELED(driveLetterNode.getParentPath(), "", "drive letter node parent path");
    VUNIT_ASSERT_EQUAL_LABELED(driveLetterNode.isDirectory(), true, "drive letter node is dir"); // this test assumes that drive C: exists; not true in some installations
    VUNIT_ASSERT_EQUAL_LABELED(driveLetterNode.isFile(), false, "drive letter node is not file");

    VString adornedChildNodePath(VSTRING_ARGS("%s:%s%s", driveLetter.chars(), (adornedWithSlash ? VFSNode::PATH_SEPARATOR_CHARS : ""), childNodeName.chars()));
    VFSNode childNode(driveLetterNode, childNodeName);
    this->logStatus(VSTRING_FORMAT("Testing '%s'...", adornedChildNodePath.chars()));
    VUNIT_ASSERT_EQUAL_LABELED(childNode.getName(), childNodeName, "drive letter child node name");
    VUNIT_ASSERT_EQUAL_LABELED(childNode.getParentPath(), driveLetterNodeName, "drive letter child node parent path");
    VUNIT_ASSERT_EQUAL_LABELED(childNode.isDirectory(), childIsDirectory, "drive letter child node is/not dir check"); // this test assumes that C:Windows dir exists; not true in some installations
    VUNIT_ASSERT_EQUAL_LABELED(childNode.isFile(), !childIsDirectory, "drive letter child node is/not file check");

    VString childFileName("child.txt");
    VString expectedChildPath(VSTRING_ARGS("%s:%c%s", driveLetter.chars(), VFSNode::PATH_SEPARATOR_CHAR, childFileName.chars()));
    VFSNode childFileNode(driveLetterNode, childFileName);
    this->logStatus(VSTRING_FORMAT("Testing '%s + %s'...", adornedDriveLetterPath.chars(), childFileName.chars()));
    VUNIT_ASSERT_EQUAL_LABELED(childFileNode.getName(), childFileName, "drive letter child file node name");
    VUNIT_ASSERT_EQUAL_LABELED(childFileNode.getParentPath(), driveLetterNodeName, "drive letter child file node parent path");
    VUNIT_ASSERT_EQUAL_LABELED(childFileNode.getPath(), expectedChildPath, "drive letter child file node path");
}
