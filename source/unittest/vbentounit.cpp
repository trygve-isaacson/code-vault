/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

/** @file */

#include "vbentounit.h"
#include "vbento.h"
#include "vexception.h"
#include "vchar.h"

VBentoUnit::VBentoUnit(bool logOnSuccess, bool throwOnError) :
    VUnit("VBentoUnit", logOnSuccess, throwOnError) {
}

#define ATTRIBUTE_NAME_S8 "signed-8bit-attribute"
#define ATTRIBUTE_NAME_U8 "unsigned-8bit-attribute"
#define ATTRIBUTE_NAME_S16 "signed-16bit-attribute"
#define ATTRIBUTE_NAME_U16 "unsigned-16bit-attribute"
#define ATTRIBUTE_NAME_S32 "signed-32bit-attribute"
#define ATTRIBUTE_NAME_U32 "unsigned-32bit-attribute"
#define ATTRIBUTE_NAME_S64 "signed-64bit-attribute"
#define ATTRIBUTE_NAME_U64 "unsigned-64bit-attribute"
#define ATTRIBUTE_NAME_BOOL "bool-attribute"
#define ATTRIBUTE_NAME_STRING "string-attribute"
#define ATTRIBUTE_NAME_STRING_WITH_ENCODING "string-with-encoding-attribute"
#define ATTRIBUTE_NAME_LONG_STRING "long-string-attribute"
#define ATTRIBUTE_NAME_EMPTY_STRING "empty-string-attribute"
#define ATTRIBUTE_NAME_INT "int-attribute"
#define ATTRIBUTE_NAME_FLOAT "float-attribute"
#define ATTRIBUTE_NAME_DOUBLE "double-attribute"
#define ATTRIBUTE_NAME_CHAR "char-attribute"
#define ATTRIBUTE_NAME_NULL_CHAR "null-char-attribute"
#define ATTRIBUTE_NAME_DURATION "duration-attribute"
#define ATTRIBUTE_NAME_INSTANT "instant-attribute"
#define ATTRIBUTE_NAME_SIZE "size-attribute"
#define ATTRIBUTE_NAME_ISIZE "isize-attribute"
#define ATTRIBUTE_NAME_POINT "point-attribute"
#define ATTRIBUTE_NAME_IPOINT "ipoint-attribute"
#define ATTRIBUTE_NAME_LINE "line-attribute"
#define ATTRIBUTE_NAME_ILINE "iline-attribute"
#define ATTRIBUTE_NAME_RECT "rect-attribute"
#define ATTRIBUTE_NAME_IRECT "irect-attribute"
#define ATTRIBUTE_NAME_POLYGON "polygon-attribute"
#define ATTRIBUTE_NAME_IPOLYGON "ipolygon-attribute"
#define ATTRIBUTE_NAME_COLOR "color-attribute"
#define ATTRIBUTE_NAME_BINARY_1 "binary-data-attribute-1"
#define ATTRIBUTE_NAME_BINARY_2 "binary-data-attribute-2"
#define ATTRIBUTE_NAME_CHILD_INT "child-int-attribute"
#define ATTRIBUTE_NAME_ARRAY_INT "child-array-int-attribute"
#define ATTRIBUTE_NAME_SETTER_INT "setter-int-attribute"

#define ATTRIBUTE_NAME_S8_ARRAY "signed-8bit-array-attribute"
#define ATTRIBUTE_NAME_U8_ARRAY "unsigned-8bit-array-attribute"
#define ATTRIBUTE_NAME_S16_ARRAY "signed-16bit-array-attribute"
#define ATTRIBUTE_NAME_U16_ARRAY "unsigned-16bit-array-attribute"
#define ATTRIBUTE_NAME_S32_ARRAY "signed-32bit-array-attribute"
#define ATTRIBUTE_NAME_U32_ARRAY "unsigned-32bit-array-attribute"
#define ATTRIBUTE_NAME_S64_ARRAY "signed-64bit-array-attribute"
#define ATTRIBUTE_NAME_U64_ARRAY "unsigned-64bit-array-attribute"
#define ATTRIBUTE_NAME_BOOL_ARRAY "bool-array-attribute"
#define ATTRIBUTE_NAME_STRING_ARRAY "string-array-attribute"
#define ATTRIBUTE_NAME_INT_ARRAY "int-array-attribute"
#define ATTRIBUTE_NAME_FLOAT_ARRAY "float-array-attribute"
#define ATTRIBUTE_NAME_DOUBLE_ARRAY "double-array-attribute"
#define ATTRIBUTE_NAME_CHAR_ARRAY "char-array-attribute"
#define ATTRIBUTE_NAME_DURATION_ARRAY "duration-array-attribute"
#define ATTRIBUTE_NAME_INSTANT_ARRAY "instant-array-attribute"

static const VFloat kTestFloatValue = 3.14f;
static const VDouble kTestDoubleValue = 3.14159; // note: use of more than 6 decimal places here will cause the text i/o conversion test to fail because only 6 decimal places are written out
static const VDuration kTestDurationValue = VDuration::MILLISECOND() * 42;
static const VDateAndTime kTestDateAndTimeValue = VDateAndTime(2007, 04, 20, 7, 56, 23, 986);
static VInstant gTestInstantValue;
static VPolygon gTestPolygonValue;
static VIPolygon gTestIPolygonValue;
static VMemoryStream gTestBinaryData1;
static VMemoryStream gTestBinaryData2;
static VInstant gInstantNoon2005June1UTC;

#define ATTRIBUTE_VALUE_S8 ((Vs8)100)
#define ATTRIBUTE_VALUE_U8 ((Vu8)200)
#define ATTRIBUTE_VALUE_S16 ((Vs16)300)
#define ATTRIBUTE_VALUE_U16 ((Vu16)400)
#define ATTRIBUTE_VALUE_S32 ((Vs32)500)
#define ATTRIBUTE_VALUE_U32 ((Vu32)600)
#define ATTRIBUTE_VALUE_S64 CONST_S64(700)
#define ATTRIBUTE_VALUE_U64 CONST_U64(800)
#define ATTRIBUTE_VALUE_BOOL true
#define ATTRIBUTE_VALUE_STRING "a simple string"
#define ATTRIBUTE_VALUE_ENCODED_STRING "an encoded string"
#define ATTRIBUTE_VALUE_STRING_ENCODING "US-ASCII"
#define ATTRIBUTE_VALUE_LONG_STRING "This is a string that needs to be longer than 252 characters in order to test the dynamic length prefix capability, where short data has a single byte length descriptor but longer data uses longer length descriptors. By making this string longer than 252 characters, it means that its length descriptor in the bento binary stream will require three bytes rather than one. But most strings only need one byte to describe their length."
#define ATTRIBUTE_VALUE_EMPTY_STRING VString::EMPTY()
#define ATTRIBUTE_VALUE_INT 900
#define ATTRIBUTE_VALUE_FLOAT kTestFloatValue
#define ATTRIBUTE_VALUE_DOUBLE kTestDoubleValue
#define ATTRIBUTE_VALUE_CHAR '!'
#define ATTRIBUTE_VALUE_NULL_CHAR VChar::NULL_CHAR()
#define ATTRIBUTE_VALUE_DURATION kTestDurationValue
#define ATTRIBUTE_VALUE_INSTANT gTestInstantValue
#define ATTRIBUTE_VALUE_SIZE VSize(123.456, 567.890)
#define ATTRIBUTE_VALUE_ISIZE VISize(135, 246)
#define ATTRIBUTE_VALUE_POINT VPoint(234.567, 345.678)
#define ATTRIBUTE_VALUE_IPOINT VIPoint(468, 986)
#define ATTRIBUTE_VALUE_LINE VLine(34.567, 45.678, 56.789, 67.890)
#define ATTRIBUTE_VALUE_ILINE VILine(56, 67, 78, 89)
#define ATTRIBUTE_VALUE_RECT VRect(42.567, 24.234, 10.2, 20.4)
#define ATTRIBUTE_VALUE_IRECT VIRect(300, 350, 20, 25)
#define ATTRIBUTE_VALUE_POLYGON gTestPolygonValue
#define ATTRIBUTE_VALUE_IPOLYGON gTestIPolygonValue
#define ATTRIBUTE_VALUE_COLOR VColor(20, 40, 60, 80)
#define ATTRIBUTE_VALUE_CHILD_INT 1000
#define ATTRIBUTE_VALUE_BINARY_1_TEXT "This is binary data #1 formed by a string."
#define ATTRIBUTE_VALUE_BINARY_2_TEXT "This is binary data #2, also formed by a string, but a different one."
#define ATTRIBUTE_VALUE_STRING_ARRAY_TEXT0 "one"
#define ATTRIBUTE_VALUE_STRING_ARRAY_TEXT1 "two\"quote\"two" /* test handling of embedded double-quotes, which are normally Bento text delimiters */
#define ATTRIBUTE_VALUE_STRING_ARRAY_TEXT2 "three,comma,three" /* test handling of embedded commas, which are normally Bento array element delimiters */
#define ATTRIBUTE_VALUE_SETTER_INT 2468

