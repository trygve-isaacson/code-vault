/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vhex_h
#define vhex_h

/** @file */

#include "vstring.h"

class VTextIOStream;
class VBinaryIOStream;

/**
    @ingroup toolbox
*/

/**
VHex is mainly a namespace for some global functions converting to/from
hexadecimal strings and buffer display. You can also instantiate an object
for hex dump generation.

The static functions can be used standalone; they build up functionality
to convert between buffer and hex string, using simpler functions to
convert between shorter sequences of bytes and characters.

If you want a pretty hex dump, instantiate a VHex object with formatting
parameters, and then call its printHex() method with the desired output
object.
*/
class VHex {
    public:

        /**
        Produces a hexadecimal string representation of the specified buffer data.
        @param    buffer            pointer to the data to parse
        @param    bufferLength    the length of data to parse
        @param    s                the string to format
        @param    wantLeading0x    true if you want the string prefaced with the text "0x"
        */
        static void bufferToHexString(const Vu8* buffer, Vs64 bufferLength, VString& s, bool wantLeading0x = false);
        /**
        Produces a buffer of bytes as specified by a supplied hexadecimal string representation.
        @param    hexDigits        the hexadecimal string
        @param    buffer            the buffer to fill (must be big enough!)
        @param    hasLeading0x    true if the hexadecimal string starts with a leading "0x"
        */
        static void hexStringToBuffer(const VString& hexDigits, Vu8* buffer, bool hasLeading0x = false);

        /**
        Produces a hexadecimal string representation of the specified buffer data.
        @param    text            the string to convert
        @param    hexDigits        the returned hex string
        @param    wantLeading0x    true if you want the string prefaced with the text "0x"
        */
        static void stringToHex(const VString& text, VString& hexDigits, bool wantLeading0x = false);
        /**
        Produces a buffer of bytes as specified by a supplied hexadecimal string representation.
        @param    hexDigits        the hexadecimal string
        @param    text            the returned plain text string
        @param    hasLeading0x    true if the hexadecimal string starts with a leading "0x" that must be ignored
        */
        static void hexToString(const VString& hexDigits, VString& text, bool hasLeading0x = false);

        /**
        Produces a 2-character hexadecimal string representing the supplied byte value.
        @param    byteValue    the byte value to convert to hex
        @param    s            the string to format
        */
        static void byteToHexString(Vu8 byteValue, VString& s);
        /**
        Produces two hexadecimal characters representing the supplied byte value.
        @param    byteValue        the byte value to convert to hex
        @param    highNibbleChar    pointer to the char that will contain the high nibble hex digit
        @param    lowNibbleChar    pointer to the char that will contain the low nibble hex digit
        */
        static void byteToHexChars(Vu8 byteValue, char* highNibbleChar, char* lowNibbleChar);
        /**
        Produces a byte value as specified by a supplied pair of hexadecimal chars.
        @param    twoHexDigits    pointer to buffer containing (at least) two chars
        @return the byte containing the value specified by the two hex digits
        */
        static Vu8 hexStringToByte(const char* twoHexDigits);
        /**
        Produces a byte value as specified by a supplied pair of hexadecimal chars.
        @param    highNibbleChar    a hex digit char specifying the high nibble of the byte
        @param    lowNibbleChar    a hex digit char specifying the low nibble of the byte
        @return the byte containing the value specified by the two hex digits
        */
        static Vu8 hexCharsToByte(char highNibbleChar, char lowNibbleChar);

        /**
        Produces a char representing the low nibble of a supplied byte value.
        @param    nibbleValue    a byte whose low nibble will be converted to hex
        @return the hex digit representing the supplied byte's low nibble
        */
        static char nibbleToHexChar(Vu8 nibbleValue);
        /**
        Produces a byte value with a zero high nibble and a low nibble as specified by a
        supplied hex digit char.
        @param    hexChar    the hex digit
        @return a byte value whose low nibble is specified by the hexChar param
        */
        static Vu8 hexCharToNibble(char hexChar);

