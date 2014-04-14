/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
*/

/** @file */

#include "vbinaryiounit.h"

#include "vbinaryiostream.h"
#include "vmemorystream.h"

VBinaryIOUnit::VBinaryIOUnit(bool logOnSuccess, bool throwOnError) :
    VUnit("VBinaryIOUnit", logOnSuccess, throwOnError) {
}

void VBinaryIOUnit::run() {
    VMemoryStream    buffer;
    VBinaryIOStream    stream(buffer);

    // Note: When comparing VFloats (which are probably upcasted to VDouble in-memory)
    //    you need to force the correct comparison. Using a const does this. Alternatively,
    //    you could explicitly cast the float value.
    const VFloat kFloatValue = 3.14f;
    const VDouble kDoubleValue = 3.1415926;

    stream.writeS8(-8);
    stream.writeU8(208);    // exceeds 7 bits
    stream.writeS16(-16);
    stream.writeU16(40016);    // exceeds 15 bits
    stream.writeS32(-32L);
    stream.writeU32(4000000032UL);    // exceeds 31 bits
    stream.writeS64(CONST_S64(-64));
    stream.writeU64((static_cast<Vu64>(V_MAX_S64)) + (static_cast<Vu64>(CONST_S64(64))));      // exceeds 63 bits
    stream.writeFloat(kFloatValue);
    stream.writeDouble(kDoubleValue);
    stream.writeBool(true);
    stream.writeString("Zevon");

    (void) stream.seek0();

    Vs8     s8 = stream.readS8();
    Vu8     u8 = stream.readU8();
    Vs16    s16 = stream.readS16();
    Vu16    u16 = stream.readU16();
    Vs32    s32 = stream.readS32();
    Vu32    u32 = stream.readU32();
    Vs64    s64 = stream.readS64();
    Vu64    u64 = stream.readU64();
    VFloat  f5 = stream.readFloat();
    VDouble f6 = stream.readDouble();
    bool    b = stream.readBool();
    VString s = stream.readString();

    VUNIT_ASSERT_EQUAL_LABELED(s8, (Vs8) -8, "s8");
    VUNIT_ASSERT_EQUAL_LABELED(u8, (Vu8) 208, "u8");
    VUNIT_ASSERT_EQUAL_LABELED(s16, (Vs16) -16, "s16");
    VUNIT_ASSERT_EQUAL_LABELED(u16, (Vu16) 40016, "u16");
    VUNIT_ASSERT_EQUAL_LABELED(s32, (Vs32) -32L, "s32");
    VUNIT_ASSERT_EQUAL_LABELED(u32, (Vu32) 4000000032UL, "u32");
    VUNIT_ASSERT_EQUAL_LABELED(s64, (Vs64) CONST_S64(-64), "s64");
    VUNIT_ASSERT_EQUAL_LABELED(u64, (Vu64) static_cast<Vu64>(V_MAX_S64) + CONST_U64(64), "u64");
    VUNIT_ASSERT_EQUAL_LABELED(f5, kFloatValue, "float");
    VUNIT_ASSERT_EQUAL_LABELED(f6, kDoubleValue, "double");
    VUNIT_ASSERT_EQUAL_LABELED(b, true, "bool");
    VUNIT_ASSERT_EQUAL_LABELED(s, "Zevon", "string");

    // Let's also verify a known 64-bit double-precision binary layout,
    // so that we catch any future platform oddities.
    VDouble knownDouble = 3.1415926;
    stream.seek0();
    stream.writeDouble(knownDouble);
    stream.seek0();
    Vu8 bytes[8];
    bytes[0] = stream.readU8();
    bytes[1] = stream.readU8();
    bytes[2] = stream.readU8();
    bytes[3] = stream.readU8();
    bytes[4] = stream.readU8();
    bytes[5] = stream.readU8();
    bytes[6] = stream.readU8();
    bytes[7] = stream.readU8();
    VUNIT_ASSERT_EQUAL_LABELED(bytes[0], (Vu8) 0x40, "double byte[0]");
    VUNIT_ASSERT_EQUAL_LABELED(bytes[1], (Vu8) 0x09, "double byte[1]");
    VUNIT_ASSERT_EQUAL_LABELED(bytes[2], (Vu8) 0x21, "double byte[2]");
    VUNIT_ASSERT_EQUAL_LABELED(bytes[3], (Vu8) 0xFB, "double byte[3]");
    VUNIT_ASSERT_EQUAL_LABELED(bytes[4], (Vu8) 0x4D, "double byte[4]");
    VUNIT_ASSERT_EQUAL_LABELED(bytes[5], (Vu8) 0x12, "double byte[5]");
    VUNIT_ASSERT_EQUAL_LABELED(bytes[6], (Vu8) 0xD8, "double byte[6]");
    VUNIT_ASSERT_EQUAL_LABELED(bytes[7], (Vu8) 0x4A, "double byte[7]");
}

