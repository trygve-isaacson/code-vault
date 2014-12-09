/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

/** @file */

#include "vstreamsunit.h"

#include "vwritebufferedstream.h"
#include "vbinaryiostream.h"
#include "vstreamcopier.h"
#include "vexception.h"

#include "vtextstreamtailer.h"
#include "vmutexlocker.h"

VStreamsUnit::VStreamsUnit(bool logOnSuccess, bool throwOnError) :
    VUnit("VStreamsUnit", logOnSuccess, throwOnError) {
}

void VStreamsUnit::run() {
    // We want to test some of the classes and edge case APIs that are not
    // exercised as part of the other unit tests. No need to test the
    // individual typed binary i/o calls, since they are already covered.
    // But various other things are only touched incidentally, so we want
    // to cover them here.

    // VMemoryStream is useful for validation because you can use
    // operator== and it will compare the EOF positions and the contents
    // of two memory streams for equality.

    this->_testWriteBufferedStream();
    this->_testStreamCopier();
    this->_testBufferOwnership();
    this->_testReadOnlyStream();
    this->_testOverloadedStreamCopyAPIs();
    this->_testStreamTailer();
}

void VStreamsUnit::_testWriteBufferedStream() {
    // Test VWriteBufferedStream. We'll have it buffer to a (another)
    // memory stream so we don't have to use a file. We'll write some
    // text to it and verify it. We'll seek and skip.

    VMemoryStream           rawStream;
    VWriteBufferedStream    bufferedStream(rawStream);
    VBinaryIOStream         io(bufferedStream);

    io.writeS32(1234);
    io.writeS32(5678);
    io.seek(CONST_S64(-4), SEEK_CUR);
    io.writeS32(9012);
    io.writeS32(3456);
    io.flush();
    VUNIT_ASSERT_EQUAL_LABELED(rawStream.getIOOffset(), CONST_S64(12), "write-buffered stream offset");

    io.writeS32(7890);
    io.writeS32(2468);
    io.flush();
    VUNIT_ASSERT_EQUAL_LABELED(rawStream.getIOOffset(), CONST_S64(20), "write-buffered stream offset");

    VBinaryIOStream    verifier(rawStream);
    verifier.seek0();
    VUNIT_ASSERT_EQUAL_LABELED(verifier.readS32(), 1234, "write-buffered stream check 1");
    VUNIT_ASSERT_EQUAL_LABELED(verifier.readS32(), 9012, "write-buffered stream check 2");
    VUNIT_ASSERT_EQUAL_LABELED(verifier.readS32(), 3456, "write-buffered stream check 3");
    VUNIT_ASSERT_EQUAL_LABELED(verifier.readS32(), 7890, "write-buffered stream check 4");
    VUNIT_ASSERT_EQUAL_LABELED(verifier.readS32(), 2468, "write-buffered stream check 5");
}

