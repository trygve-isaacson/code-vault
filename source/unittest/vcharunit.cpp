/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vcharunit.h"
#include "vchar.h"

VCharUnit::VCharUnit(bool logOnSuccess, bool throwOnError)
: VUnit("VCharUnit", logOnSuccess, throwOnError)
    {
    }

void VCharUnit::run()
    {
    VChar    x1('x');
    VChar    x2(0x78);
    this->test(x1 == 'x', "character ctor");
    this->test(x2 == 'x', "integer ctor");
    this->test(x1 == x2, "ctor equality");
    
    x1 = 'y';
    x2 = 0x79; 
    this->test(x1 == 'y', "character assignment");
    this->test(x2 == 'y', "integer assignment");
    this->test(x1 == x2, "assignment equality");
    
    x1 = 'a';
    this->test(x1.isLowerCase(), "lower case");
    this->test(! x1.isUpperCase(), "not upper case");
    x2 = 'A';
    this->test(! x2.isLowerCase(), "not lower case");
    this->test(x2.isUpperCase(), "upper case");
    x2.toLowerCase();
    this->test(x2.isLowerCase(), "to lower case");
    this->test(x2 == x1, "to lower case equality");
    x1.toUpperCase();
    this->test(x1.isUpperCase(), "to upper case");
    this->test(x1 == 'A', "to upper case equality");
    
    x1 = 'b';
    VChar bigB = x1.upperCase();
    this->test(bigB == 'B', "return upper case");
    VChar littleB = bigB.lowerCase();
    this->test(littleB == 'b', "return lower case");
    this->test(littleB.charValue() == 'b', "char value");
    this->test(littleB.intValue() == 0x62, "int value");
    
    x1.set('c');
    this->test(x1 == 'c', "set char");
    x1.set(0x64);
    this->test(x1 == 'd', "set int");
    
    x1 = 'd';
    char littleD = x1;
    this->test(littleD == 'd', "operator char");
    
    VChar    i1('i');
    VChar    i2('i');
    VChar    j1('j');
    VChar    j2('j');
    this->test(i1 != j1, "inequality");
    this->test(i1 < j1, "LT");
    this->test(! (i1 < i2), "not LT");
    this->test(j1 > i1, "GT");
    this->test(! (j1 > j2), "not GT");
    this->test(i1 <= i2, "LTE 1");
    this->test(i1 <= j1, "LTE 2");
    this->test(j1 >= j2, "GTE 1");
    this->test(j1 >= i1, "GTE 2");
    this->test(! (j1 <= i1), "not LTE");
    this->test(! (i1 >= j1), "not GTE");
    
    // Test the known ranges of alpha/numeric/whitespace values.
    for (int i = 0; i < 256; ++i)
        {
        VChar    c(i);

        if ((i <= 0x20) || (i == 0x7F))
            {
            // This is the range VChar considers "whitespace".
            this->test(
                !c.isLowerCase() &&
                !c.isUpperCase() &&
                (c.intValue() == i) &&
                !c.isAlpha() &&
                !c.isNumeric() &&
                !c.isAlphaNumeric() &&
                c.isWhitespace(),
                VString("%d char properties", i));
            }
        else if (i <= 0x2F)
            {
            // This is all punctuation.
            this->test(
                !c.isLowerCase() &&
                !c.isUpperCase() &&
                (c.intValue() == i) &&
                !c.isAlpha() &&
                !c.isNumeric() &&
                !c.isAlphaNumeric() &&
                !c.isWhitespace(),
                VString("%d char properties", i));
            }
        else if (i <= 0x39)
            {
            // This is 0 thru 9.
            this->test(
                !c.isLowerCase() &&
                !c.isUpperCase() &&
                (c.intValue() == i) &&
                !c.isAlpha() &&
                c.isNumeric() &&
                c.isAlphaNumeric() &&
                !c.isWhitespace(),
                VString("%d char properties", i));
            }
        else if (i <= 0x40)
            {
            // This is all punctuation.
            this->test(
                !c.isLowerCase() &&
                !c.isUpperCase() &&
                (c.intValue() == i) &&
                !c.isAlpha() &&
                !c.isNumeric() &&
                !c.isAlphaNumeric() &&
                !c.isWhitespace(),
                VString("%d char properties", i));
            }
        else if (i <= 0x5A)
            {
            // This is A thru Z.
            this->test(
                !c.isLowerCase() &&
                c.isUpperCase() &&
                (c.intValue() == i) &&
                c.isAlpha() &&
                !c.isNumeric() &&
                c.isAlphaNumeric() &&
                !c.isWhitespace(),
                VString("%d char properties", i));
            }
        else if (i <= 0x60)
            {
            // This is all punctuation.
            this->test(
                !c.isLowerCase() &&
                !c.isUpperCase() &&
                (c.intValue() == i) &&
                !c.isAlpha() &&
                !c.isNumeric() &&
                !c.isAlphaNumeric() &&
                !c.isWhitespace(),
                VString("%d char properties", i));
            }
        else if (i <= 0x7A)
            {
            // This is a thru z.
            this->test(
                c.isLowerCase() &&
                !c.isUpperCase() &&
                (c.intValue() == i) &&
                c.isAlpha() &&
                !c.isNumeric() &&
                c.isAlphaNumeric() &&
                !c.isWhitespace(),
                VString("%d char properties", i));
            }
        else if (i <= 0x7E)
            {
            // This is all punctuation.
            this->test(
                !c.isLowerCase() &&
                !c.isUpperCase() &&
                (c.intValue() == i) &&
                !c.isAlpha() &&
                !c.isNumeric() &&
                !c.isAlphaNumeric() &&
                !c.isWhitespace(),
                VString("%d char properties", i));
            }
        else // (we already checked 0x7F) so 0x80 <= i <= 0xFF
            {
            // Properties of stuff at 0x80 and higher are not well-defined
            // and may vary based on the platform's ideas about upper case,
            // lower case, alphanumeric-ness, etc. Just test the basics.
            this->test(
                (c.intValue() == i) &&
                !c.isWhitespace(),
                VString("%d char properties", i));
            }
        }

    }

