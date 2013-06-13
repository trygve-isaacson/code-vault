/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

#ifndef vbinaryiostream_h
#define vbinaryiostream_h

/** @file */

#include "viostream.h"

class VString;
class VInstant;
class VDuration;

/**
    @ingroup viostream_derived
*/

/**
VBinaryIOStream is a concrete VIOStream subclass that provides well-typed
stream i/o using network byte order for its data.

Because it uses network byte order, you can use VBinaryIOStream to read
and write data to sockets and files, and be guaranteed that the data
can travel across different host processors with different native byte
ordering, and be processed correctly. For example, you can have a network
client on an X86 processor communcating with a server on a Sun SPARC or
Motorola PowerPC processor. Or, you can write a file on such a client,
and manually move it to such a server and read it correctly. It also means
that Java code, which uses network byte order inherently, can use the
file format and wire protocols easily, without byte order work. This is the
key to platform-independent data representation. The byte order transformation
is handled inside this class.

The read methods all throw a VException if the data cannot be read, and in
particular throw a VEOFException if EOF is encountered (but a read on a socket
stream will normally just block rather than acting like EOF has been
encountered).

The network byte order representation of data types, implemented by this
class, is as follows:

<table>
<tr>
    <td>type</td>
    <td>format</td>
</tr>
<tr>
    <td>Vs8 and Vu8</td>
    <td>1 byte</td>
</tr>
<tr>
    <td>Vs16 and Vu16</td>
    <td>2 bytes in network byte order (big endian)</td>
</tr>
<tr>
    <td>Vs32 and Vu32</td>
    <td>4 bytes in network byte order (big endian)</td>
</tr>
<tr>
    <td>int</td>
    <td>The readInt32/writeInt32 APIs are provided for convenience with
     the int data type, and use 4 bytes in network byte order (big endian)
     exactly like Vs32; the API names are intended to be completely
     clear about the number of bits used</td>
</tr>
<tr>
    <td>Vs64 and Vu64</td>
    <td>8 bytes in network byte order (big endian)</td>
</tr>
<tr>
    <td>VFloat</td>
    <td>4 bytes in network byte order (big endian)</td>
</tr>
<tr>
    <td>VDouble</td>
    <td>8 bytes in network byte order (big endian)</td>
</tr>
<tr>
    <td>bool</td>
    <td>1 byte: 0 for false, 1 for true</td>
</tr>
<tr>
    <td>VString</td>
    <td>A length indicator "n", followed by n bytes of text. The length
        indicator is dynamically sized to avoid wasting space for small
        items, while allowing huge items. For example, an empty string
        takes 1 byte, and short strings have only 1 byte of overhead
        to indicate the length. See below.</td>
</tr>
<tr>
    <td>"String 32"</td>
    <td>A Vs32 (4 bytes) length indicator "n", followed by n bytes of text.
        Unlike VString described above, this form always has a length indicator
        that is 4 bytes long, regardless of string length. Thus an empty string
        takes 4 bytes, and every string has 4 bytes of overhead to indicate the
        length.</td>
</tr>
<tr>
    <td>VInstant</td>
    <td>The VInstant's internal Vs64 value.</td>
</tr>
<tr>
    <td>VDuration</td>
    <td>The VDuration's internal Vs64 value.</td>
</tr>
</table>

The dynamically sized length indicator mode for VString allows short strings
to have their length encoded in 1 byte, while still allowing for strings
that exceed even 64K to still have their length encoded:

<table>
<tr>
    <td>
    Data Length
    </td>
    <td>
    Length Indicator Length
    </td>
    <td>
    Length Indicator Format
    </td>
</tr>
<tr>
    <td>
    n &lt;= 0x7F
    </td>
    <td>
    1 byte
    </td>
    <td>
    1 length byte (S8)
    </td>
</tr>
<tr>
    <td>
    0x80 &lt;= n &lt;= 0x7FFF
    </td>
    <td>
    3 bytes
    </td>
    <td>
    indicator byte = -1, followed by 2 length bytes (S16)
    </td>
</tr>
<tr>
    <td>
    0x8000 &lt;= n &lt;= 0x7FFFFFFF
    </td>
    <td>
    5 bytes
    </td>
    <td>
    indicator byte = -2, followed by 4 length bytes (S32)
    </td>
</tr>
<tr>
    <td>
    0x80000000 &lt;= n
    </td>
    <td>
    9 bytes
    </td>
    <td>
    indicator byte = -3, followed by 8 length bytes (S64)
    </td>
</tr>
</table>

Of course, since VString uses an "int" to describe its length, it does not
inherently support 64-bit length strings. This is reasonable since a
string is not a suitable data structure for data larger than 4 gigabytes!
You can still use arbitrary buffers and VMemoryStream objects for such large
data, just not VString.
*/
class VBinaryIOStream : public VIOStream {
    public:

        /**
        Constructs the object with an underlying raw stream.
        @param    rawStream    the raw stream on which I/O will be performed
        */
        VBinaryIOStream(VStream& rawStream);
        /**
        Destructor.
        */
        virtual ~VBinaryIOStream() {}

        /**
        Reads a signed 8-bit value from the stream.
        @return    the Vs8
        */
        Vs8 readS8();
        /**
        Reads an unsigned 8-bit value from the stream.
        @return    the Vu8
        */
        Vu8 readU8();
        /**
        Reads a signed 16-bit value from the stream.
        @return    the Vs16
        */
        Vs16 readS16();
        /**
        Reads an unsigned 16-bit value from the stream.
        @return    the Vu16
        */
        Vu16 readU16();
        /**
        Reads a signed 32-bit value from the stream.
        @return    the Vs32
        */
        Vs32 readS32();
        /**
        Reads a signed 32-bit value from the stream, and casts the return value to int.
        This is useful if you use int in your code and the data in question is known to
        fit in an int value, so that you do not have truncation concerns.
        @return    the Vs32 as int
        */
        int readInt32();
        /**
        Reads an unsigned 32-bit value from the stream.
        @return    the Vu32
        */
        Vu32 readU32();
        /**
        Reads a signed 64-bit value from the stream.
        @return    the Vs64
        */
        Vs64 readS64();
        /**
        Reads an unsigned 64-bit value from the stream.
        @return    the Vu64
        */
        Vu64 readU64();
        /**
        Reads a single-precision floating-point value from the stream.
        @see VBinaryIOStream::writeFloat
        @return    the VFloat
        */
        VFloat readFloat();
        /**
        Reads a double-precision floating-point value from the stream.
        @see VBinaryIOStream::writeFloat
        @return    the VFloat
        */
        VDouble readDouble();
        /**
        Reads a bool value from the stream.
        @return    the bool
        */
        bool readBool();
        /**
        Reads a VString value from the stream, assuming it is prefaced by
        dynamically-sized length indicator as done in writeString.
        @param    s    a VString to format
        */
        void readString(VString& s);
        /**
        Reads a VString value from the stream, assuming it is prefaced by
        dynamically-sized length indicator as done in writeString, using a
        more natural syntax than readString(s), but incurring the overhead
        of creating two temporary copies instead of zero.
        @return    the VString value
        */
        VString readString();
        /**
        Reads a VString value from the stream, assuming it is prefaced by
        a 32-bit length indicator as done in writeString32.
        @param    s    a VString to format
        */
        void readString32(VString& s);
        /**
        Reads a VString value from the stream, assuming it is prefaced by
        a 32-bit length indicator as done in writeString32, using a
        more natural syntax than readString(s), but incurring the overhead
        (depending on the compiler) of creating a temporary copy or two
        instead of zero.
        @return    the VString value
        */
        VString readString32();
        /**
        Reads a VInstant value from the stream.
        @param    i    a VInstant to set
        */
        void readInstant(VInstant& i);
        /**
        Reads a VInstant value from the stream, using a more natural syntax
        than readInstant(i), but incurring the overhead
        (depending on the compiler) of creating a temporary copy or two
        instead of zero.
        @return    the VInstant value
        */
        VInstant readInstant();
        /**
        Reads a VDuration value from the stream.
        @param    d    a VDuration to set
        */
        void readDuration(VDuration& d);
        /**
        Reads a VDuration value from the stream, using a more natural syntax
        than readDuration(d), but incurring the overhead
        (depending on the compiler) of creating a temporary copy or two
        instead of zero.
        @return    the VDuration value
        */
        VDuration readDuration();
        /**
        Reads a length/count/size indicator that has been dynamically sized
        via writeDynamicCount.
        @return the count value
        */
        Vs64 readDynamicCount();