void VStreamsUnit::_testStreamCopier() {
    // Test VStreamCopier. We'll copy between streams using the different
    // constructor and init forms, and verify the results.
    // We use {} scopes so we can force the lifetimes and re-use variable names.

    VMemoryStream   copierRawStream1;
    VBinaryIOStream copierIOStream1(copierRawStream1);
    VMemoryStream   copierRawStream2;
    VBinaryIOStream copierIOStream2(copierRawStream2);

    copierIOStream1.writeString("Here is a string to be stored in raw stream 1 and later copied into raw stream 2 for comparison purposes.");

    // Note that we are using a chunk size (64) smaller than the amount of data,
    // in order to check the behavior of iterating over the chunks.

    /* subtest scope */ {
        copierIOStream1.seek0();
        copierRawStream2.setEOF(0);
        VStreamCopier    copier(64, &copierRawStream1, &copierRawStream2);
        while (copier.copyChunk())
            { }
        VUNIT_ASSERT_TRUE_LABELED(copierRawStream1 == copierRawStream2, "stream copier construct raw->raw");
    }

    /* subtest scope */ {
        copierIOStream1.seek0();
        copierRawStream2.setEOF(0);
        VStreamCopier    copier(64, &copierIOStream1, &copierIOStream2);
        while (copier.copyChunk())
            { }
        VUNIT_ASSERT_TRUE_LABELED(copierRawStream1 == copierRawStream2, "stream copier construct io->io");
    }

    /* subtest scope */ {
        copierIOStream1.seek0();
        copierRawStream2.setEOF(0);
        VStreamCopier    copier(64, &copierRawStream1, &copierIOStream2);
        while (copier.copyChunk())
            { }
        VUNIT_ASSERT_TRUE_LABELED(copierRawStream1 == copierRawStream2, "stream copier construct raw->io");
    }

    /* subtest scope */ {
        copierIOStream1.seek0();
        copierRawStream2.setEOF(0);
        VStreamCopier    copier(64, &copierIOStream1, &copierRawStream2);
        while (copier.copyChunk())
            { }
        VUNIT_ASSERT_TRUE_LABELED(copierRawStream1 == copierRawStream2, "stream copier construct io->raw");
    }

    VStreamCopier    copier;

    copierIOStream1.seek0();
    copierRawStream2.setEOF(0);
    copier.init(64, &copierRawStream1, &copierRawStream2);
    while (copier.copyChunk())
        { }
    VUNIT_ASSERT_TRUE_LABELED(copierRawStream1 == copierRawStream2, "stream copier init raw->raw");

    copierIOStream1.seek0();
    copierRawStream2.setEOF(0);
    copier.init(64, &copierIOStream1, &copierIOStream2);
    while (copier.copyChunk())
        { }
    VUNIT_ASSERT_TRUE_LABELED(copierRawStream1 == copierRawStream2, "stream copier init io->io");

    copierIOStream1.seek0();
    copierRawStream2.setEOF(0);
    copier.init(64, &copierRawStream1, &copierIOStream2);
    while (copier.copyChunk())
        { }
    VUNIT_ASSERT_TRUE_LABELED(copierRawStream1 == copierRawStream2, "stream copier init raw->io");

    copierIOStream1.seek0();
    copierRawStream2.setEOF(0);
    copier.init(64, &copierIOStream1, &copierRawStream2);
    while (copier.copyChunk())
        { }
    VUNIT_ASSERT_TRUE_LABELED(copierRawStream1 == copierRawStream2, "stream copier init io->raw");

    VUNIT_ASSERT_EQUAL_LABELED(copier.numBytesCopied(), copierRawStream1.getEOFOffset(), "stream copier num bytes copied");
}

void VStreamsUnit::_testBufferOwnership() {
    // Tests for buffer ownership. Multiple streams and one buffer.
    // This scope forces destructors at the end. Some of this tests proper
    // deallocation of shared buffers when the streams are destructed, where an error
    // would cause a crash when we exit this scope.

    VMemoryStream share1;
    Vu8* buffer = share1.getBuffer();
    VMemoryStream share2(buffer, VMemoryStream::kAllocatedByOperatorNew, false, share1.getBufferSize(), share1.getEOFOffset());
    VMemoryStream share3(buffer, VMemoryStream::kAllocatedByOperatorNew, false, share1.getBufferSize(), share1.getEOFOffset());
    VMemoryStream share4(buffer, VMemoryStream::kAllocatedByOperatorNew, false, share1.getBufferSize(), share1.getEOFOffset());
    // At this point we have 4 streams, and stream 1 owns the buffer.
    // Transfer ownership to stream 2.
    share1.orphanBuffer(); // relinquish ownership
    share2.adoptBuffer(buffer, VMemoryStream::kAllocatedByOperatorNew, true, share1.getBufferSize(), share1.getEOFOffset());
    // Give stream 4 an entirely different buffer of length 10.
    Vu8* buffer4 = new Vu8[10];
    share4.adoptBuffer(buffer4, VMemoryStream::kAllocatedByOperatorNew, true, 10, 0);

    // Verify that if we write 10 or fewer bytes, share4 is using the same buffer.
    VBinaryIOStream io4(share4);
    io4.writeS32(1);
    io4.writeS32(2);
    VUNIT_ASSERT_TRUE_LABELED(share4.getBuffer() == buffer4, "same heap buffer before EOF");
    // Verify that if we go beyond 10 bytes, share4 has to allocate a new buffer. Our buffer4 is now an invalid pointer (it's been deleted).
    io4.writeS32(3);
    VUNIT_ASSERT_TRUE_LABELED(share4.getBuffer() != buffer4, "different heap buffer after EOF");
    // Now give it a buffer on the stack and make sure that reallocation doesn't crash, rather it properly switches to a heap buffer.
    Vu8 stackBuffer4[10];
    share4.adoptBuffer(stackBuffer4, VMemoryStream::kAllocatedOnStack, true, 10, 0); // "true" is required to allow us to write to the stream
    io4.writeS32(1);
    io4.writeS32(2);
    VUNIT_ASSERT_TRUE_LABELED(share4.getBuffer() == stackBuffer4, "same stack buffer before EOF");
    // Verify that if we go beyond 10 bytes, share4 has to allocate a new buffer. Our buffer4 is now an invalid pointer (it's been deleted).
    io4.writeS32(3);
    VUNIT_ASSERT_TRUE_LABELED(share4.getBuffer() != stackBuffer4, "new heap buffer after EOF");

}

