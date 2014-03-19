/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vstringunit.h"
#include "vchar.h"
#include "vexception.h"
#include "vhex.h"

static int _getOffset(void* objectPtr, void* fieldPtr) {
    Vs64 objAddr = (Vs64) objectPtr;
    Vs64 fieldAddr = (Vs64) fieldPtr;
    Vs64 delta = fieldAddr - objAddr;

    return (int) delta;
}

VStringUnit::VStringUnit(bool logOnSuccess, bool throwOnError) :
    VUnit("VStringUnit", logOnSuccess, throwOnError) {
}

void VStringUnit::run() {
    // Start by testing assignment and concatenation.
    VString    s("(A)");

    VUNIT_ASSERT_EQUAL_LABELED(s, "(A)", "literal ctor");

    s += s;
    VUNIT_ASSERT_EQUAL_LABELED(s, "(A)(A)", "self concat");

    s += "(B)";
    VUNIT_ASSERT_EQUAL_LABELED(s, "(A)(A)(B)", "literal concat");

    s += s;
    VUNIT_ASSERT_EQUAL_LABELED(s, "(A)(A)(B)(A)(A)(B)", "self concat 2");

    s += "(C)";
    VUNIT_ASSERT_EQUAL_LABELED(s, "(A)(A)(B)(A)(A)(B)(C)", "literal concat 2");

    s = s;
    VUNIT_ASSERT_EQUAL_LABELED(s, "(A)(A)(B)(A)(A)(B)(C)", "self assign");

    s.toLowerCase();
    VUNIT_ASSERT_EQUAL_LABELED(s, "(a)(a)(b)(a)(a)(b)(c)", "to lower case");

    s.toUpperCase();
    VUNIT_ASSERT_EQUAL_LABELED(s, "(A)(A)(B)(A)(A)(B)(C)", "to upper case");

    // Test the length.
    VUNIT_ASSERT_EQUAL_LABELED(s.length(), 21, "length");

    // Test array indexing.
    VUNIT_ASSERT_TRUE_LABELED(s[19] == 'C', "array element");
    VUNIT_ASSERT_TRUE_LABELED(s.charAt(19) == 'C', "char at");

    // Test operator+.
    VString    sum1 = VString('X') + 'Y';
    VUNIT_ASSERT_EQUAL_LABELED(sum1, "XY", "operator+ char");
    VString    sum2 = VString("school") + "bus";
    VUNIT_ASSERT_EQUAL_LABELED(sum2, "schoolbus", "operator+ char*");
    VString    sum3 = VString("race") + VString("car");
    VUNIT_ASSERT_EQUAL_LABELED(sum3, "racecar", "operator+ VString&");

    // Test comparison and equality.
    s = "Banana";
    VUNIT_ASSERT_TRUE_LABELED("Apple" < s, "operator <");
    VUNIT_ASSERT_TRUE_LABELED("Banana" <= s, "operator <=");
    VUNIT_ASSERT_TRUE_LABELED("Banana" == s, "operator ==");
    VUNIT_ASSERT_TRUE_LABELED("Banana" >= s, "operator >=");
    VUNIT_ASSERT_TRUE_LABELED("Cherry" > s, "operator >");
    VUNIT_ASSERT_TRUE_LABELED("BANANA" != s, "operator !=");
    VUNIT_ASSERT_TRUE_LABELED(s.equalsIgnoreCase("BANANA"), "equalsIgnoreCase");
    VUNIT_ASSERT_TRUE_LABELED(s.equalsIgnoreCase(VString("BANANA")), "equalsIgnoreCase");
    VUNIT_ASSERT_FALSE_LABELED(s.equalsIgnoreCase("Fanana"), "! equalsIgnoreCase");
    VUNIT_ASSERT_FALSE_LABELED(s.equalsIgnoreCase(VString("Fanana")), "! equalsIgnoreCase");

    VUNIT_ASSERT_TRUE_LABELED(s.compare(VString("Apple")) > 0, "compare >");
    VUNIT_ASSERT_TRUE_LABELED(s.compare("Apple") > 0, "compare >");
    VUNIT_ASSERT_TRUE_LABELED(s.compare(VString("Banana")) == 0, "compare ==");
    VUNIT_ASSERT_TRUE_LABELED(s.compare("Banana") == 0, "compare ==");
    VUNIT_ASSERT_TRUE_LABELED(s.compare(VString("Cherry")) < 0, "compare <");
    VUNIT_ASSERT_TRUE_LABELED(s.compare("Cherry") < 0, "compare <");

    VUNIT_ASSERT_TRUE_LABELED(s.compareIgnoreCase(VString("Apple")) > 0, "compareIgnoreCase >");
    VUNIT_ASSERT_TRUE_LABELED(s.compareIgnoreCase("Apple") > 0, "compareIgnoreCase >");
    VUNIT_ASSERT_TRUE_LABELED(s.compareIgnoreCase(VString("Banana")) == 0, "compareIgnoreCase ==");
    VUNIT_ASSERT_TRUE_LABELED(s.compareIgnoreCase("Banana") == 0, "compareIgnoreCase ==");
    VUNIT_ASSERT_TRUE_LABELED(s.compareIgnoreCase(VString("Cherry")) < 0, "compareIgnoreCase <");
    VUNIT_ASSERT_TRUE_LABELED(s.compareIgnoreCase("Cherry") < 0, "compareIgnoreCase <");

    VUNIT_ASSERT_TRUE_LABELED(s.startsWith("Ban"), "startsWith literal");
    VUNIT_ASSERT_TRUE_LABELED(s.startsWithIgnoreCase("bAN"), "startsWithIgnoreCase literal");
    VUNIT_ASSERT_TRUE_LABELED(s.startsWith('B'), "startsWith char");
    VUNIT_ASSERT_FALSE_LABELED(s.startsWith("Bananas"), "! startsWith literal 1");
    VUNIT_ASSERT_FALSE_LABELED(s.startsWith("Baz"), "! startsWith literal 2");
    VUNIT_ASSERT_FALSE_LABELED(s.startsWithIgnoreCase("bANx"), "! startsWithIgnoreCase literal 1");
    VUNIT_ASSERT_FALSE_LABELED(s.startsWithIgnoreCase("xbAN"), "! startsWithIgnoreCase literal 2");
    VUNIT_ASSERT_FALSE_LABELED(s.startsWith('b'), "! startsWith char");
    VUNIT_ASSERT_TRUE_LABELED(s.endsWith("nana"), "endsWith literal");
    VUNIT_ASSERT_TRUE_LABELED(s.endsWithIgnoreCase("nANa"), "endsWithIgnoreCase literal");
    VUNIT_ASSERT_TRUE_LABELED(s.endsWith('a'), "endsWith char");
    VUNIT_ASSERT_FALSE_LABELED(s.endsWith("Yellow Banana"), "! endsWith literal 1");
    VUNIT_ASSERT_FALSE_LABELED(s.endsWith("abcdefghijklmnopqrstuvwxyz"), "! endsWith literal 2");
    VUNIT_ASSERT_FALSE_LABELED(s.endsWithIgnoreCase("XnANa"), "! endsWithIgnoreCase literal 1");
    VUNIT_ASSERT_FALSE_LABELED(s.endsWithIgnoreCase("nANaX"), "! endsWithIgnoreCase literal 2");
    VUNIT_ASSERT_FALSE_LABELED(s.endsWith('x'), "! endsWith char");

    // Test empty string constant behavior.
    VUNIT_ASSERT_TRUE_LABELED(VString::EMPTY().isEmpty(), "kEmptyString is empty");
    VUNIT_ASSERT_TRUE_LABELED(VString::EMPTY().length() == 0, "kEmptyString length is zero");
    VUNIT_ASSERT_TRUE_LABELED(VString::EMPTY() == "", "kEmptyString equals empty string literal");
    s.format("A%sB", VString::EMPTY().chars());
    VUNIT_ASSERT_EQUAL_LABELED(s, "AB", "kEmptyString is empty formatting element");
    s = "";
    VUNIT_ASSERT_EQUAL_LABELED(s, VString::EMPTY(), "kEmptyString equals an empty VString");
    VString newlyConstructedString;
    VUNIT_ASSERT_EQUAL_LABELED(newlyConstructedString, VString::EMPTY(), "kEmptyString equals a new constructed VString");

    // Test assigning empty strings into non-empty strings.
    s = "foo";
    s = newlyConstructedString;
    VUNIT_ASSERT_TRUE_LABELED(s.isEmpty(), "Assign empty VString&");
    s = &newlyConstructedString;
    VUNIT_ASSERT_TRUE_LABELED(s.isEmpty(), "Assign empty VString*");
    s = (VString*) NULL;
    VUNIT_ASSERT_TRUE_LABELED(s.isEmpty(), "Assign NULL VString*");
    s = "";
    VUNIT_ASSERT_TRUE_LABELED(s.isEmpty(), "Assign empty char*");
    s = (char*) NULL;
    VUNIT_ASSERT_TRUE_LABELED(s.isEmpty(), "Assign NULL char*");

    // Test re-assignment and non-shared memory.
    VString    a("a");
    VString    b("b");

    a = b;
    b = "something else";

    VUNIT_ASSERT_EQUAL_LABELED(a, "b", "reassignment 1");
    VUNIT_ASSERT_EQUAL_LABELED(b, "something else", "reassignment 2");

#ifdef VAULT_VARARG_STRING_FORMATTING_SUPPORT
    // Test formatting.
    const char* nullPointer = NULL;
    VString nullFormatted(nullPointer);
    VUNIT_ASSERT_EQUAL_LABELED(nullFormatted, VString::EMPTY(), "null ctor formatting");

    VString    formatted(VSTRING_ARGS("%s is %d years old", "Spot", 5));
    VUNIT_ASSERT_EQUAL_LABELED(formatted, "Spot is 5 years old", "ctor formatting");

    formatted.format("%s is %d years old", "Rover", 3);
    VUNIT_ASSERT_EQUAL_LABELED(formatted, "Rover is 3 years old", "sprintf");

    formatted.format(nullPointer);
    VUNIT_ASSERT_EQUAL_LABELED(formatted, VString::EMPTY(), "null formatting");
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
        VUNIT_ASSERT_FAILURE("Intentional preflight allocation failure"); // If we get here, the test failed.
    } catch (const VException& ex) {
        this->logStatus(ex.what());
        VUNIT_ASSERT_SUCCESS("Intentional preflight allocation failure"); // If we get here, the test succeeded.
        VUNIT_ASSERT_EQUAL_LABELED(preflightFail, "d'oh!", "No change during preflight allocation failure"); // verify that the string was not changed during the failure
    }

    // Test copying out.
    char    testBuffer[256];    // Largest legal Pascal string buffer.
    VString    testSource("This text should be copied out.");
    testSource.copyToBuffer(testBuffer, sizeof(testBuffer));
    VString    testTarget(testBuffer);
    VUNIT_ASSERT_EQUAL_LABELED(testTarget, "This text should be copied out.", "copy to chars");
    // Test copying out to undersized buffer.
    char    smallBuffer[5]; // holds a string of length 4, plus a null terminator
    VString smallFit3("abc");
    smallFit3.copyToBuffer(smallBuffer, 5);
    VString smallVerify3(smallBuffer);
    VUNIT_ASSERT_EQUAL_LABELED(smallFit3, smallVerify3, "copyToBuffer len = n-2");
    VString smallFit4("defg");
    smallFit4.copyToBuffer(smallBuffer, 5);
    VString smallVerify4(smallBuffer);
    VUNIT_ASSERT_EQUAL_LABELED(smallFit4, smallVerify4, "copyToBuffer len = n-1");
    VString smallWontFit5("ghijk");
    smallWontFit5.copyToBuffer(smallBuffer, 5);
    VString smallVerify5(smallBuffer);
    VUNIT_ASSERT_EQUAL_LABELED("ghij", smallVerify5, "copyToBuffer len = n");
    // Test copying in.
    testTarget = "           "; // clear out some of what we expect to alter
    testTarget.copyFromBuffer(testBuffer, 0, 4);
    VUNIT_ASSERT_EQUAL_LABELED(testTarget, "This", "copy from chars");
    // Test copying to and from Pascal.
    testTarget.copyToPascalString(testBuffer);
    VUNIT_ASSERT_TRUE_LABELED((testBuffer[0] == 4) && (testBuffer[1] == 'T') && (testBuffer[2] == 'h') && (testBuffer[3] == 'i') && (testBuffer[4] == 's'), "copy to Pascal");
    testTarget = "           "; // clear out some of what we expect to alter
    testTarget.copyFromPascalString(testBuffer);
    VUNIT_ASSERT_EQUAL_LABELED(testTarget, "This", "copy from Pascal");
    testTarget = "It's only important that this string is longer than 255 chars, because 255 is the limit of what you can legally fit in a Pascal string. We are trying to validate that when given a really long VString, the function for copying into a Pascal string buffer is correctly limiting the number of characters copied out to exactly 255, and setting the length byte accordingly.";
    VUNIT_ASSERT_TRUE_LABELED(testTarget.length() > 255, "copy to Pascal limit setup");
    testTarget.copyToPascalString(testBuffer);
    VUNIT_ASSERT_TRUE_LABELED(((Vu8)testBuffer[0] == 255) && (testBuffer[255] == testTarget[254]), "copy to Pascal limit");

    // Test substring operations.
    s = "The Big Heat";
    VString    sub;
    s.getSubstring(sub, 0, 3);            // start of string
    VUNIT_ASSERT_EQUAL_LABELED(sub, "The", "substring test 1");
    s.getSubstring(sub, 1, 3);            // one past start of string
    VUNIT_ASSERT_EQUAL_LABELED(sub, "he", "substring test 2");
    s.getSubstring(sub, -4, 3);            // start of string but out of range
    VUNIT_ASSERT_EQUAL_LABELED(sub, "The", "substring test 3");
    s.getSubstring(sub, 8);                // end of string with default
    VUNIT_ASSERT_EQUAL_LABELED(sub, "Heat", "substring test 4");
    s.getSubstring(sub, 8, 12);            // end of string exactly
    VUNIT_ASSERT_EQUAL_LABELED(sub, "Heat", "substring test 5");
    s.getSubstring(sub, 8, 11);            // one short of end of string
    VUNIT_ASSERT_EQUAL_LABELED(sub, "Hea", "substring test 6");
    s.getSubstring(sub, 8, 15);            // end of string but out of range
    VUNIT_ASSERT_EQUAL_LABELED(sub, "Heat", "substring test 7");
    s.getSubstring(sub, 4, 7);            // interior of string
    VUNIT_ASSERT_EQUAL_LABELED(sub, "Big", "substring test 8");
    s.getSubstring(sub, 0);                // entire string
    VUNIT_ASSERT_EQUAL_LABELED(sub, "The Big Heat", "substring test 9");
    s.getSubstring(sub, -5);            // entire string but start out of range, end default
    VUNIT_ASSERT_EQUAL_LABELED(sub, "The Big Heat", "substring test 10");
    s.getSubstring(sub, 0, 50);            // entire string but end out of range
    VUNIT_ASSERT_EQUAL_LABELED(sub, "The Big Heat", "substring test 11");
    s.getSubstring(sub, -7, 70);        // entire string but start and end out of range
    VUNIT_ASSERT_EQUAL_LABELED(sub, "The Big Heat", "substring test 12");

    // Test substring-in-place operations.
    s = "The Big Heat"; s.substringInPlace(0, 3);            // start of string
    VUNIT_ASSERT_EQUAL_LABELED(s, "The", "substring-in-place test 1");
    s = "The Big Heat"; s.substringInPlace(1, 3);            // one past start of string
    VUNIT_ASSERT_EQUAL_LABELED(s, "he", "substring-in-place test 2");
    s = "The Big Heat"; s.substringInPlace(-4, 3);            // start of string but out of range
    VUNIT_ASSERT_EQUAL_LABELED(s, "The", "substring-in-place test 3");
    s = "The Big Heat"; s.substringInPlace(8);                // end of string with default
    VUNIT_ASSERT_EQUAL_LABELED(s, "Heat", "substring-in-place test 4");
    s = "The Big Heat"; s.substringInPlace(8, 12);            // end of string exactly
    VUNIT_ASSERT_EQUAL_LABELED(s, "Heat", "substring-in-place test 5");
    s = "The Big Heat"; s.substringInPlace(8, 11);            // one short of end of string
    VUNIT_ASSERT_EQUAL_LABELED(s, "Hea", "substring-in-place test 6");
    s = "The Big Heat"; s.substringInPlace(8, 15);            // end of string but out of range
    VUNIT_ASSERT_EQUAL_LABELED(s, "Heat", "substring-in-place test 7");
    s = "The Big Heat"; s.substringInPlace(4, 7);            // interior of string
    VUNIT_ASSERT_EQUAL_LABELED(s, "Big", "substring-in-place test 8");
    s = "The Big Heat"; s.substringInPlace(0);                // entire string
    VUNIT_ASSERT_EQUAL_LABELED(s, "The Big Heat", "substring-in-place test 9");
    s = "The Big Heat"; s.substringInPlace(-5);            // entire string but start out of range, end default
    VUNIT_ASSERT_EQUAL_LABELED(s, "The Big Heat", "substring-in-place test 10");
    s = "The Big Heat"; s.substringInPlace(0, 50);            // entire string but end out of range
    VUNIT_ASSERT_EQUAL_LABELED(s, "The Big Heat", "substring-in-place test 11");
    s = "The Big Heat"; s.substringInPlace(-7, 70);        // entire string but start and end out of range
    VUNIT_ASSERT_EQUAL_LABELED(s, "The Big Heat", "substring-in-place test 12");

    // Test insert operations.
    s = "ABCDEFGH";
    s.insert('x');    // insert char at start (0)
    VUNIT_ASSERT_EQUAL_LABELED(s, "xABCDEFGH", "insert test 1");
    s.insert("QRS");    // insert string at start (0)
    VUNIT_ASSERT_EQUAL_LABELED(s, "QRSxABCDEFGH", "insert test 2");
    s.insert('y', 4);    // insert char at some offset
    VUNIT_ASSERT_EQUAL_LABELED(s, "QRSxyABCDEFGH", "insert test 3");
    s.insert("TUV", 3);    // insert string at some offset
    VUNIT_ASSERT_EQUAL_LABELED(s, "QRSTUVxyABCDEFGH", "insert test 4");
    s.insert('j', s.length());    // insert char at end
    VUNIT_ASSERT_EQUAL_LABELED(s, "QRSTUVxyABCDEFGHj", "insert test 5");
    s.insert("KLM", s.length());    // insert string at end
    VUNIT_ASSERT_EQUAL_LABELED(s, "QRSTUVxyABCDEFGHjKLM", "insert test 6");
    s.insert('n', s.length() - 1);    // insert char at (end - 1)
    VUNIT_ASSERT_EQUAL_LABELED(s, "QRSTUVxyABCDEFGHjKLnM", "insert test 7");
    s.insert("HELLO", s.length() - 1);    // insert string at (end - 1)
    VUNIT_ASSERT_EQUAL_LABELED(s, "QRSTUVxyABCDEFGHjKLnHELLOM", "insert test 8");
    s.insert("");    // insert empty string at start (0)
    VUNIT_ASSERT_EQUAL_LABELED(s, "QRSTUVxyABCDEFGHjKLnHELLOM", "insert test 8");
    s.insert("", 8);    // insert empty string at some offset
    VUNIT_ASSERT_EQUAL_LABELED(s, "QRSTUVxyABCDEFGHjKLnHELLOM", "insert test 8");
    s.insert("", s.length());    // insert empty string at end
    VUNIT_ASSERT_EQUAL_LABELED(s, "QRSTUVxyABCDEFGHjKLnHELLOM", "insert test 8");
    s.insert("", s.length() - 1);    // insert empty string at (end - 1)
    VUNIT_ASSERT_EQUAL_LABELED(s, "QRSTUVxyABCDEFGHjKLnHELLOM", "insert test 8");

    // Test inserts on an empty string.
    s = VString::EMPTY();
    s.insert('x');
    VUNIT_ASSERT_EQUAL_LABELED(s, "x", "insert test 9");
    s = VString::EMPTY();
    s.insert("ABC");
    VUNIT_ASSERT_EQUAL_LABELED(s, "ABC", "insert test 10");
    s = VString::EMPTY();
    s.insert('x', 5);    // this will also test out-of-bounds handling (currently it forces in-bounds; I think an exception would be better)
    VUNIT_ASSERT_EQUAL_LABELED(s, "x", "insert test 9");
    s = VString::EMPTY();
    s.insert("ABC", 5);    // this will also test out-of-bounds handling (currently it forces in-bounds; I think an exception would be better)
    VUNIT_ASSERT_EQUAL_LABELED(s, "ABC", "insert test 10");

    // What the heck, let's do those same tests with an unallocated string buffer. Should be the same since preflight will always allocate the required buffer.
    {
        VString s2;
        s2.insert('x');
        VUNIT_ASSERT_EQUAL_LABELED(s2, "x", "insert test 11");
    }
    {
        VString s2;
        s2.insert("ABC");
        VUNIT_ASSERT_EQUAL_LABELED(s2, "ABC", "insert test 12");
    }
    {
        VString s2;
        s2.insert('x', 5);    // this will also test out-of-bounds handling (currently it forces in-bounds; I think an exception would be better)
        VUNIT_ASSERT_EQUAL_LABELED(s2, "x", "insert test 13");
    }
    {
        VString s2;
        s2.insert("ABC", 5);    // this will also test out-of-bounds handling (currently it forces in-bounds; I think an exception would be better)
        VUNIT_ASSERT_EQUAL_LABELED(s2, "ABC", "insert test 14");
    }

    // We also need to verify that insert handles inserting from itself.
    s = "California";
    s.insert(s);
    VUNIT_ASSERT_EQUAL_LABELED(s, "CaliforniaCalifornia", "insert test 15");
    s = "Oregon";
    s.insert(s, 3);
    VUNIT_ASSERT_EQUAL_LABELED(s, "OreOregongon", "insert test 16");
    s = "Nevada";
    s.insert(s, 5);
    VUNIT_ASSERT_EQUAL_LABELED(s, "NevadNevadaa", "insert test 17");
    s = "Arizona";
    s.insert(s, s.length());
    VUNIT_ASSERT_EQUAL_LABELED(s, "ArizonaArizona", "insert test 18");

    // Test trim operation.
    s = "This string should not be trimmed.";
    s.trim();
    VUNIT_ASSERT_EQUAL_LABELED(s, "This string should not be trimmed.", "trim test 1");
    s = "   This string had leading whitespace.";
    s.trim();
    VUNIT_ASSERT_EQUAL_LABELED(s, "This string had leading whitespace.", "trim test 2");
    s = "This string had trailing whitespace.    ";
    s.trim();
    VUNIT_ASSERT_EQUAL_LABELED(s, "This string had trailing whitespace.", "trim test 3");
    s = "    This string had leading and trailing whitespace.    ";
    s.trim();
    VUNIT_ASSERT_EQUAL_LABELED(s, "This string had leading and trailing whitespace.", "trim test 4");
    s = "    ";
    s.trim();
    VUNIT_ASSERT_EQUAL_LABELED(s, VString::EMPTY(), "trim test 5");
    s = "";
    s.trim();
    VUNIT_ASSERT_EQUAL_LABELED(s, VString::EMPTY(), "trim test 6");

    int    numCreatures;
    // Test replacing the whole string with another value, with empty, and again with repetition.
    // Have the replacement be smaller, to test that we can replace multiple where the "cursor" is not always moving to the right.
    s = "fish";
    numCreatures = s.replace("fish", "dog");
    VUNIT_ASSERT_EQUAL_LABELED(s, "dog", "replace test comparison a->b");
    VUNIT_ASSERT_EQUAL_LABELED(numCreatures, 1, "replace test count a->b");
    s = "fish";
    numCreatures = s.replace("fish", "");
    VUNIT_ASSERT_EQUAL_LABELED(s, "", "replace test comparison a->empty");
    VUNIT_ASSERT_EQUAL_LABELED(numCreatures, 1, "replace test count a->empty");
    s = "fishfishfish";
    numCreatures = s.replace("fish", "dog");
    VUNIT_ASSERT_EQUAL_LABELED(s, "dogdogdog", "replace test comparison aaa->bbb");
    VUNIT_ASSERT_EQUAL_LABELED(numCreatures, 3, "replace test count aaa->bbb");
    s = "fishfishfish";
    numCreatures = s.replace("fish", "");
    VUNIT_ASSERT_EQUAL_LABELED(s, "", "replace test comparison aaa->emptyemptyempty");
    VUNIT_ASSERT_EQUAL_LABELED(numCreatures, 3, "replace test count aaa->emptyemptyempty");

    // Another sequence of replacement tests.
    s = "one fish, two fish, red fish, blue fish, fishfishfish";
    // Test replacing with longer string.
    numCreatures = s.replace("fish", "dog");
    VUNIT_ASSERT_EQUAL_LABELED(s, "one dog, two dog, red dog, blue dog, dogdogdog", "replace test 1a");
    VUNIT_ASSERT_EQUAL_LABELED(numCreatures, 7, "replace test 1b");
    // Test replacing with shorter string.
    numCreatures = s.replace("dog", "fish");
    VUNIT_ASSERT_EQUAL_LABELED(s, "one fish, two fish, red fish, blue fish, fishfishfish", "replace test 2a");
    VUNIT_ASSERT_EQUAL_LABELED(numCreatures, 7, "replace test 2b");
    // Test replacing with same length string.
    numCreatures = s.replace("fish", "bird");
    VUNIT_ASSERT_EQUAL_LABELED(s, "one bird, two bird, red bird, blue bird, birdbirdbird", "replace test 3a");
    VUNIT_ASSERT_EQUAL_LABELED(numCreatures, 7, "replace test 3b");
    // Test replacing with empty string.
    numCreatures = s.replace("bird", VString::EMPTY());
    VUNIT_ASSERT_EQUAL_LABELED(s, "one , two , red , blue , ", "replace test 4a");
    VUNIT_ASSERT_EQUAL_LABELED(numCreatures, 7, "replace test 4b");
    // Test string-not-found.
    numCreatures = s.replace("dogs", "cats");
    VUNIT_ASSERT_EQUAL_LABELED(s, "one , two , red , blue , ", "replace test 5a");
    VUNIT_ASSERT_EQUAL_LABELED(numCreatures, 0, "replace test 5b");
    // Test finding an empty string. Should never "find" an empty string.
    numCreatures = s.replace(VString::EMPTY(), "uh-oh");
    VUNIT_ASSERT_EQUAL_LABELED(s, "one , two , red , blue , ", "replace test 6a");
    VUNIT_ASSERT_EQUAL_LABELED(numCreatures, 0, "replace test 6b");
    // Test replace method with char parameters, search char in string.
    numCreatures = s.replace(VChar('e'), VChar('E'));
    VUNIT_ASSERT_EQUAL_LABELED(s, "onE , two , rEd , bluE , ", "replace test 7a");
    VUNIT_ASSERT_EQUAL_LABELED(numCreatures, 3, "replace test 7b");
    // Test replace method with char parameters, search char not in string.
    numCreatures = s.replace(VChar('k'), VChar('K'));
    VUNIT_ASSERT_EQUAL_LABELED(s, "onE , two , rEd , bluE , ", "replace test 8a");
    VUNIT_ASSERT_EQUAL_LABELED(numCreatures, 0, "replace test 8b");

    // Test array operator assignment.
    s[0] = 'O';
    s[6] = 'T';
    s[12] = 'R';
    s[18] = 'B';
    VUNIT_ASSERT_EQUAL_LABELED(s, "OnE , Two , REd , BluE , ", "array operator assignment");
    s.set(20, 'e');
    s.set(21, 'u');
    VUNIT_ASSERT_EQUAL_LABELED(s, "OnE , Two , REd , Bleu , ", "set() assignment");

    // Case-insensitive replace() validation:
    int numOccurrences;
    s = "Send lawyers, guns, more LAWYERS, and money.";
    numOccurrences = s.replace("Lawyers", "doctors", false /* not case-sensitive search */);
    VUNIT_ASSERT_EQUAL_LABELED(s, "Send doctors, guns, more doctors, and money.", "replace test case-insensitive 1a");
    VUNIT_ASSERT_EQUAL_LABELED(numOccurrences, 2, "replace test case-insensitive 1b");
    numOccurrences = s.replace(VChar('S'), VChar('X'), false /* not case-sensitive search */);
    VUNIT_ASSERT_EQUAL_LABELED(s, "Xend doctorX, gunX, more doctorX, and money.", "replace test case-insensitive 2a");
    VUNIT_ASSERT_EQUAL_LABELED(numOccurrences, 4, "replace test case-insensitive 2b");

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

    s = ni; VUNIT_ASSERT_EQUAL_LABELED(s, "-1", "=ni");
    s = posi; VUNIT_ASSERT_EQUAL_LABELED(s, "1", "=posi");
    s = u8; VUNIT_ASSERT_EQUAL_LABELED(s, "2", "=u8");
    s = b8; VUNIT_ASSERT_EQUAL_LABELED(s, "254", "=b8");
    s = n8; VUNIT_ASSERT_EQUAL_LABELED(s, "-2", "=n8");
    s = p8; VUNIT_ASSERT_EQUAL_LABELED(s, "2", "=p8");
    s = u16; VUNIT_ASSERT_EQUAL_LABELED(s, "3", "=u16");
    s = b16; VUNIT_ASSERT_EQUAL_LABELED(s, "65533", "=b16");
    s = n16; VUNIT_ASSERT_EQUAL_LABELED(s, "-3", "=n16");
    s = p16; VUNIT_ASSERT_EQUAL_LABELED(s, "3", "=p16");
    s = u32; VUNIT_ASSERT_EQUAL_LABELED(s, "4", "=u32");
    s = b32; VUNIT_ASSERT_EQUAL_LABELED(s, "4294967292", "=b32");
    s = n32; VUNIT_ASSERT_EQUAL_LABELED(s, "-4", "=n32");
    s = p32; VUNIT_ASSERT_EQUAL_LABELED(s, "4", "=p32");
    s = u64; VUNIT_ASSERT_EQUAL_LABELED(s, "5", "=u64");
