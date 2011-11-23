/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

#include "vcolorunit.h"

#include "vcolor.h"
#include "vmemorystream.h"
#include "vsettings.h"

VColorUnit::VColorUnit(bool logOnSuccess, bool throwOnError) :
    VUnit("VColorUnit", logOnSuccess, throwOnError) {
}

void VColorUnit::run() {
    this->_testVColor();
    this->_testVStringColorMapper();
    this->_testVIntegerColorMapper();
    this->_testVDoubleColorMapper();
    this->_testVStringRangeColorMapper();
    this->_testVIntegerRangeColorMapper();
    this->_testVDoubleRangeColorMapper();
    this->_testVColorPalette();
}

void VColorUnit::_testVColor() {
    // VColor basic tests.

    VColor c1;
    this->test(c1 == VColor(0, 0, 0, 255), "color default constructor");

    VColor c2(10, 20, 30);
    this->test(c2 == VColor(10, 20, 30, 255), "color rgb constructor");

    VColor c3(40, 50, 60, 70);
    this->test(c3 == VColor(40, 50, 60, 70), "color rgba constructor");

    VColor c4(80, 90, 100, 110);
    this->test(c4.getRed() == 80 &&
               c4.getGreen() == 90 &&
               c4.getBlue() == 100 &&
               c4.getAlpha() == 110, "color rgba constructor values");

    c4.setValues(120, 130, 140);
    this->test(c4.getRed() == 120 &&
               c4.getGreen() == 130 &&
               c4.getBlue() == 140 &&
               c4.getAlpha() == 255, "color rgb setValues");

    c4.setValues(150, 160, 170, 180);
    this->test(c4.getRed() == 150 &&
               c4.getGreen() == 160 &&
               c4.getBlue() == 170 &&
               c4.getAlpha() == 180, "color rgba setValues");

    // Test the constraining the values to the 0..255 range.
    VColor tooLow(-1, -200, -3000, -40000);
    this->test(tooLow == VColor(0, 0, 0, 0), "constrain negative values");

    VColor tooHigh(1000, 22000, 333000, 4444000);
    this->test(tooHigh == VColor(255, 255, 255, 255), "constrain large values");

    VColor css123("#123");
    this->test(css123.getRed() == 0x11 && css123.getGreen() == 0x22 && css123.getBlue() == 0x33, "css 3-digit hex");

    VColor css112233("#112233");
    this->test(css112233.getRed() == 0x11 && css112233.getGreen() == 0x22 && css112233.getBlue() == 0x33, "css 6-digit hex");

    this->test(css123 == css112233, "css hex equality");
    this->test(css123 == VColor(0x11, 0x22, 0x33), "css hex values");

    VColor cssRGB("rgb(10, 20, 30)");
    this->test(cssRGB.getRed() == 10 && cssRGB.getGreen() == 20 && cssRGB.getBlue() == 30, "css rgb");

    this->test(cssRGB == VColor(10, 20, 30), "css rgb values");

    VColor aqua("aqua"); this->test(aqua == VColor(0, 255, 255), "css named color aqua");
    VColor cyan("cyan"); this->test(cyan == VColor(0, 255, 255), "css named color cyan"); // Seen in some docs as alternate name.
    VColor black("black"); this->test(black == VColor(), "css named color black");
    VColor blue("blue"); this->test(blue == VColor(0, 0, 255), "css named color blue");
    VColor fuchsia("fuchsia"); this->test(fuchsia == VColor(255, 0, 255), "css named color fuchsia");
    VColor magenta("magenta"); this->test(magenta == VColor(255, 0, 255), "css named color magenta"); // Seen in some docs as alternate name.
    VColor green("green"); this->test(green == VColor(0, 128, 0), "css named color green");
    VColor gray("gray"); this->test(gray == VColor(128, 128, 128), "css named color gray");
    VColor grey("grey"); this->test(grey == VColor(128, 128, 128), "css named color grey"); // Be nice: Allow UK spelling!
    VColor lime("lime"); this->test(lime == VColor(0, 255, 0), "css named color lime");
    VColor maroon("maroon"); this->test(maroon == VColor(128, 0, 0), "css named color maroon");
    VColor navy("navy"); this->test(navy == VColor(0, 0, 128), "css named color navy");
    VColor olive("olive"); this->test(olive == VColor(128, 128, 0), "css named color olive");
    VColor orange("orange"); this->test(orange == VColor(255, 165, 0), "css named color orange");
    VColor purple("purple"); this->test(purple == VColor(128, 0, 128), "css named color purple");
    VColor red("red"); this->test(red == VColor(255, 0, 0), "css named color red");
    VColor silver("silver"); this->test(silver == VColor(192, 192, 192), "css named color silver");
    VColor teal("teal"); this->test(teal == VColor(0, 128, 128), "css named color teal");
    VColor white("white"); this->test(white == VColor(255, 255, 255), "css named color white");
    VColor yellow("yellow"); this->test(yellow == VColor(255, 255, 0), "css named color yellow");

    // A few basic tests of Lightness calculation.
    VUNIT_ASSERT_EQUAL(0.0, VColor::BLACK().getLightness());
    VUNIT_ASSERT_EQUAL(1.0, VColor::WHITE().getLightness());
    VColor contrastWithBlack = VColorPair::generateContrastingForeground(VColor::BLACK());
    VUNIT_ASSERT_EQUAL(contrastWithBlack, VColor::WHITE());
    VColor contrastWithWhite = VColorPair::generateContrastingForeground(VColor::WHITE());
    VUNIT_ASSERT_EQUAL(contrastWithWhite, VColor::BLACK());

    // If we reject whitespace incorrectly, exceptions will be thrown and the tests will fail.
    VColor fff(255, 255, 255);
    VColor whitespace;
    whitespace.setCSSColor(" #fff");
    this->test(whitespace == fff, "leading hex whitespace");
    whitespace.setCSSColor("#fff ");
    this->test(whitespace == fff, "trailing hex whitespace");
    whitespace.setCSSColor(" #fff ");
    this->test(whitespace == fff, "leading and trailing hex whitespace");
    whitespace.setCSSColor(" rgb(255,255,255)");
    this->test(whitespace == fff, "leading rgb whitespace");
    whitespace.setCSSColor(" rgb(255,255,255) ");
    this->test(whitespace == fff, "trailing rgb whitespace");
    whitespace.setCSSColor(" rgb(255,255,255) ");
    this->test(whitespace == fff, "leading and trailing whitespace");
    whitespace.setCSSColor(" rgb(      255,       255,        255           ) ");
    this->test(whitespace == fff, "leading and trailing and interior whitespace");

    // Negative tests on css input values.

    // RGB out of range should be constrained.
    VColor cssTooHigh("rgb(500, 600, 700)");
    this->test(cssTooHigh == VColor(255, 255, 2550), "css rgb above range constrain");
    VColor cssTooLow("rgb(-5, -600, -70000)");
    this->test(cssTooLow == VColor(0, 0, 0), "css rgb below range constrain");

    // Malformed hex values.
    try {
        VColor bad("#ffz"); // illegal hex character 'z' in otherwise correct short string
        this->test(false, "failed to detect bad hex character"); // <-- should never get here
    } catch (const VRangeException&) {
        this->test(true, "successfully detected bad hex character"); // <-- should get here
    }

    try {
        VColor bad("#"); // wrong number of hex characters (too few)
        this->test(false, "failed to detect lack of hex characters"); // <-- should never get here
    } catch (const VRangeException&) {
        this->test(true, "successfully detected lack of hex characters"); // <-- should get here
    }

    try {
        VColor bad("#01F4E"); // wrong number of hex characters (in between)
        this->test(false, "failed to detect illegal hex length"); // <-- should never get here
    } catch (const VRangeException&) {
        this->test(true, "successfully detected illegal hex length"); // <-- should get here
    }

    try {
        VColor bad("#22AA44C"); // wrong number of hex characters (too many)
        this->test(false, "failed to detect too many hex characters"); // <-- should never get here
    } catch (const VRangeException&) {
        this->test(true, "successfully detected too many hex characters"); // <-- should get here
    }

    try {
        VColor bad("#23F*7B"); // illegal hex character '*' in otherwise correct full string
        this->test(false, "failed to detect bad hex character"); // <-- should never get here
    } catch (const VRangeException&) {
        this->test(true, "successfully detected bad hex character"); // <-- should get here
    }

    try {
        VColor bad("rgb(255, oops, 180)"); // illegal rgb value
        this->test(false, "failed to detect illegal rgb element value"); // <-- should never get here
    } catch (const VRangeException&) {
        this->test(true, "successfully detected illegal rgb element value"); // <-- should get here
    }

    try {
        VColor bad("rgb(255)"); // illegal rgb value
        this->test(false, "failed to detect missing rgb values"); // <-- should never get here
    } catch (const VRangeException&) {
        this->test(true, "successfully detected missing rgb values"); // <-- should get here
    }

    try {
        VColor bad("rgb(255, 180, 128, 100)"); // illegal rgb value
        this->test(false, "failed to detect extraneous rgb value"); // <-- should never get here
    } catch (const VRangeException&) {
        this->test(true, "successfully detected extraneous rgb value"); // <-- should get here
    }

    try {
        VColor bad("rgb(255, , 128)"); // illegal rgb value
        this->test(false, "failed to detect empty rgb value"); // <-- should never get here
    } catch (const VRangeException&) {
        this->test(true, "successfully detected empty rgb value"); // <-- should get here
    }

    try {
        VColor bad("no-such-color-name"); // unknown color name
        this->test(false, "failed to detect unknown color name"); // <-- should never get here
    } catch (const VRangeException&) {
        this->test(true, "successfully detected unknown color name"); // <-- should get here
    }

}

