/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vfsnodeunit.h"

#include "vbinaryiostream.h"
#include "vbufferedfilestream.h"
#include "vdirectiofilestream.h"
#include "vexception.h"
#include "vtextiostream.h"

VFSNodeUnit::VFSNodeUnit(bool logOnSuccess, bool throwOnError)
: VUnit("VFSNodeUnit", logOnSuccess, throwOnError)
    {
    }

void VFSNodeUnit::run()
    {
    // Note that we also do testing of streams and file i/o here.

    VFSNode    testDirRoot("vfsnodetest_temp");
    (void) testDirRoot.rm();

    VFSNode    testDirDeep("vfsnodetest_temp/one/two/three");
    this->test(! testDirDeep.exists(), "initial state 1");
    testDirDeep.mkdirs();
    this->test(testDirDeep.exists(), "deep mkdirs");

    VFSNode    testDirDeeper;
    testDirDeep.getChildNode("four", testDirDeeper);
    this->test(! testDirDeeper.exists(), "initial state 2");
    testDirDeeper.mkdirs();
    this->test(testDirDeeper.exists(), "one-deep mkdirs");
    
    // Now that we have created a deep directory structure, let's do some
    // file i/o streams stuff here.
    
    VFSNode    testTextFileNode;
    testDirDeeper.getChildNode("test_text_file.txt", testTextFileNode);

    VBufferedFileStream btfs(testTextFileNode);
    this->_testTextFileIO("starting Buffered Text IO tests", testTextFileNode, btfs);
    (void) testTextFileNode.rm();
    this->test(! testTextFileNode.exists(), "buffered text file removed");
    
    VDirectIOFileStream dtfs(testTextFileNode);
    this->_testTextFileIO("starting Unbuffered Text IO tests", testTextFileNode, dtfs);
    (void) testTextFileNode.rm();
    this->test(! testTextFileNode.exists(), "unbuffered text file removed");
    
    VFSNode    testBinaryFileNode;
    testDirDeeper.getChildNode("test_binary_file", testBinaryFileNode);

    VBufferedFileStream bbfs(testBinaryFileNode);
    this->_testBinaryFileIO("starting Buffered Binary IO tests", testBinaryFileNode, bbfs);
    (void) testBinaryFileNode.rm();
    this->test(! testBinaryFileNode.exists(), "buffered binary file removed");
    
    VDirectIOFileStream dbfs(testBinaryFileNode);
    this->_testBinaryFileIO("starting Unbuffered Binary IO tests", testBinaryFileNode, dbfs);
    (void) testBinaryFileNode.rm();
    this->test(! testBinaryFileNode.exists(), "unbuffered binary file removed");
    
    // Done with exercising file i/o and streams. Clean up our litter.
    
    VString    deepPath;
    testDirDeeper.getParentPath(deepPath);
    this->test(deepPath == "vfsnodetest_temp/one/two/three", "get parent path");
    
    VString    nodeName;
    testDirDeeper.getName(nodeName);
    this->test(nodeName == "four", "get deep node name");
    
    VFSNode    shallowNode("shallow");
    shallowNode.getName(nodeName);
    this->test(nodeName == "shallow", "get shallow node name");

    (void) testDirRoot.rm();
    this->test(! testDirRoot.exists(), "rm tree");
    
    // Test some of the path string manipulation.
    
    VString testPath3("one/two/three");
    VFSNode testPath3Node(testPath3);

    VString testPath2;
    testPath3Node.getParentPath(testPath2);
    VFSNode testPath2Node(testPath2);
    this->test(testPath2, "one/two", "parent of level 3 path");    

    VString testPath1;
    testPath2Node.getParentPath(testPath1);
    VFSNode testPath1Node(testPath1);
    this->test(testPath1, "one", "parent of level 2 path");    

    VString testPath0;
    testPath1Node.getParentPath(testPath0);
    VFSNode testPath0Node(testPath0);
    this->test(testPath0, "", "parent of level 1 path");
    
    // Test assignment operator.
    VFSNode someNode("a/b/c/d");
    VFSNode copiedNode;
    copiedNode = someNode;
    this->test(copiedNode.getPath(), "a/b/c/d", "assignment operator");
    }

void VFSNodeUnit::_testTextFileIO(const VString& seriesLabel, VFSNode& node, VAbstractFileStream& fileStream)
    {
    // This output line is just to mark which kind of file i/o we're doing:
    this->logStatus(seriesLabel);

    VTextIOStream io(fileStream);
    fileStream.openWrite();

    io.writeLine("This is line 1.");
    io.writeLine("This is the second line.");
    io.writeLine("This is the third and final line.");
    io.flush();
    
    fileStream.close();
    
    this->test(node.size() != 0, "non-empty file");
    this->test(node.isFile(), "is file");
    this->test(! node.isDirectory(), "is not directory");
    
    fileStream.openReadOnly();

    VString    line;
    io.readLine(line); this->test(line == "This is line 1.", "line 1 match");
    io.readLine(line); this->test(line == "This is the second line.", "line 2 match");
    io.readLine(line); this->test(line == "This is the third and final line.", "line 3 match");
    try
        {
        // We should not be able to read any more lines.
        io.readLine(line);
        // If we get here, there's junk past the proper end of the file.
        this->test(false, "EOF mark position");
        }
    catch (const VEOFException& /*ex*/)
        {
        this->test(true, "EOF mark position");
        }
    catch (...)
        {
        this->test(false, "EOF mark position (unexpected exception type)");
        }

    fileStream.close();
    }

void VFSNodeUnit::_testBinaryFileIO(const VString& seriesLabel, VFSNode& node, VAbstractFileStream& fileStream)
    {
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
    io.writeBool(true);
    io.writeString("hello");
    io.flush();
    
    fileStream.close();
    
    this->test(node.size() != 0, "non-empty file");
    this->test(node.isFile(), "is file");
    this->test(! node.isDirectory(), "is not directory");
    
    fileStream.openReadOnly();

    this->test(io.readS8() == 1, "S8 match");
    this->test(io.readU8() == 2, "U8 match");
    this->test(io.readS16() == 3, "S16 match");
    this->test(io.readU16() == 4, "U16 match");
    this->test(io.readS32() == 5L, "S32 match");
    this->test(io.readU32() == 6L, "U32 match");
    this->test(io.readS64() == CONST_S64(7), "S64 match");
    this->test(io.readU64() == CONST_U64(8), "U64 match");
    this->test(io.readFloat() == 9.9f, "Float match");
    this->test(io.readBool() == true, "Bool match");
    this->test(io.readString() == "hello", "String match");
    try
        {
        // We should not be able to read any more data.
        (void) io.readU8();
        // If we get here, there's junk past the proper end of the file.
        this->test(false, "EOF mark position");
        }
    catch (const VEOFException& /*ex*/)
        {
        this->test(true, "EOF mark position");
        }
    catch (...)
        {
        this->test(false, "EOF mark position (unexpected exception type)");
        }

    fileStream.close();
    }