//    s = b64; VUNIT_ASSERT_EQUAL_LABELED(s, "?", "=b64");    // what is 64-bit "-5" as postive decimal? need 64-bit hex calculator
    s = n64; VUNIT_ASSERT_EQUAL_LABELED(s, "-5", "=n64");
    s = p64; VUNIT_ASSERT_EQUAL_LABELED(s, "5", "=p64");

    // Test operator+= conversions.

    s = "x"; s += ni; VUNIT_ASSERT_EQUAL_LABELED(s, "x-1", "+=ni");
    s = "x"; s += posi; VUNIT_ASSERT_EQUAL_LABELED(s, "x1", "+=posi");
    s = "x"; s += u8; VUNIT_ASSERT_EQUAL_LABELED(s, "x2", "+=u8");
    s = "x"; s += b8; VUNIT_ASSERT_EQUAL_LABELED(s, "x254", "+=b8");
    s = "x"; s += n8; VUNIT_ASSERT_EQUAL_LABELED(s, "x-2", "+=n8");
    s = "x"; s += p8; VUNIT_ASSERT_EQUAL_LABELED(s, "x2", "+=p8");
    s = "x"; s += u16; VUNIT_ASSERT_EQUAL_LABELED(s, "x3", "+=u16");
    s = "x"; s += b16; VUNIT_ASSERT_EQUAL_LABELED(s, "x65533", "+=b16");
    s = "x"; s += n16; VUNIT_ASSERT_EQUAL_LABELED(s, "x-3", "+=n16");
    s = "x"; s += p16; VUNIT_ASSERT_EQUAL_LABELED(s, "x3", "+=p16");
    s = "x"; s += u32; VUNIT_ASSERT_EQUAL_LABELED(s, "x4", "+=u32");
    s = "x"; s += b32; VUNIT_ASSERT_EQUAL_LABELED(s, "x4294967292", "+=b32");
    s = "x"; s += n32; VUNIT_ASSERT_EQUAL_LABELED(s, "x-4", "+=n32");
    s = "x"; s += p32; VUNIT_ASSERT_EQUAL_LABELED(s, "x4", "+=p32");
    s = "x"; s += u64; VUNIT_ASSERT_EQUAL_LABELED(s, "x5", "+=u64");
