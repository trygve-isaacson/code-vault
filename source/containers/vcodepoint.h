/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

#ifndef vcodepoint_h
#define vcodepoint_h

/** @file */

#include "vtypes.h"

class VChar;
class VString;
class VBinaryIOStream;
class VTextIOStream;

/**
This class stores a Unicode code point, which is similar to a 'char' except that the range of values
is vastly larger than what fits in one byte.
Because we often trade in UTF-8 (especially for VString), we have helper methods here for getting
the length of the code point if it were represented in UTF-8, as well as the ability to create a
(small) VString containing the code point in UTF-8.
Ideally, you should deprecate use of char and VChar (at least when building and examining VString
objects) in favor of VCodePoint and VString::iterator which understands UTF-8. This will make it easier
to manipulate UTF-8 VStrings with proper semantics.
*/
class VCodePoint {

    public:
    
        /**
        Creates the code point by specifying the integer value.
        For example, ASCII 'a' is 97 or 0x61, and GREEK CAPITAL LETTER OMEGA is 937 or 0x03A9.
        @param  i   the code point integer value
        */
        explicit VCodePoint(int i);
        /**
        Creates the code point by specifying a C char value. Because a char is a single byte,
        this will only work for values < 256. Non-ASCII values ( > 127) will be interpreted
        as the code point having the same value.
        @param  c   the C char whose value to treat as a code point
        */
        explicit VCodePoint(char c);
        /**
        Creates the code point by specifying a VChar, which wraps C char value. Because a char is a single byte,
        this will only work for values < 256. Non-ASCII values ( > 127) will be interpreted
        as the code point having the same value.
        @param  c   the C char whose value to treat as a code point
        */
        explicit VCodePoint(const VChar& c);
        /**
        Creates the code point by specifying the Unicode "U+" notational value. This constructor
        makes the "U+" prefix optional, though it is recommended to include it for clarity in your code.
        You do NOT need to supply an even number of digits by prepending a zero.
        For example, ASCII 'a' is "U+61", and GREEK CAPITAL LETTER OMEGA is "U+03A9".
        @param  hexNotation   the Unicode code point string to interpret
        */
        explicit VCodePoint(const VString& hexNotation);
        /**
        Creates the code point by examining a byte buffer at a specified offset, where there exists
        a valid UTF-8 formatted code point. For example, if the code point is ASCII it will be
        a single byte; otherwise, the first byte will be the start of a 1- to 4-byte UTF-8 sequence.
        @param  buffer      a pointer to a byte buffer to examine
        @param  startOffset the offset into buffer, from which to start, and where there must exist a valid UTF-8 formatted code point
        */
        explicit VCodePoint(const Vu8* buffer, int startOffset);
        /**
        Creates the code point by reading one or more bytes from the supplied stream, where the stream contains a
        valid UTF-8 formatted code point. For example, if the code point is ASCII it will be
        a single byte; otherwise, the first byte will be the start of a 1- to 4-byte UTF-8 sequence.
        Note that VCodePoint treats binary and text streams the same since UTF-8 can be viewed as a space-efficient binary encoding.
        @param  stream  the stream to read from
        */
        explicit VCodePoint(VBinaryIOStream& stream);
        /**
        Creates the code point by reading one or more bytes from the supplied stream, where the stream contains a
        valid UTF-8 formatted code point. For example, if the code point is ASCII it will be
        a single byte; otherwise, the first byte will be the start of a 1- to 4-byte UTF-8 sequence.
        Note that VCodePoint treats binary and text streams the same since UTF-8 can be viewed as a space-efficient binary encoding.
        @param  utf8Stream  the stream to read from
        */
        explicit VCodePoint(VTextIOStream& utf8Stream);
        /**
        Creates the code point by reading one or two code units from the supplied wide string, where the string
        contains a valid UTF-16 formatted code point. A UTF-16 code point may be composed of a single code unit
        for the "simpler" characters, or two code units for the rest. If the wide string ends in the middle of
        a two-unit code point, a VEOFException will be thrown if you attempt to read the split code point at the
        end of the wide string.
        @param  utf16WideString the UTF-16 encode wide string to read
        @param  atIndex         the index into the string from which to read the one or two code units
        */
        explicit VCodePoint(const std::wstring& utf16WideString, int atIndex);