#define NODE_NAME_ROOT "root"
#define NODE_NAME_CHILD "child"
#define NODE_NAME_INT_ARRAY "int-array"
#define NODE_NAME_INT_ARRAY_ELEMENT "int-array-element"
#define NODE_NAME_INITIALIZED_ARRAYS "initialized_arrays"
#define NODE_NAME_ASSIGNED_ARRAYS "assigned_arrays"
#define NODE_NAME_APPENDED_ARRAYS "appended_arrays"

void VBentoUnit::run() {
    this->_verifyDynamicLengths();

    // We'll just populate a Bento container with every type, and then
    // retrieve them and validate.

    VBentoNode    root(NODE_NAME_ROOT);
    this->_buildTestData(root);
    this->_verifyContents(root, "tree");

    VString rootText;
    root.writeToBentoTextString(rootText);
    this->logStatus(rootText);

    // Test the same stuff being correctly streamed.
    VMemoryStream buffer;
    VBinaryIOStream stream(buffer);
    root.writeToStream(stream);

    stream.seek0(); // rewind to start of stream
    VBentoNode other(stream);

    this->_verifyContents(other, "stream");

    VBentoNode rootFromText;
    rootFromText.readFromBentoTextString(rootText);

    this->_verifyContents(rootFromText, "text");

    // Test comparison operators, which should compare the node name.
    // This allows a list of named nodes to be sorted by STL with no extra work.
    VBentoNode a("a");
    VBentoNode b("b");
    this->test(a < b, "a < b");
    this->test(b > a, "b > a");
    this->test(a <= b, "a <= b");
    this->test(b >= a, "b >= a");
    this->test(a >= a, "a >= a");
    this->test(b <= b, "b <= b");

    // Test assignment operator behavior. Verify that base class and subclass data are assigned.
    VBentoString source("source.name", "source.value", VString::EMPTY());
    VBentoString s2("s2.name", "s2.value", VString::EMPTY());
    s2 = source;
    VUNIT_ASSERT_EQUAL(s2.getName(), source.getName());
    VUNIT_ASSERT_EQUAL(s2.getName(), "source.name");
    VUNIT_ASSERT_EQUAL(s2.getValue(), source.getValue());
    VUNIT_ASSERT_EQUAL(s2.getValue(), "source.value");
    VBentoString s3;
    s3 = source;
    VUNIT_ASSERT_EQUAL(s3.getName(), source.getName());
    VUNIT_ASSERT_EQUAL(s3.getName(), "source.name");
    VUNIT_ASSERT_EQUAL(s3.getValue(), source.getValue());
    VUNIT_ASSERT_EQUAL(s3.getValue(), "source.value");
}

void VBentoUnit::_verifyDynamicLengths() {
    Vs64 aOneByteLength = CONST_S64(251);
    VUNIT_ASSERT_EQUAL(CONST_S64(1), VBentoNode::_getLengthOfLength(aOneByteLength));

    Vs64 biggestOneByteLength =  CONST_S64(252);
    VUNIT_ASSERT_EQUAL(CONST_S64(1), VBentoNode::_getLengthOfLength(biggestOneByteLength));

    for (Vs64 aThreeByteLength = biggestOneByteLength + 1; aThreeByteLength < biggestOneByteLength + 10; ++aThreeByteLength) {
        VUNIT_ASSERT_EQUAL_LABELED(CONST_S64(3), VBentoNode::_getLengthOfLength(aThreeByteLength), VSTRING_S64(aThreeByteLength));
    }

    Vs64 biggestThreeByteLength =  CONST_S64(0x000000000000FFFF);
    VUNIT_ASSERT_EQUAL(CONST_S64(3), VBentoNode::_getLengthOfLength(biggestThreeByteLength));

    for (Vs64 aFiveByteLength = biggestThreeByteLength + 1; aFiveByteLength < biggestThreeByteLength + 10; ++aFiveByteLength) {
        VUNIT_ASSERT_EQUAL_LABELED(CONST_S64(5), VBentoNode::_getLengthOfLength(aFiveByteLength), VSTRING_S64(aFiveByteLength));
    }

    Vs64 biggestFiveByteLength =  CONST_S64(0x00000000FFFFFFFF);
    VUNIT_ASSERT_EQUAL(CONST_S64(5), VBentoNode::_getLengthOfLength(biggestFiveByteLength));

    for (Vs64 aNineByteLength = biggestFiveByteLength + 1; aNineByteLength < biggestFiveByteLength + 10; ++aNineByteLength) {
        VUNIT_ASSERT_EQUAL_LABELED(CONST_S64(9), VBentoNode::_getLengthOfLength(aNineByteLength), VSTRING_S64(aNineByteLength));
    }
}