//    s = "x"; s += b64; VUNIT_ASSERT_EQUAL_LABELED(s, "x?", "+=b64");    // what is 64-bit "-5" as postive decimal? need 64-bit hex calculator
    s = "x"; s += n64; VUNIT_ASSERT_EQUAL_LABELED(s, "x-5", "+=n64");
    s = "x"; s += p64; VUNIT_ASSERT_EQUAL_LABELED(s, "x5", "+=p64");

    // Miscellaneous API coverage.
    s = "12345";
    s.truncateLength(3);
    VUNIT_ASSERT_EQUAL_LABELED(s, "123", "truncate length");

    s = "foo";
    VUNIT_ASSERT_FALSE_LABELED(s.isEmpty(), "not is empty");
    s = VString::EMPTY();
    VUNIT_ASSERT_TRUE_LABELED(s.isEmpty(), "is empty");

    s = "hello";

    VChar e = s.at(1);
    VUNIT_ASSERT_EQUAL_LABELED(e, 'e', "at");
    e = s[1];
    VUNIT_ASSERT_EQUAL_LABELED(e, 'e', "VChar[]");
    char& cref = s[1];
    VUNIT_ASSERT_TRUE_LABELED(cref == 'e', "char&[]");
    VUNIT_ASSERT_TRUE_LABELED(s.charAt(1) == 'e', "charAt");

    s = "Stringinastring";    // Note that "in" appears in 3 places.
    VUNIT_ASSERT_TRUE_LABELED(s.indexOf('i') == 3, "indexOf(char)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOf('i', 4) == 6, "indexOf(char, n)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOf('i', 7) == 12, "indexOf(char, n)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOf('i', 13) == -1, "indexOf(char, n)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOf('z') == -1, "indexOf(char, n)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOf('i', -1) == -1, "indexOf(char, -1)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOf('i', -2) == -1, "indexOf(char, -2)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOf('i', s.length()) == -1, "indexOf(char, end)");
    VUNIT_ASSERT_TRUE_LABELED(s.contains('i'), "contains(char)");
    VUNIT_ASSERT_FALSE_LABELED(s.contains('x'), "!contains(char)");
    VUNIT_ASSERT_TRUE_LABELED(s.contains('i', 12), "contains(char, 12)");
    VUNIT_ASSERT_FALSE_LABELED(s.contains('i', 13), "!contains(char, 13)");
    VUNIT_ASSERT_FALSE_LABELED(s.contains('i', -1), "!contains(char, -1)");
    VUNIT_ASSERT_FALSE_LABELED(s.contains('i', -2), "!contains(char, -2)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOf("in") == 3, "indexOf(const VString&)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOf("in", 4) == 6, "indexOf(const VString&, n)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOf("in", 7) == 12, "indexOf(const VString&, n)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOf("in", 13) == -1, "indexOf(const VString&, n)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOf("in", -1) == -1, "indexOf(const VString&, -1)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOf("in", -2) == -1, "indexOf(const VString&, -2)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOf("in", s.length()) == -1, "indexOf(const VString&, end)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOf("inordinate") == -1, "indexOf(const VString&)");
    VUNIT_ASSERT_TRUE_LABELED(s.contains("in"), "contains(const VString&)");
    VUNIT_ASSERT_FALSE_LABELED(s.contains("xxx"), "!contains(const VString&)");
    VUNIT_ASSERT_TRUE_LABELED(s.contains("in", 12), "contains(const VString&, 12)");
    VUNIT_ASSERT_FALSE_LABELED(s.contains("in", 13), "!contains(const VString&, 13)");
    VUNIT_ASSERT_FALSE_LABELED(s.contains("in", -1), "!contains(const VString&, -1)");
    VUNIT_ASSERT_FALSE_LABELED(s.contains("in", -2), "!contains(const VString&, -2)");

    VUNIT_ASSERT_TRUE_LABELED(s.indexOfIgnoreCase('I') == 3, "indexOfIgnoreCase(char)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOfIgnoreCase('I', 4) == 6, "indexOfIgnoreCase(char, n)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOfIgnoreCase('I', 7) == 12, "indexOfIgnoreCase(char, n)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOfIgnoreCase('I', 13) == -1, "indexOfIgnoreCase(char, n)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOfIgnoreCase('I', -1) == -1, "indexOfIgnoreCase(char, -1)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOfIgnoreCase('I', -2) == -1, "indexOfIgnoreCase(char, -2)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOfIgnoreCase('I', s.length()) == -1, "indexOfIgnoreCase(char, end)");
    VUNIT_ASSERT_TRUE_LABELED(s.containsIgnoreCase('I'), "contains(char)");
    VUNIT_ASSERT_FALSE_LABELED(s.containsIgnoreCase('x'), "!containsIgnoreCase(char)");
    VUNIT_ASSERT_TRUE_LABELED(s.containsIgnoreCase('I', 12), "containsIgnoreCase(char, 12)");
    VUNIT_ASSERT_FALSE_LABELED(s.containsIgnoreCase('I', 13), "!containsIgnoreCase(char, 13)");
    VUNIT_ASSERT_FALSE_LABELED(s.containsIgnoreCase('I', -1), "!containsIgnoreCase(char, -1)");
    VUNIT_ASSERT_FALSE_LABELED(s.containsIgnoreCase('I', -2), "!containsIgnoreCase(char, -2)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOfIgnoreCase('Z') == -1, "indexOfIgnoreCase(char, n)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOfIgnoreCase("In") == 3, "indexOfIgnoreCase(const VString&)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOfIgnoreCase("In", 4) == 6, "indexOfIgnoreCase(const VString&, n)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOfIgnoreCase("In", 7) == 12, "indexOfIgnoreCase(const VString&, n)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOfIgnoreCase("In", 13) == -1, "indexOfIgnoreCase(const VString&, n)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOfIgnoreCase("In", -1) == -1, "indexOfIgnoreCase(const VString&, -1)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOfIgnoreCase("In", -2) == -1, "indexOfIgnoreCase(const VString&, -2)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOfIgnoreCase("In", s.length()) == -1, "indexOfIgnoreCase(const VString&, end)");
    VUNIT_ASSERT_TRUE_LABELED(s.indexOfIgnoreCase("Inordinate") == -1, "indexOfIgnoreCase(const VString&)");
    VUNIT_ASSERT_TRUE_LABELED(s.containsIgnoreCase("In"), "containsIgnoreCase(const VString&)");
    VUNIT_ASSERT_FALSE_LABELED(s.containsIgnoreCase("xxx"), "!containsIgnoreCase(const VString&)");
    VUNIT_ASSERT_TRUE_LABELED(s.containsIgnoreCase("In", 12), "containsIgnoreCase(const VString&, 12)");
    VUNIT_ASSERT_FALSE_LABELED(s.containsIgnoreCase("In", 13), "!containsIgnoreCase(const VString&, 13)");
    VUNIT_ASSERT_FALSE_LABELED(s.containsIgnoreCase("In", -1), "!containsIgnoreCase(const VString&, -1)");
    VUNIT_ASSERT_FALSE_LABELED(s.containsIgnoreCase("In", -2), "!containsIgnoreCase(const VString&, -2)");

    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOf('i'), 12, "lastIndexOf(char)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOf('i', 11), 6, "lastIndexOf(char, n)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOf('i', 5), 3, "lastIndexOf(char, n)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOf('i', 2), -1, "lastIndexOf(char, n)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOf('i', -2), -1, "lastIndexOf(char, -2)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOf('z'), -1, "lastIndexOf(char, n)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOf("in"), 12, "lastIndexOf(const VString&)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOf("in", 11), 6, "lastIndexOf(const VString&, n)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOf("in", 5), 3, "lastIndexOf(const VString&, n)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOf("in", 2), -1, "lastIndexOf(const VString&, n)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOf("in", -2), -1, "lastIndexOf(const VString&, -2)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOf("inordinate"), -1, "lastIndexOf(const VString&)");

    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOfIgnoreCase('I'), 12, "lastIndexOfIgnoreCase(char)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOfIgnoreCase('I', 11), 6, "lastIndexOfIgnoreCase(char, n)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOfIgnoreCase('I', 5), 3, "lastIndexOfIgnoreCase(char, n)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOfIgnoreCase('I', 2), -1, "lastIndexOfIgnoreCase(char, n)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOfIgnoreCase('I', -2), -1, "lastIndexOfIgnoreCase(char, -2)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOfIgnoreCase('Z'), -1, "lastIndexOfIgnoreCase(char, n)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOfIgnoreCase("In"), 12, "lastIndexOfIgnoreCase(const VString&)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOfIgnoreCase("In", 11), 6, "lastIndexOfIgnoreCase(const VString&, n)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOfIgnoreCase("In", 5), 3, "lastIndexOfIgnoreCase(const VString&, n)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOfIgnoreCase("In", 2), -1, "lastIndexOfIgnoreCase(const VString&, n)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOfIgnoreCase("In", -2), -1, "lastIndexOfIgnoreCase(const VString&, -2)");
    VUNIT_ASSERT_EQUAL_LABELED(s.lastIndexOfIgnoreCase("Inordinate"), -1, "lastIndexOfIgnoreCase(const VString&)");

    VString    region1("Thunderhill");
    VString    region1mixed("tHunderHill");
    VString    region2("under");
    VString    region2mixed("uNDEr");
    VString    region3("hil");
    VString    region3mixed("hIL");
    VUNIT_ASSERT_TRUE_LABELED(region1.regionMatches(2, region2, 0, 5), "regionMatches 1");
    VUNIT_ASSERT_TRUE_LABELED(region1.regionMatches(7, region3, 0, 3), "regionMatches 2");
    VUNIT_ASSERT_FALSE_LABELED(region1.regionMatches(7, region3, 0, 4), "! regionMatches 1");
    VUNIT_ASSERT_FALSE_LABELED(region2.regionMatches(0, region3, 0, 3), "! regionMatches 2");
    VUNIT_ASSERT_TRUE_LABELED(region1mixed.regionMatches(2, region2mixed, 0, 5, false /* not case-sensitive */), "regionMatches 1 case insensitive");
    VUNIT_ASSERT_TRUE_LABELED(region1mixed.regionMatches(7, region3mixed, 0, 3, false /* not case-sensitive */), "regionMatches 2 case insensitive");
    VUNIT_ASSERT_FALSE_LABELED(region1mixed.regionMatches(7, region3mixed, 0, 4, false /* not case-sensitive */), "! regionMatches 1 case insensitive");
    VUNIT_ASSERT_FALSE_LABELED(region2mixed.regionMatches(0, region3mixed, 0, 3, false /* not case-sensitive */), "! regionMatches 2 case insensitive");

