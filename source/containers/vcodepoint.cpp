/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
*/

/** @file */

#include "vcodepoint.h"

#include "vchar.h"
#include "vstring.h"
#include "vbinaryiostream.h"
#include "vexception.h"
#include "vhex.h"

// VCodePoint -----------------------------------------------------------------

VCodePoint::VCodePoint(int i)
    : mIntValue(i)
    , mUTF8Length(VCodePoint::_determineUTF8Length(mIntValue))
    {
}

VCodePoint::VCodePoint(char c)
    : mIntValue(VChar(c).intValue())
    , mUTF8Length(VCodePoint::_determineUTF8Length(mIntValue))
    {
}

VCodePoint::VCodePoint(const VChar& c)
    : mIntValue(c.intValue())
    , mUTF8Length(VCodePoint::_determineUTF8Length(mIntValue))
    {
}

VCodePoint::VCodePoint(const VString& hexNotation)
    : mIntValue(0)
    , mUTF8Length(0)
    {
    // If the string starts with "U+" we skip it.
    // From there we assume the rest is hexadecimal, at most 8 digits.
    int length = hexNotation.length();
    int start = 0;
    if (hexNotation.startsWith("U+")) {
        start += 2;
    }
    
    if (length - start > 8) {
        throw VRangeException(VSTRING_FORMAT("VCodePoint: attempt to construct with invalid notation '%s'.", hexNotation.chars()));
    }
    
    int valueByteIndex = 0;
    for (int index = length-1; index >= start; ) {
        VChar lowNibbleChar = hexNotation[index];
        --index;

        VChar highNibbleChar = '0';
        if (index >= start) {
            highNibbleChar = hexNotation[index];
            --index;
        }
        
        if (!highNibbleChar.isHexadecimal() || !lowNibbleChar.isHexadecimal()) {
            throw VRangeException(VSTRING_FORMAT("VCodePoint: attempt to construct with invalid notation '%s'.", hexNotation.chars()));
        }
        
        // At this point we have the two hex chars. Convert to a byte, and or it into the result at the appropriate location.
        Vs32 byteValue = (Vs32) VHex::hexCharsToByte(highNibbleChar, lowNibbleChar);
        byteValue <<= (valueByteIndex * 8);
        Vs32 mask = 0x000000FF << (valueByteIndex * 8);
        
        mIntValue |= (int) (byteValue & mask);
        
        ++valueByteIndex;
    }

    mUTF8Length = VCodePoint::_determineUTF8Length(mIntValue);
}

VCodePoint::VCodePoint(const Vu8* buffer, int startOffset)
    : mIntValue(0)
    , mUTF8Length(0)
    {
    const Vu8* source = buffer + startOffset;

    int numBytesToRead = 1;
    Vu8 source0 = source[0];
    
    // In UTF-8 the number of leading 1 bits on the first byte tells us how many "extra" bytes make up the code point in the buffer.
    if (((source0 & 0x80) != 0x00) && ((source0 & 0x40) != 0x00)) { // test for binary 11?????? (2 bits found so far)
        ++numBytesToRead;
        
        if ((source0 & 0x20) != 0x00) { // test for binary ??1????? (3 bits found so far)
            ++numBytesToRead;
        
            if ((source0 & 0x10) != 0x00) { // test for binary ???1???? (4 bits found total)
                ++numBytesToRead;
            }
        }
    }
    
    if (numBytesToRead == 1) {
        mIntValue = source0;

    } else if (numBytesToRead == 2) {
        mIntValue |= ((source0 & 0x1F) << 6);
        mIntValue |=  (source[1] & 0x3F);

    } else if (numBytesToRead == 3) {
        mIntValue |= ((source0 & 0x0F) << 12);
        mIntValue |= ((source[1] & 0x3F) << 6);
        mIntValue |=  (source[2] & 0x3F);

    } else /* numBytesToRead is 4 */ {
        mIntValue |= ((source0 & 0x07) << 18);
        mIntValue |= ((source[1] & 0x3F) << 12);
        mIntValue |= ((source[2] & 0x3F) << 6);
        mIntValue |=  (source[3] & 0x3F);
    }

    mUTF8Length = numBytesToRead;
}

