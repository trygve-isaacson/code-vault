/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

/** @file */

#include "vbentounit.h"
#include "vbento.h"
#include "vexception.h"
#include "vchar.h"

VBentoUnit::VBentoUnit(bool logOnSuccess, bool throwOnError)
: VUnit("VBentoUnit", logOnSuccess, throwOnError)
    {
    }

#define VERY_LONG_STRING "This is a string that needs to be longer than 252 characters in order to test the dynamic length prefix capability, where short data has a single byte length descriptor but longer data uses longer length descriptors. By making this string longer than 252 characters, it means that its length descriptor in the bento binary stream will require three bytes rather than one. But most strings only need one byte to describe their length."
static const VFloat kTestFloatValue = 3.14f;
static const VDouble kTestDoubleValue = 3.14159; // note: use of more than 6 decimal places here will cause the text i/o conversion test to fail because only 6 decimal places are written out
static const VDuration kTestDurationValue = VDuration::MILLISECOND() * 42;
static const VDateAndTime kTestInstantValue = VDateAndTime(2007, 04, 20, 7, 56, 23, 986);

void VBentoUnit::run()
    {
    this->_verifyDynamicLengths();

    // We'll just populate a Bento container with every type, and then
    // retrieve them and validate.

    VBentoNode    root("root");
    this->_buildTestData(root);
    this->_verifyContents(root, "tree");

    VString rootText;
    root.writeToBentoTextString(rootText);
    this->logStatus(rootText);

    // Test the same stuff being correctly streamed.
    VMemoryStream buffer;
    VBinaryIOStream stream(buffer);
    root.writeToStream(stream);

    stream.seek(0, SEEK_SET); // rewind to start of stream
    VBentoNode other(stream);

    this->_verifyContents(other, "stream");

    VBentoNode rootFromText;
    rootFromText.readFromBentoTextString(rootText);

    this->_verifyContents(rootFromText, "text");
    }

void VBentoUnit::_verifyDynamicLengths()
    {
    Vs64 aOneByteLength = CONST_S64(251);
    this->test(1 == VBentoNode::_getLengthOfLength(aOneByteLength), VString("length of %lld == 1", aOneByteLength));

    Vs64 biggestOneByteLength =  CONST_S64(252);
    this->test(1 == VBentoNode::_getLengthOfLength(biggestOneByteLength), VString("length of %lld == 1", biggestOneByteLength));

    for (Vs64 aThreeByteLength = biggestOneByteLength+1; aThreeByteLength < biggestOneByteLength+10; ++aThreeByteLength) {
        this->test(3 == VBentoNode::_getLengthOfLength(aThreeByteLength), VString("length of %lld == 3", aThreeByteLength));
    }

    Vs64 biggestThreeByteLength =  CONST_S64(0x000000000000FFFF);
    this->test(3 == VBentoNode::_getLengthOfLength(biggestThreeByteLength), VString("length of %lld == 3", biggestThreeByteLength));

    for (Vs64 aFiveByteLength = biggestThreeByteLength + 1; aFiveByteLength < biggestThreeByteLength + 10; ++aFiveByteLength) {
        this->test(5 == VBentoNode::_getLengthOfLength(aFiveByteLength), VString("length of %lld == 5", aFiveByteLength));
    }

    Vs64 biggestFiveByteLength =  CONST_S64(0x00000000FFFFFFFF);
    this->test(5 == VBentoNode::_getLengthOfLength(biggestFiveByteLength), VString("length of %lld == 5", biggestFiveByteLength));

    for (Vs64 aNineByteLength = biggestFiveByteLength + 1; aNineByteLength < biggestFiveByteLength + 10; ++aNineByteLength) {
        this->test(9 == VBentoNode::_getLengthOfLength(aNineByteLength), VString("length of %lld == 9", aNineByteLength));
    }
    }

void VBentoUnit::_buildTestData(VBentoNode& root)
    {
    root.addS8("s8", 100);
    root.addU8("u8", 200);
    root.addS16("s16", 300);
    root.addU16("u16", 400);
    root.addS32("s32", 500);
    root.addU32("u32", 600);
    root.addS64("s64", CONST_S64(700));
    root.addU64("u64", CONST_U64(800));
    root.addBool("bool", true);
    root.addString("vstr", "bento unit test");
    root.addString("lstr", VERY_LONG_STRING);
    root.addString("estr", VString::EMPTY());
    root.addInt("int", 900);
    root.addFloat("flot", kTestFloatValue);
    root.addDouble("doub", kTestDoubleValue);
    root.addChar("char", VChar('!'));
    root.addChar("nulc", VChar::NULL_CHAR());
    
    root.addDuration("dura", kTestDurationValue);
    VInstant instant;
    instant.setDateAndTime(kTestInstantValue, VInstant::UTC_TIME_ZONE_ID());
    root.addInstant("inst", instant);

    VBentoNode*    childNode = new VBentoNode("child"); // exercise the new + addChildNode method of adding
    childNode->addS32("ch32", 1000);
    root.addChildNode(childNode);

    // Construct an array of integers at root/intarray/value[]
    VBentoNode* intarray = root.addNewChildNode("intarray"); // exercise the addNewChildNode method of adding
    for (int i = 0; i < 10; ++i)
        {
        VBentoNode* arrayElement = intarray->addNewChildNode("arrayElement");
        arrayElement->addS32("value", i);
        }
    }

