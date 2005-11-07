/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vstringunit.h"
#include "vchar.h"

VStringUnit::VStringUnit(bool logOnSuccess, bool throwOnError)
: VUnit("VStringUnit", logOnSuccess, throwOnError)
    {
    }

void VStringUnit::run()
    {
    // Start by testing assignment and concatenation.
    VString    s("(A)");
    
    this->test(s, "(A)", "literal ctor");

    s += s;
    this->test(s, "(A)(A)", "self concat");

    s += "(B)";
    this->test(s, "(A)(A)(B)", "literal concat");

    s += s;
    this->test(s, "(A)(A)(B)(A)(A)(B)", "self concat 2");

    s += "(C)";
    this->test(s, "(A)(A)(B)(A)(A)(B)(C)", "literal concat 2");
    
    s = s;
    this->test(s, "(A)(A)(B)(A)(A)(B)(C)", "self assign");
    
    s.toLowerCase();
    this->test(s, "(a)(a)(b)(a)(a)(b)(c)", "to lower case");
    
    s.toUpperCase();
    this->test(s, "(A)(A)(B)(A)(A)(B)(C)", "to upper case");
    
    // Test the length.
    this->test(s.length() == 21, "length");
    
    // Test array indexing.
    this->test(s[19] == 'C', "array element");
    this->test(s.charAt(19) == 'C', "char at");
    
    // Test operator+.
    VString    sum1 = VString('X') + 'Y';
    this->test(sum1 == "XY", "operator+ char");
    VString    sum2 = VString("school") + "bus";
    this->test(sum2 == "schoolbus", "operator+ char*");
    VString    sum3 = VString("race") + VString("car");
    this->test(sum3 == "racecar", "operator+ VString&");
    
    // Test comparison and equality.
    s = "Banana";
    this->test("Apple" < s, "operator <");
    this->test("Banana" <= s, "operator <=");
    this->test("Banana" == s, "operator ==");
    this->test("Banana" >= s, "operator >=");
    this->test("Cherry" > s, "operator >");
    this->test("BANANA" != s, "operator !=");
    this->test(s.equalsIgnoreCase("BANANA"), "equalsIgnoreCase");
    this->test(s.equalsIgnoreCase(VString("BANANA")), "equalsIgnoreCase");
    this->test(! s.equalsIgnoreCase("Fanana"), "! equalsIgnoreCase");
    this->test(! s.equalsIgnoreCase(VString("Fanana")), "! equalsIgnoreCase");

    this->test(s.compare(VString("Apple")) > 0, "compare >");
    this->test(s.compare("Apple") > 0, "compare >");
    this->test(s.compare(VString("Banana")) == 0, "compare ==");
    this->test(s.compare("Banana") == 0, "compare ==");
    this->test(s.compare(VString("Cherry")) < 0, "compare <");
    this->test(s.compare("Cherry") < 0, "compare <");

    this->test(s.compareIgnoreCase(VString("Apple")) > 0, "compareIgnoreCase >");
    this->test(s.compareIgnoreCase("Apple") > 0, "compareIgnoreCase >");
    this->test(s.compareIgnoreCase(VString("Banana")) == 0, "compareIgnoreCase ==");
    this->test(s.compareIgnoreCase("Banana") == 0, "compareIgnoreCase ==");
    this->test(s.compareIgnoreCase(VString("Cherry")) < 0, "compareIgnoreCase <");
    this->test(s.compareIgnoreCase("Cherry") < 0, "compareIgnoreCase <");

    this->test(s.startsWith("Ban"), "startsWith literal");
    this->test(s.startsWith('B'), "startsWith char");
    this->test(! s.startsWith("Bananas"), "! startsWith literal 1");
    this->test(! s.startsWith("Baz"), "! startsWith literal 2");
    this->test(! s.startsWith('b'), "! startsWith char");
    this->test(s.endsWith("nana"), "endsWith literal");
    this->test(s.endsWith('a'), "endsWith char");
    this->test(! s.endsWith("Yellow Banana"), "! endsWith literal 1");
    this->test(! s.endsWith("abcdefghijklmnopqrstuvwxyz"), "! endsWith literal 2");
    this->test(! s.endsWith('x'), "! endsWith char");
    
    // Test empty string constant behavior.
    this->test(VString::kEmptyString.isEmpty(), "kEmptyString is empty");
    this->test(VString::kEmptyString.length() == 0, "kEmptyString length is zero");
    this->test(VString::kEmptyString == "", "kEmptyString equals empty string literal");
    s.format("A%sB", VString::kEmptyString.chars());
    this->test(s == "AB", "kEmptyString is empty formatting element");
    s = "";
    this->test(s == VString::kEmptyString, "kEmptyString equals an empty VString");
    VString newlyConstructedString;
    this->test(newlyConstructedString == VString::kEmptyString, "kEmptyString equals a new constructed VString");

    // Test assigning empty strings into non-empty strings.
    s = "foo";
    s = newlyConstructedString;
    this->test(s.isEmpty(), "Assign empty VString&");
    s = &newlyConstructedString;
    this->test(s.isEmpty(), "Assign empty VString*");
    s = (VString*) NULL;
    this->test(s.isEmpty(), "Assign NULL VString*");
    s = "";
    this->test(s.isEmpty(), "Assign empty char*");
    s = (char*) NULL;
    this->test(s.isEmpty(), "Assign NULL char*");
        
    // Test re-assignment and non-shared memory.
    VString    a("a");
    VString    b("b");
    
    a = b;
    b = "something else";
    
    this->test(a, "b", "reassignment 1");
    this->test(b, "something else", "reassignment 2");
    
    // Test formatting.
    VString    formatted("%s is %d years old", "Spot", 5);
    this->test(formatted, "Spot is 5 years old", "ctor formatting");
    
    formatted.format("%s is %d years old", "Rover", 3);
    this->test(formatted, "Rover is 3 years old", "sprintf");
    
    // Test copying out.
    char    testBuffer[256];    // Largest legal Pascal string buffer.
    VString    testSource("This text should be copied out.");
    testSource.copyToBuffer(testBuffer);
    VString    testTarget(testBuffer);
    this->test(testTarget, "This text should be copied out.", "copy to chars");
    // Test copying in.
    testTarget = "           "; // clear out some of what we expect to alter
    testTarget.copyFromBuffer(testBuffer, 0, 4);
    this->test(testTarget, "This", "copy from chars");
    // Test copying to and from Pascal.
    testTarget.copyToPascalString(testBuffer);
    this->test((testBuffer[0] == 4) && (testBuffer[1] == 'T') && (testBuffer[2] = 'h') && (testBuffer[3] == 'i') && (testBuffer[4] == 's'), "copy to Pascal");
    testTarget = "           "; // clear out some of what we expect to alter
    testTarget.copyFromPascalString(testBuffer);
    this->test(testTarget, "This", "copy from Pascal");
    testTarget = "It's only important that this string is longer than 255 chars, because 255 is the limit of what you can legally fit in a Pascal string. We are trying to validate that when given a really long VString, the function for copying into a Pascal string buffer is correctly limiting the number of characters copied out to exactly 255, and setting the length byte accordingly.";
    this->test(testTarget.length() > 255, "copy to Pascal limit setup");
    testTarget.copyToPascalString(testBuffer);
    this->test((testBuffer[0] == (char) 0xFF) && (testBuffer[255] == testTarget[254]), "copy to Pascal limit");
    
    // Test substring operations.
    s = "The Big Heat";
    VString    sub;
    s.getSubstring(sub, 0, 3);            // start of string
    this->test(sub, "The", "substring test 1");
    s.getSubstring(sub, 1, 3);            // one past start of string
    this->test(sub, "he", "substring test 2");
    s.getSubstring(sub, -4, 3);            // start of string but out of range
    this->test(sub, "The", "substring test 3");
    s.getSubstring(sub, 8);                // end of string with default
    this->test(sub, "Heat", "substring test 4");
    s.getSubstring(sub, 8, 12);            // end of string exactly
    this->test(sub, "Heat", "substring test 5");
    s.getSubstring(sub, 8, 11);            // one short of end of string
    this->test(sub, "Hea", "substring test 6");
    s.getSubstring(sub, 8, 15);            // end of string but out of range
    this->test(sub, "Heat", "substring test 7");
    s.getSubstring(sub, 4, 7);            // interior of string
    this->test(sub, "Big", "substring test 8");
    s.getSubstring(sub, 0);                // entire string
    this->test(sub, "The Big Heat", "substring test 9");
    s.getSubstring(sub, -5);            // entire string but start out of range, end default
    this->test(sub, "The Big Heat", "substring test 10");
    s.getSubstring(sub, 0, 50);            // entire string but end out of range
    this->test(sub, "The Big Heat", "substring test 11");
    s.getSubstring(sub, -7, 70);        // entire string but start and end out of range
    this->test(sub, "The Big Heat", "substring test 12");
    
    // Test substring-in-place operations.
    s = "The Big Heat"; s.substringInPlace(0, 3);            // start of string
    this->test(s, "The", "substring-in-place test 1");
    s = "The Big Heat"; s.substringInPlace(1, 3);            // one past start of string
    this->test(s, "he", "substring-in-place test 2");
    s = "The Big Heat"; s.substringInPlace(-4, 3);            // start of string but out of range
    this->test(s, "The", "substring-in-place test 3");
    s = "The Big Heat"; s.substringInPlace(8);                // end of string with default
    this->test(s, "Heat", "substring-in-place test 4");
    s = "The Big Heat"; s.substringInPlace(8, 12);            // end of string exactly
    this->test(s, "Heat", "substring-in-place test 5");
    s = "The Big Heat"; s.substringInPlace(8, 11);            // one short of end of string
    this->test(s, "Hea", "substring-in-place test 6");
    s = "The Big Heat"; s.substringInPlace(8, 15);            // end of string but out of range
    this->test(s, "Heat", "substring-in-place test 7");
    s = "The Big Heat"; s.substringInPlace(4, 7);            // interior of string
    this->test(s, "Big", "substring-in-place test 8");
    s = "The Big Heat"; s.substringInPlace(0);                // entire string
    this->test(s, "The Big Heat", "substring-in-place test 9");
    s = "The Big Heat"; s.substringInPlace(-5);            // entire string but start out of range, end default
    this->test(s, "The Big Heat", "substring-in-place test 10");
    s = "The Big Heat"; s.substringInPlace(0, 50);            // entire string but end out of range
    this->test(s, "The Big Heat", "substring-in-place test 11");
    s = "The Big Heat"; s.substringInPlace(-7, 70);        // entire string but start and end out of range
    this->test(s, "The Big Heat", "substring-in-place test 12");
    
    // Test insert operations.
    s = "ABCDEFGH";
    s.insert('x');    // insert char at start (0)
    this->test(s, "xABCDEFGH", "insert test 1");
    s.insert("QRS");    // insert string at start (0)
    this->test(s, "QRSxABCDEFGH", "insert test 2");
    s.insert('y', 4);    // insert char at some offset
    this->test(s, "QRSxyABCDEFGH", "insert test 3");
    s.insert("TUV", 3);    // insert string at some offset
    this->test(s, "QRSTUVxyABCDEFGH", "insert test 4");
    s.insert('j', s.length());    // insert char at end
    this->test(s, "QRSTUVxyABCDEFGHj", "insert test 5");
    s.insert("KLM", s.length());    // insert string at end
    this->test(s, "QRSTUVxyABCDEFGHjKLM", "insert test 6");
    s.insert('n', s.length() - 1);    // insert char at (end - 1)
    this->test(s, "QRSTUVxyABCDEFGHjKLnM", "insert test 7");
    s.insert("HELLO", s.length() - 1);    // insert string at (end - 1)
    this->test(s, "QRSTUVxyABCDEFGHjKLnHELLOM", "insert test 8");
    s.insert("");    // insert empty string at start (0)
    this->test(s, "QRSTUVxyABCDEFGHjKLnHELLOM", "insert test 8");
    s.insert("", 8);    // insert empty string at some offset
    this->test(s, "QRSTUVxyABCDEFGHjKLnHELLOM", "insert test 8");
    s.insert("", s.length());    // insert empty string at end
    this->test(s, "QRSTUVxyABCDEFGHjKLnHELLOM", "insert test 8");
    s.insert("", s.length() - 1);    // insert empty string at (end - 1)
    this->test(s, "QRSTUVxyABCDEFGHjKLnHELLOM", "insert test 8");
    
    // Test inserts on an empty string.
    s = VString::kEmptyString;
    s.insert('x');
    this->test(s, "x", "insert test 9");
    s = VString::kEmptyString;
    s.insert("ABC");
    this->test(s, "ABC", "insert test 10");
    s = VString::kEmptyString;
    s.insert('x', 5);    // this will also test out-of-bounds handling (currently it forces in-bounds; I think an exception would be better)
    this->test(s, "x", "insert test 9");
    s = VString::kEmptyString;
    s.insert("ABC", 5);    // this will also test out-of-bounds handling (currently it forces in-bounds; I think an exception would be better)
    this->test(s, "ABC", "insert test 10");
    
    // What the heck, let's do those same tests with an unallocated string buffer. Should be the same since preflight will always allocate the required buffer.
    { VString s2;
        s2.insert('x');
        this->test(s2, "x", "insert test 11"); }
    { VString s2;
        s2.insert("ABC");
        this->test(s2, "ABC", "insert test 12"); }
    { VString s2;
        s2.insert('x', 5);    // this will also test out-of-bounds handling (currently it forces in-bounds; I think an exception would be better)
        this->test(s2, "x", "insert test 13"); }
    { VString s2;
        s2.insert("ABC", 5);    // this will also test out-of-bounds handling (currently it forces in-bounds; I think an exception would be better)
        this->test(s2, "ABC", "insert test 14"); }
    
    // We also need to verify that insert handles inserting from itself.
    s = "California";
    s.insert(s);
    this->test(s, "CaliforniaCalifornia", "insert test 15");
    s = "Oregon";
    s.insert(s, 3);
    this->test(s, "OreOregongon", "insert test 16");
    s = "Nevada";
    s.insert(s, 5);
    this->test(s, "NevadNevadaa", "insert test 17");
    s = "Arizona";
    s.insert(s, s.length());
    this->test(s, "ArizonaArizona", "insert test 18");
    
    // Test trim operation.
    s = "This string should not be trimmed.";
    s.trim();
    this->test(s, "This string should not be trimmed.", "trim test 1");
    s = "   This string had leading whitespace.";
    s.trim();
    this->test(s, "This string had leading whitespace.", "trim test 2");
    s = "This string had trailing whitespace.    ";
    s.trim();
    this->test(s, "This string had trailing whitespace.", "trim test 3");
    s = "    This string had leading and trailing whitespace.    ";
    s.trim();
    this->test(s, "This string had leading and trailing whitespace.", "trim test 4");
    s = "    ";
    s.trim();
    this->test(s, VString::kEmptyString, "trim test 5");
    s = "";
    s.trim();
    this->test(s, VString::kEmptyString, "trim test 6");
    
    int    numCreatures;
    s = "one fish, two fish, red fish, blue fish, fishfishfish";
    // Test replacing with longer string.
    numCreatures = s.replace("fish", "dog");
    this->test(s, "one dog, two dog, red dog, blue dog, dogdogdog", "replace test 1a");
    this->test(numCreatures == 7, "replace test 1b");
    // Test replacing with shorter string.
    numCreatures = s.replace("dog", "fish");
    this->test(s, "one fish, two fish, red fish, blue fish, fishfishfish", "replace test 2a");
    this->test(numCreatures == 7, "replace test 2b");
    // Test replacing with same length string.
    numCreatures = s.replace("fish", "bird");
    this->test(s, "one bird, two bird, red bird, blue bird, birdbirdbird", "replace test 3a");
    this->test(numCreatures == 7, "replace test 3b");
    // Test replacing with empty string.
    numCreatures = s.replace("bird", VString::kEmptyString);
    this->test(s, "one , two , red , blue , ", "replace test 4a");
    this->test(numCreatures == 7, "replace test 4b");
    // Test string-not-found.
    numCreatures = s.replace("dogs", "cats");
    this->test(s, "one , two , red , blue , ", "replace test 5a");
    this->test(numCreatures == 0, "replace test 5b");
    // Test finding an empty string. Should never "find" an empty string.
    numCreatures = s.replace(VString::kEmptyString, "uh-oh");
    this->test(s, "one , two , red , blue , ", "replace test 6a");
    this->test(numCreatures == 0, "replace test 6b");
    // Test replace method with char parameters, search char in string.
    numCreatures = s.replace(VChar('e'), VChar('E'));
    this->test(s, "onE , two , rEd , bluE , ", "replace test 7a");
    this->test(numCreatures == 3, "replace test 7b");
    // Test replace method with char parameters, search char not in string.
    numCreatures = s.replace(VChar('k'), VChar('K'));
    this->test(s, "onE , two , rEd , bluE , ", "replace test 8a");
    this->test(numCreatures == 0, "replace test 8b");
    // Test array operator assignment.
    s[0] = 'O';
    s[6] = 'T';
    s[12] = 'R';
    s[18] = 'B';
    this->test(s, "OnE , Two , REd , BluE , ", "array operator assignment");
    s.set(20, 'e');
    s.set(21, 'u');
    this->test(s, "OnE , Two , REd , Bleu , ", "set() assignment");

    // Test operator= conversions.
    // For each integer size/kind, we make sure to test unsigned, "big" unsigned (too big for signed), negative, and postive.
    int        ni = -1;
    int        pi = 1;
    Vu8        u8 = 2;
    Vu8        b8 = 0xFE;
    Vs8        n8 = -2;
    Vs8        p8 = 2;
    Vu16    u16 = 3;
    Vu16    b16 = 0xFFFD;
    Vs16    n16 = -3;
    Vs16    p16 = 3;
    Vu32    u32 = 4;
    Vu32    b32 = 0xFFFFFFFC;
    Vs32    n32 = -4;
    Vs32    p32 = 4;
    Vu64    u64 = CONST_U64(5);
//    Vu64    b64 = CONST_U64(0xFFFFFFFFFFFFFFFB);
    Vs64    n64 = CONST_S64(-5);
    Vs64    p64 = CONST_S64(5);

    s = ni; this->test(s == "-1", "=ni");
    s = pi; this->test(s == "1", "=pi");
    s = u8; this->test(s == "2", "=u8");
    s = b8; this->test(s == "254", "=b8");
    s = n8; this->test(s == "-2", "=n8");
    s = p8; this->test(s == "2", "=p8");
    s = u16; this->test(s == "3", "=u16");
    s = b16; this->test(s == "65533", "=b16");
    s = n16; this->test(s == "-3", "=n16");
    s = p16; this->test(s == "3", "=p16");
    s = u32; this->test(s == "4", "=u32");
    s = b32; this->test(s == "4294967292", "=b32");
    s = n32; this->test(s == "-4", "=n32");
    s = p32; this->test(s == "4", "=p32");
    s = u64; this->test(s == "5", "=u64");
//    s = b64; this->test(s == "?", "=b64");    // what is 64-bit "-5" as postive decimal? need 64-bit hex calculator
    s = n64; this->test(s == "-5", "=n64");
    s = p64; this->test(s == "5", "=p64");
        
    // Test operator+= conversions.

    s = "x"; s += ni; this->test(s == "x-1", "+=ni");
    s = "x"; s += pi; this->test(s == "x1", "+=pi");
    s = "x"; s += u8; this->test(s == "x2", "+=u8");
    s = "x"; s += b8; this->test(s == "x254", "+=b8");
    s = "x"; s += n8; this->test(s == "x-2", "+=n8");
    s = "x"; s += p8; this->test(s == "x2", "+=p8");
    s = "x"; s += u16; this->test(s == "x3", "+=u16");
    s = "x"; s += b16; this->test(s == "x65533", "+=b16");
    s = "x"; s += n16; this->test(s == "x-3", "+=n16");
    s = "x"; s += p16; this->test(s == "x3", "+=p16");
    s = "x"; s += u32; this->test(s == "x4", "+=u32");
    s = "x"; s += b32; this->test(s == "x4294967292", "+=b32");
    s = "x"; s += n32; this->test(s == "x-4", "+=n32");
    s = "x"; s += p32; this->test(s == "x4", "+=p32");
    s = "x"; s += u64; this->test(s == "x5", "+=u64");
//    s = "x"; s += b64; this->test(s == "x?", "+=b64");    // what is 64-bit "-5" as postive decimal? need 64-bit hex calculator
    s = "x"; s += n64; this->test(s == "x-5", "+=n64");
    s = "x"; s += p64; this->test(s == "x5", "+=p64");
    
    // Miscellaneous API coverage.
    s = "12345";
    s.truncateLength(3);
    this->test(s == "123", "truncate length");
    
    s = "foo";
    this->test(! s.isEmpty(), "not is empty");
    s = VString::kEmptyString;
    this->test(s.isEmpty(), "is empty");
    
    s = "hello";
    
    VChar e = s.at(1);
    this->test(e == 'e', "at");
    e = s[1];
    this->test(e == 'e', "VChar[]");
    char& cref = s[1];
    this->test(cref == 'e', "char&[]");
    this->test(s.charAt(1) == 'e', "charAt");

    s = "Stringinastring";    // Note that "in" appears in 3 places.
    this->test(s.indexOf('i') == 3, "indexOf(char)");
    this->test(s.indexOf('i', 4) == 6, "indexOf(char, n)");
    this->test(s.indexOf('i', 7) == 12, "indexOf(char, n)");
    this->test(s.indexOf('i', 13) == -1, "indexOf(char, n)");
    this->test(s.indexOf('z') == -1, "indexOf(char, n)");
    this->test(s.indexOf("in") == 3, "indexOf(const VString&)");
    this->test(s.indexOf("in", 4) == 6, "indexOf(const VString&, n)");
    this->test(s.indexOf("in", 7) == 12, "indexOf(const VString&, n)");
    this->test(s.indexOf("in", 13) == -1, "indexOf(const VString&, n)");
    this->test(s.indexOf("inordinate") == -1, "indexOf(const VString&)");
    this->test(s.lastIndexOf('i') == 12, "lastIndexOf(char)");
    this->test(s.lastIndexOf('i', 11) == 6, "lastIndexOf(char, n)");
    this->test(s.lastIndexOf('i', 5) == 3, "lastIndexOf(char, n)");
    this->test(s.lastIndexOf('i', 2) == -1, "lastIndexOf(char, n)");
    this->test(s.lastIndexOf('z') == -1, "lastIndexOf(char, n)");
    this->test(s.lastIndexOf("in") == 12, "lastIndexOf(const VString&)");
    this->test(s.lastIndexOf("in", 11) == 6, "lastIndexOf(const VString&, n)");
    this->test(s.lastIndexOf("in", 5) == 3, "lastIndexOf(const VString&, n)");
    this->test(s.lastIndexOf("in", 2) == -1, "lastIndexOf(const VString&, n)");
    this->test(s.lastIndexOf("inordinate") == -1, "lastIndexOf(const VString&)");
    
    VString    region1("Thunderhill");
    VString    region2("under");
    VString    region3("hil");
    this->test(region1.regionMatches(2, region2, 0, 5), "regionMatches 1");
    this->test(region1.regionMatches(7, region3, 0, 3), "regionMatches 2");
    this->test(! region1.regionMatches(7, region3, 0, 4), "! regionMatches 1");
    this->test(! region2.regionMatches(0, region3, 0, 3), "! regionMatches 2");
    }