void VBentoUnit::_buildTestData(VBentoNode& root) {
    gTestInstantValue.setDateAndTime(kTestDateAndTimeValue, VInstant::UTC_TIME_ZONE_ID());
    gTestPolygonValue.add(VPoint(1.1, 2.2));
    gTestPolygonValue.add(VPoint(3.3, -4.4));
    gTestPolygonValue.add(VPoint(-5.5, -6.6));
    gTestPolygonValue.add(VPoint(-7.7, 8.8));
    gTestIPolygonValue.add(VIPoint(9, 10));
    gTestIPolygonValue.add(VIPoint(11, -12));
    gTestIPolygonValue.add(VIPoint(-13, -14));
    gTestIPolygonValue.add(VIPoint(-15, 16));
    VBinaryIOStream binary1(gTestBinaryData1);
    binary1.writeString(ATTRIBUTE_VALUE_BINARY_1_TEXT);
    VBinaryIOStream binary2(gTestBinaryData2);
    binary2.writeString(ATTRIBUTE_VALUE_BINARY_2_TEXT);
    const VDateAndTime kDateAndTimeNoon2005June1(2005, 6, 1, 12, 0, 0, 0);
    gInstantNoon2005June1UTC.setDateAndTime(kDateAndTimeNoon2005June1, VInstant::UTC_TIME_ZONE_ID());

    root.addS8(ATTRIBUTE_NAME_S8, ATTRIBUTE_VALUE_S8);
    root.addU8(ATTRIBUTE_NAME_U8, ATTRIBUTE_VALUE_U8);
    root.addS16(ATTRIBUTE_NAME_S16, ATTRIBUTE_VALUE_S16);
    root.addU16(ATTRIBUTE_NAME_U16, ATTRIBUTE_VALUE_U16);
    root.addS32(ATTRIBUTE_NAME_S32, ATTRIBUTE_VALUE_S32);
    root.addU32(ATTRIBUTE_NAME_U32, ATTRIBUTE_VALUE_U32);
    root.addS64(ATTRIBUTE_NAME_S64, ATTRIBUTE_VALUE_S64);
    root.addU64(ATTRIBUTE_NAME_U64, ATTRIBUTE_VALUE_U64);
    root.addBool(ATTRIBUTE_NAME_BOOL, ATTRIBUTE_VALUE_BOOL);
    root.addString(ATTRIBUTE_NAME_STRING, ATTRIBUTE_VALUE_STRING);
    root.addString(ATTRIBUTE_NAME_STRING_WITH_ENCODING, ATTRIBUTE_VALUE_ENCODED_STRING, ATTRIBUTE_VALUE_STRING_ENCODING);
    root.addString(ATTRIBUTE_NAME_LONG_STRING, ATTRIBUTE_VALUE_LONG_STRING);
    root.addString(ATTRIBUTE_NAME_EMPTY_STRING, ATTRIBUTE_VALUE_EMPTY_STRING);
    root.addInt(ATTRIBUTE_NAME_INT, ATTRIBUTE_VALUE_INT);
    root.addFloat(ATTRIBUTE_NAME_FLOAT, ATTRIBUTE_VALUE_FLOAT);
    root.addDouble(ATTRIBUTE_NAME_DOUBLE, ATTRIBUTE_VALUE_DOUBLE);
    root.addChar(ATTRIBUTE_NAME_CHAR, VChar(ATTRIBUTE_VALUE_CHAR));
    root.addChar(ATTRIBUTE_NAME_NULL_CHAR, ATTRIBUTE_VALUE_NULL_CHAR);
    root.addDuration(ATTRIBUTE_NAME_DURATION, ATTRIBUTE_VALUE_DURATION);
    root.addInstant(ATTRIBUTE_NAME_INSTANT, ATTRIBUTE_VALUE_INSTANT);
    root.addSize(ATTRIBUTE_NAME_SIZE, ATTRIBUTE_VALUE_SIZE);
    root.addISize(ATTRIBUTE_NAME_ISIZE, ATTRIBUTE_VALUE_ISIZE);
    root.addPoint(ATTRIBUTE_NAME_POINT, ATTRIBUTE_VALUE_POINT);
    root.addIPoint(ATTRIBUTE_NAME_IPOINT, ATTRIBUTE_VALUE_IPOINT);
    root.addLine(ATTRIBUTE_NAME_LINE, ATTRIBUTE_VALUE_LINE);
    root.addILine(ATTRIBUTE_NAME_ILINE, ATTRIBUTE_VALUE_ILINE);
    root.addRect(ATTRIBUTE_NAME_RECT, ATTRIBUTE_VALUE_RECT);
    root.addIRect(ATTRIBUTE_NAME_IRECT, ATTRIBUTE_VALUE_IRECT);
    root.addPolygon(ATTRIBUTE_NAME_POLYGON, ATTRIBUTE_VALUE_POLYGON);
    root.addIPolygon(ATTRIBUTE_NAME_IPOLYGON, ATTRIBUTE_VALUE_IPOLYGON);
    root.addColor(ATTRIBUTE_NAME_COLOR, ATTRIBUTE_VALUE_COLOR);
    root.addBinary(ATTRIBUTE_NAME_BINARY_1, gTestBinaryData1.getBuffer(), gTestBinaryData1.getEOFOffset());
    root.addBinary(ATTRIBUTE_NAME_BINARY_2, gTestBinaryData2.getBuffer(), VMemoryStream::kAllocatedByOperatorNew, false /* don't adopt buffer, share it */, gTestBinaryData2.getEOFOffset(), gTestBinaryData2.getEOFOffset());
    root.setInt(ATTRIBUTE_NAME_SETTER_INT, 99);
    root.setInt(ATTRIBUTE_NAME_SETTER_INT, ATTRIBUTE_VALUE_SETTER_INT);

    VBentoNode*    childNode = new VBentoNode(NODE_NAME_CHILD); // exercise the new + addChildNode method of adding
    childNode->addS32(ATTRIBUTE_NAME_CHILD_INT, ATTRIBUTE_VALUE_CHILD_INT);
    root.addChildNode(childNode);

    // Construct an array of integers at root/intarray/value[]
    VBentoNode* intarray = root.addNewChildNode(NODE_NAME_INT_ARRAY); // exercise the addNewChildNode method of adding
    for (int i = 0; i < 10; ++i) {
        VBentoNode* arrayElement = intarray->addNewChildNode(NODE_NAME_INT_ARRAY_ELEMENT);
        arrayElement->addS32(ATTRIBUTE_NAME_ARRAY_INT, i);
    }

    /*
    TESTS FOR ARRAY ATTRIBUTE TYPES.
    */
    VBentoNode* initializedArrays = root.addNewChildNode(NODE_NAME_INITIALIZED_ARRAYS);
    VBentoNode* assignedArrays = root.addNewChildNode(NODE_NAME_ASSIGNED_ARRAYS);
    VBentoNode* appendedArrays = root.addNewChildNode(NODE_NAME_APPENDED_ARRAYS);

    // VBentoS8Array data.
    {
        Vs8Array vectorOfS8;
        vectorOfS8.push_back(0);
        vectorOfS8.push_back(1);
        vectorOfS8.push_back(2);
        VBentoS8Array* s8Array;

        s8Array = initializedArrays->addS8Array(ATTRIBUTE_NAME_S8_ARRAY, vectorOfS8);

        s8Array = assignedArrays->addS8Array(ATTRIBUTE_NAME_S8_ARRAY);
        s8Array->setValue(vectorOfS8);

        s8Array = appendedArrays->addS8Array(ATTRIBUTE_NAME_S8_ARRAY, vectorOfS8);
        s8Array->appendValues(vectorOfS8);
    }

    // VBentoS16Array data.
    {
        Vs16Array vectorOfS16;
        vectorOfS16.push_back(10);
        vectorOfS16.push_back(20);
        vectorOfS16.push_back(30);
        VBentoS16Array* s16Array;

        s16Array = initializedArrays->addS16Array(ATTRIBUTE_NAME_S16_ARRAY, vectorOfS16);

        s16Array = assignedArrays->addS16Array(ATTRIBUTE_NAME_S16_ARRAY);
        s16Array->setValue(vectorOfS16);

        s16Array = appendedArrays->addS16Array(ATTRIBUTE_NAME_S16_ARRAY, vectorOfS16);
        s16Array->appendValues(vectorOfS16);
    }

    // VBentoS32Array data.
    {
        Vs32Array vectorOfS32;
        vectorOfS32.push_back(100);
        vectorOfS32.push_back(200);
        vectorOfS32.push_back(300);
        VBentoS32Array* s32Array;

        s32Array = initializedArrays->addS32Array(ATTRIBUTE_NAME_S32_ARRAY, vectorOfS32);

        s32Array = assignedArrays->addS32Array(ATTRIBUTE_NAME_S32_ARRAY);
        s32Array->setValue(vectorOfS32);

        s32Array = appendedArrays->addS32Array(ATTRIBUTE_NAME_S32_ARRAY, vectorOfS32);
        s32Array->appendValues(vectorOfS32);
    }

    // VBentoS64Array data.
    {
        Vs64Array vectorOfS64;
        vectorOfS64.push_back(CONST_S64(1000));
        vectorOfS64.push_back(CONST_S64(2000));
        vectorOfS64.push_back(CONST_S64(3000));
        VBentoS64Array* s64Array;

        s64Array = initializedArrays->addS64Array(ATTRIBUTE_NAME_S64_ARRAY, vectorOfS64);

        s64Array = assignedArrays->addS64Array(ATTRIBUTE_NAME_S64_ARRAY);
        s64Array->setValue(vectorOfS64);

        s64Array = appendedArrays->addS64Array(ATTRIBUTE_NAME_S64_ARRAY, vectorOfS64);
        s64Array->appendValues(vectorOfS64);
    }

    // VBentoStringArray data.
    {
        VStringVector vectorOfString;
        vectorOfString.push_back(ATTRIBUTE_VALUE_STRING_ARRAY_TEXT0);
        vectorOfString.push_back(ATTRIBUTE_VALUE_STRING_ARRAY_TEXT1);
        vectorOfString.push_back(ATTRIBUTE_VALUE_STRING_ARRAY_TEXT2);
        VBentoStringArray* stringArray;

        stringArray = initializedArrays->addStringArray(ATTRIBUTE_NAME_STRING_ARRAY, vectorOfString);

        stringArray = assignedArrays->addStringArray(ATTRIBUTE_NAME_STRING_ARRAY);
        stringArray->setValue(vectorOfString);

        stringArray = appendedArrays->addStringArray(ATTRIBUTE_NAME_STRING_ARRAY, vectorOfString);
        stringArray->appendValues(vectorOfString);
    }

    // VBentoBoolArray data.
    {
        VBoolArray vectorOfBool;
        vectorOfBool.push_back(true);
        vectorOfBool.push_back(false);
        vectorOfBool.push_back(true);
        VBentoBoolArray* boolArray;

        boolArray = initializedArrays->addBoolArray(ATTRIBUTE_NAME_BOOL_ARRAY, vectorOfBool);

        boolArray = assignedArrays->addBoolArray(ATTRIBUTE_NAME_BOOL_ARRAY);
        boolArray->setValue(vectorOfBool);

        boolArray = appendedArrays->addBoolArray(ATTRIBUTE_NAME_BOOL_ARRAY, vectorOfBool);
        boolArray->appendValues(vectorOfBool);
    }

    // VBentoDoubleArray data.
    {
        VDoubleArray vectorOfDouble;
        vectorOfDouble.push_back(123.456);
        vectorOfDouble.push_back(456.789);
        vectorOfDouble.push_back(246.135);
        VBentoDoubleArray* doubleArray;

        doubleArray = initializedArrays->addDoubleArray(ATTRIBUTE_NAME_DOUBLE_ARRAY, vectorOfDouble);

        doubleArray = assignedArrays->addDoubleArray(ATTRIBUTE_NAME_DOUBLE_ARRAY);
        doubleArray->setValue(vectorOfDouble);

        doubleArray = appendedArrays->addDoubleArray(ATTRIBUTE_NAME_DOUBLE_ARRAY, vectorOfDouble);
        doubleArray->appendValues(vectorOfDouble);
    }

    // VBentoDurationArray data.
    {
        VDurationVector vectorOfDuration;
        vectorOfDuration.push_back(VDuration::MILLISECOND() * 55);
        vectorOfDuration.push_back(VDuration::SECOND() * 66);
        vectorOfDuration.push_back(VDuration::HOUR() * 77);
        VBentoDurationArray* durationArray;

        durationArray = initializedArrays->addDurationArray(ATTRIBUTE_NAME_DURATION_ARRAY, vectorOfDuration);

        durationArray = assignedArrays->addDurationArray(ATTRIBUTE_NAME_DURATION_ARRAY);
        durationArray->setValue(vectorOfDuration);

        durationArray = appendedArrays->addDurationArray(ATTRIBUTE_NAME_DURATION_ARRAY, vectorOfDuration);
        durationArray->appendValues(vectorOfDuration);
    }

    // VBentoInstantArray data.
    {
        VInstantVector vectorOfInstant;
        vectorOfInstant.push_back(VInstant::NEVER_OCCURRED());
        vectorOfInstant.push_back(VInstant::INFINITE_PAST());
        vectorOfInstant.push_back(gInstantNoon2005June1UTC);
        VBentoInstantArray* instantArray;

        instantArray = initializedArrays->addInstantArray(ATTRIBUTE_NAME_INSTANT_ARRAY, vectorOfInstant);

        instantArray = assignedArrays->addInstantArray(ATTRIBUTE_NAME_INSTANT_ARRAY);
        instantArray->setValue(vectorOfInstant);

        instantArray = appendedArrays->addInstantArray(ATTRIBUTE_NAME_INSTANT_ARRAY, vectorOfInstant);
        instantArray->appendValues(vectorOfInstant);
    }
}