// These are mostly just background colors, with default contrasting foreground.
// Can't be static globals due to lack of static init time behavior.
static const VColorPair BLACK_BG() { return VColorPair(VColor("black")); }
static const VColorPair RED_BG() { return VColorPair(VColor("red")); }
static const VColorPair ORANGE_BG() { return VColorPair(VColor("orange")); }
static const VColorPair YELLOW_BG() { return VColorPair(VColor("yellow")); }
static const VColorPair GREEN_BG() { return VColorPair(VColor("green")); }
static const VColorPair BLUE_BG() { return VColorPair(VColor("blue")); }
static const VColorPair UNNAMED_BG() { return VColorPair(VColor("#123456")); } // test one color pair that is not a named CSS color
static const VColorPair SWEDISH_FLAG() { return VColorPair(VColor("blue"), VColor("yellow")); } // test one color pair that specifies bg and fg
static const VColorPair SILVER_BG() { return VColorPair(VColor("silver")); }
static const VColorPair OLIVE_BG() { return VColorPair(VColor("olive")); }

void VColorUnit::_testVStringColorMapper() {
    VStringColorMapper mapper;

    mapper.setDefaultColors(BLACK_BG());
    mapper.addColors("error", RED_BG());
    mapper.addColors("warning", YELLOW_BG());
    mapper.addColors("ok", GREEN_BG());
    mapper.addColors("cool", BLUE_BG());
    mapper.addColors("citrus", ORANGE_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors("oops"), BLACK_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("cool"), BLUE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("warning"), YELLOW_BG());
}

