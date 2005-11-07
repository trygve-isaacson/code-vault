/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vstreamsunit.h"

#include "vwritebufferedstream.h"
#include "vbinaryiostream.h"
#include "vstreamcopier.h"

VStreamsUnit::VStreamsUnit(bool logOnSuccess, bool throwOnError)
: VUnit("VStreamsUnit", logOnSuccess, throwOnError)
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
    this->test(rawStream.offset() == CONST_S64(12), "write-buffered stream offset");

    io.writeS32(7890);
    io.writeS32(2468);
    io.flush();
    this->test(rawStream.offset() == CONST_S64(20), "write-buffered stream offset");
    
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
    
    this->test(copier.numBytesCopied() == copierRawStream1.eofOffset(), "stream copier num bytes copied");

    }