void VBentoUnit::_verifyContents(const VBentoNode& node, const VString& labelPrefix) {
    // We'll call the throwing getters. We test for the correct result,
    // but also know that if the value is not found, it'll throw.
    try {
        VUNIT_ASSERT_EQUAL_LABELED(node.getName(), NODE_NAME_ROOT, labelPrefix);
        VUNIT_ASSERT_EQUAL_LABELED(node.getS8(ATTRIBUTE_NAME_S8), ATTRIBUTE_VALUE_S8, labelPrefix);
        VUNIT_ASSERT_EQUAL_LABELED(node.getU8(ATTRIBUTE_NAME_U8), ATTRIBUTE_VALUE_U8, labelPrefix);
        VUNIT_ASSERT_EQUAL_LABELED(node.getS16(ATTRIBUTE_NAME_S16), ATTRIBUTE_VALUE_S16, labelPrefix);
        VUNIT_ASSERT_EQUAL_LABELED(node.getU16(ATTRIBUTE_NAME_U16), ATTRIBUTE_VALUE_U16, labelPrefix);
        VUNIT_ASSERT_EQUAL_LABELED(node.getS32(ATTRIBUTE_NAME_S32), ATTRIBUTE_VALUE_S32, labelPrefix);
        VUNIT_ASSERT_EQUAL_LABELED(node.getU32(ATTRIBUTE_NAME_U32), ATTRIBUTE_VALUE_U32, labelPrefix);
        VUNIT_ASSERT_EQUAL_LABELED(node.getS64(ATTRIBUTE_NAME_S64), ATTRIBUTE_VALUE_S64, labelPrefix);
        VUNIT_ASSERT_EQUAL_LABELED(node.getU64(ATTRIBUTE_NAME_U64), ATTRIBUTE_VALUE_U64, labelPrefix);
        VUNIT_ASSERT_EQUAL_LABELED(node.getBool(ATTRIBUTE_NAME_BOOL), ATTRIBUTE_VALUE_BOOL, labelPrefix);
        VString s1 = node.getString(ATTRIBUTE_NAME_STRING);
        VUNIT_ASSERT_EQUAL_LABELED(s1, ATTRIBUTE_VALUE_STRING, labelPrefix);

        VString encodedString = node.getString(ATTRIBUTE_NAME_STRING_WITH_ENCODING);
        VUNIT_ASSERT_EQUAL_LABELED(encodedString, ATTRIBUTE_VALUE_ENCODED_STRING, labelPrefix);
        const VBentoAttribute* encodedStringAttribute = node._findAttribute(ATTRIBUTE_NAME_STRING_WITH_ENCODING, VBentoString::DATA_TYPE_ID());
        this->testAssertion(encodedStringAttribute != NULL, __FILE__, __LINE__, labelPrefix, "Find attribute with encoding");
        if (encodedStringAttribute != NULL) // line above will have failed unit test
            VUNIT_ASSERT_EQUAL_LABELED(static_cast<const VBentoString*>(encodedStringAttribute)->getEncoding(), ATTRIBUTE_VALUE_STRING_ENCODING, labelPrefix);

        VString s2 = node.getString(ATTRIBUTE_NAME_LONG_STRING);
        VUNIT_ASSERT_EQUAL_LABELED(s2, ATTRIBUTE_VALUE_LONG_STRING, labelPrefix);
        VString s3 = node.getString(ATTRIBUTE_NAME_EMPTY_STRING);
        VUNIT_ASSERT_EQUAL_LABELED(s3, ATTRIBUTE_VALUE_EMPTY_STRING, labelPrefix);
        VUNIT_ASSERT_EQUAL_LABELED(node.getInt(ATTRIBUTE_NAME_INT), ATTRIBUTE_VALUE_INT, labelPrefix);
        VUNIT_ASSERT_EQUAL_LABELED(node.getFloat(ATTRIBUTE_NAME_FLOAT), ATTRIBUTE_VALUE_FLOAT, labelPrefix);
        VUNIT_ASSERT_EQUAL_LABELED(node.getDouble(ATTRIBUTE_NAME_DOUBLE), ATTRIBUTE_VALUE_DOUBLE, labelPrefix);
        VUNIT_ASSERT_EQUAL_LABELED(node.getChar(ATTRIBUTE_NAME_CHAR), ATTRIBUTE_VALUE_CHAR, labelPrefix);
        VUNIT_ASSERT_EQUAL_LABELED(node.getChar(ATTRIBUTE_NAME_CHAR), VChar(ATTRIBUTE_VALUE_CHAR), labelPrefix);
        VUNIT_ASSERT_EQUAL_LABELED(node.getChar(ATTRIBUTE_NAME_NULL_CHAR), ATTRIBUTE_VALUE_NULL_CHAR, labelPrefix);
        VUNIT_ASSERT_EQUAL_LABELED(node.getDuration(ATTRIBUTE_NAME_DURATION), ATTRIBUTE_VALUE_DURATION, labelPrefix);
        VUNIT_ASSERT_EQUAL_LABELED(node.getInstant(ATTRIBUTE_NAME_INSTANT), ATTRIBUTE_VALUE_INSTANT, labelPrefix);
        this->test(node.getSize(ATTRIBUTE_NAME_SIZE) == ATTRIBUTE_VALUE_SIZE, VSTRING_FORMAT("%s size", labelPrefix.chars()));
        this->test(node.getISize(ATTRIBUTE_NAME_ISIZE) == ATTRIBUTE_VALUE_ISIZE, VSTRING_FORMAT("%s isize", labelPrefix.chars()));
        this->test(node.getPoint(ATTRIBUTE_NAME_POINT) == ATTRIBUTE_VALUE_POINT, VSTRING_FORMAT("%s point", labelPrefix.chars()));
        this->test(node.getIPoint(ATTRIBUTE_NAME_IPOINT) == ATTRIBUTE_VALUE_IPOINT, VSTRING_FORMAT("%s ipoint", labelPrefix.chars()));
        this->test(node.getLine(ATTRIBUTE_NAME_LINE) == ATTRIBUTE_VALUE_LINE, VSTRING_FORMAT("%s line", labelPrefix.chars()));
        this->test(node.getILine(ATTRIBUTE_NAME_ILINE) == ATTRIBUTE_VALUE_ILINE, VSTRING_FORMAT("%s irect", labelPrefix.chars()));
        this->test(node.getRect(ATTRIBUTE_NAME_RECT) == ATTRIBUTE_VALUE_RECT, VSTRING_FORMAT("%s rect", labelPrefix.chars()));
        this->test(node.getIRect(ATTRIBUTE_NAME_IRECT) == ATTRIBUTE_VALUE_IRECT, VSTRING_FORMAT("%s irect", labelPrefix.chars()));
        this->test(node.getPolygon(ATTRIBUTE_NAME_POLYGON) == ATTRIBUTE_VALUE_POLYGON, VSTRING_FORMAT("%s polygon", labelPrefix.chars()));
        this->test(node.getIPolygon(ATTRIBUTE_NAME_IPOLYGON) == ATTRIBUTE_VALUE_IPOLYGON, VSTRING_FORMAT("%s ipolygon", labelPrefix.chars()));
        this->test(node.getColor(ATTRIBUTE_NAME_COLOR) == ATTRIBUTE_VALUE_COLOR, VSTRING_FORMAT("%s color", labelPrefix.chars()));
        VUNIT_ASSERT_EQUAL_LABELED(node.getInt(ATTRIBUTE_NAME_SETTER_INT), ATTRIBUTE_VALUE_SETTER_INT, labelPrefix);

        // Test case-insensitivity on node and attribute names.
        VString caseInsensitiveCheckName(ATTRIBUTE_NAME_S8);
        caseInsensitiveCheckName.toUpperCase();
        VUNIT_ASSERT_EQUAL_LABELED(node.getS8(caseInsensitiveCheckName), ATTRIBUTE_VALUE_S8, labelPrefix);
        caseInsensitiveCheckName.toLowerCase();
        VUNIT_ASSERT_EQUAL_LABELED(node.getS8(caseInsensitiveCheckName), ATTRIBUTE_VALUE_S8, labelPrefix);
        caseInsensitiveCheckName = NODE_NAME_CHILD;
        caseInsensitiveCheckName.toUpperCase();
        VUNIT_ASSERT_TRUE_LABELED(node.findNode(caseInsensitiveCheckName) != NULL, labelPrefix);
        caseInsensitiveCheckName.toLowerCase();
        VUNIT_ASSERT_TRUE_LABELED(node.findNode(caseInsensitiveCheckName) != NULL, labelPrefix);

        this->test(node.getParentNode() == NULL, VSTRING_FORMAT("%s parent of root is null", labelPrefix.chars()));

        VReadOnlyMemoryStream binary1Reader = node.getBinary(ATTRIBUTE_NAME_BINARY_1);
        this->test(binary1Reader == gTestBinaryData1, VSTRING_FORMAT("%s binary 1 data equality", labelPrefix.chars()));
        // We built gTestBinaryData1 using copy semantics, so the reader's buffer must be a different one.
        this->test(binary1Reader.getBuffer() != gTestBinaryData1.getBuffer(), VSTRING_FORMAT("%s binary 1 buffer separation", labelPrefix.chars()));
        VBinaryIOStream binary1IO(binary1Reader);
        VString binary1Text;
        binary1IO.readString(binary1Text);
        this->test(binary1Text == ATTRIBUTE_VALUE_BINARY_1_TEXT, VSTRING_FORMAT("%s binary 1 text comparison", labelPrefix.chars()));

        VReadOnlyMemoryStream binary2Reader = node.getBinary(ATTRIBUTE_NAME_BINARY_2);
        this->test(binary2Reader == gTestBinaryData2, VSTRING_FORMAT("%s binary 2 data equality", labelPrefix.chars()));
        // We built gTestBinaryData2 with shared buffer semantics, but we can't compare pointers because we may have been called to
        // test the contents of a serialized+deserialized stream, or to test the original in-memory Bento tree, so our
        // reader here may or may not have a copy of the original buffer at this point, depending on how we were called.
        VBinaryIOStream binary2IO(binary2Reader);
        VString binary2Text;
        binary2IO.readString(binary2Text);
        this->test(binary2Text == ATTRIBUTE_VALUE_BINARY_2_TEXT, VSTRING_FORMAT("%s binary 2 text comparison", labelPrefix.chars()));

        const VBentoNode* child = node.findNode(NODE_NAME_CHILD);
        this->test(child != NULL, VSTRING_FORMAT("%s child", labelPrefix.chars()));
        if (child != NULL)    // in case we aren't aborting on earlier failures
            this->test(child->getS32(ATTRIBUTE_NAME_CHILD_INT) == ATTRIBUTE_VALUE_CHILD_INT, VSTRING_FORMAT("%s ch32", labelPrefix.chars()));

        this->test(child->getParentNode() == &node, VSTRING_FORMAT("%s child has correct parent", labelPrefix.chars()));

        const VBentoNode* intarray = node.findNode(NODE_NAME_INT_ARRAY);
        this->test(intarray != NULL, VSTRING_FORMAT("%s intarray", labelPrefix.chars()));
        if (intarray != NULL) {
            this->test(intarray->getNodes().size() == 10, VSTRING_FORMAT("%s intarray length", labelPrefix.chars()));
            int arrayIndex = 0;
            for (VBentoNodePtrVector::const_iterator i = intarray->getNodes().begin(); i != intarray->getNodes().end(); ++i) {
                this->test((*i)->getName() == NODE_NAME_INT_ARRAY_ELEMENT, VSTRING_FORMAT("%s intarray element name", labelPrefix.chars()));
                this->test((*i)->getS32(ATTRIBUTE_NAME_ARRAY_INT) == arrayIndex, VSTRING_FORMAT("%s intarray element %d", labelPrefix.chars(), arrayIndex));
                ++arrayIndex;
            }
        }

        const VBentoNode* initializedArrays = node.findNode(NODE_NAME_INITIALIZED_ARRAYS);
        const VBentoNode* assignedArrays = node.findNode(NODE_NAME_ASSIGNED_ARRAYS);
        const VBentoNode* appendedArrays = node.findNode(NODE_NAME_APPENDED_ARRAYS);

        // VBentoS8Array tests.
        {
            const Vs8Array& s8Array = initializedArrays->getS8Array(ATTRIBUTE_NAME_S8_ARRAY);
            this->test(s8Array.size() == 3, VSTRING_FORMAT("%s initialized s8Array size", labelPrefix.chars()));
            this->test(s8Array[0] == 0 && s8Array[1] == 1 && s8Array[2] == 2, VSTRING_FORMAT("%s initialized s8Array values", labelPrefix.chars()));
        }
        {
            const Vs8Array& s8Array = assignedArrays->getS8Array(ATTRIBUTE_NAME_S8_ARRAY);
            this->test(s8Array.size() == 3, VSTRING_FORMAT("%s assigned s8Array size", labelPrefix.chars()));
            this->test(s8Array[0] == 0 && s8Array[1] == 1 && s8Array[2] == 2, VSTRING_FORMAT("%s assigned s8Array values", labelPrefix.chars()));
        }
        {
            const Vs8Array& s8Array = appendedArrays->getS8Array(ATTRIBUTE_NAME_S8_ARRAY);
            this->test(s8Array.size() == 6, VSTRING_FORMAT("%s appended s8Array size", labelPrefix.chars()));
            this->test(s8Array[0] == 0 && s8Array[1] == 1 && s8Array[2] == 2 &&
                       s8Array[3] == 0 && s8Array[4] == 1 && s8Array[5] == 2, VSTRING_FORMAT("%s appended s8Array values", labelPrefix.chars()));
        }

        // VBentoS16Array tests.
        {
            const Vs16Array& s16Array = initializedArrays->getS16Array(ATTRIBUTE_NAME_S16_ARRAY);
            this->test(s16Array.size() == 3, VSTRING_FORMAT("%s initialized s16Array size", labelPrefix.chars()));
            this->test(s16Array[0] == 10 && s16Array[1] == 20 && s16Array[2] == 30, VSTRING_FORMAT("%s initialized s16Array values", labelPrefix.chars()));
        }
        {
            const Vs16Array& s16Array = assignedArrays->getS16Array(ATTRIBUTE_NAME_S16_ARRAY);
            this->test(s16Array.size() == 3, VSTRING_FORMAT("%s assigned s16Array size", labelPrefix.chars()));
            this->test(s16Array[0] == 10 && s16Array[1] == 20 && s16Array[2] == 30, VSTRING_FORMAT("%s assigned s16Array values", labelPrefix.chars()));
        }
        {
            const Vs16Array& s16Array = appendedArrays->getS16Array(ATTRIBUTE_NAME_S16_ARRAY);
            this->test(s16Array.size() == 6, VSTRING_FORMAT("%s appended s16Array size", labelPrefix.chars()));
            this->test(s16Array[0] == 10 && s16Array[1] == 20 && s16Array[2] == 30 &&
                       s16Array[3] == 10 && s16Array[4] == 20 && s16Array[5] == 30, VSTRING_FORMAT("%s appended s16Array values", labelPrefix.chars()));
        }

        // VBentoS32Array tests.
        {
            const Vs32Array& s32Array = initializedArrays->getS32Array(ATTRIBUTE_NAME_S32_ARRAY);
            this->test(s32Array.size() == 3, VSTRING_FORMAT("%s initialized s32Array size", labelPrefix.chars()));
            this->test(s32Array[0] == 100 && s32Array[1] == 200 && s32Array[2] == 300, VSTRING_FORMAT("%s initialized s32Array values", labelPrefix.chars()));
        }
        {
            const Vs32Array& s32Array = assignedArrays->getS32Array(ATTRIBUTE_NAME_S32_ARRAY);
            this->test(s32Array.size() == 3, VSTRING_FORMAT("%s assigned s32Array size", labelPrefix.chars()));
            this->test(s32Array[0] == 100 && s32Array[1] == 200 && s32Array[2] == 300, VSTRING_FORMAT("%s assigned s32Array values", labelPrefix.chars()));
        }
        {
            const Vs32Array& s32Array = appendedArrays->getS32Array(ATTRIBUTE_NAME_S32_ARRAY);
            this->test(s32Array.size() == 6, VSTRING_FORMAT("%s appended s32Array size", labelPrefix.chars()));
            this->test(s32Array[0] == 100 && s32Array[1] == 200 && s32Array[2] == 300 &&
                       s32Array[3] == 100 && s32Array[4] == 200 && s32Array[5] == 300, VSTRING_FORMAT("%s appended s32Array values", labelPrefix.chars()));
        }

        // VBentoS64Array tests.
        {
            const Vs64Array& s64Array = initializedArrays->getS64Array(ATTRIBUTE_NAME_S64_ARRAY);
            this->test(s64Array.size() == 3, VSTRING_FORMAT("%s initialized s64Array size", labelPrefix.chars()));
            this->test(s64Array[0] == CONST_S64(1000) && s64Array[1] == CONST_S64(2000) && s64Array[2] == CONST_S64(3000), VSTRING_FORMAT("%s initialized s64Array values", labelPrefix.chars()));
        }
        {
            const Vs64Array& s64Array = assignedArrays->getS64Array(ATTRIBUTE_NAME_S64_ARRAY);
            this->test(s64Array.size() == 3, VSTRING_FORMAT("%s assigned s64Array size", labelPrefix.chars()));
            this->test(s64Array[0] == CONST_S64(1000) && s64Array[1] == CONST_S64(2000) && s64Array[2] == CONST_S64(3000), VSTRING_FORMAT("%s assigned s64Array values", labelPrefix.chars()));
        }
        {
            const Vs64Array& s64Array = appendedArrays->getS64Array(ATTRIBUTE_NAME_S64_ARRAY);
            this->test(s64Array.size() == 6, VSTRING_FORMAT("%s appended s64Array size", labelPrefix.chars()));
            this->test(s64Array[0] == CONST_S64(1000) && s64Array[1] == CONST_S64(2000) && s64Array[2] == CONST_S64(3000) &&
                       s64Array[3] == CONST_S64(1000) && s64Array[4] == CONST_S64(2000) && s64Array[5] == CONST_S64(3000), VSTRING_FORMAT("%s appended s64Array values", labelPrefix.chars()));
        }

        // VBentoStringArray tests.
        {
            const VStringVector& stringArray = initializedArrays->getStringArray(ATTRIBUTE_NAME_STRING_ARRAY);
            this->test(stringArray.size() == 3, VSTRING_FORMAT("%s initialized stringArray size", labelPrefix.chars()));
            this->test(stringArray[0] == ATTRIBUTE_VALUE_STRING_ARRAY_TEXT0 && stringArray[1] == ATTRIBUTE_VALUE_STRING_ARRAY_TEXT1 && stringArray[2] == ATTRIBUTE_VALUE_STRING_ARRAY_TEXT2, VSTRING_FORMAT("%s initialized stringArray values", labelPrefix.chars()));
        }
        {
            const VStringVector& stringArray = assignedArrays->getStringArray(ATTRIBUTE_NAME_STRING_ARRAY);
            this->test(stringArray.size() == 3, VSTRING_FORMAT("%s assigned stringArray size", labelPrefix.chars()));
            this->test(stringArray[0] == ATTRIBUTE_VALUE_STRING_ARRAY_TEXT0 && stringArray[1] == ATTRIBUTE_VALUE_STRING_ARRAY_TEXT1 && stringArray[2] == ATTRIBUTE_VALUE_STRING_ARRAY_TEXT2, VSTRING_FORMAT("%s assigned stringArray values", labelPrefix.chars()));
        }
        {
            const VStringVector& stringArray = appendedArrays->getStringArray(ATTRIBUTE_NAME_STRING_ARRAY);
            this->test(stringArray.size() == 6, VSTRING_FORMAT("%s appended stringArray size", labelPrefix.chars()));
            this->test(stringArray[0] == ATTRIBUTE_VALUE_STRING_ARRAY_TEXT0 && stringArray[1] == ATTRIBUTE_VALUE_STRING_ARRAY_TEXT1 && stringArray[2] == ATTRIBUTE_VALUE_STRING_ARRAY_TEXT2 &&
                       stringArray[3] == ATTRIBUTE_VALUE_STRING_ARRAY_TEXT0 && stringArray[4] == ATTRIBUTE_VALUE_STRING_ARRAY_TEXT1 && stringArray[5] == ATTRIBUTE_VALUE_STRING_ARRAY_TEXT2, VSTRING_FORMAT("%s appended stringArray values", labelPrefix.chars()));
        }

        // VBentoBoolArray tests.
        {
            const VBoolArray& boolArray = initializedArrays->getBoolArray(ATTRIBUTE_NAME_BOOL_ARRAY);
            this->test(boolArray.size() == 3, VSTRING_FORMAT("%s initialized boolArray size", labelPrefix.chars()));
            this->test(boolArray[0] == true && boolArray[1] == false && boolArray[2] == true, VSTRING_FORMAT("%s initialized boolArray values", labelPrefix.chars()));
        }
        {
            const VBoolArray& boolArray = assignedArrays->getBoolArray(ATTRIBUTE_NAME_BOOL_ARRAY);
            this->test(boolArray.size() == 3, VSTRING_FORMAT("%s assigned boolArray size", labelPrefix.chars()));
            this->test(boolArray[0] == true && boolArray[1] == false && boolArray[2] == true, VSTRING_FORMAT("%s assigned boolArray values", labelPrefix.chars()));
        }
        {
            const VBoolArray& boolArray = appendedArrays->getBoolArray(ATTRIBUTE_NAME_BOOL_ARRAY);
            this->test(boolArray.size() == 6, VSTRING_FORMAT("%s appended boolArray size", labelPrefix.chars()));
            this->test(boolArray[0] == true && boolArray[1] == false && boolArray[2] == true &&
                       boolArray[3] == true && boolArray[4] == false && boolArray[5] == true, VSTRING_FORMAT("%s appended boolArray values", labelPrefix.chars()));
        }

        // VBentoDoubleArray tests.
        {
            const VDoubleArray& doubleArray = initializedArrays->getDoubleArray(ATTRIBUTE_NAME_DOUBLE_ARRAY);
            this->test(doubleArray.size() == 3, VSTRING_FORMAT("%s initialized doubleArray size", labelPrefix.chars()));
            this->test(doubleArray[0] == 123.456 && doubleArray[1] == 456.789 && doubleArray[2] == 246.135, VSTRING_FORMAT("%s initialized doubleArray values", labelPrefix.chars()));
        }
        {
            const VDoubleArray& doubleArray = assignedArrays->getDoubleArray(ATTRIBUTE_NAME_DOUBLE_ARRAY);
            this->test(doubleArray.size() == 3, VSTRING_FORMAT("%s assigned doubleArray size", labelPrefix.chars()));
            this->test(doubleArray[0] == 123.456 && doubleArray[1] == 456.789 && doubleArray[2] == 246.135, VSTRING_FORMAT("%s assigned doubleArray values", labelPrefix.chars()));
        }
        {
            const VDoubleArray& doubleArray = appendedArrays->getDoubleArray(ATTRIBUTE_NAME_DOUBLE_ARRAY);
            this->test(doubleArray.size() == 6, VSTRING_FORMAT("%s appended doubleArray size", labelPrefix.chars()));
            this->test(doubleArray[0] == 123.456 && doubleArray[1] == 456.789 && doubleArray[2] == 246.135 &&
                       doubleArray[3] == 123.456 && doubleArray[4] == 456.789 && doubleArray[5] == 246.135, VSTRING_FORMAT("%s appended doubleArray values", labelPrefix.chars()));
        }

        // VBentoDurationArray tests.
        const VDuration kDuration0 = VDuration::MILLISECOND() * 55;
        const VDuration kDuration1 = VDuration::SECOND() * 66;
        const VDuration kDuration2 = VDuration::HOUR() * 77;
        {
            const VDurationVector& durationArray = initializedArrays->getDurationArray(ATTRIBUTE_NAME_DURATION_ARRAY);
            this->test(durationArray.size() == 3, VSTRING_FORMAT("%s initialized durationArray size", labelPrefix.chars()));
            this->test(durationArray[0] == kDuration0 && durationArray[1] == kDuration1 && durationArray[2] == kDuration2, VSTRING_FORMAT("%s initialized durationArray values", labelPrefix.chars()));
        }
        {
            const VDurationVector& durationArray = assignedArrays->getDurationArray(ATTRIBUTE_NAME_DURATION_ARRAY);
            this->test(durationArray.size() == 3, VSTRING_FORMAT("%s assigned durationArray size", labelPrefix.chars()));
            this->test(durationArray[0] == kDuration0 && durationArray[1] == kDuration1 && durationArray[2] == kDuration2, VSTRING_FORMAT("%s assigned durationArray values", labelPrefix.chars()));
        }
        {
            const VDurationVector& durationArray = appendedArrays->getDurationArray(ATTRIBUTE_NAME_DURATION_ARRAY);
            this->test(durationArray.size() == 6, VSTRING_FORMAT("%s appended durationArray size", labelPrefix.chars()));
            this->test(durationArray[0] == kDuration0 && durationArray[1] == kDuration1 && durationArray[2] == kDuration2 &&
                       durationArray[3] == kDuration0 && durationArray[4] == kDuration1 && durationArray[5] == kDuration2, VSTRING_FORMAT("%s appended durationArray values", labelPrefix.chars()));
        }

        // VBentoInstantArray tests.
        {
            const VInstantVector& instantArray = initializedArrays->getInstantArray(ATTRIBUTE_NAME_INSTANT_ARRAY);
            this->test(instantArray.size() == 3, VSTRING_FORMAT("%s initialized instantArray size", labelPrefix.chars()));
            this->test(instantArray[0] == VInstant::NEVER_OCCURRED() && instantArray[1] == VInstant::INFINITE_PAST() && instantArray[2] == gInstantNoon2005June1UTC, VSTRING_FORMAT("%s initialized instantArray values", labelPrefix.chars()));
        }
        {
            const VInstantVector& instantArray = assignedArrays->getInstantArray(ATTRIBUTE_NAME_INSTANT_ARRAY);
            this->test(instantArray.size() == 3, VSTRING_FORMAT("%s assigned instantArray size", labelPrefix.chars()));
            this->test(instantArray[0] == VInstant::NEVER_OCCURRED() && instantArray[1] == VInstant::INFINITE_PAST() && instantArray[2] == gInstantNoon2005June1UTC, VSTRING_FORMAT("%s assigned instantArray values", labelPrefix.chars()));
        }
        {
            const VInstantVector& instantArray = appendedArrays->getInstantArray(ATTRIBUTE_NAME_INSTANT_ARRAY);
            this->test(instantArray.size() == 6, VSTRING_FORMAT("%s appended instantArray size", labelPrefix.chars()));
            this->test(instantArray[0] == VInstant::NEVER_OCCURRED() && instantArray[1] == VInstant::INFINITE_PAST() && instantArray[2] == gInstantNoon2005June1UTC &&
                       instantArray[3] == VInstant::NEVER_OCCURRED() && instantArray[4] == VInstant::INFINITE_PAST() && instantArray[5] == gInstantNoon2005June1UTC, VSTRING_FORMAT("%s appended instantArray values", labelPrefix.chars()));
        }

        // Test non-throwing missing value handling.
        this->test(node.getS8("non-existent", -42) == -42, VSTRING_FORMAT("%s default s8", labelPrefix.chars())); // <0 to verify correct sign handling
        this->test(node.getU8("non-existent", 200) == 200, VSTRING_FORMAT("%s default u8", labelPrefix.chars())); // >127 to verify correct sign handling
        this->test(node.getS16("non-existent", 999) == 999, VSTRING_FORMAT("%s default s16", labelPrefix.chars()));
        this->test(node.getU16("non-existent", 999) == 999, VSTRING_FORMAT("%s default u16", labelPrefix.chars()));
        this->test(node.getS32("non-existent", 999) == 999, VSTRING_FORMAT("%s default s32", labelPrefix.chars()));
        this->test(node.getU32("non-existent", 999) == 999, VSTRING_FORMAT("%s default u32", labelPrefix.chars()));
        this->test(node.getS64("non-existent", 999) == 999, VSTRING_FORMAT("%s default s64", labelPrefix.chars()));
        this->test(node.getU64("non-existent", 999) == 999, VSTRING_FORMAT("%s default u64", labelPrefix.chars()));
        this->test(node.getBool("non-existent", true) == true, VSTRING_FORMAT("%s default bool", labelPrefix.chars()));
        this->test(node.getString("non-existent", "999") == "999", VSTRING_FORMAT("%s default string", labelPrefix.chars()));
        this->test(node.getInt("non-existent", 999) == 999, VSTRING_FORMAT("%s default int", labelPrefix.chars()));
        this->test(node.getFloat("non-existent", kTestFloatValue) == kTestFloatValue, VSTRING_FORMAT("%s default float", labelPrefix.chars()));
        this->test(node.getDouble("non-existent", kTestDoubleValue) == kTestDoubleValue, VSTRING_FORMAT("%s default double", labelPrefix.chars()));
        this->test(node.getChar("non-existent", 'x') == 'x', VSTRING_FORMAT("%s default char", labelPrefix.chars()));
        VDuration defaultDuration = VDuration::MILLISECOND() * 986;
        this->test(node.getDuration("non-existent", defaultDuration) == defaultDuration, VSTRING_FORMAT("%s default duration", labelPrefix.chars()));
        VInstant defaultInstant;
        defaultInstant.setDateAndTime(VDateAndTime(2007, 3, 4, 5, 6, 7, 8), VInstant::UTC_TIME_ZONE_ID());
        this->test(node.getInstant("non-existent", defaultInstant) == defaultInstant, VSTRING_FORMAT("%s default instant", labelPrefix.chars()));

        this->test(node.getSize("non-existent", VSize(1.1, 2.2)) == VSize(1.1, 2.2), VSTRING_FORMAT("%s default size", labelPrefix.chars()));
        this->test(node.getISize("non-existent", VISize(3, 4)) == VISize(3, 4), VSTRING_FORMAT("%s default isize", labelPrefix.chars()));
        this->test(node.getPoint("non-existent", VPoint(1.1, 2.2)) == VPoint(1.1, 2.2), VSTRING_FORMAT("%s default point", labelPrefix.chars()));
        this->test(node.getIPoint("non-existent", VIPoint(3, 4)) == VIPoint(3, 4), VSTRING_FORMAT("%s default ipoint", labelPrefix.chars()));
        this->test(node.getLine("non-existent", VLine(1.1, 2.2, 3.3, 4.4)) == VLine(1.1, 2.2, 3.3, 4.4), VSTRING_FORMAT("%s default line", labelPrefix.chars()));
        this->test(node.getILine("non-existent", VILine(5, 6, 7, 8)) == VILine(5, 6, 7, 8), VSTRING_FORMAT("%s default iline", labelPrefix.chars()));
        this->test(node.getRect("non-existent", VRect(1.1, 2.2, 3.3, 4.4)) == VRect(1.1, 2.2, 3.3, 4.4), VSTRING_FORMAT("%s default rect", labelPrefix.chars()));
        this->test(node.getIRect("non-existent", VIRect(5, 6, 7, 8)) == VIRect(5, 6, 7, 8), VSTRING_FORMAT("%s default irect", labelPrefix.chars()));
        VPolygon defaultPolygon; defaultPolygon.add(VPoint(1.1, 2.2)); defaultPolygon.add(VPoint(3.3, 4.4));
        this->test(node.getPolygon("non-existent", defaultPolygon) == defaultPolygon, VSTRING_FORMAT("%s default polygon", labelPrefix.chars()));
        VIPolygon defaultIPolygon; defaultIPolygon.add(VIPoint(5, 6)); defaultIPolygon.add(VIPoint(7, 8));
        this->test(node.getIPolygon("non-existent", defaultIPolygon) == defaultIPolygon, VSTRING_FORMAT("%s default ipolygon", labelPrefix.chars()));
        this->test(node.getColor("non-existent", VColor(9, 10, 11, 12)) == VColor(9, 10, 11, 12), VSTRING_FORMAT("%s default color", labelPrefix.chars()));

        VMemoryStream defaultBinary;
        VBinaryIOStream defaultBinaryIO(defaultBinary);
        defaultBinaryIO.writeString("default");
        VReadOnlyMemoryStream defaultBinaryReader(defaultBinary.getBuffer(), defaultBinary.getEOFOffset());
        bool defaultBinaryResult = node.getBinary("non-existent", defaultBinaryReader);
        this->test(!defaultBinaryResult, VSTRING_FORMAT("%s default binary result", labelPrefix.chars()));
        this->test(defaultBinaryReader.getBuffer() == defaultBinary.getBuffer(), VSTRING_FORMAT("%s default binary data", labelPrefix.chars()));

        Vs8Array emptyS8Array;
        this->test(initializedArrays->getS8Array("non-existent", emptyS8Array).size() == 0, VSTRING_FORMAT("%s default s8 array", labelPrefix.chars()));
        Vs16Array emptyS16Array;
        this->test(initializedArrays->getS16Array("non-existent", emptyS16Array).size() == 0, VSTRING_FORMAT("%s default s16 array", labelPrefix.chars()));
        Vs32Array emptyS32Array;
        this->test(initializedArrays->getS32Array("non-existent", emptyS32Array).size() == 0, VSTRING_FORMAT("%s default s32 array", labelPrefix.chars()));
        Vs64Array emptyS64Array;
        this->test(initializedArrays->getS64Array("non-existent", emptyS64Array).size() == 0, VSTRING_FORMAT("%s default s64 array", labelPrefix.chars()));
        VStringVector emptyStringArray;
        this->test(initializedArrays->getStringArray("non-existent", emptyStringArray).size() == 0, VSTRING_FORMAT("%s default string array", labelPrefix.chars()));
        VBoolArray emptyBoolArray;
        this->test(initializedArrays->getBoolArray("non-existent", emptyBoolArray).size() == 0, VSTRING_FORMAT("%s default bool array", labelPrefix.chars()));
        VDoubleArray emptyDoubleArray;
        this->test(initializedArrays->getDoubleArray("non-existent", emptyDoubleArray).size() == 0, VSTRING_FORMAT("%s default double array", labelPrefix.chars()));
        VDurationVector emptyDurationArray;
        this->test(initializedArrays->getDurationArray("non-existent", emptyDurationArray).size() == 0, VSTRING_FORMAT("%s default duration array", labelPrefix.chars()));
        VInstantVector emptyInstantArray;
        this->test(initializedArrays->getInstantArray("non-existent", emptyInstantArray).size() == 0, VSTRING_FORMAT("%s default instant array", labelPrefix.chars()));

        // Test non-throwing missing value handling.
        // Each of these SHOULD throw an exception.
        // If it doesn't throw, is failed.
        try { (void) node.getS8("non-existent"); this->test(false, VSTRING_FORMAT("%s throw s8", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw s8", labelPrefix.chars()));}

        try { (void) node.getU8("non-existent"); this->test(false, VSTRING_FORMAT("%s throw u8", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw u8", labelPrefix.chars()));}

        try { (void) node.getS16("non-existent"); this->test(false, VSTRING_FORMAT("%s throw s16", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw s16", labelPrefix.chars()));}

        try { (void) node.getU16("non-existent"); this->test(false, VSTRING_FORMAT("%s throw u16", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw u16", labelPrefix.chars()));}

        try { (void) node.getS32("non-existent"); this->test(false, VSTRING_FORMAT("%s throw s32", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw s32", labelPrefix.chars()));}

        try { (void) node.getU32("non-existent"); this->test(false, VSTRING_FORMAT("%s throw u32", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw u32", labelPrefix.chars()));}

        try { (void) node.getS64("non-existent"); this->test(false, VSTRING_FORMAT("%s throw s64", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw s64", labelPrefix.chars()));}

        try { (void) node.getU64("non-existent"); this->test(false, VSTRING_FORMAT("%s throw u64", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw u64", labelPrefix.chars()));}

        try { (void) node.getBool("non-existent"); this->test(false, VSTRING_FORMAT("%s throw bool", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw bool", labelPrefix.chars()));}

        try { (void) node.getString("non-existent"); this->test(false, VSTRING_FORMAT("%s throw string", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw string", labelPrefix.chars()));}

        try { (void) node.getInt("non-existent"); this->test(false, VSTRING_FORMAT("%s throw int", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw int", labelPrefix.chars()));}

        try { (void) node.getFloat("non-existent"); this->test(false, VSTRING_FORMAT("%s throw float", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw float", labelPrefix.chars()));}

        try { (void) node.getDouble("non-existent"); this->test(false, VSTRING_FORMAT("%s throw double", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw double", labelPrefix.chars()));}

        try { (void) node.getChar("non-existent"); this->test(false, VSTRING_FORMAT("%s throw char", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw char", labelPrefix.chars()));}

        try { (void) node.getDuration("non-existent"); this->test(false, VSTRING_FORMAT("%s throw duration", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw duration", labelPrefix.chars()));}

        try { (void) node.getInstant("non-existent"); this->test(false, VSTRING_FORMAT("%s throw instant", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw instant", labelPrefix.chars()));}

        try { (void) node.getSize("non-existent"); this->test(false, VSTRING_FORMAT("%s throw size", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw size", labelPrefix.chars()));}

        try { (void) node.getISize("non-existent"); this->test(false, VSTRING_FORMAT("%s throw isize", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw isize", labelPrefix.chars()));}

        try { (void) node.getPoint("non-existent"); this->test(false, VSTRING_FORMAT("%s throw point", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw point", labelPrefix.chars()));}

        try { (void) node.getIPoint("non-existent"); this->test(false, VSTRING_FORMAT("%s throw ipoint", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw ipoint", labelPrefix.chars()));}

        try { (void) node.getLine("non-existent"); this->test(false, VSTRING_FORMAT("%s throw line", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw line", labelPrefix.chars()));}

        try { (void) node.getILine("non-existent"); this->test(false, VSTRING_FORMAT("%s throw iline", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw iline", labelPrefix.chars()));}

        try { (void) node.getRect("non-existent"); this->test(false, VSTRING_FORMAT("%s throw rect", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw rect", labelPrefix.chars()));}

        try { (void) node.getIRect("non-existent"); this->test(false, VSTRING_FORMAT("%s throw irect", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw irect", labelPrefix.chars()));}

        try { (void) node.getPolygon("non-existent"); this->test(false, VSTRING_FORMAT("%s throw polygon", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw polygon", labelPrefix.chars()));}

        try { (void) node.getIPolygon("non-existent"); this->test(false, VSTRING_FORMAT("%s throw ipolygon", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw ipolygon", labelPrefix.chars()));}

        try { (void) node.getColor("non-existent"); this->test(false, VSTRING_FORMAT("%s throw color", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw color", labelPrefix.chars()));}

        try { (void) node.getBinary("non-existent"); this->test(false, VSTRING_FORMAT("%s throw binary", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw binary", labelPrefix.chars()));}

        try { (void) node.getS8Array("non-existent"); this->test(false, VSTRING_FORMAT("%s throw s8 array", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw s8 array", labelPrefix.chars()));}

        try { (void) node.getS16Array("non-existent"); this->test(false, VSTRING_FORMAT("%s throw s16 array", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw s16 array", labelPrefix.chars()));}

        try { (void) node.getS32Array("non-existent"); this->test(false, VSTRING_FORMAT("%s throw s32 array", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw s32 array", labelPrefix.chars()));}

        try { (void) node.getS64Array("non-existent"); this->test(false, VSTRING_FORMAT("%s throw s64 array", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw s64 array", labelPrefix.chars()));}

        try { (void) node.getBoolArray("non-existent"); this->test(false, VSTRING_FORMAT("%s throw bool array", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw bool array", labelPrefix.chars()));}

        try { (void) node.getStringArray("non-existent"); this->test(false, VSTRING_FORMAT("%s throw string array", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw string array", labelPrefix.chars()));}

        try { (void) node.getDoubleArray("non-existent"); this->test(false, VSTRING_FORMAT("%s throw double array", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw double array", labelPrefix.chars()));}

        try { (void) node.getDurationArray("non-existent"); this->test(false, VSTRING_FORMAT("%s throw duration array", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw duration array", labelPrefix.chars()));}

        try { (void) node.getInstantArray("non-existent"); this->test(false, VSTRING_FORMAT("%s throw instant array", labelPrefix.chars())); }
        catch (const VException& /*ex*/) { this->test(true, VSTRING_FORMAT("%s throw instant array", labelPrefix.chars()));}

    } catch (const VException& ex) {
        this->test(false, VSTRING_FORMAT("VBentoUnit %s threw an exception: %s", labelPrefix.chars(), ex.what()));
    }
}

