/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

/** @file */

#include "vbinaryiostream.h"

#include "vinstant.h"
#include "vexception.h"

static const Vs64 MAX_ONE_BYTE_LENGTH = CONST_S64(0x00000000000000FC);
static const Vu8 THREE_BYTE_LENGTH_INDICATOR_BYTE = 0xFF;
static const Vu8 FIVE_BYTE_LENGTH_INDICATOR_BYTE = 0xFE;
static const Vu8 NINE_BYTE_LENGTH_INDICATOR_BYTE = 0xFD;

#undef sscanf

VBinaryIOStream::VBinaryIOStream(VStream& inRawStream) :
VIOStream(inRawStream)
    {
    }

Vs8 VBinaryIOStream::readS8()
    {
    Vs8    value;
    this->readGuaranteed(reinterpret_cast<Vu8*> (&value), CONST_S64(1));
    return value;
    }

Vu8 VBinaryIOStream::readU8()
    {
    Vu8    value;
    this->readGuaranteed(&value, CONST_S64(1));
    return value;
    }

Vs16 VBinaryIOStream::readS16()
    {
    Vs16    value;
    this->readGuaranteed(reinterpret_cast<Vu8*> (&value), CONST_S64(2));
#ifdef VBYTESWAP_NEEDED    // Wrapping the NTOHS family macros in this avoids gcc "no-op"
                        // warning. Sigh. Kind of defeats the elegance of using a macro
                        // to keep the code clean and crisp.
    V_BYTESWAP_NTOHS_IN_PLACE(value);
#endif
    return value;
    }

Vu16 VBinaryIOStream::readU16()
    {
    Vu16    value;
    this->readGuaranteed(reinterpret_cast<Vu8*> (&value), CONST_S64(2));
#ifdef VBYTESWAP_NEEDED
    V_BYTESWAP_NTOHS_IN_PLACE(value);
#endif
    return value;
    }

Vs32 VBinaryIOStream::readS32()
    {
    Vs32    value;
    this->readGuaranteed(reinterpret_cast<Vu8*> (&value), CONST_S64(4));
#ifdef VBYTESWAP_NEEDED
    V_BYTESWAP_NTOHL_IN_PLACE(value);
#endif
    return value;
    }

Vu32 VBinaryIOStream::readU32()
    {
    Vu32    value;
    this->readGuaranteed(reinterpret_cast<Vu8*> (&value), CONST_S64(4));
#ifdef VBYTESWAP_NEEDED
    V_BYTESWAP_NTOHL_IN_PLACE(value);
#endif
    return value;
    }

Vs64 VBinaryIOStream::readS64()
    {
    Vs64    value;
    this->readGuaranteed(reinterpret_cast<Vu8*> (&value), CONST_S64(8));
#ifdef VBYTESWAP_NEEDED
    V_BYTESWAP_NTOH64_IN_PLACE(value);
#endif
    return value;
    }

Vu64 VBinaryIOStream::readU64()
    {
    Vu64    value;
    this->readGuaranteed(reinterpret_cast<Vu8*> (&value), CONST_S64(8));
#ifdef VBYTESWAP_NEEDED
    V_BYTESWAP_NTOH64_IN_PLACE(value);
#endif
    return value;
    }

VFloat VBinaryIOStream::readFloat()
    {
    VFloat    value;
    this->readGuaranteed(reinterpret_cast<Vu8*> (&value), CONST_S64(4));
#ifdef VBYTESWAP_NEEDED
    V_BYTESWAP_NTOHF_IN_PLACE(value);
#endif
    return value;
    }

VDouble VBinaryIOStream::readDouble()
    {
    VDouble    value;
    this->readGuaranteed(reinterpret_cast<Vu8*> (&value), CONST_S64(8));
#ifdef VBYTESWAP_NEEDED
    V_BYTESWAP_NTOHD_IN_PLACE(value);
#endif
    return value;
    }

bool VBinaryIOStream::readBool()
    {
    return (this->readU8() != 0);
    }