        /**
        Returns the length of this code point if it is formatted as UTF-8. The answer is always in the range 1 to 4.
        @return obvious
        */
        int getUTF8Length() const { return mUTF8Length; }
        /**
        Returns the number of code units in this code point if it is formatted as UTF-16. The answer is always 1 or 2.
        @return obvious
        */
        int getUTF16Length() const { return mUTF16Length; }
        /**
        Returns the code point integer value.
        @return obvious
        */
        int intValue() const { return mIntValue; }
        /**
        Returns a VString, that is to say the UTF-8 form of the code point as a short VString of 1 to 4 bytes.
        This is how you take a code point and turn it into a string that can be inserted or appended into
        another, longer, string.
        @return the code point in UTF-8 VString form
        */
        VString toString() const;
        /**
        Returns a VChar containing the character value if it is ASCII (code points 0 through 127), or throws
        a VRangeException if not. Unless you prefer to catch the exception, you should normally call isASCII()
        before invoking this conversion. The primary use case is when you are parsing a string and looking for
        specific ASCII syntax in it; conversion to VChar lets you 'switch' on single-quoted char constants.
        @return a VChar representation of this ASCII code point
        */
        VChar toASCIIChar() const;
        /**
        Returns a std::wstring in UTF-16 format, that is to say a small array of one or two UTF-16 code units.
        This is how you take a code point and turn it into a wstring that can be inserted or appended into
        another, longer, wstring. If you need to interface with Windows "wide" APIs, wstring is used; normally
        you will just take a VString and get the entire wstring from it via VString::toUTF16().
        @return the code point in UTF-16 std::wstring form
        */
        std::wstring toUTF16WideString() const;
        /**
        Writes the code point to a binary stream in UTF-8 form (1 to 4 bytes).
        @param  stream  the stream to write to
        */
        void writeToBinaryStream(VBinaryIOStream& stream) const;
        /**
        Returns true if the code point is value zero. This corresponds to an ASCII NUL character, and might
        be useful in rare cases where you are reading a null terminator from a buffer, or when initializing
        a code point with value zero and checking it later.
        @return true if the value is zero
        */
        bool isNull() const { return mIntValue == 0; }
        /**
        The inverse of isNull().
        @return true if the value is not zero
        */
        bool isNotNull() const { return mIntValue != 0; }
        /**
        Returns true if this code point represents an ASCII value (code points 0 through 127).
        This test should always be made prior to calling toASCIIChar() since that method will
        throw a range exception if the code point is not an ASCII character, unless you intend
        to catch such exceptions.
        @return true if the value is in the range 0 to 127.
        */
        bool isASCII() const { return mUTF8Length == 1; }
        /**
        Avoid using these. This is a temporary bridge from VChar / char, in code migrating to VCodePoint.
        */
        bool isWhitespace() const;
        bool isAlpha() const;
        bool isNumeric() const;
        bool isAlphaNumeric() const;
        bool isHexadecimal() const;

        /**
        Returns true if the two code points are the same.
        */
        friend inline bool operator==(const VCodePoint& p1, const VCodePoint& p2);
        
        // Helper utility functions for dealing with UTF-8 buffers in VString and VStringIterator:
        