void VStreamsUnit::_testReadOnlyStream() {
    // Test read-only memory streams.
    // Multiple streams can share a buffer; no shared i/o state between them; none deletes the buffer.
    // The scope here forces destruction, which would crash if one of the streams tried to delete the buffer.

    // First we'll fill it with 4 integer values to be verified by reading via streams.
    Vu8 readOnlyBuffer[16];
    /* local scope */ {
        VMemoryStream initializer(readOnlyBuffer, VMemoryStream::kAllocatedOnStack, false, 16, 0);
        VBinaryIOStream initializerIO(initializer);
        initializerIO.writeS32(1);
        initializerIO.writeS32(2);
        initializerIO.writeS32(3);
        initializerIO.writeS32(4);
    }

    // Now we'll create 3 read-only streams using that buffer that we've initialized.
    VReadOnlyMemoryStream r1(readOnlyBuffer, 16);
    VBinaryIOStream ro1(r1);
    VReadOnlyMemoryStream r2(readOnlyBuffer, 16);
    VBinaryIOStream ro2(r2);
    VReadOnlyMemoryStream r3(readOnlyBuffer, 16);
    VBinaryIOStream ro3(r3);

    // Now we'll read in an interleaved fashion, testing that each reader sees the full sequence of bytes.
    VUNIT_ASSERT_EQUAL_LABELED(ro1.readS32(), 1, "ro1 1");
    VUNIT_ASSERT_EQUAL_LABELED(ro2.readS32(), 1, "ro2 1");
    VUNIT_ASSERT_EQUAL_LABELED(ro3.readS32(), 1, "ro3 1");
    VUNIT_ASSERT_EQUAL_LABELED(ro1.readS32(), 2, "ro1 2");
    VUNIT_ASSERT_EQUAL_LABELED(ro2.readS32(), 2, "ro2 2");
    VUNIT_ASSERT_EQUAL_LABELED(ro3.readS32(), 2, "ro3 2");
    VUNIT_ASSERT_EQUAL_LABELED(ro1.readS32(), 3, "ro1 3");
    VUNIT_ASSERT_EQUAL_LABELED(ro2.readS32(), 3, "ro2 3");
    VUNIT_ASSERT_EQUAL_LABELED(ro3.readS32(), 3, "ro3 3");
    VUNIT_ASSERT_EQUAL_LABELED(ro1.readS32(), 4, "ro1 4");
    // Briefly test a couple of seeks and reads backward in the stream.
    ro2.seek(-8, SEEK_CUR);
    VUNIT_ASSERT_EQUAL_LABELED(ro2.readS32(), 2, "ro2 2 after seek");
    VUNIT_ASSERT_EQUAL_LABELED(ro3.readS32(), 4, "ro3 4");
    VUNIT_ASSERT_EQUAL_LABELED(ro2.readS32(), 3, "ro2 3");
    VUNIT_ASSERT_EQUAL_LABELED(ro2.readS32(), 4, "ro2 4");

    // Now that we're at the presumed EOF, verify that a read will throw EOF.
    try {
        (void) ro1.readS32();
        VUNIT_ASSERT_FAILURE("EOF was not thrown on read past EOF");
    } catch (const VEOFException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("EOF thrown on read past EOF");
    }

    // Verify that the EOF exception does not affect that or any other reader.
    ro2.seek(-4, SEEK_CUR);
    VUNIT_ASSERT_EQUAL_LABELED(ro2.readS32(), 4, "ro2 4");
    ro1.seek(-8, SEEK_CUR);
    VUNIT_ASSERT_EQUAL_LABELED(ro1.readS32(), 3, "ro1 3");

    // Verify that any attempt to write will throw EOF, regardless of io offset.
    ro3.seek0(); // go back to start of stream
    try {
        ro3.writeS32(1);
        VUNIT_ASSERT_FAILURE("EOF was not thrown on writing to a read-only stream");
    } catch (const VEOFException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("EOF thrown on writing to a read-only stream");
    }

}

