/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
*/

#ifndef vcodepoint_h
#define vcodepoint_h

/** @file */

#include "vtypes.h"

class VChar;
class VString;
class VBinaryIOStream;

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
        Creates the code point by reading one or more bytes from the supplied stream, where the
        stream contains a valid UTF-8 formatted code point. For example, if the code point is ASCII it will be
        a single byte; otherwise, the first byte will be the start of a 1- to 4-byte UTF-8 sequence.
        @param  stream  the stream to read from
        */
        explicit VCodePoint(VBinaryIOStream& stream);

        /**
        Returns the length of this code point if it is formatted as UTF-8. The answer is always in the range 1 to 4.
        @return obvious
        */
        int getUTF8Length() const { return mUTF8Length; }
        /**
        Returns the code point integer value.
        @return obvious
        */
        int intValue() const { return mIntValue; }
        /**
        Returns a VString, that is the UTF-8 form of the code point as a short VString of 1 to 4 bytes.
        This is how you take a code point and turn it into a string that can be inserted or appended into
        another, longer, string.
        @return the code point in UTF-8 VString form
        */
        VString toString() const;

        /**
        Returns true if the two code points are the same.
        */
        friend inline bool operator==(const VCodePoint& p1, const VCodePoint& p2);
        
        // Helper utility functions:
        
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
        Returns true if the specified byte from a UTF-8 byte stream is a a continuation byte;
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

    private:
    
        /**
        For use by our constructors (typically the stream and buffer based constructors), initializes
        mIntValue and mUTF8Length from a byte count and up to 4 bytes. The callers should pass 0 for
        any bytes that are not useful; numBytesToUse indicates which bytes are useful.
        @param  numBytesToUse   range 1 to 4, indicates how many bytes form the code point
        @param  byte0           byte[0] of the sequence
        @param  byte1           byte[1] of the sequence, if numBytesToUse >= 2; ignored otherwise
        @param  byte2           byte[2] of the sequence, if numBytesToUse >= 3; ignored otherwise
        @param  byte3           byte[3] of the sequence, if numBytesToUse = 4; ignored otherwise
        */
        void _initFromBytes(int numBytesToUse, Vu8 byte0, Vu8 byte1, Vu8 byte2, Vu8 byte3);
    
        int mIntValue;      ///< The Unicode integer value of the code point.
        int mUTF8Length;    ///< The number of bytes the code point will occupy in UTF-8 form.
};

inline bool operator==(const VCodePoint& p1, const VCodePoint& p2) { return p1.mIntValue == p2.mIntValue; }

#endif /* vcodepoint_h */
