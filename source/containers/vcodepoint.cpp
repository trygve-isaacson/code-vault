/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
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
    , mUTF8Length(VCodePoint::getUTF8LengthFromCodePointValue(mIntValue))
    , mUTF16Length(VCodePoint::getUTF16LengthFromCodePointValue(mIntValue))
    {
}

VCodePoint::VCodePoint(char c)
    : mIntValue(VChar(c).intValue())
    , mUTF8Length(VCodePoint::getUTF8LengthFromCodePointValue(mIntValue))
    , mUTF16Length(VCodePoint::getUTF16LengthFromCodePointValue(mIntValue))
    {
}

VCodePoint::VCodePoint(const VChar& c)
    : mIntValue(c.intValue())
    , mUTF8Length(VCodePoint::getUTF8LengthFromCodePointValue(mIntValue))
    , mUTF16Length(VCodePoint::getUTF16LengthFromCodePointValue(mIntValue))
    {
}

VCodePoint::VCodePoint(const VString& hexNotation)
    : mIntValue(0)
    , mUTF8Length(0)
    , mUTF16Length(0)
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

    mUTF8Length = VCodePoint::getUTF8LengthFromCodePointValue(mIntValue);
    mUTF16Length = VCodePoint::getUTF16LengthFromCodePointValue(mIntValue);
}

VCodePoint::VCodePoint(const Vu8* buffer, int startOffset)
    : mIntValue(0)
    , mUTF8Length(0)
    , mUTF16Length(0)
    {
    const Vu8* source = buffer + startOffset;

    Vu8 source0 = source[0];
    int numBytesToRead = VCodePoint::getUTF8LengthFromUTF8StartByte(source0);
    
    this->_initFromUTF8Bytes(numBytesToRead, source0,
        (numBytesToRead > 1) ? source[1] : 0,
        (numBytesToRead > 2) ? source[2] : 0,
        (numBytesToRead > 3) ? source[3] : 0);
}

VCodePoint::VCodePoint(VBinaryIOStream& stream) {
    Vu8 source0 = stream.readU8();
    int numBytesToRead = VCodePoint::getUTF8LengthFromUTF8StartByte(source0);
    
    this->_initFromUTF8Bytes(numBytesToRead, source0,
        (numBytesToRead > 1) ? stream.readU8() : 0,
        (numBytesToRead > 2) ? stream.readU8() : 0,
        (numBytesToRead > 3) ? stream.readU8() : 0);
}

VCodePoint::VCodePoint(const std::wstring& utf16WideString, int atIndex) {
    wchar_t firstUnit = utf16WideString[atIndex];
    if (!VCodePoint::isUTF16SurrogateCodeUnit(firstUnit)) {
        mIntValue = firstUnit;
    } else {
        if (static_cast<int>(utf16WideString.length()) <= (atIndex + 1)) {
            throw VEOFException("Reached end of utf16WideString in the middle of a two-unit code point."); // Note: Stream-oriented reading is the way to avoid this case when reading in chunks.
        }
        
        this->_initFromUTF16Surrogates(firstUnit, utf16WideString[atIndex + 1]);
    }
}

VString VCodePoint::toString() const {
    VString s;
    
    // Use of 0x40 (decimal 64) here is to chop a number into 6-bit parts.
    // 0x40 is binary 01000000, so
    //      n / 0x40 effectively strips off the low 6 bits
    //      n % 0x40 effectively strips off all but the low 6 bits
    //      n / 0x40 % 0x40 effectively yields the "next" 6 bits by combining those two operations
    
    switch (VCodePoint::getUTF8LengthFromCodePointValue(mIntValue)) {

        case 1:
            s += (char) (mIntValue);                        // first byte binary:   0xxxxxxx (with 7 used bits)
            break;

        case 2:
            s += (char) (0xC0 + mIntValue / 0x40);          // first byte binary:   110xxxxx (with highest 5 bits)
            s += (char) (0x80 + mIntValue % 0x40);          // second byte binary:  10xxxxxx (with next 6 bits)
            break;

        case 3:
            s += (char) (0xE0 + mIntValue / 0x1000);        // first byte binary:   1110xxxx (with highest 4 bits)
            s += (char) (0x80 + mIntValue / 0x40 % 0x40);   // second byte binary:  10xxxxxx (with next 6 bits)
            s += (char) (0x80 + mIntValue % 0x40);          // third byte binary:   10xxxxxx (with next 6 bits)
            break;

        case 4:
            s += (char) (0xF0 + mIntValue / 0x40000);       // first byte binary:   11110xxx (with highest 3 bits)
            s += (char) (0x80 + mIntValue / 0x1000 % 0x40); // second byte binary:  10xxxxxx (with next 6 bits)
            s += (char) (0x80 + mIntValue / 0x40 % 0x40);   // third byte binary:   10xxxxxx (with next 6 bits)
            s += (char) (0x80 + mIntValue % 0x40);          // fourth byte binary:  10xxxxxx (with next 6 bits)
            break;
            
        default:
            throw VRangeException(VSTRING_FORMAT("VCodePoint::toString() for an invalid UTF-8 code point 0x%X", mIntValue));
            break;
    }

    return s;
}

