/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vcolor_h
#define vcolor_h

/** @file */

#include "vtypes.h"

#include "vstring.h"
#include "vbinaryiostream.h"

#ifdef VAULT_QT_SUPPORT
#include <QColor>
#endif

class VSettingsNode;

/**
VColor defines a color value in terms of RGB and Alpha. The value of each component is constrained
to values 0..255, but for ease of use the APIs all use plain old "int" types.
*/
class VColor {
    public:

        // For clarity and speed, we provide constant accessors for the standard CSS colors.
        // These are the fastest way to use this set of colors, because it can avoid unnecessary
        // construction or copying in some use cases.
        static const VColor& AQUA();
        static const VColor& BLACK();
        static const VColor& BLUE();
        static const VColor& FUCHSIA();
        static const VColor& GREEN();
        static const VColor& GRAY();
        static const VColor& LIME();
        static const VColor& MAROON();
        static const VColor& NAVY();
        static const VColor& OLIVE();
        static const VColor& ORANGE();
        static const VColor& PURPLE();
        static const VColor& RED();
        static const VColor& SILVER();
        static const VColor& TEAL();
        static const VColor& WHITE();
        static const VColor& YELLOW();

        /**
        Default constructor yields black with full opacity.
        */
        VColor() : mRed(0), mGreen(0), mBlue(0), mAlpha(255) {}
        /**
        Constructor for specifying each r/g/b component with optional alpha.
        @param  r   the red component (will be constrained to 0..255)
        @param  g   the green component (will be constrained to 0..255)
        @param  b   the blue component (will be constrained to 0..255)
        @param  alpha   the alpha channel level (0 means fully transparent, 255 means fully opaque) (will be constrained to 0..255)
        */
        VColor(int r, int g, int b, int alpha = 255) : mRed(VColor::_constrain(r)), mGreen(VColor::_constrain(g)), mBlue(VColor::_constrain(b)), mAlpha(VColor::_constrain(alpha)) {}
        /**
        Constructor for specifying using a CSS-style color string. There are
        four formats for the string:
            Hexadecimal format:
                #rrggbb -- each hex pair indicates the value for that r/g/b component
            Abbreviated hexadecimal format:
                #rgb -- this is nothing more than a shortcut for #rrggbb
            RGB integer format:
                rgb(x, y, z) -- x, y, and z are integer strings for the r, g, and b components
                The values will be constrained to 0..255.
                Whitespace inside the parentheses is OK.
            HTML+CSS color names, for example:
                maroon
                There are 17 color names defined: The 16 HTML values plus the 1 additional CSS
                value (orange). Use here is case insensitive.
                @see <http://en.wikipedia.org/wiki/Web_colors>
                @see <http://en.wikipedia.org/wiki/HTML_color_names>
        Not supported: rgb level percentages (e.g., "rgb(10%, 20%, 30%)")
        Not supported: specifying alpha level
        @param cssColor the string indicating the color value in one of the formats noted above;
                        leading and trailing whitespace is OK
        */
        explicit VColor(const VString& cssColor) : mRed(0), mGreen(0), mBlue(0), mAlpha(255) { this->setCSSColor(cssColor); }
        //explicit VColor(const char* cssColor) : mRed(0), mGreen(0), mBlue(0), mAlpha(255) { this->setCSSColor(cssColor); }
        /**
        Constructor for reading the rgba value from a binary stream.
        */
        VColor(VBinaryIOStream& stream) : mRed(0), mGreen(0), mBlue(0), mAlpha(255) { this->readFromStream(stream); }
        ~VColor() {}

#ifdef VAULT_QT_SUPPORT
        explicit VColor(const QColor& c) : mRed(VColor::_constrain(c.red())), mGreen(VColor::_constrain(c.green())), mBlue(VColor::_constrain(c.blue())), mAlpha(VColor::_constrain(c.alpha())) {}
#endif

        /**
        Reads the color value from a binary stream. The stream data consists of 4 bytes: r, g, b, and a values.
        @param stream the stream to read from
        */
        void readFromStream(VBinaryIOStream& stream) { this->_setStreamValue(stream.readU32()); }
        /**
        Writes the color value to a binary stream. The stream data consists of 4 bytes: r, g, b, and a values.
        */
        void writeToStream(VBinaryIOStream& stream) const { stream.writeU32(this->_getStreamValue()); }