        /**
        Writes a signed 8-bit value to the stream.
        @param    i    the Vs8
        */
        void writeS8(Vs8 i);
        /**
        Writes an unsigned 8-bit value to the stream.
        @param    i    the Vu8
        */
        void writeU8(Vu8 i);
        /**
        Writes a signed 16-bit value to the stream.
        @param    i    the Vs16
        */
        void writeS16(Vs16 i);
        /**
        Writes an unsigned 16-bit value to the stream.
        @param    i    the Vu16
        */
        void writeU16(Vu16 i);
        /**
        Writes a signed 32-bit value to the stream.
        @param    i    the Vs32
        */
        void writeS32(Vs32 i);
        /**
        Writes a signed 32-bit value to the stream, casting from the supplied int.
        This is useful if you use int in your code and the data in question is known to
        fit in a signed 32-bit value, so that you do not have truncation concerns.
        @param    i    the int to be written as a Vs32 value
        */
        void writeInt32(int i);
        /**
        Writes a signed 32-bit value to the stream, casting from the supplied size_type.
        This is useful if you have a size_type or STL collection in your code and the data
        in question is known to fit in a signed 32-bit value, so that you do not have
        truncation concerns.
        @param    i    the int to be written as a Vs32 value
        */
        void writeSize32(VSizeType i);
        /**
        Writes an unsigned 32-bit value to the stream.
        @param    i    the Vu32
        */
        void writeU32(Vu32 i);
        /**
        Writes a signed 64-bit value to the stream.
        @param    i    the Vs64
        */
        void writeS64(Vs64 i);
        /**
        Writes an unsigned 64-bit value to the stream.
        @param    i    the Vu64
        */
        void writeU64(Vu64 i);
        /**
        Writes a single-precision floating-point value to the stream.
        @param    f            the VFloat
        */
        void writeFloat(VFloat f);
        /**
        Writes a double-precision floating-point value to the stream.
        @param    d            the VDouble
        */
        void writeDouble(VDouble d);
        /**
        Writes a bool value to the stream.
        @param    i    the bool
        */
        void writeBool(bool i);
        /**
        Writes a VString value to the stream prefaced by dynamically-sized length indicator.
        The length indicator is 1 byte for strings of length 252 or less, and is larger if
        the string length needs more bytes to be represented.
        @param    s    the VString
        */
        void writeString(const VString& s);
        /**
        Writes a VString value to the stream prefaced by a 32-bit length indicator.
        @param    s    the VString
        */
        void writeString32(const VString& s);
        /**
        Writes a VInstant value to the stream.
        @param    i    the VInstant
        */
        void writeInstant(const VInstant& i);
        /**
        Writes a VDuration value to the stream.
        @param    d    the VDuration
        */
        void writeDuration(const VDuration& d);
        /**
        Writes a length/count/size indicator that is dynamically sized to fit
        the actual value, with an encoding so that it can be read. The purpose is to be
        as compact as possible for typical small counts, while still allowing
        for arbitrarily large values that might be encountered less frequently.
        @param    count    the count value
        */
        void writeDynamicCount(Vs64 count);

        /**
        Returns the number of bytes that the specified count value would take in
        a stream when streamed using the dynamic count format.
        @param    count    the count value
        @return the number of bytes that count would occupy in a stream
        */
        static int getDynamicCountLength(Vs64 count);

    private:

        // Prevent copy construction and assignment since there is no provision for sharing a raw stream.
        VBinaryIOStream(const VBinaryIOStream& other);
        VBinaryIOStream& operator=(const VBinaryIOStream& other);

};

#endif /* vbinaryiostream_h */