void VColorUnit::_testVIntegerColorMapper() {
    VIntegerColorMapper mapper;

    mapper.setDefaultColors(BLACK_BG());
    mapper.addColors(1, RED_BG());
    mapper.addColors(2, YELLOW_BG());
    mapper.addColors(3, GREEN_BG());
    mapper.addColors(4, BLUE_BG());
    mapper.addColors(5, ORANGE_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors(2), YELLOW_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(4), BLUE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(5), ORANGE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("5"), ORANGE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(5.0), ORANGE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(9), BLACK_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("bzzzt"), BLACK_BG());
}

void VColorUnit::_testVDoubleColorMapper() {
    VDoubleColorMapper mapper;

    mapper.setDefaultColors(BLACK_BG());
    mapper.addColors(1.1, RED_BG());
    mapper.addColors(2.2, YELLOW_BG());
    mapper.addColors(3.3, GREEN_BG());
    mapper.addColors(4.4, BLUE_BG());
    mapper.addColors(5.0, ORANGE_BG());

    // Since we use 6 decimal digits internally, we can test to verify nearby values don't match.
    VUNIT_ASSERT_EQUAL(mapper.getColors(2.1), BLACK_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(2.2), YELLOW_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(2.3), BLACK_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors(4.3), BLACK_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(4.4), BLUE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(4.5), BLACK_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors(5.0), ORANGE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("5"), ORANGE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(5), ORANGE_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors(9.2), BLACK_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("bzzzt"), BLACK_BG());
}