        int getRed() const { return static_cast<int>(mRed); }
        int getGreen() const { return static_cast<int>(mGreen); }
        int getBlue() const { return static_cast<int>(mBlue); }
        int getAlpha() const { return static_cast<int>(mAlpha); }
        VDouble getLightness() const; ///< @return the L in HSL, as a value in the range 0.0 to 1.0.
        VString getCSSColor() const;
        void setRed(int val) { mRed = VColor::_constrain(val); }
        void setGreen(int val) { mGreen = VColor::_constrain(val); }
        void setBlue(int val) { mBlue = VColor::_constrain(val); }
        void setAlpha(int val) { mAlpha = VColor::_constrain(val); }
        void setValues(int r, int g, int b, int alpha = 255) { this->setRed(r); this->setGreen(g); this->setBlue(b); this->setAlpha(alpha); }
        void setCSSColor(const VString& cssColor); ///< See the css color constructor above for details.

        friend inline bool operator==(const VColor& lhs, const VColor& rhs);  // exact equality
        friend inline bool operator!=(const VColor& lhs, const VColor& rhs);  // exact inequality
        // The < operator is needed for to support STL sort(); others provided for completeness.
        // It would be just as valid, perhaps more so, to sort based on HSV values or something like that.
        friend inline bool operator< (const VColor& lhs, const VColor& rhs);
        friend inline bool operator<=(const VColor& lhs, const VColor& rhs);
        friend inline bool operator>=(const VColor& lhs, const VColor& rhs);
        friend inline bool operator> (const VColor& lhs, const VColor& rhs);

#ifdef VAULT_QT_SUPPORT
        void setQColor(const QColor& c) { this->setValues(c.red(), c.blue(), c.green(), c.alpha()); }
        QColor getQColor() const { return QColor(static_cast<int>(mRed), static_cast<int>(mGreen), static_cast<int>(mBlue), static_cast<int>(mAlpha)); }
#endif

    private:

        static Vu8 _constrain(int val) { return static_cast<Vu8>(V_CONSTRAIN_MINMAX(val, 0, 255)); }
        void _setStreamValue(Vu32 value) { mRed = (Vu8)((value & 0xFF000000) >> 24); mGreen = (Vu8)((value & 0x00FF0000) >> 16); mBlue = (Vu8)((value & 0x0000FF00) >> 8); mAlpha = (Vu8)(value & 0x000000FF); }
        Vu32 _getStreamValue() const { return (mRed << 24) | (mGreen << 16) | (mBlue << 8) | mAlpha; }

        Vu8 mRed;
        Vu8 mGreen;
        Vu8 mBlue;
        Vu8 mAlpha;
};

inline bool operator==(const VColor& lhs, const VColor& rhs) { return lhs.mRed == rhs.mRed && lhs.mGreen == rhs.mGreen && lhs.mBlue == rhs.mBlue && lhs.mAlpha == rhs.mAlpha; }
inline bool operator!=(const VColor& lhs, const VColor& rhs) { return !operator==(lhs, rhs); }
inline bool operator< (const VColor& lhs, const VColor& rhs) { return lhs._getStreamValue() < rhs._getStreamValue(); }
inline bool operator<=(const VColor& lhs, const VColor& rhs) { return !operator>(lhs, rhs); }
inline bool operator>=(const VColor& lhs, const VColor& rhs) { return !operator<(lhs, rhs); }
inline bool operator> (const VColor& lhs, const VColor& rhs) { return  operator<(rhs, lhs); }

// VColorPair -----------------------------------------------------------------

/**
VColorPair holds a foreground and background color as a single object for ease
of management and use in the other APIs. For consistency, we always reference
the background as the first element, and the foreground as the second element.
Also for consistency and brevity, we abbreviate using "bg" and "fg" everywhere.
For efficiency, thoughout the color palette classes we provide flexible APIs
that can return a color pair, a particular color, or a fg/bg color as asked for.
*/
class VColorPair {
    public:

        VColorPair();
        VColorPair(const VColor& bg); // fg set to contrast with bg
        VColorPair(const VColor& bg, const VColor& fg);
        ~VColorPair() {} // non-virtual for space efficiency

        const VColor& getBg() const { return mBg; }
        const VColor& getFg() const { return mFg; }
        const VColor& getColor(bool isBg) const { return isBg ? mBg : mFg; }

        // Mainly for debugging purposes, a string describing the color pair.
        VString getCSSColor() const;

