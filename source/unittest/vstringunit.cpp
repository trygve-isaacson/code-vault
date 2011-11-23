/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

/** @file */

#include "vstringunit.h"
#include "vchar.h"
#include "vexception.h"

VStringUnit::VStringUnit(bool logOnSuccess, bool throwOnError) :
    VUnit("VStringUnit", logOnSuccess, throwOnError) {
}

void VStringUnit::run() {
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
    this->test(s.startsWithIgnoreCase("bAN"), "startsWithIgnoreCase literal");
    this->test(s.startsWith('B'), "startsWith char");
    this->test(! s.startsWith("Bananas"), "! startsWith literal 1");
    this->test(! s.startsWith("Baz"), "! startsWith literal 2");
    this->test(! s.startsWithIgnoreCase("bANx"), "! startsWithIgnoreCase literal 1");
    this->test(! s.startsWithIgnoreCase("xbAN"), "! startsWithIgnoreCase literal 2");
    this->test(! s.startsWith('b'), "! startsWith char");
    this->test(s.endsWith("nana"), "endsWith literal");
    this->test(s.endsWithIgnoreCase("nANa"), "endsWithIgnoreCase literal");
    this->test(s.endsWith('a'), "endsWith char");
    this->test(! s.endsWith("Yellow Banana"), "! endsWith literal 1");
    this->test(! s.endsWith("abcdefghijklmnopqrstuvwxyz"), "! endsWith literal 2");
    this->test(! s.endsWithIgnoreCase("XnANa"), "! endsWithIgnoreCase literal 1");
    this->test(! s.endsWithIgnoreCase("nANaX"), "! endsWithIgnoreCase literal 2");
    this->test(! s.endsWith('x'), "! endsWith char");

    // Test empty string constant behavior.
    this->test(VString::EMPTY().isEmpty(), "kEmptyString is empty");
    this->test(VString::EMPTY().length() == 0, "kEmptyString length is zero");
    this->test(VString::EMPTY() == "", "kEmptyString equals empty string literal");
    s.format("A%sB", VString::EMPTY().chars());
    this->test(s == "AB", "kEmptyString is empty formatting element");
    s = "";
    this->test(s == VString::EMPTY(), "kEmptyString equals an empty VString");
    VString newlyConstructedString;
    this->test(newlyConstructedString == VString::EMPTY(), "kEmptyString equals a new constructed VString");

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

#ifdef VAULT_VARARG_STRING_FORMATTING_SUPPORT
    // Test formatting.
    const char* nullPointer = NULL;
    VString nullFormatted(nullPointer);
    this->test(nullFormatted, VString::EMPTY(), "null ctor formatting");

    VString    formatted(VSTRING_ARGS("%s is %d years old", "Spot", 5));
    this->test(formatted, "Spot is 5 years old", "ctor formatting");

    formatted.format("%s is %d years old", "Rover", 3);
    this->test(formatted, "Rover is 3 years old", "sprintf");

    formatted.format(nullPointer);
    this->test(formatted, VString::EMPTY(), "null formatting");
#endif

    VString preflightFail("d'oh!");
    try {
        this->logStatus("VStringUnit will now intentionally invoke a memory allocation failure in VString::preflight.");
        std::cout << "VStringUnit will now intentionally invoke a memory allocation failure in VString::preflight; this may result in console error output." << std::endl;
#ifndef VPLATFORM_WIN
        preflightFail.preflightWithSimulatedFailure(); // on Linux and Mac, preflight() succeeds in allocated gigantic buffers!
#else
        preflightFail.preflight(static_cast<int>(V_MAX_S32 - CONST_S64(1)));
#endif
        this->test(false, "Intentional preflight allocation failure"); // If we get here, the test failed.
    } catch (const VException& ex) {
        this->logStatus(ex.what());
        this->test(true, "Intentional preflight allocation failure"); // If we get here, the test succeeded.
        this->test(preflightFail, "d'oh!", "No change during preflight allocation failure"); // verify that the string was not changed during the failure
    }

    // Test copying out.
    char    testBuffer[256];    // Largest legal Pascal string buffer.
    VString    testSource("This text should be copied out.");
    testSource.copyToBuffer(testBuffer, sizeof(testBuffer));
    VString    testTarget(testBuffer);
    this->test(testTarget, "This text should be copied out.", "copy to chars");
    // Test copying out to undersized buffer.
    char    smallBuffer[5]; // holds a string of length 4, plus a null terminator
    VString smallFit3("abc");
    smallFit3.copyToBuffer(smallBuffer, 5);
    VString smallVerify3(smallBuffer);
    this->test(smallFit3, smallVerify3, "copyToBuffer len = n-2");
    VString smallFit4("defg");
    smallFit4.copyToBuffer(smallBuffer, 5);
    VString smallVerify4(smallBuffer);
    this->test(smallFit4, smallVerify4, "copyToBuffer len = n-1");
    VString smallWontFit5("ghijk");
    smallWontFit5.copyToBuffer(smallBuffer, 5);
    VString smallVerify5(smallBuffer);
    this->test("ghij", smallVerify5, "copyToBuffer len = n");
    // Test copying in.
    testTarget = "           "; // clear out some of what we expect to alter
    testTarget.copyFromBuffer(testBuffer, 0, 4);
    this->test(testTarget, "This", "copy from chars");
    // Test copying to and from Pascal.
    testTarget.copyToPascalString(testBuffer);
    this->test((testBuffer[0] == 4) && (testBuffer[1] == 'T') && (testBuffer[2] == 'h') && (testBuffer[3] == 'i') && (testBuffer[4] == 's'), "copy to Pascal");
    testTarget = "           "; // clear out some of what we expect to alter
    testTarget.copyFromPascalString(testBuffer);
    this->test(testTarget, "This", "copy from Pascal");
    testTarget = "It's only important that this string is longer than 255 chars, because 255 is the limit of what you can legally fit in a Pascal string. We are trying to validate that when given a really long VString, the function for copying into a Pascal string buffer is correctly limiting the number of characters copied out to exactly 255, and setting the length byte accordingly.";
    this->test(testTarget.length() > 255, "copy to Pascal limit setup");
    testTarget.copyToPascalString(testBuffer);
    this->test(((Vu8)testBuffer[0] == 255) && (testBuffer[255] == testTarget[254]), "copy to Pascal limit");

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
    s = VString::EMPTY();
    s.insert('x');
    this->test(s, "x", "insert test 9");
    s = VString::EMPTY();
    s.insert("ABC");
    this->test(s, "ABC", "insert test 10");
    s = VString::EMPTY();
    s.insert('x', 5);    // this will also test out-of-bounds handling (currently it forces in-bounds; I think an exception would be better)
    this->test(s, "x", "insert test 9");
    s = VString::EMPTY();
    s.insert("ABC", 5);    // this will also test out-of-bounds handling (currently it forces in-bounds; I think an exception would be better)
    this->test(s, "ABC", "insert test 10");

    // What the heck, let's do those same tests with an unallocated string buffer. Should be the same since preflight will always allocate the required buffer.
    {
        VString s2;
        s2.insert('x');
        this->test(s2, "x", "insert test 11");
    }
    {
        VString s2;
        s2.insert("ABC");
        this->test(s2, "ABC", "insert test 12");
    }
    {
        VString s2;
        s2.insert('x', 5);    // this will also test out-of-bounds handling (currently it forces in-bounds; I think an exception would be better)
        this->test(s2, "x", "insert test 13");
    }
    {
        VString s2;
        s2.insert("ABC", 5);    // this will also test out-of-bounds handling (currently it forces in-bounds; I think an exception would be better)
        this->test(s2, "ABC", "insert test 14");
    }

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
    this->test(s, VString::EMPTY(), "trim test 5");
    s = "";
    s.trim();
    this->test(s, VString::EMPTY(), "trim test 6");

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
    numCreatures = s.replace("bird", VString::EMPTY());
    this->test(s, "one , two , red , blue , ", "replace test 4a");
    this->test(numCreatures == 7, "replace test 4b");
    // Test string-not-found.
    numCreatures = s.replace("dogs", "cats");
    this->test(s, "one , two , red , blue , ", "replace test 5a");
    this->test(numCreatures == 0, "replace test 5b");
    // Test finding an empty string. Should never "find" an empty string.
    numCreatures = s.replace(VString::EMPTY(), "uh-oh");
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

    // Case-insensitive replace() validation:
    int numOccurrences;
    s = "Send lawyers, guns, more LAWYERS, and money.";
    numOccurrences = s.replace("Lawyers", "doctors", false /* not case-sensitive search */);
    this->test(s, "Send doctors, guns, more doctors, and money.", "replace test case-insensitive 1a");
    this->test(numOccurrences == 2, "replace test case-insensitive 1b");
    numOccurrences = s.replace(VChar('S'), VChar('X'), false /* not case-sensitive search */);
    this->test(s, "Xend doctorX, gunX, more doctorX, and money.", "replace test case-insensitive 2a");
    this->test(numOccurrences == 4, "replace test case-insensitive 2b");

    // Test operator= conversions.
    // For each integer size/kind, we make sure to test unsigned, "big" unsigned (too big for signed), negative, and postive.
    int        ni = -1;
    int        posi = 1;
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
    s = posi; this->test(s == "1", "=posi");
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
    s = "x"; s += posi; this->test(s == "x1", "+=posi");
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
    s = VString::EMPTY();
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
    this->test(s.indexOf('i', -1) == -1, "indexOf(char, -1)");
    this->test(s.indexOf('i', -2) == -1, "indexOf(char, -2)");
    this->test(s.indexOf('i', s.length()) == -1, "indexOf(char, end)");
    this->test(s.contains('i'), "contains(char)");
    this->test(!s.contains('x'), "!contains(char)");
    this->test(s.contains('i', 12), "contains(char, 12)");
    this->test(!s.contains('i', 13), "!contains(char, 13)");
    this->test(!s.contains('i', -1), "!contains(char, -1)");
    this->test(!s.contains('i', -2), "!contains(char, -2)");
    this->test(s.indexOf("in") == 3, "indexOf(const VString&)");
    this->test(s.indexOf("in", 4) == 6, "indexOf(const VString&, n)");
    this->test(s.indexOf("in", 7) == 12, "indexOf(const VString&, n)");
    this->test(s.indexOf("in", 13) == -1, "indexOf(const VString&, n)");
    this->test(s.indexOf("in", -1) == -1, "indexOf(const VString&, -1)");
    this->test(s.indexOf("in", -2) == -1, "indexOf(const VString&, -2)");
    this->test(s.indexOf("in", s.length()) == -1, "indexOf(const VString&, end)");
    this->test(s.indexOf("inordinate") == -1, "indexOf(const VString&)");
    this->test(s.contains("in"), "contains(const VString&)");
    this->test(!s.contains("xxx"), "!contains(const VString&)");
    this->test(s.contains("in", 12), "contains(const VString&, 12)");
    this->test(!s.contains("in", 13), "!contains(const VString&, 13)");
    this->test(!s.contains("in", -1), "!contains(const VString&, -1)");
    this->test(!s.contains("in", -2), "!contains(const VString&, -2)");

    this->test(s.indexOfIgnoreCase('I') == 3, "indexOfIgnoreCase(char)");
    this->test(s.indexOfIgnoreCase('I', 4) == 6, "indexOfIgnoreCase(char, n)");
    this->test(s.indexOfIgnoreCase('I', 7) == 12, "indexOfIgnoreCase(char, n)");
    this->test(s.indexOfIgnoreCase('I', 13) == -1, "indexOfIgnoreCase(char, n)");
    this->test(s.indexOfIgnoreCase('I', -1) == -1, "indexOfIgnoreCase(char, -1)");
    this->test(s.indexOfIgnoreCase('I', -2) == -1, "indexOfIgnoreCase(char, -2)");
    this->test(s.indexOfIgnoreCase('I', s.length()) == -1, "indexOfIgnoreCase(char, end)");
    this->test(s.containsIgnoreCase('I'), "contains(char)");
    this->test(!s.containsIgnoreCase('x'), "!containsIgnoreCase(char)");
    this->test(s.containsIgnoreCase('I', 12), "containsIgnoreCase(char, 12)");
    this->test(!s.containsIgnoreCase('I', 13), "!containsIgnoreCase(char, 13)");
    this->test(!s.containsIgnoreCase('I', -1), "!containsIgnoreCase(char, -1)");
    this->test(!s.containsIgnoreCase('I', -2), "!containsIgnoreCase(char, -2)");
    this->test(s.indexOfIgnoreCase('Z') == -1, "indexOfIgnoreCase(char, n)");
    this->test(s.indexOfIgnoreCase("In") == 3, "indexOfIgnoreCase(const VString&)");
    this->test(s.indexOfIgnoreCase("In", 4) == 6, "indexOfIgnoreCase(const VString&, n)");
    this->test(s.indexOfIgnoreCase("In", 7) == 12, "indexOfIgnoreCase(const VString&, n)");
    this->test(s.indexOfIgnoreCase("In", 13) == -1, "indexOfIgnoreCase(const VString&, n)");
    this->test(s.indexOfIgnoreCase("In", -1) == -1, "indexOfIgnoreCase(const VString&, -1)");
    this->test(s.indexOfIgnoreCase("In", -2) == -1, "indexOfIgnoreCase(const VString&, -2)");
    this->test(s.indexOfIgnoreCase("In", s.length()) == -1, "indexOfIgnoreCase(const VString&, end)");
    this->test(s.indexOfIgnoreCase("Inordinate") == -1, "indexOfIgnoreCase(const VString&)");
    this->test(s.containsIgnoreCase("In"), "containsIgnoreCase(const VString&)");
    this->test(!s.containsIgnoreCase("xxx"), "!containsIgnoreCase(const VString&)");
    this->test(s.containsIgnoreCase("In", 12), "containsIgnoreCase(const VString&, 12)");
    this->test(!s.containsIgnoreCase("In", 13), "!containsIgnoreCase(const VString&, 13)");
    this->test(!s.containsIgnoreCase("In", -1), "!containsIgnoreCase(const VString&, -1)");
    this->test(!s.containsIgnoreCase("In", -2), "!containsIgnoreCase(const VString&, -2)");

    this->test(s.lastIndexOf('i') == 12, "lastIndexOf(char)");
    this->test(s.lastIndexOf('i', 11) == 6, "lastIndexOf(char, n)");
    this->test(s.lastIndexOf('i', 5) == 3, "lastIndexOf(char, n)");
    this->test(s.lastIndexOf('i', 2) == -1, "lastIndexOf(char, n)");
    this->test(s.lastIndexOf('i', -2) == -1, "lastIndexOf(char, -2)");
    this->test(s.lastIndexOf('z') == -1, "lastIndexOf(char, n)");
    this->test(s.lastIndexOf("in") == 12, "lastIndexOf(const VString&)");
    this->test(s.lastIndexOf("in", 11) == 6, "lastIndexOf(const VString&, n)");
    this->test(s.lastIndexOf("in", 5) == 3, "lastIndexOf(const VString&, n)");
    this->test(s.lastIndexOf("in", 2) == -1, "lastIndexOf(const VString&, n)");
    this->test(s.lastIndexOf("in", -2) == -1, "lastIndexOf(const VString&, -2)");
    this->test(s.lastIndexOf("inordinate") == -1, "lastIndexOf(const VString&)");

    this->test(s.lastIndexOfIgnoreCase('I') == 12, "lastIndexOfIgnoreCase(char)");
    this->test(s.lastIndexOfIgnoreCase('I', 11) == 6, "lastIndexOfIgnoreCase(char, n)");
    this->test(s.lastIndexOfIgnoreCase('I', 5) == 3, "lastIndexOfIgnoreCase(char, n)");
    this->test(s.lastIndexOfIgnoreCase('I', 2) == -1, "lastIndexOfIgnoreCase(char, n)");
    this->test(s.lastIndexOfIgnoreCase('I', -2) == -1, "lastIndexOfIgnoreCase(char, -2)");
    this->test(s.lastIndexOfIgnoreCase('Z') == -1, "lastIndexOfIgnoreCase(char, n)");
    this->test(s.lastIndexOfIgnoreCase("In") == 12, "lastIndexOfIgnoreCase(const VString&)");
    this->test(s.lastIndexOfIgnoreCase("In", 11) == 6, "lastIndexOfIgnoreCase(const VString&, n)");
    this->test(s.lastIndexOfIgnoreCase("In", 5) == 3, "lastIndexOfIgnoreCase(const VString&, n)");
    this->test(s.lastIndexOfIgnoreCase("In", 2) == -1, "lastIndexOfIgnoreCase(const VString&, n)");
    this->test(s.lastIndexOfIgnoreCase("In", -2) == -1, "lastIndexOfIgnoreCase(const VString&, -2)");
    this->test(s.lastIndexOfIgnoreCase("Inordinate") == -1, "lastIndexOfIgnoreCase(const VString&)");

    VString    region1("Thunderhill");
    VString    region1mixed("tHunderHill");
    VString    region2("under");
    VString    region2mixed("uNDEr");
    VString    region3("hil");
    VString    region3mixed("hIL");
    this->test(region1.regionMatches(2, region2, 0, 5), "regionMatches 1");
    this->test(region1.regionMatches(7, region3, 0, 3), "regionMatches 2");
    this->test(! region1.regionMatches(7, region3, 0, 4), "! regionMatches 1");
    this->test(! region2.regionMatches(0, region3, 0, 3), "! regionMatches 2");
    this->test(region1mixed.regionMatches(2, region2mixed, 0, 5, false /* not case-sensitive */), "regionMatches 1 case insensitive");
    this->test(region1mixed.regionMatches(7, region3mixed, 0, 3, false /* not case-sensitive */), "regionMatches 2 case insensitive");
    this->test(! region1mixed.regionMatches(7, region3mixed, 0, 4, false /* not case-sensitive */), "! regionMatches 1 case insensitive");
    this->test(! region2mixed.regionMatches(0, region3mixed, 0, 3, false /* not case-sensitive */), "! regionMatches 2 case insensitive");

#ifdef VAULT_BOOST_STRING_FORMATTING_SUPPORT
    boost::format formatter("Descending order arguments: %3% %2% %1%.");
    formatter % "one" % 2.47 % 3;

    VString fmt1(formatter);
    this->test(fmt1 == "Descending order arguments: 3 2.47 one.", "format constructor");

    VString fmt2("This should get overwritten.");
    fmt2 = formatter;
    this->test(fmt2 == "Descending order arguments: 3 2.47 one.", "format operator=");

    VString prefix("Append here: ");
    VString fmt3 = prefix + formatter;
    this->test(fmt3 == "Append here: Descending order arguments: 3 2.47 one.", "format operator+");

    VString fmt4("Append here: ");
    fmt4 += formatter;
    this->test(fmt4 == "Append here: Descending order arguments: 3 2.47 one.", "format operator+=");
#endif

    // This set of tests covers valid and invalid input to postflight and thus _setLength.
    VString rangeTester;
    rangeTester.postflight(0); // should succeed since no buffer is necessary
    this->test(true, "postflight 0 for null buffer");

    try {
        rangeTester.postflight(-1); // should throw a VRangeException
        this->test(false, "postflight -1 exception for null buffer");
    } catch (const VRangeException& /*ex*/) {
        this->test(true, "postflight -1 exception for null buffer");
    }

    try {
        rangeTester.postflight(1); // should throw a VRangeException
        this->test(false, "postflight >0 exception for null buffer");
    } catch (const VRangeException& /*ex*/) {
        this->test(true, "postflight >0 exception for null buffer");
    }

    // Note: Now that VString uses chunk-sized allocations, a test
    // of postflight cannot assume the exact buffer size created by
    // preflight. So here our negative test uses a very large value
    // that is larger than the chunk size; if the chunk size constant
    // used by preflight is changed, this test may need to be updated.

    rangeTester.preflight(3); // just enough room for "abc"
    char* buffer = rangeTester.buffer();
    buffer[0] = 'a'; buffer[1] = 'b'; buffer[2] = 'c'; buffer[3] = 0;
    try {
        rangeTester.postflight(200); // should throw a VRangeException if value is too large compared to preflight chunk size
        this->test(false, "postflight >=mBufferLength exception");
    } catch (const VRangeException& /*ex*/) {
        this->test(true, "postflight >=mBufferLength exception");
    }

    rangeTester.postflight(3); // should succeed
    this->test(true, "postflight mBufferLength-1");

    // These tests cover invalid input to preflight.
    try {
        rangeTester.preflight(-1); // should throw a VRangeException
        this->test(false, "preflight <0 exception");
    } catch (const VRangeException& /*ex*/) {
        this->test(true, "preflight <0 exception");
    }

    // Test handling of null terminating character access.
    const VString nullCharString;
    VChar nullVChar;

    nullVChar = nullCharString.at(0);
    this->test(nullVChar == VChar::NULL_CHAR(), "null VChar at(0)");

    nullVChar = nullCharString[0];
    this->test(nullVChar == VChar::NULL_CHAR(), "null VChar [0]");

    char nullChar = nullCharString[0];
    this->test(nullChar == (char) 0, "null char [0]");

    VString nonConstNullCharString;
    // This one should go out of bounds and throw
    try {
        nonConstNullCharString[0] = '!';
        this->test(false, "null char& [0] did not throw the correct exception");
    } catch (const VException& /*ex*/) {
        this->test(true, "null char& [0] threw the correct exception");
    }

    VString parseTest;

    // Positive tests.
    parseTest = "12345";
    this->test(parseTest.parseInt() == 12345, "parseInt a");
    parseTest = "-4567";
    this->test(parseTest.parseInt() == -4567, "parseInt b");
    parseTest = "+2468";
    this->test(parseTest.parseInt() == 2468, "parseInt c");
    parseTest = "42000000000";
    this->test(parseTest.parseS64() == CONST_S64(42000000000), "parseS64 a");
    parseTest = "-43000000000";
    this->test(parseTest.parseS64() == CONST_S64(-43000000000), "parseS64 b");
    parseTest.format(VSTRING_FORMATTER_U64, CONST_U64(0x8000000000001111));
    this->test(parseTest.parseU64() == CONST_U64(0x8000000000001111), "parseU64 a");
    parseTest = "1.23456";
    this->test(parseTest.parseDouble() == 1.23456, "parseDouble a");
    parseTest = "1.23456e+3";
    this->test(parseTest.parseDouble() == 1234.56, "parseDouble b");
    parseTest = "123456";
    this->test(parseTest.parseDouble() == 123456.0, "parseDouble c");
    parseTest = "";
    this->test(parseTest.parseDouble() == 0.0, "parseDouble d");
    // Seems like these should throw, but sscanf accepts them. parseDouble could
    // use some more strict additional checking.
    parseTest = "1..3";
    this->test(parseTest.parseDouble() == 1.0, "parseDouble e");
    parseTest = "1.2e!4";
    this->test(parseTest.parseDouble() == 1.2, "parseDouble f");

    // Negative tests.
    try {
        parseTest = "12.345";
        (void) parseTest.parseInt();
        this->test(false, "parseInt with illegal decimal");
    } catch (const VException& /*ex*/) {
        this->test(true, "parseInt with illegal decimal");
    }

    try {
        parseTest = "12-345";
        (void) parseTest.parseInt();
        this->test(false, "parseInt with out of order minus");
    } catch (const VException& /*ex*/) {
        this->test(true, "parseInt with out of order minus");
    }

    try {
        parseTest = "12+345";
        (void) parseTest.parseInt();
        this->test(false, "parseInt with out of order plus");
    } catch (const VException& /*ex*/) {
        this->test(true, "parseInt with out of order plus");
    }

    try {
        parseTest = "12q345";
        (void) parseTest.parseInt();
        this->test(false, "parseInt with illegal character");
    } catch (const VException& /*ex*/) {
        this->test(true, "parseInt with illegal character");
    }

    try {
        parseTest = "foo";
        (void) parseTest.parseDouble();
        this->test(false, "parseDouble with bad format a");
    } catch (const VException& /*ex*/) {
        this->test(true, "parseDouble with bad format a");
    }

    // Bug fix validation: Take a substring of an empty string that has no buffer.

    VString initializedEmptyString; // initialized to empty means it has no buffer
    VString shouldBecomeEmpty("1234567");
    initializedEmptyString.getSubstring(shouldBecomeEmpty, 0, 2);
    VUNIT_ASSERT_EQUAL_LABELED(shouldBecomeEmpty, VString::EMPTY(), "substring of an initialized empty string");

    VString forcedToEmptyString("abcdef");
    forcedToEmptyString.truncateLength(0); // make it empty; old way keeps buffer, new way discards buffer
    shouldBecomeEmpty = "123456789";
    forcedToEmptyString.getSubstring(shouldBecomeEmpty, 0, 2);
    VUNIT_ASSERT_EQUAL_LABELED(shouldBecomeEmpty, VString::EMPTY(), "substring of a truncated to empty string");

    initializedEmptyString.substringInPlace(0, 2);
    VUNIT_ASSERT_EQUAL_LABELED(initializedEmptyString, VString::EMPTY(), "substring-in-place of an initialized empty string");

    forcedToEmptyString.substringInPlace(0, 2);
    VUNIT_ASSERT_EQUAL_LABELED(forcedToEmptyString, VString::EMPTY(), "substring-in-place of a truncated to empty string");

    // New API: split()

    VStringVector splitResult;
    VString splitInput("one,two,three,,fivee"); // extra ee used for trailing split test

    // simple split
    splitInput.split(splitResult, ','); // "one" "two" "three" "" "fivee"
    VUNIT_ASSERT_EQUAL_LABELED((int) splitResult.size(), 5, "split test 1 size");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[0], "one",   "split test 1 [0]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[1], "two",   "split test 1 [1]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[2], "three", "split test 1 [2]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[3], "",      "split test 1 [3]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[4], "fivee", "split test 1 [4]");
    VStringVector returnResult1 = splitInput.split(',');
    this->test(returnResult1 == splitResult, "split return 1");

    // limited split
    splitInput.split(splitResult, ',', 3); // "one" "two", "three,,fivee"
    VUNIT_ASSERT_EQUAL_LABELED((int) splitResult.size(), 3, "split test 2 size");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[0], "one",         "split test 2 [0]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[1], "two",         "split test 2 [1]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[2], "three,,fivee", "split test 2 [2]");
    VStringVector returnResult2 = splitInput.split(',', 3);
    this->test(returnResult2 == splitResult, "split return 2");

    // strip trailing empty strings
    splitInput.split(splitResult, 'e'); // "on" ",two,thr", "", ",four,fiv" "" <-- last one should get discarded
    VUNIT_ASSERT_EQUAL_LABELED((int) splitResult.size(), 4, "split test 3 size");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[0], "on",        "split test 3 [0]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[1], ",two,thr",  "split test 3 [1]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[2], "",          "split test 3 [2]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[3], ",,fiv",     "split test 3 [3]");
    VStringVector returnResult3 = splitInput.split('e');
    this->test(returnResult3 == splitResult, "split return 3");

    // don't strip trailing empty strings
    splitInput.split(splitResult, 'e', 0, false); // "on" ",two,thr", "", ",four,fiv" "" <-- last one should NOT get discarded
    VUNIT_ASSERT_EQUAL_LABELED((int) splitResult.size(), 5, "split test 4 size");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[0], "on",        "split test 4 [0]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[1], ",two,thr",  "split test 4 [1]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[2], "",          "split test 4 [2]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[3], ",,fiv",     "split test 4 [3]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[4], "",          "split test 4 [4]");
    VStringVector returnResult4 = splitInput.split('e', 0, false);
    this->test(returnResult4 == splitResult, "split return 4");

    // Change to vararg constructor to allow "%" to avoid unwanted formatting.
    VString percentSign("%");
    this->test(percentSign == '%', "percent sign literal constructor");
}