#ifdef VAULT_BOOST_STRING_FORMATTING_SUPPORT
    boost::format formatter("Descending order arguments: %3% %2% %1%.");
    formatter % "one" % 2.47 % 3;

    VString fmt1(formatter);
    VUNIT_ASSERT_EQUAL_LABELED(fmt1, "Descending order arguments: 3 2.47 one.", "format constructor");

    VString fmt2("This should get overwritten.");
    fmt2 = formatter;
    VUNIT_ASSERT_EQUAL_LABELED(fmt2, "Descending order arguments: 3 2.47 one.", "format operator=");

    VString prefix("Append here: ");
    VString fmt3 = prefix + formatter;
    VUNIT_ASSERT_EQUAL_LABELED(fmt3, "Append here: Descending order arguments: 3 2.47 one.", "format operator+");

    VString fmt4("Append here: ");
    fmt4 += formatter;
    VUNIT_ASSERT_EQUAL_LABELED(fmt4, "Append here: Descending order arguments: 3 2.47 one.", "format operator+=");
#endif

    // This set of tests covers valid and invalid input to postflight and thus _setLength.
    VString rangeTester;
    rangeTester.postflight(0); // should succeed since no buffer is necessary
    VUNIT_ASSERT_SUCCESS("postflight 0 for null buffer");

    try {
        rangeTester.postflight(-1); // should throw a VRangeException
        VUNIT_ASSERT_FAILURE("postflight -1 exception for null buffer");
    } catch (const VRangeException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("postflight -1 exception for null buffer");
    }

    /* With SSO support in VString, the following test is no longer valid,
    because a string will always have a buffer. Below we replace this with
    a similar test that does a postflight that is sure to be way out of
    range of the buffer.
    
    try {
        rangeTester.postflight(1); // should throw a VRangeException
        VUNIT_ASSERT_FAILURE("postflight >0 exception for null buffer");
    } catch (const VRangeException&) {
        VUNIT_ASSERT_SUCCESS("postflight >0 exception for null buffer");
    }
    */
    try {
        rangeTester.postflight(INT_MAX); // should throw a VRangeException
        VUNIT_ASSERT_FAILURE("postflight INT_MAX exception for internal buffer");
    } catch (const VRangeException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("postflight INT_MAX exception for internal buffer");
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
        VUNIT_ASSERT_FAILURE("postflight >=mBufferLength exception");
    } catch (const VRangeException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("postflight >=mBufferLength exception");
    }

    rangeTester.postflight(3); // should succeed
    VUNIT_ASSERT_SUCCESS("postflight mBufferLength-1");

    // These tests cover invalid input to preflight.
    try {
        rangeTester.preflight(-1); // should throw a VRangeException
        VUNIT_ASSERT_FAILURE("preflight <0 exception");
    } catch (const VRangeException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("preflight <0 exception");
    }

    // Test handling of null terminating character access.
    const VString nullCharString;
    VChar nullVChar;

    nullVChar = nullCharString.at(0);
    VUNIT_ASSERT_EQUAL_LABELED(nullVChar, VChar::NULL_CHAR(), "null VChar at(0)");

    nullVChar = nullCharString[0];
    VUNIT_ASSERT_EQUAL_LABELED(nullVChar, VChar::NULL_CHAR(), "null VChar [0]");

    char nullChar = nullCharString[0];
    VUNIT_ASSERT_EQUAL_LABELED(nullChar, (char) 0, "null char [0]");

    VString nonConstNullCharString;
    // This one should go out of bounds and throw
    try {
        nonConstNullCharString[0] = '!';
        VUNIT_ASSERT_FAILURE("null char& [0] did not throw the correct exception");
    } catch (const VException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("null char& [0] threw the correct exception");
    }

    VString parseTest;

    // Positive tests.
    parseTest = "12345";
    VUNIT_ASSERT_EQUAL_LABELED(parseTest.parseInt(), 12345, "parseInt a");
    parseTest = "-4567";
    VUNIT_ASSERT_EQUAL_LABELED(parseTest.parseInt(), -4567, "parseInt b");
    parseTest = "+2468";
    VUNIT_ASSERT_EQUAL_LABELED(parseTest.parseInt(), 2468, "parseInt c");
    parseTest = "42000000000";
    VUNIT_ASSERT_EQUAL_LABELED(parseTest.parseS64(), CONST_S64(42000000000), "parseS64 a");
    parseTest = "-43000000000";
    VUNIT_ASSERT_EQUAL_LABELED(parseTest.parseS64(), CONST_S64(-43000000000), "parseS64 b");
    parseTest.format(VSTRING_FORMATTER_U64, CONST_U64(0x8000000000001111));
    VUNIT_ASSERT_EQUAL_LABELED(parseTest.parseU64(), CONST_U64(0x8000000000001111), "parseU64 a");
    parseTest = "1.23456";
    VUNIT_ASSERT_EQUAL_LABELED(parseTest.parseDouble(), 1.23456, "parseDouble a");
    parseTest = "1.23456e+3";
    VUNIT_ASSERT_EQUAL_LABELED(parseTest.parseDouble(), 1234.56, "parseDouble b");
    parseTest = "123456";
    VUNIT_ASSERT_EQUAL_LABELED(parseTest.parseDouble(), 123456.0, "parseDouble c");
    parseTest = "";
    VUNIT_ASSERT_EQUAL_LABELED(parseTest.parseDouble(), 0.0, "parseDouble d");
    // Seems like these should throw, but sscanf accepts them. parseDouble could
    // use some more strict additional checking.
    parseTest = "1..3";
    VUNIT_ASSERT_EQUAL_LABELED(parseTest.parseDouble(), 1.0, "parseDouble e");
    parseTest = "1.2e!4";
    VUNIT_ASSERT_EQUAL_LABELED(parseTest.parseDouble(), 1.2, "parseDouble f");

    // Negative tests.
    try {
        parseTest = "12.345";
        (void) parseTest.parseInt();
        VUNIT_ASSERT_FAILURE("parseInt with illegal decimal");
    } catch (const VException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("parseInt with illegal decimal");
    }

    try {
        parseTest = "12-345";
        (void) parseTest.parseInt();
        VUNIT_ASSERT_FAILURE("parseInt with out of order minus");
    } catch (const VException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("parseInt with out of order minus");
    }

    try {
        parseTest = "12+345";
        (void) parseTest.parseInt();
        VUNIT_ASSERT_FAILURE("parseInt with out of order plus");
    } catch (const VException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("parseInt with out of order plus");
    }

    try {
        parseTest = "12q345";
        (void) parseTest.parseInt();
        VUNIT_ASSERT_FAILURE("parseInt with illegal character");
    } catch (const VException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("parseInt with illegal character");
    }

    try {
        parseTest = "foo";
        (void) parseTest.parseDouble();
        VUNIT_ASSERT_FAILURE("parseDouble with bad format a");
    } catch (const VException& /*ex*/) {
        VUNIT_ASSERT_SUCCESS("parseDouble with bad format a");
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
    VUNIT_ASSERT_TRUE_LABELED(returnResult1 == splitResult, "split return 1");

    // limited split
    splitInput.split(splitResult, ',', 3); // "one" "two", "three,,fivee"
    VUNIT_ASSERT_EQUAL_LABELED((int) splitResult.size(), 3, "split test 2 size");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[0], "one",         "split test 2 [0]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[1], "two",         "split test 2 [1]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[2], "three,,fivee", "split test 2 [2]");
    VStringVector returnResult2 = splitInput.split(',', 3);
    VUNIT_ASSERT_TRUE_LABELED(returnResult2 == splitResult, "split return 2");

    // strip trailing empty strings
    splitInput.split(splitResult, 'e'); // "on" ",two,thr", "", ",four,fiv" "" <-- last one should get discarded
    VUNIT_ASSERT_EQUAL_LABELED((int) splitResult.size(), 4, "split test 3 size");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[0], "on",        "split test 3 [0]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[1], ",two,thr",  "split test 3 [1]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[2], "",          "split test 3 [2]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[3], ",,fiv",     "split test 3 [3]");
    VStringVector returnResult3 = splitInput.split('e');
    VUNIT_ASSERT_TRUE_LABELED(returnResult3 == splitResult, "split return 3");

    // don't strip trailing empty strings
    splitInput.split(splitResult, 'e', 0, false); // "on" ",two,thr", "", ",four,fiv" "" <-- last one should NOT get discarded
    VUNIT_ASSERT_EQUAL_LABELED((int) splitResult.size(), 5, "split test 4 size");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[0], "on",        "split test 4 [0]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[1], ",two,thr",  "split test 4 [1]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[2], "",          "split test 4 [2]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[3], ",,fiv",     "split test 4 [3]");
    VUNIT_ASSERT_EQUAL_LABELED(splitResult[4], "",          "split test 4 [4]");
    VStringVector returnResult4 = splitInput.split('e', 0, false);
    VUNIT_ASSERT_TRUE_LABELED(returnResult4 == splitResult, "split return 4");

    // Change to vararg constructor to allow "%" to avoid unwanted formatting.
    VString percentSign("%");
    VUNIT_ASSERT_EQUAL_LABELED(percentSign, '%', "percent sign literal constructor");

    VString sss;