        /**
        Returns white if the specified background is "dark", and
        black if the specified background is "light".
        @param  bg  a background color value
        @return white or black, to best contrast with the background
        */
        static VColor generateContrastingForeground(const VColor& bg);
        /**
        This function returns a VColorPair based on the supplied bg/fg colors, which may be
        empty if not applicable. This function also catches malformed color values skips them.
        @param  bgCssColor  background color as a CSS color string (see VColor); if empty, the
                    result is constructed with white bg color
        @param  fgCssColor  foreground color as a CSS color string (see VColor); if empty, the
                    result is constructed with no fg color (a contrasting color will be generated)
        */
        static VColorPair safeConstructColorPair(const VString& bgCssColor, const VString& fgCssColor);

        friend inline bool operator==(const VColorPair& c1, const VColorPair& c2);  // exact equality
        friend inline bool operator!=(const VColorPair& c1, const VColorPair& c2);  // exact inequality

    private:

        VColor mBg;
        VColor mFg;
};

inline bool operator==(const VColorPair& c1, const VColorPair& c2) { return c1.mBg == c2.mBg && c1.mFg == c2.mFg; }
inline bool operator!=(const VColorPair& c1, const VColorPair& c2) { return c1.mBg != c2.mBg || c1.mFg != c2.mFg; }

// VColorMapper ---------------------------------------------------------------

/**
VColorMapper is an abstract base class that defines the API for looking up the colors
assigned to a particular data value (string, integer, Vs64, or double). Concrete
subclasses are defined that cover the particular ways that colors can be mapped
for the different types, including mapped values and ranges. If a given instance
of color map does not support the specified value type directly, it will convert
it (for example, if the map defines colors for integers, but you ask for the color
for a string, it will try to convert the string to an integer and look that up).
When a color is not found for a data value, the default color is returned.
If you need to map colors for other data types, convert them to these types. For
example, use strings or ints for enums and booleans, and use Vs64 for durations
or instants. (The XML syntax allows you to use duration and instant strings, and
it will convert them into Vs64 values in a mapper, so your runtime code can
lookup up a color for the Vs64 value returned by VDuration::getDurationMilliseconds()
or VInstant::getValue().)

Note: I am using the word "mapper" here primarily to avoid collision/confusion
with the STL "map" type. It looks weird to have a map of maps. This way, a color
palette has a map that finds "mappers" from their names.
*/
class VColorMapper {
    public:

        VColorMapper() : mDefaultColors() {}
        virtual ~VColorMapper() {}

        virtual void readColors(const VSettingsNode& mapperNode, VStringVector* errorList);

        virtual VColorPair getColors(const VString& stringValue) const = 0;
        virtual VColorPair getColors(int intValue) const = 0;
        virtual VColorPair getColors(Vs64 int64Value) const = 0;
        virtual VColorPair getColors(VDouble doubleValue) const = 0;

        void setDefaultColors(const VColorPair& defaultColors) { mDefaultColors = defaultColors; }

    protected:

        virtual void _readColorElement(const VSettingsNode& colorNode) = 0; // *Should* throw VException if element is invalid.
        VColorPair _readColorPair(const VSettingsNode& colorNode);

        VColorPair mDefaultColors;
};

// VColorPalette --------------------------------------------------------------

class VSettingsNode;
typedef std::map<VString, VColorMapper*> VColorPaletteMap;

/**
VColorPalette is a collection of color mappers keyed by name. Often you will just
need a global color palette, but you can instantiate several if needed. In general, all
of the methods that build the color palette take an optional VStringVector for error
messages; if not null, any code that encounters an error on a single color or mapper
(that would not prevent the rest of the palette from being built) should append to
the error strings. This allows the caller to decide what to do with those errors, yet
proceed with using the valid part of the palette.
*/
class VColorPalette {
    public:

        VColorPalette();
        VColorPalette(const VSettingsNode& paletteNode, VStringVector* errorList);
        virtual ~VColorPalette();

        const VString& getName() const { return mName; }
        void setName(const VString& name) { mName = name; }

        void adoptColorMapper(const VString& mapperName, VColorMapper* mapper);
        const VColorMapper* findMapper(const VString& mapperName) const;

        VColorPair getColors(const VString& mapperName, const VString& stringValue) const;
        VColorPair getColors(const VString& mapperName, int intValue) const;
        VColorPair getColors(const VString& mapperName, Vs64 int64Value) const;
        VColorPair getColors(const VString& mapperName, VDouble doubleValue) const;

    private:

        void _addMapper(const VSettingsNode& mapperNode, VStringVector* errorList);
        VColorMapper* _readNewMapper(const VString& mapperType, const VSettingsNode& mapperNode, bool usesPrefixMode, VStringVector* errorList);
        void _addMapperNameAliases(VColorMapper* mapper, const VSettingsNode& mapperNode, VStringVector* errorList);