void VBentoUnit::_verifyContents(const VBentoNode& node, const VString& labelPrefix)
    {
    // We'll call the throwing getters. We test for the correct result,
    // but also know that if the value is not found, it'll throw.
    try
        {
        this->test(node.getName() == "root", VString("%s name", labelPrefix.chars()));
        this->test(node.getS8("s8") == 100, VString("%s s8", labelPrefix.chars()));
        this->test(node.getU8("u8") == 200, VString("%s u8", labelPrefix.chars()));
        this->test(node.getS16("s16") == 300, VString("%s s16", labelPrefix.chars()));
        this->test(node.getU16("u16") == 400, VString("%s u16", labelPrefix.chars()));
        this->test(node.getS32("s32") == 500, VString("%s s32", labelPrefix.chars()));
        this->test(node.getU32("u32") == 600, VString("%s u32", labelPrefix.chars()));
        this->test(node.getS64("s64") == CONST_S64(700), VString("%s s64", labelPrefix.chars()));
        this->test(node.getU64("u64") == CONST_U64(800), VString("%s u64", labelPrefix.chars()));
        this->test(node.getBool("bool") == true, VString("%s bool", labelPrefix.chars()));
        VString s1 = node.getString("vstr");
        this->test(s1 == "bento unit test", VString("%s vstr", labelPrefix.chars()));
        VString s2 = node.getString("lstr");
        this->test(s2 == VERY_LONG_STRING, VString("%s lstr", labelPrefix.chars()));
        VString s3 = node.getString("estr");
        this->test(s3 == VString::EMPTY(), VString("%s empty string", labelPrefix.chars()));
        this->test(node.getInt("int") == 900, VString("%s int", labelPrefix.chars()));
        this->test(node.getFloat("flot") == kTestFloatValue, VString("%s float", labelPrefix.chars()));
        this->test(node.getDouble("doub") == kTestDoubleValue, VString("%s double", labelPrefix.chars()));
        this->test(node.getChar("char") == '!', VString("%s char", labelPrefix.chars()));
        this->test(node.getChar("nulc") == VChar::NULL_CHAR(), VString("%s null char", labelPrefix.chars()));
        this->test(node.getDuration("dura") == kTestDurationValue, VString("%s duration", labelPrefix.chars()));
        VInstant instant;
        instant.setDateAndTime(kTestInstantValue, VInstant::UTC_TIME_ZONE_ID());
        this->test(node.getInstant("inst") == instant, VString("%s instant", labelPrefix.chars()));

        const VBentoNode* child = node.findNode("child");
        this->test(child != NULL, VString("%s child", labelPrefix.chars()));
        if (child != NULL)    // in case we aren't aborting on earlier failures
            this->test(child->getS32("ch32") == 1000, VString("%s ch32", labelPrefix.chars()));

        const VBentoNode* intarray = node.findNode("intarray");
        this->test(intarray != NULL, VString("%s intarray", labelPrefix.chars()));
        if (intarray != NULL)
            {
            this->test(intarray->getNodes().size() == 10, VString("%s intarray length", labelPrefix.chars()));
            int arrayIndex = 0;
            for (VBentoNodePtrVector::const_iterator i = intarray->getNodes().begin(); i != intarray->getNodes().end(); ++i)
                {
                this->test((*i)->getName() == "arrayElement", VString("%s intarray element name", labelPrefix.chars()));
                this->test((*i)->getS32("value") == arrayIndex, VString("%s intarray element %d", labelPrefix.chars(), arrayIndex));
                ++arrayIndex;
                }
            }

        // Test non-throwing missing value handling.
        this->test(node.getS8("non-existent", -42) == -42, VString("%s default s8", labelPrefix.chars())); // <0 to verify correct sign handling
        this->test(node.getU8("non-existent", 200) == 200, VString("%s default u8", labelPrefix.chars())); // >127 to verify correct sign handling
        this->test(node.getS16("non-existent", 999) == 999, VString("%s default s16", labelPrefix.chars()));
        this->test(node.getU16("non-existent", 999) == 999, VString("%s default u16", labelPrefix.chars()));
        this->test(node.getS32("non-existent", 999) == 999, VString("%s default s32", labelPrefix.chars()));
        this->test(node.getU32("non-existent", 999) == 999, VString("%s default u32", labelPrefix.chars()));
        this->test(node.getS64("non-existent", 999) == 999, VString("%s default s64", labelPrefix.chars()));
        this->test(node.getU64("non-existent", 999) == 999, VString("%s default u64", labelPrefix.chars()));
        this->test(node.getBool("non-existent", true) == true, VString("%s default bool", labelPrefix.chars()));
        this->test(node.getString("non-existent", "999") == "999", VString("%s default string", labelPrefix.chars()));
        this->test(node.getInt("non-existent", 999) == 999, VString("%s default int", labelPrefix.chars()));
        this->test(node.getFloat("non-existent", kTestFloatValue) == kTestFloatValue, VString("%s default float", labelPrefix.chars()));
        this->test(node.getDouble("non-existent", kTestDoubleValue) == kTestDoubleValue, VString("%s default double", labelPrefix.chars()));
        this->test(node.getChar("non-existent", 'x') == 'x', VString("%s default char", labelPrefix.chars()));
        VDuration defaultDuration = VDuration::MILLISECOND() * 986;
        this->test(node.getDuration("non-existent", defaultDuration) == defaultDuration, VString("%s default duration", labelPrefix.chars()));
        VInstant defaultInstant;
        defaultInstant.setDateAndTime(VDateAndTime(2007, 3, 4, 5, 6, 7, 8), VInstant::UTC_TIME_ZONE_ID());
        this->test(node.getInstant("non-existent", defaultInstant) == defaultInstant, VString("%s default instant", labelPrefix.chars()));

        // Test non-throwing missing value handling.
        // Each of these SHOULD throw an exception.
        // If it doesn't throw, is failed.
        try { (void) node.getS8("non-existent"); this->test(false, VString("%s throw s8", labelPrefix.chars())); }
            catch (const VException& /*ex*/) { this->test(true, VString("%s throw s8", labelPrefix.chars()));}

        try { (void) node.getU8("non-existent"); this->test(false, VString("%s throw u8", labelPrefix.chars())); }
            catch (const VException& /*ex*/) { this->test(true, VString("%s throw u8", labelPrefix.chars()));}

        try { (void) node.getS16("non-existent"); this->test(false, VString("%s throw s16", labelPrefix.chars())); }
            catch (const VException& /*ex*/) { this->test(true, VString("%s throw s16", labelPrefix.chars()));}

        try { (void) node.getU16("non-existent"); this->test(false, VString("%s throw u16", labelPrefix.chars())); }
            catch (const VException& /*ex*/) { this->test(true, VString("%s throw u16", labelPrefix.chars()));}

        try { (void) node.getS32("non-existent"); this->test(false, VString("%s throw s32", labelPrefix.chars())); }
            catch (const VException& /*ex*/) { this->test(true, VString("%s throw s32", labelPrefix.chars()));}

        try { (void) node.getU32("non-existent"); this->test(false, VString("%s throw u32", labelPrefix.chars())); }
            catch (const VException& /*ex*/) { this->test(true, VString("%s throw u32", labelPrefix.chars()));}

        try { (void) node.getS64("non-existent"); this->test(false, VString("%s throw s64", labelPrefix.chars())); }
            catch (const VException& /*ex*/) { this->test(true, VString("%s throw s64", labelPrefix.chars()));}

        try { (void) node.getU64("non-existent"); this->test(false, VString("%s throw u64", labelPrefix.chars())); }
            catch (const VException& /*ex*/) { this->test(true, VString("%s throw u64", labelPrefix.chars()));}

        try { (void) node.getBool("non-existent"); this->test(false, VString("%s throw bool", labelPrefix.chars())); }
            catch (const VException& /*ex*/) { this->test(true, VString("%s throw bool", labelPrefix.chars()));}

        try { (void) node.getString("non-existent"); this->test(false, VString("%s throw string", labelPrefix.chars())); }
            catch (const VException& /*ex*/) { this->test(true, VString("%s throw string", labelPrefix.chars()));}

        try { (void) node.getInt("non-existent"); this->test(false, VString("%s throw int", labelPrefix.chars())); }
            catch (const VException& /*ex*/) { this->test(true, VString("%s throw int", labelPrefix.chars()));}

        try { (void) node.getFloat("non-existent"); this->test(false, VString("%s throw float", labelPrefix.chars())); }
            catch (const VException& /*ex*/) { this->test(true, VString("%s throw float", labelPrefix.chars()));}

        try { (void) node.getDouble("non-existent"); this->test(false, VString("%s throw double", labelPrefix.chars())); }
            catch (const VException& /*ex*/) { this->test(true, VString("%s throw double", labelPrefix.chars()));}

        try { (void) node.getChar("non-existent"); this->test(false, VString("%s throw char", labelPrefix.chars())); }
            catch (const VException& /*ex*/) { this->test(true, VString("%s throw char", labelPrefix.chars()));}

        try { (void) node.getDuration("non-existent"); this->test(false, VString("%s throw duration", labelPrefix.chars())); }
            catch (const VException& /*ex*/) { this->test(true, VString("%s throw duration", labelPrefix.chars()));}

        try { (void) node.getInstant("non-existent"); this->test(false, VString("%s throw instant", labelPrefix.chars())); }
            catch (const VException& /*ex*/) { this->test(true, VString("%s throw instant", labelPrefix.chars()));}

        }
    catch (const VException& ex)
        {
        this->test(false, VString("VBentoUnit %s threw an exception: %s", labelPrefix.chars(), ex.what()));
        }
    }