        /**
        Returns the UTF-8 length of a code point given the first UTF-8 byte of the code point.
        The length can be simply deduced by the value in the byte.
        @param  startByte   the first byte of a 1- to 4-byte UTF-8 code point sequence
        @return the number of bytes in the UTF-8 sequence
        */
        static int getUTF8LengthFromUTF8StartByte(Vu8 startByte);
        /**
        Returns the UTF-8 length of a code point given the code point's integer value.
        @param  intValue    a code point value
        @return the number of bytes in the corresponding UTF-8 sequence
        */
        static int getUTF8LengthFromCodePointValue(int intValue);
        /**
        Returns true if the specified byte from a UTF-8 byte stream is a continuation byte;
        that is to say, if it is not a byte value that starts a code point sequence.
        @param  byteValue   a byte from a UTF-8 byte stream
        @return true if the byte is NOT the start of a code point; false if it IS the start of a code point
        */
        static bool isUTF8ContinuationByte(Vu8 byteValue);
        /**
        Returns the number of code points in the specified UTF-8 byte stream.
        @param  buffer      the UTF-8 byte buffer to examine
        @param  numBytes    the number of bytes in the buffer to examine
        @return the number of code points counted
        */
        static int countUTF8CodePoints(const Vu8* buffer, int numBytes);
        /**
        Returns the offset of the previous UTF-8 code point start, given the offset of a given
        code point. The answer should be 1 to 4 bytes less than the specified offset, since
        UTF-8 uses 1 to 4 bytes per code point. You must not call this function with offset 0
        in a physical buffer, because the function would look "left" of the valid buffer memory.
        Of course, if the buffer parameter points inside a physical buffer (not to the start of
        one) then that caveat does not apply.
        @param  buffer  the buffer to examine
        @param  offset  the offset to start looking "left" of in the buffer
        @return the offset of the previous code point start; should in the range (offset-4) to (offset-1)
        */
        static int getPreviousUTF8CodePointOffset(const Vu8* buffer, int offset);

        /**
        Returns the UTF-16 length of a code point given the code point's integer value.
        @param  intValue    a code point value
        @return the number of code units in the corresponding UTF-16 sequence
        */
        static int getUTF16LengthFromCodePointValue(int intValue);
        /**
        Returns true if the specified code unit from a UTF-16 sequence is a surrogate;
        that is to say, if it is part of a 2-unit sequence rather than being itself a complete
        1-unit sequence. Generally you will use this to identify the lead surrogate, and then
        read the trail surrogate by getting the next unit in the sequence.
        @param  codeUnit    a code unit from a UTF-16 sequence
        @return true if the code unit is NOT a complete code point; false if it IS a surrogate
        */
        static bool isUTF16SurrogateCodeUnit(wchar_t codeUnit);

    private:
    
        /**
        For use by our constructors (typically the stream and buffer based constructors), initializes
        our internals from a byte count and up to 4 bytes of UTF-8 encoding. The caller
        should pass 0 for any bytes that are not useful; numBytesToUse indicates which bytes are useful.
        @param  numBytesToUse   range 1 to 4, indicates how many UTF-8 bytes form the code point
        @param  byte0           byte[0] of the sequence
        @param  byte1           byte[1] of the sequence, if numBytesToUse >= 2; ignored otherwise
        @param  byte2           byte[2] of the sequence, if numBytesToUse >= 3; ignored otherwise
        @param  byte3           byte[3] of the sequence, if numBytesToUse = 4; ignored otherwise
        */
        void _initFromUTF8Bytes(int numBytesToUse, Vu8 byte0, Vu8 byte1, Vu8 byte2, Vu8 byte3);
        
        /**
        For use by our constructors (typically the stream and buffer based constructors), initializes
        our internals from a parameter count and up to 2 UTF-16 code units in the form of Vu16 values.
        The caller should pass 0 for any code units that are not useful; numUnitsToUse indicates which
        code units are useful.
        @param  numUnitsToUse   range 1 to 2, indicates how many UTF-16 code units form the code point
        @param  leadSurrogate   unit[0] of the sequence
        @param  trailSurrogate  unit[1] of the sequence, if numBytesToUse = 2; ignored otherwise
        */
        void _initFromUTF16Surrogates(wchar_t leadSurrogate, wchar_t trailSurrogate);
    
        int mIntValue;      ///< The Unicode integer value of the code point.
        int mUTF8Length;    ///< The number of bytes the code point will occupy in UTF-8 form. (1 to 4)
        int mUTF16Length;   ///< The number of code units the code point will occupy in UTF-16 form. (1 or 2)

        friend inline bool operator==(const VCodePoint& cp, char c);
        friend inline bool operator==(char c, const VCodePoint& cp);
};

inline bool operator==(const VCodePoint& p1, const VCodePoint& p2) { return p1.mIntValue == p2.mIntValue; }
inline bool operator==(const VCodePoint& cp, char c) { return cp.mIntValue == (int) c; }
inline bool operator==(char c, const VCodePoint& cp) { return cp.mIntValue == (int) c; }

#endif /* vcodepoint_h */
