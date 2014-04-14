/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
*/

/** @file */

#include "vcharunit.h"
#include "vchar.h"

VCharUnit::VCharUnit(bool logOnSuccess, bool throwOnError) :
    VUnit("VCharUnit", logOnSuccess, throwOnError) {
}

void VCharUnit::run() {
    VChar    x1('x');
    VChar    x2(0x78);
    VUNIT_ASSERT_EQUAL_LABELED(x1, 'x', "character ctor");
    VUNIT_ASSERT_EQUAL_LABELED(x2, 'x', "integer ctor");
    VUNIT_ASSERT_EQUAL_LABELED(x1, x2, "ctor equality");

    x1 = 'y';
    x2 = 0x79;
    VUNIT_ASSERT_EQUAL_LABELED(x1, 'y', "character assignment");
    VUNIT_ASSERT_EQUAL_LABELED(x2, 'y', "integer assignment");
    VUNIT_ASSERT_EQUAL_LABELED(x1, x2, "assignment equality");

    x1 = 'a';
    VUNIT_ASSERT_TRUE_LABELED(x1.isLowerCase(), "lower case");
    VUNIT_ASSERT_TRUE_LABELED(! x1.isUpperCase(), "not upper case");
    x2 = 'A';
    VUNIT_ASSERT_TRUE_LABELED(! x2.isLowerCase(), "not lower case");
    VUNIT_ASSERT_TRUE_LABELED(x2.isUpperCase(), "upper case");
    x2.toLowerCase();
    VUNIT_ASSERT_TRUE_LABELED(x2.isLowerCase(), "to lower case");
    VUNIT_ASSERT_EQUAL_LABELED(x2, x1, "to lower case equality");
    x1.toUpperCase();
    VUNIT_ASSERT_TRUE_LABELED(x1.isUpperCase(), "to upper case");
    VUNIT_ASSERT_EQUAL_LABELED(x1, 'A', "to upper case equality");

    x1 = 'b';
    VChar bigB = x1.upperCase();
    VUNIT_ASSERT_EQUAL_LABELED(bigB, 'B', "return upper case");
    VChar littleB = bigB.lowerCase();
    VUNIT_ASSERT_EQUAL_LABELED(littleB, 'b', "return lower case");
    VUNIT_ASSERT_EQUAL_LABELED(littleB.charValue(), 'b', "char value");
    VUNIT_ASSERT_EQUAL_LABELED(littleB.intValue(), 0x62, "int value");

    x1.set('c');
    VUNIT_ASSERT_EQUAL_LABELED(x1, 'c', "set char");
    x1.set(0x64);
    VUNIT_ASSERT_EQUAL_LABELED(x1, 'd', "set int");

    x1 = 'd';
    char littleD = x1;
    VUNIT_ASSERT_EQUAL_LABELED(littleD, 'd', "operator char");

    VChar    i1('i');
    VChar    i2('i');
    VChar    j1('j');
    VChar    j2('j');
    VUNIT_ASSERT_TRUE_LABELED(i1 != j1, "inequality");
    VUNIT_ASSERT_TRUE_LABELED(i1 < j1, "LT");
    VUNIT_ASSERT_TRUE_LABELED(!(i1 < i2), "not LT");
    VUNIT_ASSERT_TRUE_LABELED(j1 > i1, "GT");
    VUNIT_ASSERT_TRUE_LABELED(!(j1 > j2), "not GT");
    VUNIT_ASSERT_TRUE_LABELED(i1 <= i2, "LTE 1");
    VUNIT_ASSERT_TRUE_LABELED(i1 <= j1, "LTE 2");
    VUNIT_ASSERT_TRUE_LABELED(j1 >= j2, "GTE 1");
    VUNIT_ASSERT_TRUE_LABELED(j1 >= i1, "GTE 2");
    VUNIT_ASSERT_TRUE_LABELED(!(j1 <= i1), "not LTE");
    VUNIT_ASSERT_TRUE_LABELED(!(i1 >= j1), "not GTE");

    VUNIT_ASSERT_TRUE_LABELED(VChar::equalsIgnoreCase(VChar('x'), VChar('X')), "equalsIgnoreCase 1");
    VUNIT_ASSERT_TRUE_LABELED(VChar::equalsIgnoreCase('x', VChar('X')), "equalsIgnoreCase 2");
    VUNIT_ASSERT_TRUE_LABELED(VChar::equalsIgnoreCase(VChar('x'), 'X'), "equalsIgnoreCase 3");
    VUNIT_ASSERT_TRUE_LABELED(VChar::equalsIgnoreCase('x', 'X'), "equalsIgnoreCase 4");
    VUNIT_ASSERT_TRUE_LABELED(VChar::equalsIgnoreCase(VChar('5'), VChar('5')), "equalsIgnoreCase 5"); // test numbers
    VUNIT_ASSERT_TRUE_LABELED(VChar::equalsIgnoreCase(VChar('!'), VChar('!')), "equalsIgnoreCase 6"); // test punctuation
    VUNIT_ASSERT_TRUE_LABELED(VChar::equalsIgnoreCase(VChar(' '), VChar(' ')), "equalsIgnoreCase 7"); // test whitespace

    VUNIT_ASSERT_FALSE_LABELED(VChar::equalsIgnoreCase(VChar('x'), VChar('y')), "!equalsIgnoreCase 1");
    VUNIT_ASSERT_FALSE_LABELED(VChar::equalsIgnoreCase('x', VChar('y')), "!equalsIgnoreCase 2");
    VUNIT_ASSERT_FALSE_LABELED(VChar::equalsIgnoreCase(VChar('x'), 'y'), "!equalsIgnoreCase 3");
    VUNIT_ASSERT_FALSE_LABELED(VChar::equalsIgnoreCase('x', 'y'), "!equalsIgnoreCase 4");
    VUNIT_ASSERT_FALSE_LABELED(VChar::equalsIgnoreCase(VChar('5'), VChar('6')), "!equalsIgnoreCase 5"); // test numbers
    VUNIT_ASSERT_FALSE_LABELED(VChar::equalsIgnoreCase(VChar('!'), VChar('@')), "!equalsIgnoreCase 6"); // test punctuation
    VUNIT_ASSERT_FALSE_LABELED(VChar::equalsIgnoreCase(VChar(' '), VChar('\t')), "!equalsIgnoreCase 7"); // test whitespace

    // Test the known ranges of alpha/numeric/whitespace values.
    for (int i = 0; i < 256; ++i) {
        VChar    c(i);

        if ((i <= 0x20) || (i == 0x7F)) {
            // This is the range VChar considers "whitespace".
            this->test(
                !c.isLowerCase() &&
                !c.isUpperCase() &&
                (c.intValue() == i) &&
                !c.isAlpha() &&
                !c.isNumeric() &&
                !c.isAlphaNumeric() &&
                c.isWhitespace(),
                VSTRING_FORMAT("%d char properties", i));
        } else if (i <= 0x2F) {
            // This is all punctuation.
            this->test(
                !c.isLowerCase() &&
                !c.isUpperCase() &&
                (c.intValue() == i) &&
                !c.isAlpha() &&
                !c.isNumeric() &&
                !c.isAlphaNumeric() &&
                !c.isWhitespace(),
                VSTRING_FORMAT("%d char properties", i));
        } else if (i <= 0x39) {
            // This is 0 thru 9.
            this->test(
                !c.isLowerCase() &&
                !c.isUpperCase() &&
                (c.intValue() == i) &&
                !c.isAlpha() &&
                c.isNumeric() &&
                c.isAlphaNumeric() &&
                !c.isWhitespace(),
                VSTRING_FORMAT("%d char properties", i));
        } else if (i <= 0x40) {
            // This is all punctuation.
            this->test(
                !c.isLowerCase() &&
                !c.isUpperCase() &&
                (c.intValue() == i) &&
                !c.isAlpha() &&
                !c.isNumeric() &&
                !c.isAlphaNumeric() &&
                !c.isWhitespace(),
                VSTRING_FORMAT("%d char properties", i));
        } else if (i <= 0x5A) {
            // This is A thru Z.
            this->test(
                !c.isLowerCase() &&
                c.isUpperCase() &&
                (c.intValue() == i) &&
                c.isAlpha() &&
                !c.isNumeric() &&
                c.isAlphaNumeric() &&
                !c.isWhitespace(),
                VSTRING_FORMAT("%d char properties", i));
        } else if (i <= 0x60) {
            // This is all punctuation.
            this->test(
                !c.isLowerCase() &&
                !c.isUpperCase() &&
                (c.intValue() == i) &&
                !c.isAlpha() &&
                !c.isNumeric() &&
                !c.isAlphaNumeric() &&
                !c.isWhitespace(),
                VSTRING_FORMAT("%d char properties", i));
        } else if (i <= 0x7A) {
            // This is a thru z.
            this->test(
                c.isLowerCase() &&
                !c.isUpperCase() &&
                (c.intValue() == i) &&
                c.isAlpha() &&
                !c.isNumeric() &&
                c.isAlphaNumeric() &&
                !c.isWhitespace(),
                VSTRING_FORMAT("%d char properties", i));
        } else if (i <= 0x7E) {
            // This is all punctuation.
            this->test(
                !c.isLowerCase() &&
                !c.isUpperCase() &&
                (c.intValue() == i) &&
                !c.isAlpha() &&
                !c.isNumeric() &&
                !c.isAlphaNumeric() &&
                !c.isWhitespace(),
                VSTRING_FORMAT("%d char properties", i));
        } else { // (we already checked 0x7F) so 0x80 <= i <= 0xFF
            // Properties of stuff at 0x80 and higher are not well-defined
            // and may vary based on the platform's ideas about upper case,
            // lower case, alphanumeric-ness, etc. Just test the basics.
            this->test(
                (c.intValue() == i) &&
                !c.isWhitespace(),
                VSTRING_FORMAT("%d char properties", i));
        }
    }

}