void VColorUnit::_testVStringRangeColorMapper() {
    VStringRangeColorMapper mapper(false);
    // Insert out of alphabetical order and verify that the vector ends up in order.
    mapper.setDefaultColors(BLACK_BG());
    mapper.addColors("squash", GREEN_BG());
    mapper.addColors("strawberry", RED_BG());
    mapper.addColors("lemon", YELLOW_BG());
    mapper.addColors("ocean", BLUE_BG());
    mapper.addColors("pumpkin", ORANGE_BG());
    mapper.addColors("something", UNNAMED_BG());
    mapper.addColors("sweden", SWEDISH_FLAG());
    // A couple of values for wildcard use case handling.
    VColorPair a(VColor("#aaa"));
    VColorPair b(VColor("#bbb"));
    VColorPair c(VColor("#ccc"));
    mapper.addColors("A", a);
    mapper.addColors("B", b);
    mapper.addColors("C", c);
    // Make sure we understand how upper and lower case strings sort.
    VColorPair mocha(VColor("#5F4525"));
    mapper.addColors("MOCHA", mocha);

    /*
        int index = 0;
        for (VStringRangeVector::const_iterator i = mapper.mColorRanges.begin(); i != mapper.mColorRanges.end(); ++i)
            {
            this->logStatus(VSTRING_FORMAT("range[%d] >= %s %s", index, (*i).mRangeMin.chars(), (*i).mColors.getCSSColor().chars()));
            ++index;
            }
    */

    // Verify proper size and order of values. Note that we are peeking inside, and internally the values are folded to lower case.
    VUNIT_ASSERT_EQUAL((int) mapper.mColorRanges.size(), 11);
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[0].mRangeMin, "a");
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[1].mRangeMin, "b");
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[2].mRangeMin, "c");
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[3].mRangeMin, "lemon");
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[4].mRangeMin, "mocha");
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[5].mRangeMin, "ocean");
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[6].mRangeMin, "pumpkin");
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[7].mRangeMin, "something");
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[8].mRangeMin, "squash");
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[9].mRangeMin, "strawberry");
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[10].mRangeMin, "sweden");

    VUNIT_ASSERT_EQUAL(mapper.getColors("!"), BLACK_BG()); // punctuation sorts before letters

    VUNIT_ASSERT_EQUAL(mapper.getColors("a"), a);
    VUNIT_ASSERT_EQUAL(mapper.getColors("ant"), a);
    VUNIT_ASSERT_EQUAL(mapper.getColors("AARDVARK"), a);

    VUNIT_ASSERT_EQUAL(mapper.getColors("b"), b);
    VUNIT_ASSERT_EQUAL(mapper.getColors("bozzio"), b);
    VUNIT_ASSERT_EQUAL(mapper.getColors("BEATLE"), b);

    VUNIT_ASSERT_EQUAL(mapper.getColors("c"), c);
    VUNIT_ASSERT_EQUAL(mapper.getColors("cat"), c);
    VUNIT_ASSERT_EQUAL(mapper.getColors("COPELAND"), c);
    VUNIT_ASSERT_EQUAL(mapper.getColors("devo"), c);
    VUNIT_ASSERT_EQUAL(mapper.getColors("landshark"), c);
    VUNIT_ASSERT_EQUAL(mapper.getColors("lemo"), c);

    VUNIT_ASSERT_EQUAL(mapper.getColors("lemon"), YELLOW_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("LEMON"), YELLOW_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("lemonade"), YELLOW_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("mikado"), YELLOW_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("mo"), YELLOW_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors("mocha"), mocha);
    VUNIT_ASSERT_EQUAL(mapper.getColors("MOCHA"), mocha);
    VUNIT_ASSERT_EQUAL(mapper.getColors("mulu the rain forest"), mocha);
    VUNIT_ASSERT_EQUAL(mapper.getColors("neo"), mocha);
    VUNIT_ASSERT_EQUAL(mapper.getColors("o"), mocha);

    VUNIT_ASSERT_EQUAL(mapper.getColors("ocean"), BLUE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("OCEANIC 815"), BLUE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("pump"), BLUE_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors("pumpkin"), ORANGE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("quartz"), ORANGE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("somebody"), ORANGE_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors("something"), UNNAMED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("somewhere"), UNNAMED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("square"), UNNAMED_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors("squash"), GREEN_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("squawk"), GREEN_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("straw"), GREEN_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors("strawberry"), RED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("SVEN"), RED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("swede"), RED_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors("sweden"), SWEDISH_FLAG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("sweeten"), SWEDISH_FLAG());
    VUNIT_ASSERT_EQUAL(mapper.getColors("z"), SWEDISH_FLAG());
}

