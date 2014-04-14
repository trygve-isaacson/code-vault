/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

/** @file */

#include "vhexunit.h"
#include "vhex.h"

#include "vmemorystream.h"
#include "vbinaryiostream.h"
#include "vtextiostream.h"
#include "vexception.h"

VHexUnit::VHexUnit(bool logOnSuccess, bool throwOnError) :
    VUnit("VHexUnit", logOnSuccess, throwOnError) {
}

void VHexUnit::run() {
    // Construct a buffer and a string with all possible byte values.
    VMemoryStream    memoryStream;
    VBinaryIOStream    io(memoryStream);

    for (int i = 0; i < 256; ++i) {
        Vu8    byteValue = static_cast<Vu8>(i);
        io.writeU8(byteValue);
    }

    // Convert the byte values to hex and validate.
    VString    hexString;
    VHex::bufferToHexString(memoryStream.getBuffer(), CONST_S64(256), hexString);

    VString    zeroTo255HexString;
    VString    temp;
    for (int i = 0; i < 256; ++i) {
        Vu8    byteValue = static_cast<Vu8>(i);
        VHex::byteToHexString(byteValue, temp);
        zeroTo255HexString += temp;
    }

    VUNIT_ASSERT_EQUAL_LABELED(hexString, zeroTo255HexString, "bufferToHexString");

    // Convert the hex string back to bytes and validate.
    VMemoryStream    bytes(256);
    VHex::hexStringToBuffer(hexString, bytes.getBuffer());
    bytes.setEOF(CONST_S64(256));    // EOF equal required for VMemoryStream equality test

    VUNIT_ASSERT_TRUE_LABELED(bytes == memoryStream, "hexStringToBuffer");

    // Call the hex dump function to print the data to a text stream.
    VMemoryStream    dumpBuffer;
    VTextIOStream    dumpStream(dumpBuffer);
    VHex            hexDump(&dumpStream);

    hexDump.printHex(memoryStream.getBuffer(), 256, 0);
    dumpStream.writeLine(VString::EMPTY()); // blank line at end, so calling readHexDump() doesn't need to catch EOF

    // Print the hex dump to the unit test output as status for review.
    dumpStream.seek0(); // back to start before reading
    VString dumpText;
    dumpStream.readAll(dumpText);
    this->logStatus(VSTRING_FORMAT("Hex dump data:\n%s", dumpText.chars()));

    // Now use the hex dump reader function to read the text, and verify
    // that we end up with the original data.
    VMemoryStream   reconstructedBuffer;
    VBinaryIOStream reconstructedStream(reconstructedBuffer);
    dumpStream.seek0(); // back to start before reading

    VHex::readHexDump(dumpStream, reconstructedStream);

    VUNIT_ASSERT_TRUE_LABELED(memoryStream == reconstructedBuffer, "VHex::readHexDump reconstructs data");
}

