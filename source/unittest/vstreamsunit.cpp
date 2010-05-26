/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vstreamsunit.h"

#include "vwritebufferedstream.h"
#include "vbinaryiostream.h"
#include "vstreamcopier.h"
#include "vexception.h"

VStreamsUnit::VStreamsUnit(bool logOnSuccess, bool throwOnError) :
VUnit("VStreamsUnit", logOnSuccess, throwOnError)
    {
    }

void VStreamsUnit::run()
    {
    // We want to test some of the classes and edge case APIs that are not
    // exercised as part of the other unit tests. No need to test the
    // individual typed binary i/o calls, since they are already covered.
    // But various other things are only touched incidentally, so we want
    // to cover them here.
    
    // VMemoryStream is useful for validation because you can use
    // operator== and it will compare the EOF positions and the contents
    // of two memory streams for equality.
    
    // Test VWriteBufferedStream. We'll have it buffer to a (another)
    // memory stream so we don't have to use a file. We'll write some
    // text to it and verify it. We'll seek and skip.
    
    VMemoryStream            rawStream;
    VWriteBufferedStream    bufferedStream(rawStream);
    VBinaryIOStream            io(bufferedStream);
    
    io.writeS32(1234);
    io.writeS32(5678);
    io.seek(CONST_S64(-4), SEEK_CUR);
    io.writeS32(9012);
    io.writeS32(3456);
    io.flush();
    this->test(rawStream.getIOOffset() == CONST_S64(12), "write-buffered stream offset");

    io.writeS32(7890);
    io.writeS32(2468);
    io.flush();
    this->test(rawStream.getIOOffset() == CONST_S64(20), "write-buffered stream offset");
    
    VBinaryIOStream    verifier(rawStream);
    verifier.seek(0, SEEK_SET);
    this->test(verifier.readS32() == 1234, "write-buffered stream check 1");
    this->test(verifier.readS32() == 9012, "write-buffered stream check 2");
    this->test(verifier.readS32() == 3456, "write-buffered stream check 3");
    this->test(verifier.readS32() == 7890, "write-buffered stream check 4");
    this->test(verifier.readS32() == 2468, "write-buffered stream check 5");

    // Test VStreamCopier. We'll copy between streams using the different
    // constructor and init forms, and verify the results.
    // We use {} scopes so we can force the lifetimes and re-use variable names.
    
    VMemoryStream    copierRawStream1;
    VBinaryIOStream    copierIOStream1(copierRawStream1);
    VMemoryStream    copierRawStream2;
    VBinaryIOStream    copierIOStream2(copierRawStream2);
    
    copierIOStream1.writeString("Here is a string to be stored in raw stream 1 and later copied into raw stream 2 for comparison purposes.");
    
    // Note that we are using a chunk size (64) smaller than the amount of data,
    // in order to check the behavior of iterating over the chunks.

        {
        copierIOStream1.seek(0, SEEK_SET);
        copierRawStream2.setEOF(0);
        VStreamCopier    copier(64, &copierRawStream1, &copierRawStream2);
        while (copier.copyChunk())
            { }
        this->test(copierRawStream1 == copierRawStream2, "stream copier construct raw->raw");
        }

        {
        copierIOStream1.seek(0, SEEK_SET);
        copierRawStream2.setEOF(0);
        VStreamCopier    copier(64, &copierIOStream1, &copierIOStream2);
        while (copier.copyChunk())
            { }
        this->test(copierRawStream1 == copierRawStream2, "stream copier construct io->io");
        }

        {
        copierIOStream1.seek(0, SEEK_SET);
        copierRawStream2.setEOF(0);
        VStreamCopier    copier(64, &copierRawStream1, &copierIOStream2);
        while (copier.copyChunk())
            { }
        this->test(copierRawStream1 == copierRawStream2, "stream copier construct raw->io");
        }

        {
        copierIOStream1.seek(0, SEEK_SET);
        copierRawStream2.setEOF(0);
        VStreamCopier    copier(64, &copierIOStream1, &copierRawStream2);
        while (copier.copyChunk())
            { }
        this->test(copierRawStream1 == copierRawStream2, "stream copier construct io->raw");
        }

    VStreamCopier    copier;

    copierIOStream1.seek(0, SEEK_SET);
    copierRawStream2.setEOF(0);
    copier.init(64, &copierRawStream1, &copierRawStream2);
    while (copier.copyChunk())
        { }
    this->test(copierRawStream1 == copierRawStream2, "stream copier init raw->raw");

    copierIOStream1.seek(0, SEEK_SET);
    copierRawStream2.setEOF(0);
    copier.init(64, &copierIOStream1, &copierIOStream2);
    while (copier.copyChunk())
        { }
    this->test(copierRawStream1 == copierRawStream2, "stream copier init io->io");

    copierIOStream1.seek(0, SEEK_SET);
    copierRawStream2.setEOF(0);
    copier.init(64, &copierRawStream1, &copierIOStream2);
    while (copier.copyChunk())
        { }
    this->test(copierRawStream1 == copierRawStream2, "stream copier init raw->io");

    copierIOStream1.seek(0, SEEK_SET);
    copierRawStream2.setEOF(0);
    copier.init(64, &copierIOStream1, &copierRawStream2);
    while (copier.copyChunk())
        { }
    this->test(copierRawStream1 == copierRawStream2, "stream copier init io->raw");
    
    this->test(copier.numBytesCopied() == copierRawStream1.getEOFOffset(), "stream copier num bytes copied");
    
    // Tests for buffer ownership. Multiple streams and one buffer.
    // This scope forces destructors at the end. Some of this tests proper
    // deallocation of shared buffers when the streams are destructed, where an error
    // would cause a crash when we exit this scope.
        {
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
        this->test(share4.getBuffer() == buffer4, "same heap buffer before EOF");
        // Verify that if we go beyond 10 bytes, share4 has to allocate a new buffer. Our buffer4 is now an invalid pointer (it's been deleted).
        io4.writeS32(3);
        this->test(share4.getBuffer() != buffer4, "different heap buffer after EOF");
        // Now give it a buffer on the stack and make sure that reallocation doesn't crash, rather it properly switches to a heap buffer.
        Vu8 stackBuffer4[10];
        share4.adoptBuffer(stackBuffer4, VMemoryStream::kAllocatedOnStack, true, 10, 0); // "true" is required to allow us to write to the stream
        io4.writeS32(1);
        io4.writeS32(2);
        this->test(share4.getBuffer() == stackBuffer4, "same stack buffer before EOF");
        // Verify that if we go beyond 10 bytes, share4 has to allocate a new buffer. Our buffer4 is now an invalid pointer (it's been deleted).
        io4.writeS32(3);
        this->test(share4.getBuffer() != stackBuffer4, "new heap buffer after EOF");
        }

    // Test read-only memory streams.
    // Multiple streams can share a buffer; no shared i/o state between them; none deletes the buffer.
    // The scope here forces destruction, which would crash if one of the streams tried to delete the buffer.
        {
        // First we'll fill it with 4 integer values to be verified by reading via streams.
        Vu8 readOnlyBuffer[16];
            {
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
        this->test(ro1.readS32() == 1, "ro1 1");
        this->test(ro2.readS32() == 1, "ro2 1");
        this->test(ro3.readS32() == 1, "ro3 1");
        this->test(ro1.readS32() == 2, "ro1 2");
        this->test(ro2.readS32() == 2, "ro2 2");
        this->test(ro3.readS32() == 2, "ro3 2");
        this->test(ro1.readS32() == 3, "ro1 3");
        this->test(ro2.readS32() == 3, "ro2 3");
        this->test(ro3.readS32() == 3, "ro3 3");
        this->test(ro1.readS32() == 4, "ro1 4");
        // Briefly test a couple of seeks and reads backward in the stream.
        ro2.seek(-8, SEEK_CUR);
        this->test(ro2.readS32() == 2, "ro2 2 after seek");
        this->test(ro3.readS32() == 4, "ro3 4");
        this->test(ro2.readS32() == 3, "ro2 3");
        this->test(ro2.readS32() == 4, "ro2 4");
        
        // Now that we're at the presumed EOF, verify that a read will throw EOF.
        try
            {
            (void) ro1.readS32();
            this->test(false, "EOF was not thrown on read past EOF");
            }
        catch (const VEOFException& /*ex*/)
            {
            this->test(true, "EOF thrown on read past EOF");
            }

        // Verify that the EOF exception does not affect that or any other reader.
        ro2.seek(-4, SEEK_CUR);
        this->test(ro2.readS32() == 4, "ro2 4");
        ro1.seek(-8, SEEK_CUR);
        this->test(ro1.readS32() == 3, "ro1 3");
        
        // Verify that any attempt to write will throw EOF, regardless of io offset.
        ro3.seek(0, SEEK_SET); // go back to start of stream
        try
            {
            ro3.writeS32(1);
            this->test(false, "EOF was not thrown on writing to a read-only stream");
            }
        catch (const VEOFException& /*ex*/)
            {
            this->test(true, "EOF thrown on writing to a read-only stream");
            }

        }
    
    // Test the overloaded streamCopy APIs.
        {
        // 2 low-level VStream objects (VMemoryStream).
        // 2 high-level VIOStream objects that use them (VTextIOStream).
        VMemoryStream vstreamFrom;
        VMemoryStream vstreamTo;
        VTextIOStream viostreamFrom(vstreamFrom);
        VTextIOStream viostreamTo(vstreamTo);
        
        // Write a very long string into the source, and reset it.
        const VString EXAMPLE_STRING("This is a very long string that we will copy from stream to stream using different overloaded APIs.");
        viostreamFrom.writeString(EXAMPLE_STRING);
        viostreamFrom.seek(0, SEEK_SET);
        
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
        
        viostreamTo.seek(0, SEEK_SET);
        this->test(viostreamTo.available() == 40, "all 40 bytes copied");
        
        VString whatWasCopied;
        viostreamTo.readAll(whatWasCopied);
        VString whatShouldHaveBeenCopied;
        EXAMPLE_STRING.getSubstring(whatShouldHaveBeenCopied, 0, 40);
        this->test(whatWasCopied, whatShouldHaveBeenCopied, "correct substring was copied");
        }
    }