void VColorUnit::_testVIntegerRangeColorMapper() {
    VIntegerRangeColorMapper mapper;
    // Insert out of order and verify that the vector ends up in order.
    mapper.setDefaultColors(BLACK_BG());
    mapper.addColors(100, GREEN_BG());
    mapper.addColors(-200, RED_BG());
    mapper.addColors(0, YELLOW_BG());
    mapper.addColors(200, BLUE_BG());
    mapper.addColors(-100, ORANGE_BG());
    mapper.addColors(300, UNNAMED_BG());
    mapper.addColors(400, SWEDISH_FLAG());

    /*
        int index = 0;
        for (VIntegerRangeVector::const_iterator i = mapper.mColorRanges.begin(); i != mapper.mColorRanges.end(); ++i)
            {
            this->logStatus(VSTRING_FORMAT("range[%d] >= " VSTRING_FORMATTER_S64 " %s", index, (*i).mRangeMin, (*i).mColors.getCSSColor().chars()));
            ++index;
            }
    */

    // Verify proper size and order of values.
    VUNIT_ASSERT_EQUAL((int) mapper.mColorRanges.size(), 7);
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[0].mRangeMin, CONST_S64(-200));
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[1].mRangeMin, CONST_S64(-100));
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[2].mRangeMin, CONST_S64(0));
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[3].mRangeMin, CONST_S64(100));
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[4].mRangeMin, CONST_S64(200));
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[5].mRangeMin, CONST_S64(300));
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[6].mRangeMin, CONST_S64(400));

    // Test -1/+1 around every boundary value, and a sampling of others.
    VUNIT_ASSERT_EQUAL(mapper.getColors(-12345), BLACK_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(-201), BLACK_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors(-200), RED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(-199), RED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(-123), RED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(-101), RED_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors(-100), ORANGE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(-99), ORANGE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(-44), ORANGE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(-1), ORANGE_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors(0), YELLOW_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(1), YELLOW_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(64), YELLOW_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(99), YELLOW_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors(100), GREEN_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(101), GREEN_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(144), GREEN_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(199), GREEN_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors(200), BLUE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(201), BLUE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(222), BLUE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(299), BLUE_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors(300), UNNAMED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(301), UNNAMED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(333), UNNAMED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(399), UNNAMED_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors(400), SWEDISH_FLAG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(401), SWEDISH_FLAG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(444), SWEDISH_FLAG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(499), SWEDISH_FLAG());

    VUNIT_ASSERT_EQUAL(mapper.getColors(500), SWEDISH_FLAG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(501), SWEDISH_FLAG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(98765), SWEDISH_FLAG());
}