void VBinaryIOStream::readString(VString& s)
    {
    Vs64    length = this->readDynamicCount();

    if (length > V_MAX_S32)
        throw VException("String with unsupported length > 2GB encountered in stream.");

    s.preflight((int) length);
    this->readGuaranteed(reinterpret_cast<Vu8*> (s.buffer()), length);
    s.postflight((int) length);
    }

VString VBinaryIOStream::readString()
    {
    /*
    Note that this API is far less efficient than the one above, because
    it incurs TWO copies instead of none -- one when a temporary VString
    is created by the compiler to hold the return value, and one when
    that temporary VString is copied to the caller's lvalue.
    */

    VString    s;
    this->readString(s);
    return s;
    }

void VBinaryIOStream::readString32(VString& s)
    {
    Vs32    length = this->readS32();

    s.preflight((int) length);
    this->readGuaranteed(reinterpret_cast<Vu8*> (s.buffer()), length);
    s.postflight((int) length);
    }

VString VBinaryIOStream::readString32()
    {
    /*
    Note that this API is far less efficient than the one above, because
    it incurs TWO copies instead of none -- one when a temporary VString
    is created by the compiler to hold the return value, and one when
    that temporary VString is copied to the caller's lvalue.
    */

    VString    s;
    this->readString32(s);
    return s;
    }

void VBinaryIOStream::readInstant(VInstant& i)
    {
    Vs64 value = this->readS64();
    i.setValue(value);
    }

VInstant VBinaryIOStream::readInstant()
    {
    /*
    Note that this API is less efficient than the one above, because
    it incurs TWO copies instead of none -- one when a temporary VInstant
    is created by the compiler to hold the return value, and one when
    that temporary VInstant is copied to the caller's lvalue. (Unless the
    compiler can optimize part of that away.)
    */
    Vs64 value = this->readS64();
    return VInstant::instantFromRawValue(value);
    }

Vs64 VBinaryIOStream::readDynamicCount()
    {
    // See comments below in writeDynamicCount for the format.

    Vu8    lengthKind = this->readU8();

    if (lengthKind == THREE_BYTE_LENGTH_INDICATOR_BYTE)
        return (Vs64) this->readU16();
    else if (lengthKind == FIVE_BYTE_LENGTH_INDICATOR_BYTE)
        return (Vs64) this->readU32();
    else if (lengthKind == NINE_BYTE_LENGTH_INDICATOR_BYTE)
        return (Vs64) this->readU64();
    else
        return (Vs64) lengthKind;
    }

void VBinaryIOStream::writeS8(Vs8 i)
    {
    Vs8    value = i;
    (void) this->write(reinterpret_cast<Vu8*> (&value), CONST_S64(1));
    }

void VBinaryIOStream::writeU8(Vu8 i)
    {
    Vu8    value = i;
    (void) this->write(&value, CONST_S64(1));
    }

void VBinaryIOStream::writeS16(Vs16 i)
    {
    Vs16    value = i;
#ifdef VBYTESWAP_NEEDED
    V_BYTESWAP_HTONS_IN_PLACE(value);
#endif
    (void) this->write(reinterpret_cast<Vu8*> (&value), CONST_S64(2));
    }

void VBinaryIOStream::writeU16(Vu16 i)
    {
    Vu16    value = i;
#ifdef VBYTESWAP_NEEDED
    V_BYTESWAP_HTONS_IN_PLACE(value);
#endif
    (void) this->write(reinterpret_cast<Vu8*> (&value), CONST_S64(2));
    }

void VBinaryIOStream::writeS32(Vs32 i)
    {
    Vs32    value = i;
#ifdef VBYTESWAP_NEEDED
    V_BYTESWAP_HTONL_IN_PLACE(value);
#endif
    (void) this->write(reinterpret_cast<Vu8*> (&value), CONST_S64(4));
    }

void VBinaryIOStream::writeU32(Vu32 i)
    {
    Vu32    value = i;
#ifdef VBYTESWAP_NEEDED
    V_BYTESWAP_HTONL_IN_PLACE(value);
#endif
    (void) this->write(reinterpret_cast<Vu8*> (&value), CONST_S64(4));
    }

