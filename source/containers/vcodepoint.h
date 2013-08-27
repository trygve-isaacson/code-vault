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
        VCodePoint(int i);
        /**
        Creates the code point by specifying a C char value. Because a char is a single byte,
        this will only work for values < 256. Non-ASCII values ( > 127) will be interpreted
        as the code point having the same value.
        @param  c   the C char whose value to treat as a code point
        */
        VCodePoint(char c);
        /**
        Creates the code point by specifying a VChar, which wraps C char value. Because a char is a single byte,
        this will only work for values < 256. Non-ASCII values ( > 127) will be interpreted
        as the code point having the same value.
        @param  c   the C char whose value to treat as a code point
        */
        VCodePoint(const VChar& c);
        /**
        Creates the code point by specifying the Unicode "U+" notational value. This constructor
        makes the "U+" prefix optional, though it is recommended to include it for clarity in your code.
        You do NOT need to supply an even number of digits by prepending a zero.
        For example, ASCII 'a' is "U+61", and GREEK CAPITAL LETTER OMEGA is "U+03A9".
        @param  hexNotation   the Unicode code point string to interpret
        */
        VCodePoint(const VString& hexNotation);
        /**
        Creates the code point by examining a byte buffer at a specified offset, where there exists
        a valid UTF-8 formatted code point. For example, if the code point is ASCII it will be
        a single byte; otherwise, the first byte will be the start of a 1- to 4-byte UTF-8 sequence.
        @param  buffer      a pointer to a byte buffer to examine
        @param  startOffset the offset into buffer, from which to start, and where there must exist a valid UTF-8 formatted code point
        */
        VCodePoint(const Vu8* buffer, int startOffset);
        /**
        Creates the code point by reading one or more bytes from the supplied stream, where the
        stream contains a valid UTF-8 formatted code point. For example, if the code point is ASCII it will be
        a single byte; otherwise, the first byte will be the start of a 1- to 4-byte UTF-8 sequence.
        @param  stream  the stream to read from
        */
        VCodePoint(VBinaryIOStream& stream);

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
        Returns a VString, that is the UTF-8 form of the code point as a short string of 1 to 4 bytes.
        This is how you take a code point and turn it into a string that can be inserted or appended into
        another, longer, string.
        @return the code point in UTF-8 string form
        */
        VString toString() const;

        /**
        Returns true if the two code points are the same.
        */
        friend inline bool operator==(const VCodePoint& p1, const VCodePoint& p2);

    private:
    
        static int _determineUTF8Length(int intValue); ///< Called at end of each constructor, determines number of bytes used for an mIntValue in UTF-8 form.
    
        int mIntValue;      ///< The Unicode integer value of the code point.
        int mUTF8Length;    ///< The number of bytes the code point will occupy in UTF-8 form.
};

inline bool operator==(const VCodePoint& p1, const VCodePoint& p2) { return p1.mIntValue == p2.mIntValue; }

#endif /* vcodepoint_h */