VCodePoint::VCodePoint(VBinaryIOStream& stream) {
    int numBytesToRead = 1;
    Vu8 source0 = stream.readU8();
    
    // In UTF-8 the number of leading 1 bits on the first byte tells us how many "extra" bytes make up the code point in the buffer.
    if (((source0 & 0x80) != 0x00) && ((source0 & 0x40) != 0x00)) { // test for binary 11?????? (2 bits found so far)
        ++numBytesToRead;
        
        if ((source0 & 0x20) != 0x00) { // test for binary ??1????? (3 bits found so far)
            ++numBytesToRead;
        
            if ((source0 & 0x10) != 0x00) { // test for binary ???1???? (4 bits found total)
                ++numBytesToRead;
            }
        }
    }
    
    if (numBytesToRead == 1) {
        mIntValue = source0;

    } else if (numBytesToRead == 2) {
        mIntValue |= ((source0 & 0x1F) << 6);
        mIntValue |=  (stream.readU8() & 0x3F);

    } else if (numBytesToRead == 3) {
        mIntValue |= ((source0 & 0x0F) << 12);
        mIntValue |= ((stream.readU8() & 0x3F) << 6);
        mIntValue |=  (stream.readU8() & 0x3F);

    } else /* numBytesToRead is 4 */ {
        mIntValue |= ((source0 & 0x07) << 18);
        mIntValue |= ((stream.readU8() & 0x3F) << 12);
        mIntValue |= ((stream.readU8() & 0x3F) << 6);
        mIntValue |=  (stream.readU8() & 0x3F);
    }

    mUTF8Length = numBytesToRead;
}

VString VCodePoint::toString() const {
    VString s;
    
    // Use of 0x40 (decimal 64) here is to chop a number into 6-bit parts.
    // 0x40 is binary 01000000, so
    //      n / 0x40 effectively strips off the low 6 bits
    //      n % 0x40 effectively strips off all but the low 6 bits
    //      n / 0x40 % 0x40 effectively yields the "next" 6 bits by combining those two operations

    if (mIntValue < 0x80) {                             // -> result is 1 byte that encodes 7 bits untransformed
        s += (char) (mIntValue);                        // first byte binary:   0xxxxxxx (with 7 used bits)

    } else if (mIntValue < 0x00000800) {                // -> result is 2 bytes that encode 11 bits
        s += (char) (0xC0 + mIntValue / 0x40);          // first byte binary:   110xxxxx (with highest 5 bits)
        s += (char) (0x80 + mIntValue % 0x40);          // second byte binary:  10xxxxxx (with next 6 bits)

    //} else if (mIntValue - 0x0000D800 < 0x00000800) {
    //    throw VRangeException(VSTRING_FORMAT("VCodePoint::toString() for an invalid UTF-8 code point 0x%X", mIntValue));

    } else if (mIntValue < 0x00010000) {                // -> result is 3 bytes that encode 16 bits
        s += (char) (0xE0 + mIntValue / 0x1000);        // first byte binary:   1110xxxx (with highest 4 bits)
        s += (char) (0x80 + mIntValue / 0x40 % 0x40);   // second byte binary:  10xxxxxx (with next 6 bits)
        s += (char) (0x80 + mIntValue % 0x40);          // third byte binary:   10xxxxxx (with next 6 bits)

    } else if (mIntValue < 0x00110000) {                // -> result is 4 bytes that encode 21 bits
        s += (char) (0xF0 + mIntValue / 0x40000);       // first byte binary:   11110xxx (with highest 3 bits)
        s += (char) (0x80 + mIntValue / 0x1000 % 0x40); // second byte binary:  10xxxxxx (with next 6 bits)
        s += (char) (0x80 + mIntValue / 0x40 % 0x40);   // third byte binary:   10xxxxxx (with next 6 bits)
        s += (char) (0x80 + mIntValue % 0x40);          // fourth byte binary:  10xxxxxx (with next 6 bits)

    } else {
        throw VRangeException(VSTRING_FORMAT("VCodePoint::toString() for an invalid UTF-8 code point 0x%X", mIntValue));
    }

    return s;
}

// static
int VCodePoint::_determineUTF8Length(int intValue) {
    if (intValue < 0x80) {
        return 1;

    } else if (intValue < 0x00000800) {
        return 2;

    } else if (intValue < 0x00010000) {
        return 3;

    } else if (intValue < 0x00110000) {
        return 4;

    } else {
        throw VRangeException(VSTRING_FORMAT("VCodePoint::_determineUTF8Length() for an invalid UTF-8 code point 0x%X", intValue));
    }
}