void VBinaryIOStream::writeS64(Vs64 i)
    {
    Vs64    value = i;
#ifdef VBYTESWAP_NEEDED
    V_BYTESWAP_HTON64_IN_PLACE(value);
#endif
    (void) this->write(reinterpret_cast<Vu8*> (&value), CONST_S64(8));
    }

void VBinaryIOStream::writeU64(Vu64 i)
    {
    Vu64    value = i;
#ifdef VBYTESWAP_NEEDED
    V_BYTESWAP_HTON64_IN_PLACE(value);
#endif
    (void) this->write(reinterpret_cast<Vu8*> (&value), CONST_S64(8));
    }

void VBinaryIOStream::writeFloat(VFloat f)
    {
    VFloat    value = f;
#ifdef VBYTESWAP_NEEDED
    V_BYTESWAP_HTONF_IN_PLACE(value);
#endif
    (void) this->write(reinterpret_cast<Vu8*> (&value), CONST_S64(4));
    }

void VBinaryIOStream::writeDouble(VDouble d)
    {
    VDouble    value = d;
#ifdef VBYTESWAP_NEEDED
    V_BYTESWAP_HTOND_IN_PLACE(value);
#endif
    (void) this->write(reinterpret_cast<Vu8*> (&value), CONST_S64(8));
    }

void VBinaryIOStream::writeBool(bool i)
    {
    this->writeU8(i ? static_cast<Vu8> (1) : static_cast<Vu8> (0));
    }

void VBinaryIOStream::writeString(const VString& s)
    {
    this->writeDynamicCount((Vs64) s.length());
    (void) this->write(reinterpret_cast<Vu8*> (s.chars()), static_cast<Vs64> (s.length()));
    }

void VBinaryIOStream::writeString32(const VString& s)
    {
    this->writeS32((Vs32) s.length());
    (void) this->write(reinterpret_cast<Vu8*> (s.chars()), static_cast<Vs64> (s.length()));
    }

void VBinaryIOStream::writeInstant(const VInstant& i)
    {
    this->writeS64(i.getValue());
    }

void VBinaryIOStream::writeDynamicCount(Vs64 count)
    {
    /*
    The idea here is use the least number of bytes possible to indicate a
    data length.
    We want to use 1 byte. Since we'll occasionally need to use that byte
    for a special indicator value, we can't quite go up to 255 in length
    indicator in 1 byte. We need 3 special indicator values, so we will use
    the values 255, 254, 253 for those. Thus in 1 byte we can indicate a
    length from 0 to 252.

    So:

    count from 0 to 252:
        write the count as 1 byte

    count over 252 but fits in 16 bits:
        first byte contains 0xFF = 255
        next two bytes contain count as a U16

    count too big for 16 bits but fits in 32 bits:
        first byte contains 0xFE = 254
        next four bytes contain count as a U32

    otherwise:
        first byte contains 0xFD = 253
        next eight bytes contain count as a U64
    */
    if (count <= MAX_ONE_BYTE_LENGTH)
        {
        this->writeU8(static_cast<Vu8> (count));
        }
    else if (count <= V_MAX_U16)
        {
        this->writeU8(THREE_BYTE_LENGTH_INDICATOR_BYTE);
        this->writeU16(static_cast<Vu16> (count));
        }
    else if (count <= V_MAX_U32)
        {
        this->writeU8(FIVE_BYTE_LENGTH_INDICATOR_BYTE);
        this->writeU32(static_cast<Vu32> (count));
        }
    else
        {
        this->writeU8(NINE_BYTE_LENGTH_INDICATOR_BYTE);
        this->writeU64(static_cast<Vu64> (count));
        }
    }

// static
int VBinaryIOStream::getDynamicCountLength(Vs64 count)
    {
    if (count <= MAX_ONE_BYTE_LENGTH)
        {
        return 1;
        }
    else if (count <= V_MAX_U16)
        {
        return 3;
        }
    else if (count <= V_MAX_U32)
        {
        return 5;
        }
    else
        {
        return 9;
        }
    }