        /**
        Produces a string representation of the specified buffer data, where any byte in the printable
        ASCII range 0x20 to 0x7E is shown as its ASCII character, and anything else is a period ('.').
        @param    buffer        pointer to the data to parse
        @param    bufferLength  the length of data to parse
        @param    s             the string to format
        */
        static void bufferToPrintableASCIIString(const Vu8* buffer, Vs64 bufferLength, VString& s);

        /**
        This function can be used to read a hex dump and create an in-memory buffer from it. The input
        text is presumed to be in the format generated by VHex::printHex, and each time you call this
        function it will return a newly allocated buffer for the next chunk of the hex dump. A chunk is
        delineated by a blank line, so this function returns after reading a blank line, with the i/o
        mark ending at the next chunk header line. If the next line in the stream is blank when you call
        this function, the outputStream returned will be empty. If you call this repeatedly on an input
        file stream, you will eventually see a VEOFException thrown by the file reader at the end of the file.
        If this function encounters a line in an unexpected format, it will simply skip that line; this
        could allow you to annotate a hex dump.
        The lines this function expects are:
        1. Lines of hex data in the form, detected by offset label, colon, hex data, ascii data e.g.:
           00000000: 00 00 00 01 00 00 00 20 11 77 6F 72 6B 2E 69 6E    .........work.in
           Only the hex bytes are processed; end of data is indicated by more than one space.
        2. Blank lines (treated as end of chunk)
        3. Anything else. (ignored)
        */
        static void readHexDump(VTextIOStream& inputStream, VBinaryIOStream& outputStream);

        /**
        Creates a hex dump object with specified parameters.
        @param    outputStream    the text stream to print to, or NULL for stdout
        @param    numBytesPerRow    the number of data bytes to display per row of text output
        @param    indentCount        the number of spaces to lead each row with
        @param    labelsInHex        true if the offset labels should be given in hex, false for decimal
        @param    showASCIIValues    true if each byte's ASCII equivalent should be displayed
        */
        VHex(VTextIOStream* outputStream = NULL, int numBytesPerRow = 16, int indentCount = 2, bool labelsInHex = false, bool showASCIIValues = true);
        /**
        Destructor.
        */
        virtual ~VHex();

        /**
        Prints a buffer of hex data according to the settings set at construction.
        @param    buffer    pointer to the buffer of data to dump
        @param    length    the number of bytes of data to dump
        @param    offset    byte offset in the buffer of the first byte of data to be dumped
        */
        void printHex(const Vu8* buffer, Vs64 length, Vs64 offset = 0);
        /**
        Resets the object so it can be re-used for a brand new hex dump.
        */
        void reset();
        /**
        Flushes any unwritten output.
        */
        void flush();

    private:

        VHex(const VHex&); // not copyable
        VHex& operator=(const VHex&); // not assignable

        /**
        Prints the pending data to the output stream or stdout.
        */
        void _printPending();

        VTextIOStream*  mOutputStream;      ///< The stream we write to, or NULL for stdout.
        int             mNumBytesPerRow;    ///< The number of bytes of binary data per output row.
        int             mIndentCount;       ///< The number of spaces to indent each row.
        bool            mLabelsInHex;       ///< True if the offset labels should be shown in hex.
        bool            mShowASCIIValues;   ///< True if the bytes' ASCII equivalents should be shown.
        int             mStartColumn;       ///< The start column of the current row.
        Vs64            mOffset;            ///< The current offset in the data being processed.
        int             mPendingBufferUsed; ///< The amount of pending data we have accumulated w/o writing.
        Vu8*            mPendingBuffer;     ///< The buffer holding the accumulated data not yet written.
        VString         mLineBuffer;        ///< The temporary output buffer, allocated here so we don't do it for every line.
};

#endif /* vhex_h */