void VColorUnit::_testVDoubleRangeColorMapper() {
    VDoubleRangeColorMapper mapper;
    // Insert out of order and verify that the vector ends up in order.
    mapper.setDefaultColors(BLACK_BG());
    mapper.addColors(100.0, GREEN_BG());
    mapper.addColors(-200.0, RED_BG());
    mapper.addColors(0.0, YELLOW_BG());
    mapper.addColors(200.0, BLUE_BG());
    mapper.addColors(-100.0, ORANGE_BG());
    mapper.addColors(300.0, UNNAMED_BG());
    mapper.addColors(400.0, SWEDISH_FLAG());

    /*
        int index = 0;
        for (VDoubleRangeVector::const_iterator i = mapper.mColorRanges.begin(); i != mapper.mColorRanges.end(); ++i)
            {
            this->logStatus(VSTRING_FORMAT("range[%d] >= " VSTRING_FORMATTER_DOUBLE " %s", index, (*i).mRangeMin, (*i).mColors.getCSSColor().chars()));
            ++index;
            }
    */

    // Verify proper size and order of values.
    VUNIT_ASSERT_EQUAL((int) mapper.mColorRanges.size(), 7);
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[0].mRangeMin, -200.0);
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[1].mRangeMin, -100.0);
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[2].mRangeMin, 0.0);
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[3].mRangeMin, 100.0);
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[4].mRangeMin, 200.0);
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[5].mRangeMin, 300.0);
    VUNIT_ASSERT_EQUAL(mapper.mColorRanges[6].mRangeMin, 400.0);

    // Test -0.1/+0.1 around every boundary value, and a sampling of others.
    VUNIT_ASSERT_EQUAL(mapper.getColors(-12345.0), BLACK_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(-200.1), BLACK_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors("-200"), RED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(-200), RED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(-200.0), RED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(-199.9), RED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(-123.0), RED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(-100.1), RED_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors("-100"), ORANGE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(-100), ORANGE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(-100.0), ORANGE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(-99.0), ORANGE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(-44.0), ORANGE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(-0.1), ORANGE_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors("0"), YELLOW_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(0), YELLOW_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(0.0), YELLOW_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(0.1), YELLOW_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(64.0), YELLOW_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(99.9), YELLOW_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors("100"), GREEN_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(100), GREEN_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(100.0), GREEN_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(100.1), GREEN_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(144.0), GREEN_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(199.9), GREEN_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors("200"), BLUE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(200), BLUE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(200.0), BLUE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(200.1), BLUE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(222.0), BLUE_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(299.9), BLUE_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors("300"), UNNAMED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(300), UNNAMED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(300.0), UNNAMED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(300.1), UNNAMED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(333.0), UNNAMED_BG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(399.9), UNNAMED_BG());

    VUNIT_ASSERT_EQUAL(mapper.getColors("400"), SWEDISH_FLAG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(400), SWEDISH_FLAG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(400.0), SWEDISH_FLAG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(400.1), SWEDISH_FLAG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(444.0), SWEDISH_FLAG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(499.9), SWEDISH_FLAG());

    VUNIT_ASSERT_EQUAL(mapper.getColors("500"), SWEDISH_FLAG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(500), SWEDISH_FLAG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(500.0), SWEDISH_FLAG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(500.1), SWEDISH_FLAG());
    VUNIT_ASSERT_EQUAL(mapper.getColors(98765.0), SWEDISH_FLAG());
}

#define PALETTE_TEXT \
"<palette name=\"test\"> \
\
 <color-map name=\"car\" type=\"string-values\" default-bg=\"olive\"> \
  <color value=\"Jaguar\" bg=\"green\" /> \
  <color value=\"Ford\" bg=\"blue\" /> \
  <color value=\"Ferrari\" bg=\"red\" /> \
  <color value=\"Porsche\" bg=\"silver\" /> \
 </color-map> \
\
 <color-map name=\"words\" type=\"string-ranges\"> \
  <alias name=\"dictionary\" /> \
  <alias name=\"thesaurus\" /> \
  <color value=\"0\" bg=\"gray\" /> \
  <color value=\"A\" bg=\"aqua\" /> \
  <color value=\"G\" bg=\"green\" /> \
  <color value=\"N\" bg=\"navy\" /> \
  <color value=\"T\" bg=\"teal\" /> \
 </color-map> \
\
 <color-map name=\"log-level\" type=\"integer-values\" default-bg=\"black\" default-fg=\"red\"> \
  <color value=\"0\" bg=\"white\" /> \
  <color value=\"20\" bg=\"red\" /> \
  <color value=\"40\" bg=\"orange\" /> \
  <color value=\"60\" bg=\"blue\" /> \
  <color value=\"80\" bg=\"green\" /> \
  <color value=\"100\" bg=\"gray\" /> \
 </color-map> \
\
 <color-map name=\"temperature\" type=\"integer-ranges\" default-fg=\"fuchsia\"> \
  <color value=\"-270\" bg=\"white\" /> \
  <color value=\"0\" bg=\"blue\" /> \
  <color value=\"32\" bg=\"green\" /> \
  <color value=\"70\" bg=\"yellow\" /> \
  <color value=\"80\" bg=\"orange\" /> \
  <color value=\"90\" bg=\"red\" /> \
 </color-map> \
\
 <color-map name=\"police-case-insensitive\" type=\"string-values\"> \
  <color value=\"Police\" bg=\"blue\" /> \
 </color-map> \
\
 <color-map name=\"police-case-sensitive\" type=\"string-values\" case-sensitive=\"true\"> \
  <color value=\"Police\" bg=\"blue\" /> \
  <color value=\"POLICE\" bg=\"silver\" /> \
 </color-map> \
\
</palette>"