#ifdef VCOMPILER_64BIT
    this->logStatus("64-bit offsets & sizes:");
#else
    this->logStatus("32-bit offsets & sizes:");
#endif
    this->logStatus(VSTRING_FORMAT("VString                       : " VSTRING_FORMATTER_INT " / " VSTRING_FORMATTER_SIZE, _getOffset(&sss, &sss),                                   sizeof(VString)));
    this->logStatus(VSTRING_FORMAT(" .mU                          : " VSTRING_FORMATTER_INT " / " VSTRING_FORMATTER_SIZE, _getOffset(&sss, &sss.mU),                                sizeof(sss.mU)));
    this->logStatus(VSTRING_FORMAT("  .mI                         : " VSTRING_FORMATTER_INT " / " VSTRING_FORMATTER_SIZE, _getOffset(&sss, &sss.mU.mI),                             sizeof(sss.mU.mI)));
    this->logStatus(VSTRING_FORMAT("   .mStringLength             : " VSTRING_FORMATTER_INT " / " VSTRING_FORMATTER_SIZE, _getOffset(&sss, &sss.mU.mI.mStringLength),               sizeof(sss.mU.mI.mStringLength)));
    this->logStatus(VSTRING_FORMAT("   .mNumCodePoints            : " VSTRING_FORMATTER_INT " / " VSTRING_FORMATTER_SIZE, _getOffset(&sss, &sss.mU.mI.mNumCodePoints),               sizeof(sss.mU.mI.mStringLength)));
    this->logStatus(VSTRING_FORMAT("   .mInternalBuffer           : " VSTRING_FORMATTER_INT " / " VSTRING_FORMATTER_SIZE, _getOffset(&sss, &sss.mU.mI.mInternalBuffer),             sizeof(sss.mU.mI.mInternalBuffer)));
    this->logStatus(VSTRING_FORMAT("  .mX                         : " VSTRING_FORMATTER_INT " / " VSTRING_FORMATTER_SIZE, _getOffset(&sss, &sss.mU.mX),                             sizeof(sss.mU.mX)));
    this->logStatus(VSTRING_FORMAT("   .mStringLength_Alias       : " VSTRING_FORMATTER_INT " / " VSTRING_FORMATTER_SIZE, _getOffset(&sss, &sss.mU.mX.mStringLength_Alias),         sizeof(sss.mU.mX.mStringLength_Alias)));
    this->logStatus(VSTRING_FORMAT("   .mNumCodePoints_Alias      : " VSTRING_FORMATTER_INT " / " VSTRING_FORMATTER_SIZE, _getOffset(&sss, &sss.mU.mX.mNumCodePoints_Alias),         sizeof(sss.mU.mX.mStringLength_Alias)));
    this->logStatus(VSTRING_FORMAT("   .mHeapBufferLength         : " VSTRING_FORMATTER_INT " / " VSTRING_FORMATTER_SIZE, _getOffset(&sss, &sss.mU.mX.mHeapBufferLength),           sizeof(sss.mU.mX.mHeapBufferLength)));
    this->logStatus(VSTRING_FORMAT("   .mHeapBufferPtr            : " VSTRING_FORMATTER_INT " / " VSTRING_FORMATTER_SIZE, _getOffset(&sss, &sss.mU.mX.mHeapBufferPtr),              sizeof(sss.mU.mX.mHeapBufferPtr)));
    
    // Unicode access tests.
    
    VCodePoint simpleNewline('\n');
    VCodePoint simpleSmallA('a');
    VCodePoint simpleCapitalA(65);
    VCodePoint simpleECircumflex(VChar(0xE9)); // Use VChar int constructor to avoid use of char value > 127.
    VCodePoint hexformNewline("U+0A");
    VCodePoint hexformSmallA("U+61");
    VCodePoint hexformCapitalA("U+41");
    VCodePoint hexformECircumflex("U+E9");
    VCodePoint hexformOmega("U+03A9");
    VCodePoint hexformKoala("U+1F428");
    // The four example cases on <http://en.wikipedia.org/wiki/Utf-8>
    VCodePoint dollar("U+0024");
    VCodePoint cent("U+00A2");
    VCodePoint euro("U+20AC");
    VCodePoint han("U+24B62");

    VUNIT_ASSERT_TRUE_LABELED(simpleNewline == hexformNewline, "code point equality - newline");
    VUNIT_ASSERT_EQUAL_LABELED(simpleNewline.intValue(), 0x0A, "code point value - newline");
    VUNIT_ASSERT_TRUE_LABELED(simpleSmallA == hexformSmallA, "code point equality - small a");
    VUNIT_ASSERT_EQUAL_LABELED(simpleSmallA.intValue(), 0x61, "code point value - small a");
    VUNIT_ASSERT_TRUE_LABELED(simpleCapitalA == hexformCapitalA, "code point equality - capital a");
    VUNIT_ASSERT_EQUAL_LABELED(simpleCapitalA.intValue(), 0x41, "code point value - capital a");
    VUNIT_ASSERT_TRUE_LABELED(simpleECircumflex == hexformECircumflex, "code point equality - e circumflex");
    VUNIT_ASSERT_EQUAL_LABELED(simpleECircumflex.intValue(), 0xE9, "code point value - e circumflex");
    VUNIT_ASSERT_EQUAL_LABELED(hexformOmega.intValue(), 0x03A9, "code point value - omega");
    VUNIT_ASSERT_EQUAL_LABELED(hexformKoala.intValue(), 0x01F428, "code point value - koala");
    
    VString sNewline(simpleNewline);
    VString sSmallA(simpleSmallA);
    VString sCapitalA(simpleCapitalA);
    VString sECircumflex(simpleECircumflex);
    VString sOmega(hexformOmega);
    VString sKoala(hexformKoala);
    
    // Interestingly, the following three currency symbols (plus Han character) exercise
    // the 1-, 2-, 3-, and 4-byte UTF-8 formats. The dollar sign is ASCII so it requires
    // one byte, the cent sign is in the 128-255 range so it requires two bytes, and the
    // euro sign is a large number so it requires three bytes; the Han symbol is in a
    // very high number block requiring 4 bytes.

    VString sDollar(dollar);
    VString sCent(cent);
    VString sEuro(euro);
    VString sHan(han);
    
    VString hexDollar; VHex::bufferToHexString(sDollar.getDataBuffer(), sDollar.length(), hexDollar, true);
    VUNIT_ASSERT_EQUAL_LABELED(hexDollar, "0x24", "code point to string - dollar");
    VString hexCent; VHex::bufferToHexString(sCent.getDataBuffer(), sCent.length(), hexCent, true);
    VUNIT_ASSERT_EQUAL_LABELED(hexCent, "0xC2A2", "code point to string - cent");
    VString hexEuro; VHex::bufferToHexString(sEuro.getDataBuffer(), sEuro.length(), hexEuro, true);
    VUNIT_ASSERT_EQUAL_LABELED(hexEuro, "0xE282AC", "code point to string - euro");
    VString hexHan; VHex::bufferToHexString(sHan.getDataBuffer(), sHan.length(), hexHan, true);
    VUNIT_ASSERT_EQUAL_LABELED(hexHan, "0xF0A4ADA2", "code point to string - han");
    
    VString utf8Test = VString("D") + dollar + VString("C") + cent + VString("E") + euro + VString("H") + han;
    VUNIT_ASSERT_EQUAL_LABELED(dollar, *(utf8Test.begin() + 1),     "iterator D addition");
    VUNIT_ASSERT_EQUAL_LABELED(cent,    *(utf8Test.begin() + 3),    "iterator C addition");
    VUNIT_ASSERT_EQUAL_LABELED(euro,    *(utf8Test.begin() + 5),    "iterator E addition");
    VUNIT_ASSERT_EQUAL_LABELED(han,     *(utf8Test.begin() + 7),    "iterator H addition");
    VUNIT_ASSERT_EQUAL_LABELED(han,     *(utf8Test.end() - 1),      "iterator H subtraction");
    VUNIT_ASSERT_EQUAL_LABELED(euro,    *(utf8Test.end() - 3),      "iterator E subtraction");
    VUNIT_ASSERT_EQUAL_LABELED(cent,    *(utf8Test.end() - 5),      "iterator C subtraction");
    VUNIT_ASSERT_EQUAL_LABELED(dollar,  *(utf8Test.end() - 7),      "iterator D subtraction");
    VString::iterator utf8Iterator = utf8Test.begin();
    ++utf8Iterator;
    VUNIT_ASSERT_EQUAL_LABELED(dollar, *utf8Iterator, "iterator D increment");
    ++utf8Iterator;
    ++utf8Iterator;
    VUNIT_ASSERT_EQUAL_LABELED(cent, *utf8Iterator, "iterator C increment");
    ++utf8Iterator;
    ++utf8Iterator;
    VUNIT_ASSERT_EQUAL_LABELED(euro, *utf8Iterator, "iterator E increment");
    ++utf8Iterator;
    ++utf8Iterator;
    VUNIT_ASSERT_EQUAL_LABELED(han, *utf8Iterator, "iterator H increment");
    ++utf8Iterator;
    VUNIT_ASSERT_TRUE_LABELED(utf8Iterator == utf8Test.end(), "iterator increment to end");
    --utf8Iterator;
    VUNIT_ASSERT_EQUAL_LABELED(han, *utf8Iterator, "iterator H decrement");
    --utf8Iterator;
    --utf8Iterator;
    VUNIT_ASSERT_EQUAL_LABELED(euro, *utf8Iterator, "iterator E decrement");
    --utf8Iterator;
    --utf8Iterator;
    VUNIT_ASSERT_EQUAL_LABELED(cent, *utf8Iterator, "iterator C decrement");
    --utf8Iterator;
    --utf8Iterator;
    VUNIT_ASSERT_EQUAL_LABELED(dollar, *utf8Iterator, "iterator D decrement");
    --utf8Iterator;
    VUNIT_ASSERT_TRUE_LABELED(utf8Iterator == utf8Test.begin(), "iterator decrement to begin");
    
    // Test that we generate an exception if we iterate out of bounds.
    try {
        VString::iterator beginIterator = utf8Test.begin();
        --beginIterator;
        VUNIT_ASSERT_TRUE_LABELED(false, "failed to catch expected out of bounds begin-1 iteration");
    } catch (const VRangeException&) {
        VUNIT_ASSERT_TRUE_LABELED(true, "caught expected out of bounds begin-1 iteration");
    }
    
    try {
        VString::iterator beginIterator = utf8Test.end();
        ++beginIterator;
        VUNIT_ASSERT_TRUE_LABELED(false, "failed to catch expected out of bounds end+1 iteration");
    } catch (const VRangeException&) {
        VUNIT_ASSERT_TRUE_LABELED(true, "caught expected out of bounds end+1 iteration");
    }
    
    // Reverse iterator tests.
    // First, mutate the string to prove that its internal code point count is maintained, since
    // the reverse iterator relies on it.
    VString::iterator ri = utf8Test.rbegin();
    VUNIT_ASSERT_EQUAL_LABELED((*ri), han, "reverse_iterator start H");
    ++ri;
    ++ri;
    VUNIT_ASSERT_EQUAL_LABELED((*ri), euro, "reverse_iterator increment E");
    ++ri;
    ++ri;
    VUNIT_ASSERT_EQUAL_LABELED((*ri), cent, "reverse_iterator increment C");
    ++ri;
    ++ri;
    VUNIT_ASSERT_EQUAL_LABELED((*ri), dollar, "reverse_iterator increment D");
    ++ri;
    ++ri;
    VUNIT_ASSERT_TRUE_LABELED(ri == utf8Test.rend(), "reverse_iterator increment to end");
    --ri;
    --ri;
    VUNIT_ASSERT_EQUAL_LABELED((*ri), dollar, "reverse_iterator decrement D");
    --ri;
    --ri;
    VUNIT_ASSERT_EQUAL_LABELED((*ri), cent, "reverse_iterator decrement C");
    --ri;
    --ri;
    VUNIT_ASSERT_EQUAL_LABELED((*ri), euro, "reverse_iterator decrement E");
    --ri;
    --ri;
    VUNIT_ASSERT_EQUAL_LABELED((*ri), han, "reverse_iterator decrement H");
    
    VString stringWithMultibyteCharacters;
    stringWithMultibyteCharacters += "Dollar = '";
    stringWithMultibyteCharacters += dollar;
    stringWithMultibyteCharacters += "'. ";
    stringWithMultibyteCharacters += "Cent = '";
    stringWithMultibyteCharacters += cent;
    stringWithMultibyteCharacters += "'. ";
    stringWithMultibyteCharacters += "Euro = '";
    stringWithMultibyteCharacters += euro;
    stringWithMultibyteCharacters += "'. ";
    stringWithMultibyteCharacters += "Han = '";
    stringWithMultibyteCharacters += han;
    stringWithMultibyteCharacters += "'. ";
    VString reconstructedStringWithMultibyteCharacters;
    
    for (VString::iterator si = stringWithMultibyteCharacters.begin(); si != stringWithMultibyteCharacters.end(); ++si) {
        VCodePoint cp = (*si);
        reconstructedStringWithMultibyteCharacters += cp;
        VString cps(cp);
    }
    
    VUNIT_ASSERT_EQUAL_LABELED(stringWithMultibyteCharacters, reconstructedStringWithMultibyteCharacters, "reconstructed string");
    this->logStatus(stringWithMultibyteCharacters);
    
    // Test code point count bookkeeping after mutation.
    int initialLength = stringWithMultibyteCharacters.length();
    VUNIT_ASSERT_EQUAL_LABELED(stringWithMultibyteCharacters.getNumCodePoints(), 49, "initial num code points");
    stringWithMultibyteCharacters.replace("Dollar", "Pound");   // 1 less character in replacement
    stringWithMultibyteCharacters.replace("$", VCodePoint("U+00A3"));   // 1-for-1 substitution of single byte code point with multi-byte code point
    VUNIT_ASSERT_EQUAL_LABELED(stringWithMultibyteCharacters.getNumCodePoints(), 48, "recalculated num code points");
    // Check the length as well. It's not that the length doesn't change, but we replaced "Dollar" with "Pound" (1 less) and '$' with U+00A3 which is a two-byte sequence (1 more)
    VUNIT_ASSERT_EQUAL_LABELED(stringWithMultibyteCharacters.length(), initialLength, "expected length after replace");
    this->logStatus(stringWithMultibyteCharacters);

    std::wstring ws1 = utf8Test.toUTF16();
    VString roundTrip(ws1);
    std::wstring ws2 = roundTrip.toUTF16();

    VUNIT_ASSERT_EQUAL_LABELED(utf8Test, roundTrip, "VString -> wstring -> VString round trip");
    VUNIT_ASSERT_TRUE_LABELED(ws1 == ws2, "wstring -> VString -> wstring round trip");
    
    // Test case taken from wstring_convert sample at http://cppreference.com/w/cpp/locale/wstring_convert/from_bytes
    VString localeExample("z");                 // latin small letter z
    localeExample += VCodePoint("U+00DF");      // latin small letter sharp s
    localeExample += VCodePoint("U+6C34");      // han character 'water, liquid, lotion, juice'
    localeExample += VCodePoint("U+0001D10B");  // musical symbol segno
    
    // should be same as "\x7A\xC3\x9F\xE6\xB0\xB4\xF0\x9D\x84\x8B"
    VUNIT_ASSERT_EQUAL_LABELED((int) localeExample.length(), 10, "localeExample.length()");
    VUNIT_ASSERT_EQUAL_LABELED((VChar) localeExample[0], VChar(0x7A), "localeExample[0]");
    VUNIT_ASSERT_EQUAL_LABELED((VChar) localeExample[1], VChar(0xC3), "localeExample[1]");
    VUNIT_ASSERT_EQUAL_LABELED((VChar) localeExample[2], VChar(0x9F), "localeExample[2]");
    VUNIT_ASSERT_EQUAL_LABELED((VChar) localeExample[3], VChar(0xE6), "localeExample[3]");
    VUNIT_ASSERT_EQUAL_LABELED((VChar) localeExample[4], VChar(0xB0), "localeExample[4]");
    VUNIT_ASSERT_EQUAL_LABELED((VChar) localeExample[5], VChar(0xB4), "localeExample[5]");
    VUNIT_ASSERT_EQUAL_LABELED((VChar) localeExample[6], VChar(0xF0), "localeExample[6]");
    VUNIT_ASSERT_EQUAL_LABELED((VChar) localeExample[7], VChar(0x9D), "localeExample[7]");
    VUNIT_ASSERT_EQUAL_LABELED((VChar) localeExample[8], VChar(0x84), "localeExample[8]");
    VUNIT_ASSERT_EQUAL_LABELED((VChar) localeExample[9], VChar(0x8B), "localeExample[9]");
    
    std::wstring wideExample = localeExample.toUTF16();
    VUNIT_ASSERT_EQUAL_LABELED((int) wideExample.length(), 5, "wideExample.length()");
    VUNIT_ASSERT_EQUAL_LABELED((int) wideExample[0], 0x7A, "wideExample[0]");
    VUNIT_ASSERT_EQUAL_LABELED((int) wideExample[1], 0xDF, "wideExample[1]");
    VUNIT_ASSERT_EQUAL_LABELED((int) wideExample[2], 0x6C34, "wideExample[2]");
    VUNIT_ASSERT_EQUAL_LABELED((int) wideExample[3], 0xD834, "wideExample[3]");
    VUNIT_ASSERT_EQUAL_LABELED((int) wideExample[4], 0xDD0B, "wideExample[4]");
    
    VUNIT_ASSERT_EQUAL_LABELED(localeExample.length(), 10, "localeExample.length()");
    VUNIT_ASSERT_EQUAL_LABELED(localeExample.getNumCodePoints(), 4, "localeExample.getNumCodePoints()");
    VUNIT_ASSERT_EQUAL_LABELED((*(localeExample.begin() + 0)).intValue(), 0x7A, "localeExample[0]");
    VUNIT_ASSERT_EQUAL_LABELED((*(localeExample.begin() + 1)).intValue(), 0xDF, "localeExample[1]");
    VUNIT_ASSERT_EQUAL_LABELED((*(localeExample.begin() + 2)).intValue(), 0x6C34, "localeExample[2]");
    VUNIT_ASSERT_EQUAL_LABELED((*(localeExample.begin() + 3)).intValue(), 0x0001D10B, "localeExample[3]");
}