void VStreamsUnit::_testOverloadedStreamCopyAPIs() {
    // Test the overloaded streamCopy APIs.

    // 2 low-level VStream objects (VMemoryStream).
    // 2 high-level VIOStream objects that use them (VTextIOStream).
    VMemoryStream vstreamFrom;
    VMemoryStream vstreamTo;
    VTextIOStream viostreamFrom(vstreamFrom);
    VTextIOStream viostreamTo(vstreamTo);

    // Write a very long string into the source, and reset it.
    const VString EXAMPLE_STRING("This is a very long string that we will copy from stream to stream using different overloaded APIs.");
    viostreamFrom.writeString(EXAMPLE_STRING);
    viostreamFrom.seek0();

    // Copy 10 bytes at a time, using each of the 4 overloaded APIs.
    // 1. VStream to VStream
    VStream::streamCopy(vstreamFrom, vstreamTo, 10);
    // 2. VIOStream to VIOStream
    VStream::streamCopy(viostreamFrom, viostreamTo, 10);
    // 3. VIOStream to VStream
    VStream::streamCopy(viostreamFrom, vstreamTo, 10);
    // 4. VStream to VIOStream
    VStream::streamCopy(vstreamFrom, viostreamTo, 10);

    // Verify that the data was correctly copied.

    viostreamTo.seek0();
    VUNIT_ASSERT_EQUAL_LABELED(viostreamTo.available(), CONST_S64(40), "all 40 bytes copied");

    VString whatWasCopied;
    viostreamTo.readAll(whatWasCopied);
    VString whatShouldHaveBeenCopied;
    EXAMPLE_STRING.getSubstring(whatShouldHaveBeenCopied, 0, 40);
    VUNIT_ASSERT_EQUAL_LABELED(whatWasCopied, whatShouldHaveBeenCopied, "correct substring was copied");
}

class TestTailHandler : public VTailHandler {
    public:
        TestTailHandler();
        virtual ~TestTailHandler() {}

        virtual void processLine(const VString& line);
        virtual void processCodePoint(const VCodePoint& c);
    
        int getNumProcessedLines() const;
        int getNumProcessedCodePoints() const;
    
        VString getProcessedLine(int index) const;
        VCodePoint getProcessedCodePoint(int index) const;

    private:
    
        mutable VMutex  mMutex;
        VStringVector   mCollectedLines;
        VString         mCollectedCodePoints;
};

TestTailHandler::TestTailHandler()
    : VTailHandler()
    , mMutex("TestTailHandler")
    , mCollectedLines()
    , mCollectedCodePoints()
    {
}

void TestTailHandler::processLine(const VString& line) {
    VMutexLocker locker(&mMutex, "TestTailHandler::processLine");
    mCollectedLines.push_back(line);
    //std::cout << "TAILED: '" << line << "'" << std::endl;
}

void TestTailHandler::processCodePoint(const VCodePoint& c) {
    VMutexLocker locker(&mMutex, "TestTailHandler::processCodePoint");
    mCollectedCodePoints += c;
}

int TestTailHandler::getNumProcessedLines() const {
    VMutexLocker locker(&mMutex, "TestTailHandler::getNumProcessedLines");
    return (int) mCollectedLines.size();
}

int TestTailHandler::getNumProcessedCodePoints() const {
    VMutexLocker locker(&mMutex, "TestTailHandler::getNumProcessedCodePoints");
    return mCollectedCodePoints.getNumCodePoints();
}

VString TestTailHandler::getProcessedLine(int index) const {
    VMutexLocker locker(&mMutex, "TestTailHandler::getProcessedLine");
    return mCollectedLines[index];
}

VCodePoint TestTailHandler::getProcessedCodePoint(int index) const {
    VMutexLocker locker(&mMutex, "TestTailHandler::getProcessedCodePoint");
    return *(mCollectedCodePoints.begin() + index);
}