void VColorUnit::_testVColorPalette() {
    const VColorPair DEFAULT_COLORS; // For readability below: will be returned if no custom default set, when a value is not found

    VMemoryStream buf;
    VTextIOStream io(buf);
    io.writeString(PALETTE_TEXT);
    io.seek0();

    VSettings settings(io);
    const VSettingsNode* paletteNode = settings.findNode("palette");
    VStringVector errorList;
    VColorPalette palette(*paletteNode, &errorList);

    VUNIT_ASSERT_EQUAL(palette.getName(), "test");

    VUNIT_ASSERT_EQUAL(palette.getColors("car", "Ferrari"), RED_BG());
    VUNIT_ASSERT_EQUAL(palette.getColors("car", "Porsche"), SILVER_BG());
    VUNIT_ASSERT_EQUAL(palette.getColors("car", "Yugo"), OLIVE_BG());

    VUNIT_ASSERT_EQUAL(palette.getColors("words", "!punctuation"), DEFAULT_COLORS);
    VUNIT_ASSERT_EQUAL(palette.getColors("words", "123"), VColorPair(VColor::GRAY()));
    VUNIT_ASSERT_EQUAL(palette.getColors("words", "Abalone"), VColorPair(VColor::AQUA()));
    VUNIT_ASSERT_EQUAL(palette.getColors("words", "French Bread"), VColorPair(VColor::AQUA()));
    VUNIT_ASSERT_EQUAL(palette.getColors("words", "Guacamole"), VColorPair(VColor::GREEN()));
    VUNIT_ASSERT_EQUAL(palette.getColors("words", "Mole"), VColorPair(VColor::GREEN()));
    VUNIT_ASSERT_EQUAL(palette.getColors("words", "Nectarine"), VColorPair(VColor::NAVY()));
    VUNIT_ASSERT_EQUAL(palette.getColors("words", "Sushi"), VColorPair(VColor::NAVY()));
    VUNIT_ASSERT_EQUAL(palette.getColors("words", "Tamale"), VColorPair(VColor::TEAL()));

    VUNIT_ASSERT_EQUAL(palette.getColors("log-level", 40), ORANGE_BG());
    VUNIT_ASSERT_EQUAL(palette.getColors("log-level", 60), BLUE_BG());
    VUNIT_ASSERT_EQUAL(palette.getColors("log-level", 999), VColorPair(VColor::BLACK(), VColor::RED()));

    VUNIT_ASSERT_EQUAL(palette.getColors("temperature", 68), GREEN_BG());
    VUNIT_ASSERT_EQUAL(palette.getColors("temperature", 70), YELLOW_BG());
    VUNIT_ASSERT_EQUAL(palette.getColors("temperature", 999), RED_BG());
    VUNIT_ASSERT_EQUAL(palette.getColors("temperature", -999), VColorPair(VColor::WHITE(), VColor::FUCHSIA()));

    VUNIT_ASSERT_EQUAL(palette.getColors("police-case-insensitive", "Police"), BLUE_BG());
    VUNIT_ASSERT_EQUAL(palette.getColors("police-case-insensitive", "POLICE"), BLUE_BG());
    VUNIT_ASSERT_EQUAL(palette.getColors("police-case-insensitive", "police"), BLUE_BG());
    VUNIT_ASSERT_EQUAL(palette.getColors("police-case-insensitive", "Sting"), DEFAULT_COLORS);

    VUNIT_ASSERT_EQUAL(palette.getColors("police-case-sensitive", "Police"), BLUE_BG());
    VUNIT_ASSERT_EQUAL(palette.getColors("police-case-sensitive", "POLICE"), SILVER_BG());
    VUNIT_ASSERT_EQUAL(palette.getColors("police-case-sensitive", "police"), DEFAULT_COLORS);
    VUNIT_ASSERT_EQUAL(palette.getColors("police-case-sensitive", "Sting"), DEFAULT_COLORS);

    const VColorMapper* words = palette.findMapper("words");
    this->test(words != NULL, "find palette: words");
    this->test(words == palette.findMapper("dictionary"), "alias palette: dictionary->words");
    this->test(words == palette.findMapper("thesaurus"), "alias palette: thesaurus->words");
}
