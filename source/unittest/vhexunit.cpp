/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vhexunit.h"
#include "vhex.h"

#include "vmemorystream.h"
#include "vbinaryiostream.h"
#include "vtextiostream.h"

VHexUnit::VHexUnit(bool logOnSuccess, bool throwOnError)
: VUnit("VHexUnit", logOnSuccess, throwOnError)
    {
    }

void VHexUnit::run()
    {
    // Construct a buffer and a string with all possible byte values.
    VMemoryStream    memoryStream;
    VBinaryIOStream    io(memoryStream);
    
    for (int i = 0; i < 256; ++i)
        {
        Vu8    byteValue = static_cast<Vu8>(i);
        io.writeU8(byteValue);
        }
    
    // Convert the byte values to hex and validate.
    VString    hexString;
    VHex::bufferToHexString(memoryStream.getBuffer(), CONST_S64(256), hexString);
    
    VString    zeroTo255HexString;
    VString    temp;
    for (int i = 0; i < 256; ++i)
        {
        Vu8    byteValue = static_cast<Vu8>(i);
        VHex::byteToHexString(byteValue, temp);
        zeroTo255HexString += temp;
        }
    
    this->test(hexString == zeroTo255HexString, "bufferToHexString");
    
    // Convert the hex string back to bytes and validate.
    VMemoryStream    bytes(256);
    VHex::hexStringToBuffer(hexString, bytes.getBuffer());
    bytes.setEOF(CONST_S64(256));    // EOF equal required for VMemoryStream equality test
    
    this->test(bytes == memoryStream, "hexStringToBuffer");

    // Call the hex dump function. Kinda hard to validate, but we should
    // at least call it so we'll catch any obvious crash bugs.
    VMemoryStream    dumpBuffer;
    VTextIOStream    dumpStream(dumpBuffer);
    VHex            hexDump(&dumpStream);
    
    hexDump.printHex(memoryStream.getBuffer(), 256, 0);

    }