std::wstring VCodePoint::toUTF16WideString() const {
    std::wstring s;
    
    switch (VCodePoint::getUTF16LengthFromCodePointValue(mIntValue)) {

        case 1:
            s += (wchar_t) (mIntValue); // first byte binary:   same as code point value
            break;

        case 2: {
            int leadSurrogate =  ((mIntValue - 0x10000) >> 10) + 0xD800;
            int trailSurrogate = ((mIntValue - 0x10000) & 0x03FF) + 0xDC00;
            s += (wchar_t) leadSurrogate;
            s += (wchar_t) trailSurrogate;
            }
            break;

        default:
            throw VRangeException(VSTRING_FORMAT("VCodePoint::toString() for an invalid UTF-8 code point 0x%X", mIntValue));
            break;
    }
    
    return s;
}

// static
int VCodePoint::getUTF8LengthFromUTF8StartByte(Vu8 startByte) {
    // In UTF-8 the number of leading 1 bits on the first byte tells us how many "extra" bytes make up the code point in the buffer.
    int utf8Length = 1;

    if (((startByte & 0x80) != 0x00) && ((startByte & 0x40) != 0x00)) { // test for binary 11?????? (2 bits found so far)
        ++utf8Length;
        
        if ((startByte & 0x20) != 0x00) { // test for binary ??1????? (3 bits found so far)
            ++utf8Length;
        
            if ((startByte & 0x10) != 0x00) { // test for binary ???1???? (4 bits found total)
                ++utf8Length;
            }
        }
    }
    
    return utf8Length;
}

// static
int VCodePoint::getUTF8LengthFromCodePointValue(int intValue) {
    if (intValue < 0x80) {
        return 1;
    } else if (intValue < 0x00000800) {
        return 2;
    } else if (intValue < 0x00010000) {
        return 3;
    } else if (intValue < 0x00110000) {
        return 4;
    } else {
        throw VRangeException(VSTRING_FORMAT("VCodePoint::getUTF8LengthFromCodePointValue() for an invalid UTF-8 code point 0x%X", intValue));
    }
}

// static
bool VCodePoint::isUTF8ContinuationByte(Vu8 byteValue) {
    // 0xC0 mask value of 0x80 (10xxxxxx) detects UTF-8 continuation bytes; anything else is start of a character (single or multi-byte).
    return ((byteValue & 0xC0) == 0x80);
}

// static
int VCodePoint::countUTF8CodePoints(const Vu8* buffer, int numBytes) {
    int numCodePoints = 0;
    int offset = 0;
    while (offset < numBytes) {
        VCodePoint cp(buffer, offset);
        ++numCodePoints;
        offset += cp.getUTF8Length();
    }

    return numCodePoints;
}

// static
int VCodePoint::getPreviousUTF8CodePointOffset(const Vu8* buffer, int offset) {
    int previousOffset = offset - 1;
    
    while ((previousOffset > 0) && VCodePoint::isUTF8ContinuationByte(buffer[previousOffset])) {
        --previousOffset;
    }
    
    return previousOffset;
}

// static
bool VCodePoint::isUTF16SurrogateCodeUnit(wchar_t codeUnit) {
    /*
    In UTF-16 two known ranges of values occupy a single code unit. Anything else uses two code units.
    The single unit ranges are:
        U+0000 to U+D7FF
        U+E000 to U+FFFF
    Therefore, only values in the remaining range indicate a lead surrogate:
        U+D800 to U+DFFF
    And anything above U+FFFF is a trail surrogate.
    */
    return
        ((codeUnit >= 0xD800) && (codeUnit <= 0xDFFF)) ||   // lead surrogate range
        (codeUnit >= 0x10000);                              // trail surrogate range
}

// static
int VCodePoint::getUTF16LengthFromCodePointValue(int intValue) {
    if (((intValue >= 0x0000) && (intValue <= 0xD7FF)) ||
        ((intValue >= 0xE000) && (intValue <= 0xFFFF))) {
        return 1;
    }

    return 2;
}

void VCodePoint::_initFromUTF8Bytes(int numBytesToUse, Vu8 byte0, Vu8 byte1, Vu8 byte2, Vu8 byte3) {
    mIntValue = 0;

    if (numBytesToUse == 1) {
        mIntValue = byte0;

    } else if (numBytesToUse == 2) {
        mIntValue |= ((byte0 & 0x1F) << 6);
        mIntValue |=  (byte1 & 0x3F);

    } else if (numBytesToUse == 3) {
        mIntValue |= ((byte0 & 0x0F) << 12);
        mIntValue |= ((byte1 & 0x3F) << 6);
        mIntValue |=  (byte2 & 0x3F);

    } else /* numBytesToUse is 4 */ {
        mIntValue |= ((byte0 & 0x07) << 18);
        mIntValue |= ((byte1 & 0x3F) << 12);
        mIntValue |= ((byte2 & 0x3F) << 6);
        mIntValue |=  (byte3 & 0x3F);
    }

    mUTF8Length = numBytesToUse;
    mUTF16Length = VCodePoint::getUTF16LengthFromCodePointValue(mIntValue);
}

void VCodePoint::_initFromUTF16Surrogates(wchar_t leadSurrogate, wchar_t trailSurrogate) {
    int x = (leadSurrogate & ((1 << 6) -1)) << 10 | trailSurrogate & ((1 << 10) -1);
    int w = (leadSurrogate >> 6) & ((1 << 5) - 1);
    int u = w + 1;
    mIntValue = u << 16 | x;

    mUTF8Length = VCodePoint::getUTF8LengthFromCodePointValue(mIntValue);
    mUTF16Length = 2;
}