        VString          mName;
        VColorPaletteMap mColorMappers;
        VStringVector    mAliases; // Tracks aliases in use in map so destructor can *safely* clean up.
};

// VStringColorMapper ---------------------------------------------------------

typedef std::map<VString, VColorPair> VStringColorMap;

/**
VStringColorMapper maps string values to colors.
*/
class VStringColorMapper : public VColorMapper {
    public:

        VStringColorMapper();
        virtual ~VStringColorMapper();

        virtual void readColors(const VSettingsNode& mapperNode, VStringVector* errorList);

        virtual VColorPair getColors(const VString& stringValue) const;
        virtual VColorPair getColors(int intValue) const;
        virtual VColorPair getColors(Vs64 int64Value) const;
        virtual VColorPair getColors(VDouble doubleValue) const;

        void addColors(const VString& stringValue, const VColorPair& colors);

    protected:

        virtual void _readColorElement(const VSettingsNode& colorNode);

    private:

        VStringColorMap mColorMap;
        bool            mCaseSensitive;
};

// VIntegerColorMapper --------------------------------------------------------

typedef std::map<Vs64, VColorPair> VIntegerColorMap;

/**
VIntegerColorMapper maps integer values to colors.

Note that we use Vs64 so that this class can handle any size integer.
We don't need separate mapper types for (32-bit) int and Vs64.
*/
class VIntegerColorMapper : public VColorMapper {
    public:

        VIntegerColorMapper();
        virtual ~VIntegerColorMapper();

        virtual VColorPair getColors(const VString& stringValue) const;
        virtual VColorPair getColors(int intValue) const;
        virtual VColorPair getColors(Vs64 int64Value) const;
        virtual VColorPair getColors(VDouble doubleValue) const;

        void addColors(Vs64 intValue, const VColorPair& colors);

    protected:

        virtual void _readColorElement(const VSettingsNode& colorNode);

    private:

        VIntegerColorMap    mColorMap;
};

// VDoubleColorMapper ---------------------------------------------------------

/**
VDoubleColorMapper maps double values to colors.

Note that because of the usual floating-point discrepancies, we don't attempt to
provide a mapping of arbitrary precision values here. Instead, we convert each
key value to a string with 6 digits after the decimal point. This way we can get
perfect matching in the map of values to that level of precision, regardless of
the math operations that generate them. It does, however come at the cost of
string conversion. If you want more precision then you probably want to use the
VDoubleRangeColorMapper and treat the values as boundaries rather than keys.
*/
class VDoubleColorMapper : public VColorMapper {
    public:

        VDoubleColorMapper();
        virtual ~VDoubleColorMapper();

        virtual VColorPair getColors(const VString& stringValue) const;
        virtual VColorPair getColors(int intValue) const;
        virtual VColorPair getColors(Vs64 int64Value) const;
        virtual VColorPair getColors(VDouble doubleValue) const;

        void addColors(VDouble doubleValue, const VColorPair& rangeColors);

    protected:

        virtual void _readColorElement(const VSettingsNode& colorNode);

    private:

        VStringColorMap mColorMap;
};

// VStringRangeColorMapper --------------------------------------------------------

/**
VStringRangeColorElement stores one range element for a VStringRangeColorMapper.
The range defines the lower bound of the range, and its color pair.
*/
class VStringRangeColorElement {
    public:

        VStringRangeColorElement(const VString& rangeMin, const VColorPair& colors) :
            mRangeMin(rangeMin), mColors(colors) {}
        ~VStringRangeColorElement() {}

        VString mRangeMin;
        VColorPair mColors;

        // Required to allow STL to sort objects of this class:
        friend inline bool operator<(const VString& s1, const VString& s2);
};

inline bool operator<(const VStringRangeColorElement& e1, const VStringRangeColorElement& e2) { return e1.mRangeMin < e2.mRangeMin; }

typedef std::vector<VStringRangeColorElement> VStringRangeVector;

/**
VStringRangeColorMapper maps ranges of string values to colors.

Note that VStringRangeColorMapper should work pretty well for some "wildcard"
string uses, because the lexical string sort works if you define string ranges
with boundary values that are the start of a set of strings. For example:
<pre>
  rangeMin[n] = "Q"
  rangeMin[n+1] = "R"
</pre>
This will match all strings starting with "Q" to range n.
To make this work as most people would expect, we fold the string values to
lower case internally. Otherwise, the normal sort order would put all upper
case letters before all lower case letters, which is not what would be expected.
*/
class VStringRangeColorMapper : public VColorMapper {
    public:

        VStringRangeColorMapper(bool usesPrefixMode);
        virtual ~VStringRangeColorMapper();

        virtual void readColors(const VSettingsNode& mapperNode, VStringVector* errorList);

        virtual VColorPair getColors(const VString& stringValue) const;
        virtual VColorPair getColors(int intValue) const;
        virtual VColorPair getColors(Vs64 int64Value) const;
        virtual VColorPair getColors(VDouble doubleValue) const;

        void addColors(const VString& rangeMin, const VColorPair& rangeColors);

    protected:

        virtual void _readColorElement(const VSettingsNode& colorNode);

    private:

        VColorPair _getColorsWithPrefixModeCheck(const VColorPair& foundColors) const;

        VStringRangeVector mColorRanges;
        bool               mCaseSensitive;
        bool               mUsesPrefixMode;

        friend class VColorUnit;
};

// VIntegerRangeColorMapper --------------------------------------------------------

/**
VIntegerRangeColorElement stores one range element for a VIntegerRangeColorMapper.
The range defines the lower bound of the range, and its color pair.
*/
class VIntegerRangeColorElement {
    public:

        VIntegerRangeColorElement(Vs64 rangeMin, const VColorPair& colors) :
            mRangeMin(rangeMin), mColors(colors) {}
        ~VIntegerRangeColorElement() {}

        Vs64 mRangeMin;
        VColorPair mColors;

        // Required to allow STL to sort objects of this class:
        friend inline bool operator<(const VString& s1, const VString& s2);
};

inline bool operator<(const VIntegerRangeColorElement& e1, const VIntegerRangeColorElement& e2) { return e1.mRangeMin < e2.mRangeMin; }

typedef std::vector<VIntegerRangeColorElement> VIntegerRangeVector;

/**
VIntegerRangeColorMapper maps ranges of integer values to colors.

Note that we use Vs64 so that this class can handle any size integer.
We don't need separate mapper types for (32-bit) int and Vs64.
*/
class VIntegerRangeColorMapper : public VColorMapper {
    public:

        VIntegerRangeColorMapper();
        virtual ~VIntegerRangeColorMapper();

        virtual VColorPair getColors(const VString& stringValue) const;
        virtual VColorPair getColors(int intValue) const;
        virtual VColorPair getColors(Vs64 int64Value) const;
        virtual VColorPair getColors(VDouble doubleValue) const;

        void addColors(Vs64 rangeMin, const VColorPair& rangeColors);

    protected:

        virtual void _readColorElement(const VSettingsNode& colorNode);

    private:

        VIntegerRangeVector mColorRanges;

        friend class VColorUnit;
};

// VDoubleRangeColorMapper --------------------------------------------------------

/**
VDoubleRangeColorElement stores one range element for a VDoubleRangeColorMapper.
The range defines the lower bound of the range, and its color pair.
*/
class VDoubleRangeColorElement {
    public:

        VDoubleRangeColorElement(VDouble rangeMin, const VColorPair& colors) :
            mRangeMin(rangeMin), mColors(colors) {}
        ~VDoubleRangeColorElement() {}

        VDouble mRangeMin;
        VColorPair mColors;

        // Required to allow STL to sort objects of this class:
        friend inline bool operator<(const VString& s1, const VString& s2);
};

inline bool operator<(const VDoubleRangeColorElement& e1, const VDoubleRangeColorElement& e2) { return e1.mRangeMin < e2.mRangeMin; }

typedef std::vector<VDoubleRangeColorElement> VDoubleRangeVector;

/**
VDoubleRangeColorMapper maps ranges of double values to colors.
*/
class VDoubleRangeColorMapper : public VColorMapper {
    public:

        VDoubleRangeColorMapper();
        virtual ~VDoubleRangeColorMapper();

        virtual VColorPair getColors(const VString& stringValue) const;
        virtual VColorPair getColors(int intValue) const;
        virtual VColorPair getColors(Vs64 int64Value) const;
        virtual VColorPair getColors(VDouble doubleValue) const;

        void addColors(VDouble rangeMin, const VColorPair& rangeColors);

    protected:

        virtual void _readColorElement(const VSettingsNode& colorNode);

    private:

        VDoubleRangeVector mColorRanges;

        friend class VColorUnit;
};

#endif /* vcolor_h */