void VStreamsUnit::_testStreamTailer() {
    /* example: how to tail the system log for a minute: */
    
    /*
    VFSNode f("/var/log/system.log");
    TestTailHandler sysLogTestHandler;
    VTextTailRunner sysLogTailRunner(f, sysLogTestHandler);
    sysLogTailRunner.start();
    VThread::sleep(10 * VDuration::SECOND());
    sysLogTailRunner.stop(); // Calling stop() is optional; can just destruct.
    */

    VFSNode tempDir = VFSNode::getKnownDirectoryNode(VFSNode::CACHED_DATA_DIRECTORY, "vault", "unittest");
    VFSNode testDirRoot(tempDir, "vstreamsunit_temp");
    (void) testDirRoot.rm();
    testDirRoot.mkdirs();
    
    // Create a test file and open it for writing, and create an output text stream for it.
    VFSNode testFileNode(testDirRoot, "tailed_file.txt");
    VBufferedFileStream outputFileStream(testFileNode);
    outputFileStream.openWrite();
    VTextIOStream outputStream(outputFileStream, VTextIOStream::kUseUnixLineEndings); // assertions below assume 1 code point written for line endings, so don't write DOS 2-byte line endings even on Windows
    
    // First, write 3 lines of initial content.
    outputStream.writeLine("zero");
    outputStream.writeLine("one");
    outputStream.writeLine("two");
    outputStream.flush();
    
    // Open the file read-only and create an input text stream for it.
    VBufferedFileStream inputFileStream(testFileNode);
    inputFileStream.openReadOnly();

    // Now create a file tailer.
    // It should "immediately" (separate thread) read the existing data (since our read mark is at the start).
    bool processByLine = true; // switch to test other mode
    bool callStop = false;     // switch to test other mode
    TestTailHandler testHandler;
    VTextTailRunner tailRunner(inputFileStream, testHandler, processByLine);
    tailRunner.start();
    
    VThread::sleep(VDuration::SECOND());
    if (processByLine) {
        VUNIT_ASSERT_EQUAL_LABELED(testHandler.getNumProcessedLines(), 3, "3 initial lines");
        VUNIT_ASSERT_EQUAL_LABELED(testHandler.getProcessedLine(0), "zero", "line zero");
        VUNIT_ASSERT_EQUAL_LABELED(testHandler.getProcessedLine(1), "one", "line one");
        VUNIT_ASSERT_EQUAL_LABELED(testHandler.getProcessedLine(2), "two", "line two");
    } else {
        VUNIT_ASSERT_EQUAL_LABELED(testHandler.getNumProcessedCodePoints(), 13, "13 initial code points");
        VUNIT_ASSERT_EQUAL_LABELED(testHandler.getProcessedCodePoint(0), VCodePoint('z'), "code point [0]");
        VUNIT_ASSERT_EQUAL_LABELED(testHandler.getProcessedCodePoint(5), VCodePoint('o'), "code point [5]");
        VUNIT_ASSERT_EQUAL_LABELED(testHandler.getProcessedCodePoint(9), VCodePoint('t'), "code point [9]");
    }
    
    // Write two more lines and verify they are processed.
    outputStream.writeLine("three");
    outputStream.writeLine("four");
    outputStream.flush();

    VThread::sleep(2 * VDuration::SECOND());
    if (processByLine) {
        VUNIT_ASSERT_EQUAL_LABELED(testHandler.getNumProcessedLines(), 5, "5 total lines");
        VUNIT_ASSERT_EQUAL_LABELED(testHandler.getProcessedLine(3), "three", "line three");
        VUNIT_ASSERT_EQUAL_LABELED(testHandler.getProcessedLine(4), "four", "line four");
    } else {
        VUNIT_ASSERT_EQUAL_LABELED(testHandler.getNumProcessedCodePoints(), 24, "24 initial code points");
        VUNIT_ASSERT_EQUAL_LABELED(testHandler.getProcessedCodePoint(13), VCodePoint('t'), "code point [13]");
        VUNIT_ASSERT_EQUAL_LABELED(testHandler.getProcessedCodePoint(19), VCodePoint('f'), "code point [19]");
    }
    
    if (callStop) {
        tailRunner.stop(); // Calling stop() is optional; can just destruct.
    }

}
