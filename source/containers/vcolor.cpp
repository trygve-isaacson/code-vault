/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

#include "vcolor.h"

#include "vchar.h"
#include "vhex.h"
#include "vsettings.h"

// VColor ------------------------------------------------------------------

// The standard CSS named colors.
const VColor& VColor::AQUA()    { static const VColor kAqua     (  0, 255, 255); return kAqua; }
const VColor& VColor::BLACK()   { static const VColor kBlack    (  0,   0,   0); return kBlack; }
const VColor& VColor::BLUE()    { static const VColor kBlue     (  0,   0, 255); return kBlue; }
const VColor& VColor::FUCHSIA() { static const VColor kFuchsia  (255,   0, 255); return kFuchsia; }
const VColor& VColor::GREEN()   { static const VColor kGreen    (  0, 128,   0); return kGreen; }
const VColor& VColor::GRAY()    { static const VColor kGray     (128, 128, 128); return kGray; }
const VColor& VColor::LIME()    { static const VColor kLime     (  0, 255,   0); return kLime; }
const VColor& VColor::MAROON()  { static const VColor kMaroon   (128,   0,   0); return kMaroon; }
const VColor& VColor::NAVY()    { static const VColor kNavy     (  0,   0, 128); return kNavy; }
const VColor& VColor::OLIVE()   { static const VColor kOlive    (128, 128,   0); return kOlive; }
const VColor& VColor::ORANGE()  { static const VColor kOrange   (255, 165,   0); return kOrange; }
const VColor& VColor::PURPLE()  { static const VColor kPurple   (128,   0, 128); return kPurple; }
const VColor& VColor::RED()     { static const VColor kRed      (255,   0,   0); return kRed; }
const VColor& VColor::SILVER()  { static const VColor kSilver   (192, 192, 192); return kSilver; }
const VColor& VColor::TEAL()    { static const VColor kTeal     (  0, 128, 128); return kTeal; }
const VColor& VColor::WHITE()   { static const VColor kWhite    (255, 255, 255); return kWhite; }
const VColor& VColor::YELLOW()  { static const VColor kYellow   (255, 255,   0); return kYellow; }

VString VColor::getCSSColor() const
    {
    if (*this == BLACK())
        return "black";
    if (*this == WHITE())
        return "white";

    if (*this == AQUA())
        return "aqua";
    if (*this == BLUE())
        return "blue";
    if (*this == FUCHSIA())
        return "fuchsia";
    if (*this == GREEN())
        return "green";
    if (*this == GRAY())
        return "gray";
    if (*this == LIME())
        return "lime";
    if (*this == MAROON())
        return "maroon";
    if (*this == NAVY())
        return "navy";
    if (*this == OLIVE())
        return "olive";
    if (*this == ORANGE())
        return "orange";
    if (*this == PURPLE())
        return "purple";
    if (*this == RED())
        return "red";
    if (*this == SILVER())
        return "silver";
    if (*this == TEAL())
        return "teal";
    if (*this == YELLOW())
        return "yellow";

    return VSTRING_FORMAT("#%02x%02x%02x", mRed, mGreen, mBlue);
    }

VDouble VColor::getLightness() const
    {
    // Calculate the Lightness value (the L in HSL; doesn't require calculating H or S).
    Vu8 maxColor = V_MAX(mRed, V_MAX(mGreen, mBlue));
    Vu8 minColor = V_MIN(mRed, V_MIN(mGreen, mBlue));
    VDouble lightness = (((VDouble) minColor) + ((VDouble) maxColor)) / 510.0; // e.g., avg of minColor and maxColor, where 0 is 0.0 and 255 is 1.0, thus divide by (2*255)
    return lightness;
    }

void VColor::setCSSColor(const VString& cssColor)
    {
    VString colorText(cssColor);
    colorText.trim(); // allow for leading/trailing whitespace in input string

    bool valid = false;
    if (colorText.startsWith('#'))
        {
        // Allowed formats:
        // #xyz is short for #xxyyzz
        // #xxyyzz is the hexadecimal r-g-b byte values
        if (colorText.length() == 4)
            {
            valid = colorText.at(1).isHexadecimal() &&
                    colorText.at(2).isHexadecimal() &&
                    colorText.at(3).isHexadecimal();
            
            if (valid)
                {
                mRed = VHex::hexCharsToByte(colorText.at(1), colorText.at(1));
                mGreen = VHex::hexCharsToByte(colorText.at(2), colorText.at(2));
                mBlue = VHex::hexCharsToByte(colorText.at(3), colorText.at(3));
                mAlpha = 255;
                }
            }
        else if (colorText.length() == 7)
            {
            valid = colorText.at(1).isHexadecimal() &&
                    colorText.at(2).isHexadecimal() &&
                    colorText.at(3).isHexadecimal() &&
                    colorText.at(4).isHexadecimal() &&
                    colorText.at(5).isHexadecimal() &&
                    colorText.at(6).isHexadecimal();
            
            if (valid)
                {
                mRed = VHex::hexCharsToByte(colorText.at(1), colorText.at(2));
                mGreen = VHex::hexCharsToByte(colorText.at(3), colorText.at(4));
                mBlue = VHex::hexCharsToByte(colorText.at(5), colorText.at(6));
                mAlpha = 255;
                }
            }
        }
    else if (colorText.startsWith("rgb(") && colorText.endsWith(')'))
        {
        // Allowed formats:
        // rgb(x,y,z) -- whitespace inside () is OK; x y and z are the r, g, b integer values
        colorText.substringInPlace(4, colorText.length() - 1);
        colorText.trim();
        int redStart = 0;
        int redEnd = colorText.indexOf(',');
        int greenStart = redEnd + 1;
        int greenEnd = colorText.indexOf(',', greenStart);
        int blueStart = greenEnd + 1;
        int blueEnd = colorText.indexOf(',', blueStart);
        
        if ((redStart < redEnd) && (redEnd < greenStart) && (greenStart < greenEnd) && (greenEnd < blueStart) && (blueEnd == -1))
            {
            VString redValue; colorText.getSubstring(redValue, redStart, redEnd); redValue.trim();
            VString greenValue; colorText.getSubstring(greenValue, greenStart, greenEnd); greenValue.trim();
            VString blueValue; colorText.getSubstring(blueValue, blueStart, blueEnd); blueValue.trim();
            
            if (redValue.isNotEmpty() && greenValue.isNotEmpty() && blueValue.isNotEmpty()) // parseInt() treats empty as meaning "0"
                {
                try
                    {
                    int r = redValue.parseInt();
                    int g = greenValue.parseInt();
                    int b = blueValue.parseInt();
                    this->setValues(r, g, b);
                    valid = true;
                    }
                catch (VRangeException& /*ex*/)
                    {
                    // Let our validity check below throw with a more informative message than "integer value out of range".
                    }
                }
            }
        }
    else if (colorText.equalsIgnoreCase("black"))
        {
        *this = BLACK();
        valid = true;
        }
    else if (colorText.equalsIgnoreCase("white"))
        {
        *this = WHITE();
        valid = true;
        }
    else if (colorText.equalsIgnoreCase("aqua") || colorText.equalsIgnoreCase("cyan"))
        {
        *this = AQUA();
        valid = true;
        }
    else if (colorText.equalsIgnoreCase("blue"))
        {
        *this = BLUE();
        valid = true;
        }
    else if (colorText.equalsIgnoreCase("fuchsia") || colorText.equalsIgnoreCase("magenta"))
        {
        *this = FUCHSIA();
        valid = true;
        }
    else if (colorText.equalsIgnoreCase("green"))
        {
        *this = GREEN();
        valid = true;
        }
    else if (colorText.equalsIgnoreCase("gray") || colorText.equalsIgnoreCase("grey"))
        {
        *this = GRAY();
        valid = true;
        }
    else if (colorText.equalsIgnoreCase("lime"))
        {
        *this = LIME();
        valid = true;
        }
    else if (colorText.equalsIgnoreCase("maroon"))
        {
        *this = MAROON();
        valid = true;
        }
    else if (colorText.equalsIgnoreCase("navy"))
        {
        *this = NAVY();
        valid = true;
        }
    else if (colorText.equalsIgnoreCase("olive"))
        {
        *this = OLIVE();
        valid = true;
        }
    else if (colorText.equalsIgnoreCase("orange"))
        {
        *this = ORANGE();
        valid = true;
        }
    else if (colorText.equalsIgnoreCase("purple"))
        {
        *this = PURPLE();
        valid = true;
        }
    else if (colorText.equalsIgnoreCase("red"))
        {
        *this = RED();
        valid = true;
        }
    else if (colorText.equalsIgnoreCase("silver"))
        {
        *this = SILVER();
        valid = true;
        }
    else if (colorText.equalsIgnoreCase("teal"))
        {
        *this = TEAL();
        valid = true;
        }
    else if (colorText.equalsIgnoreCase("yellow"))
        {
        *this = YELLOW();
        valid = true;
        }
    
    if (!valid)
        throw VRangeException(VSTRING_FORMAT("VColor::setCSSColor '%s' is invalid.", cssColor.chars()));
    }

// VColorPair -----------------------------------------------------------------

VColorPair::VColorPair() :
mBg(VColor::WHITE()),
mFg(VColor::BLACK())
    {
    }

VColorPair::VColorPair(const VColor& bg) :
mBg(bg),
mFg(VColorPair::generateContrastingForeground(bg))
    {
    }

VColorPair::VColorPair(const VColor& bg, const VColor& fg) :
mBg(bg),
mFg(fg)
    {
    }

VString VColorPair::getCSSColor() const
    {
    return VSTRING_FORMAT("%s-on-%s", mFg.getCSSColor().chars(), mBg.getCSSColor().chars());
    }

// static
VColor VColorPair::generateContrastingForeground(const VColor& bg)
    {
    return (bg.getLightness() >= 0.5) ? VColor::BLACK() : VColor::WHITE();
    }

// static
VColorPair VColorPair::safeConstructColorPair(const VString& bgCssColor, const VString& fgCssColor)
    {
    VColor bg = VColor::WHITE();
    VColor fg;
    bool hasFg = false;
    
    if (bgCssColor.isNotEmpty())
        {
        try
            {
            bg.setCSSColor(bgCssColor);
            }
        catch (...) {throw;}
        }
    
    if (fgCssColor.isNotEmpty())
        {
        try
            {
            fg.setCSSColor(fgCssColor);
            hasFg = true;
            }
        catch (...) {throw;}
        }

    if (hasFg)
        return VColorPair(bg, fg);

    return VColorPair(bg);
    }

// VColorPalette --------------------------------------------------------------

VColorPalette::VColorPalette() :
mName(),
mColorMappers(),
mAliases()
    {
    }

VColorPalette::VColorPalette(const VSettingsNode& paletteNode, VStringVector* errorList) :
mName(),
mColorMappers(),
mAliases()
    {
    mName = paletteNode.getString("name", mName);
    int numMappers = paletteNode.countNamedChildren("color-map");

    if ((numMappers == 0) && (errorList != NULL))
        errorList->push_back(VSTRING_FORMAT("Color palette '%s' has no color maps.", mName.chars()));

    for (int i = 0; i < numMappers; ++i)
        {
        const VSettingsNode* mapperNode = paletteNode.getNamedChild("color-map", i);
        this->_addMapper(*mapperNode, errorList);
        }
    }

VColorPalette::~VColorPalette()
    {
    // Remove all alias keys from mColorMappers, so that mapDeleteAllValues doesn't double-delete aliased objects.
    for (VStringVector::const_iterator i = mAliases.begin(); i != mAliases.end(); ++i)
        mColorMappers[*i] = NULL;

    vault::mapDeleteAllValues(mColorMappers);
    }

void VColorPalette::adoptColorMapper(const VString& mapperName, VColorMapper* mapper)
    {
    VColorMapper* existingMapper = mColorMappers[mapperName];
    delete existingMapper;
    mColorMappers[mapperName] = mapper;
    }

const VColorMapper* VColorPalette::findMapper(const VString& mapperName) const
    {
    VColorPaletteMap::const_iterator position = mColorMappers.find(mapperName);
    if (position == mColorMappers.end())
        return NULL;
    
    VColorMapper* result = (*position).second;
    return result;
    }

VColorPair VColorPalette::getColors(const VString& mapperName, const VString& stringValue) const
    {
    const VColorMapper* cm = this->findMapper(mapperName);
    if (cm == NULL)
        return VColorPair();

    return cm->getColors(stringValue);
    }

VColorPair VColorPalette::getColors(const VString& mapperName, int intValue) const
    {
    const VColorMapper* cm = this->findMapper(mapperName);
    if (cm == NULL)
        return VColorPair();

    return cm->getColors(intValue);
    }

VColorPair VColorPalette::getColors(const VString& mapperName, Vs64 int64Value) const
    {
    const VColorMapper* cm = this->findMapper(mapperName);
    if (cm == NULL)
        return VColorPair();

    return cm->getColors(int64Value);
    }

VColorPair VColorPalette::getColors(const VString& mapperName, VDouble doubleValue) const
    {
    const VColorMapper* cm = this->findMapper(mapperName);
    if (cm == NULL)
        return VColorPair();

    return cm->getColors(doubleValue);
    }

void VColorPalette::_addMapper(const VSettingsNode& mapperNode, VStringVector* errorList)
    {
    try
        {
        VString mapperName = mapperNode.getString("name");
        VString mapperType = mapperNode.getString("type", "string-values");
        bool usesPrefixMode = mapperNode.getBoolean("prefix-mode", false);
        VColorMapper* mapper = this->_readNewMapper(mapperType, mapperNode, usesPrefixMode, errorList);
        
        if (mapper != NULL)
            {
            // For now, we assume the palette is initialized and never subsequently modified.
            // So we don't look for an existing mapper to delete/replace.
            mColorMappers[mapperName] = mapper;
            
            this->_addMapperNameAliases(mapper, mapperNode, errorList);
            }
        }
    catch (const std::exception& ex)
        {
        if (errorList != NULL)
            errorList->push_back(VSTRING_FORMAT("At %s/%s: %s", mapperNode.getPath().chars(), mapperNode.getString("name","").chars(), ex.what()));
        }
    }

VColorMapper* VColorPalette::_readNewMapper(const VString& mapperType, const VSettingsNode& mapperNode, bool usesPrefixMode, VStringVector* errorList)
    {
    VColorMapper* mapper = NULL;
    if (mapperType == "string-values")
        {
        if (usesPrefixMode)
            mapper = new VStringRangeColorMapper(usesPrefixMode);
        else
            mapper = new VStringColorMapper();
        }
    else if (mapperType == "integer-values")
        mapper = new VIntegerColorMapper();
    else if (mapperType == "real-values")
        mapper = new VDoubleColorMapper();
    else if (mapperType == "string-ranges")
        mapper = new VStringRangeColorMapper(usesPrefixMode);
    else if (mapperType == "integer-ranges")
        mapper = new VIntegerRangeColorMapper();
    else if (mapperType == "real-ranges")
        mapper = new VDoubleRangeColorMapper();
    else if (errorList != NULL)
        errorList->push_back(VSTRING_FORMAT("At %s/%s: Invalid color-map type '%s'.", mapperNode.getPath().chars(), mapperNode.getString("name","").chars(), mapperType.chars()));

    if (mapper != NULL)
        mapper->readColors(mapperNode, errorList);

    return mapper;
    }

void VColorPalette::_addMapperNameAliases(VColorMapper* mapper, const VSettingsNode& mapperNode, VStringVector* errorList)
    {
    bool mapperUsesPrefixMode = mapperNode.getBoolean("prefix-mode", false);
    int numAliases = mapperNode.countNamedChildren("alias");
    for (int i = 0; i < numAliases; ++i)
        {
        const VSettingsNode* aliasNode = mapperNode.getNamedChild("alias", i);
        VString alias = aliasNode->getString("name", VString::EMPTY());
        if (alias.isNotEmpty())
            {
            // If the alias uses a different prefix-mode flag than the original mapper, need a separate instance,
            // since behavior is totally different. In that case it's not really an "alias" we need to track, it
            // is really another independent mapper.
            bool aliasUsesPrefixMode = aliasNode->getBoolean("prefix-mode", false);
            if (aliasUsesPrefixMode == mapperUsesPrefixMode)
                mAliases.push_back(alias);
            else
                mapper = this->_readNewMapper(mapperNode.getString("type", "string-values"), mapperNode, aliasUsesPrefixMode, errorList);
            
            mColorMappers[alias] = mapper;
            }
        }
    }

// VColorMapper ---------------------------------------------------------------

void VColorMapper::readColors(const VSettingsNode& mapperNode, VStringVector* errorList)
    {
    VString defaultBg = mapperNode.getString("default-bg", VString::EMPTY());
    VString defaultFg = mapperNode.getString("default-fg", VString::EMPTY());
    if (defaultBg.isNotEmpty() || defaultFg.isNotEmpty())
        this->setDefaultColors(VColorPair::safeConstructColorPair(defaultBg, defaultFg));

    int numColors = mapperNode.countNamedChildren("color");

    if ((numColors == 0) && (errorList != NULL))
        errorList->push_back(VSTRING_FORMAT("At %s/%s: No colors defined in color-map.", mapperNode.getPath().chars(), mapperNode.getString("name","").chars()));

    for (int i = 0; i < numColors; ++i)
        {
        const VSettingsNode* colorNode = mapperNode.getNamedChild("color", i);
        
        try
            {
            this->_readColorElement(*colorNode);
            }
        catch (const std::exception& ex)
            {
            if (errorList != NULL)
                errorList->push_back(VSTRING_FORMAT("At %s/%s: Error reading color value [%d]: %s", mapperNode.getPath().chars(), mapperNode.getString("name","").chars(), i, ex.what()));
            }
        }
    }

VColorPair VColorMapper::_readColorPair(const VSettingsNode& colorNode)
    {
    VString bg = colorNode.getString("bg", VString::EMPTY());
    VString fg = colorNode.getString("fg", VString::EMPTY());
    return VColorPair::safeConstructColorPair(bg, fg);
    }

// VStringColorMapper ---------------------------------------------------------

VStringColorMapper::VStringColorMapper() :
VColorMapper(),
mColorMap(),
mCaseSensitive(false)
    {
    }

VStringColorMapper::~VStringColorMapper()
    {
    }

void VStringColorMapper::readColors(const VSettingsNode& mapperNode, VStringVector* errorList)
    {
    mCaseSensitive = mapperNode.getBoolean("case-sensitive", mCaseSensitive);
    VColorMapper::readColors(mapperNode, errorList);
    }

VColorPair VStringColorMapper::getColors(const VString& stringValue) const
    {
    VString key(stringValue);
    if (!mCaseSensitive)
        key.toLowerCase();

    VStringColorMap::const_iterator position = mColorMap.find(key);
    if (position == mColorMap.end())
        return mDefaultColors;

    return (*position).second;
    }

VColorPair VStringColorMapper::getColors(int intValue) const
    {
    return this->getColors(VSTRING_INT(intValue));
    }

VColorPair VStringColorMapper::getColors(Vs64 int64Value) const
    {
    return this->getColors(VSTRING_S64(int64Value));
    }

VColorPair VStringColorMapper::getColors(VDouble doubleValue) const
    {
    return this->getColors(VSTRING_DOUBLE(doubleValue));
    }

void VStringColorMapper::addColors(const VString& stringValue, const VColorPair& colors)
    {
    VString key(stringValue);
    if (!mCaseSensitive)
        key.toLowerCase();

    mColorMap[key] = colors;
    }

void VStringColorMapper::_readColorElement(const VSettingsNode& colorNode)
    {
    this->addColors(colorNode.getString("value"), this->_readColorPair(colorNode));
    }

// VIntegerColorMapper --------------------------------------------------------

VIntegerColorMapper::VIntegerColorMapper() :
VColorMapper(),
mColorMap()
    {
    }

VIntegerColorMapper::~VIntegerColorMapper()
    {
    }

VColorPair VIntegerColorMapper::getColors(const VString& stringValue) const
    {
    try
        {
        Vs64 int64Value = stringValue.parseS64();
        return this->getColors(int64Value);
        }
    catch (...) {} // Will occur if the string is not a parseable integer.
    
    return mDefaultColors;
    }

VColorPair VIntegerColorMapper::getColors(int intValue) const
    {
    Vs64 int64Value = intValue;
    return this->getColors(int64Value);
    }

VColorPair VIntegerColorMapper::getColors(Vs64 int64Value) const
    {
    VIntegerColorMap::const_iterator position = mColorMap.find(int64Value);
    if (position == mColorMap.end())
        return mDefaultColors;

    return (*position).second;
    }

VColorPair VIntegerColorMapper::getColors(VDouble doubleValue) const
    {
    // No perfect way to know what a generic caller expects here; they really should
    // be using VDoubleColorMapper or VDoubleRangeColorMapper if they don't like this behavior.
    // We choose to truncate double to integer, and use that.
    // The effect is that 3, 3.0. 3.1, 3.14, etc. will all have the same output.

    Vs64 int64Value = static_cast<Vs64>(doubleValue);
    return this->getColors(int64Value);
    }

void VIntegerColorMapper::addColors(Vs64 intValue, const VColorPair& colors)
    {
    mColorMap[intValue] = colors;
    }

void VIntegerColorMapper::_readColorElement(const VSettingsNode& colorNode)
    {
    this->addColors(colorNode.getS64("value"), this->_readColorPair(colorNode));
    }

// VDoubleColorMapper ---------------------------------------------------------

VDoubleColorMapper::VDoubleColorMapper() :
VColorMapper(),
mColorMap()
    {
    }

VDoubleColorMapper::~VDoubleColorMapper()
    {
    }

VColorPair VDoubleColorMapper::getColors(const VString& stringValue) const
    {
    try
        {
        VDouble doubleValue = stringValue.parseDouble();
        return this->getColors(doubleValue);
        }
    catch (...) {} // Will occur if the string is not a parseable double.
    
    return mDefaultColors;
    }

VColorPair VDoubleColorMapper::getColors(int intValue) const
    {
    VDouble doubleValue = static_cast<VDouble>(intValue);
    return this->getColors(doubleValue);
    }

VColorPair VDoubleColorMapper::getColors(Vs64 int64Value) const
    {
    VDouble doubleValue = static_cast<VDouble>(int64Value);
    return this->getColors(doubleValue);
    }

VColorPair VDoubleColorMapper::getColors(VDouble doubleValue) const
    {
    VStringColorMap::const_iterator position = mColorMap.find(VSTRING_DOUBLE(doubleValue));
    if (position == mColorMap.end())
        return mDefaultColors;

    return (*position).second;
    }

void VDoubleColorMapper::addColors(VDouble doubleValue, const VColorPair& colors)
    {
    mColorMap[VSTRING_DOUBLE(doubleValue)] = colors;
    }

void VDoubleColorMapper::_readColorElement(const VSettingsNode& colorNode)
    {
    this->addColors(colorNode.getDouble("value"), this->_readColorPair(colorNode));
    }

// VStringRangeColorMapper --------------------------------------------------------

VStringRangeColorMapper::VStringRangeColorMapper(bool usesPrefixMode) :
VColorMapper(),
mColorRanges(),
mCaseSensitive(false),
mUsesPrefixMode(usesPrefixMode)
    {
    }

VStringRangeColorMapper::~VStringRangeColorMapper()
    {
    }

void VStringRangeColorMapper::readColors(const VSettingsNode& mapperNode, VStringVector* errorList)
    {
    mCaseSensitive = mapperNode.getBoolean("case-sensitive", mCaseSensitive);
    VColorMapper::readColors(mapperNode, errorList);
    }

VColorPair VStringRangeColorMapper::getColors(const VString& stringValue) const
    {
    if (mColorRanges.size() == 0)
        return mDefaultColors;

    VString caseAdjustedValue(stringValue);
    if (!mCaseSensitive)
        caseAdjustedValue.toLowerCase();

    // lower_bound() uses binary search, should be the fastest way.
    VStringRangeColorElement testValue(caseAdjustedValue, VColorPair());
    VStringRangeVector::const_iterator position = std::lower_bound(mColorRanges.begin(), mColorRanges.end(), testValue);
    
    // The way this works, we have to essentially decrement the position to find the spot that was "hit",
    // unless we got an exact match. To do this, we check if the found mRangeMin is greater than the value.
    if (position == mColorRanges.end())
        return this->_getColorsWithPrefixModeCheck((*(position - 1)).mColors);

    if (position == mColorRanges.begin() && position->mRangeMin > caseAdjustedValue)
        return mDefaultColors;
    
    if (position->mRangeMin > caseAdjustedValue)
        --position;

    return this->_getColorsWithPrefixModeCheck((*position).mColors);
    }

static const VColorPair EMPTY_MAP_COLOR_VALUE(VColor(1,1,1), VColor(2,2,2));

VColorPair VStringRangeColorMapper::_getColorsWithPrefixModeCheck(const VColorPair& foundColors) const
    {
    if (mUsesPrefixMode && (foundColors == EMPTY_MAP_COLOR_VALUE))
        return mDefaultColors;

    return foundColors;
    }

VColorPair VStringRangeColorMapper::getColors(int intValue) const
    {
    return this->getColors(VSTRING_INT(intValue));
    }

VColorPair VStringRangeColorMapper::getColors(Vs64 int64Value) const
    {
    return this->getColors(VSTRING_S64(int64Value));
    }

VColorPair VStringRangeColorMapper::getColors(VDouble doubleValue) const
    {
    return this->getColors(VSTRING_DOUBLE(doubleValue));
    }

void VStringRangeColorMapper::addColors(const VString& rangeMin, const VColorPair& rangeColors)
    {
    VString caseAdjustedValue(rangeMin);
    if (!mCaseSensitive)
        caseAdjustedValue.toLowerCase();

    VStringRangeColorElement rangeElement(caseAdjustedValue, rangeColors);
    VStringRangeVector::iterator position = std::lower_bound(mColorRanges.begin(), mColorRanges.end(), rangeElement);
    (void) mColorRanges.insert(position, rangeElement);

    if (mUsesPrefixMode)
        {
        VStringRangeColorElement rangeEndingElement(caseAdjustedValue + "~", EMPTY_MAP_COLOR_VALUE);
        VStringRangeVector::iterator nextPosition = std::lower_bound(mColorRanges.begin(), mColorRanges.end(), rangeEndingElement);
        (void) mColorRanges.insert(nextPosition, rangeEndingElement);
        }
    }

void VStringRangeColorMapper::_readColorElement(const VSettingsNode& colorNode)
    {
    this->addColors(colorNode.getString("value"), this->_readColorPair(colorNode));
    }

// VIntegerRangeColorMapper --------------------------------------------------------

VIntegerRangeColorMapper::VIntegerRangeColorMapper() :
VColorMapper(),
mColorRanges()
    {
    }

VIntegerRangeColorMapper::~VIntegerRangeColorMapper()
    {
    }

VColorPair VIntegerRangeColorMapper::getColors(const VString& stringValue) const
    {
    try
        {
        Vs64 int64Value = stringValue.parseS64();
        return this->getColors(int64Value);
        }
    catch (...) {} // Will occur if the string is not a parseable integer.
    
    return mDefaultColors;
    }

VColorPair VIntegerRangeColorMapper::getColors(int intValue) const
    {
    Vs64 int64Value = intValue;
    return this->getColors(int64Value);
    }

VColorPair VIntegerRangeColorMapper::getColors(Vs64 int64Value) const
    {
    if (mColorRanges.size() == 0)
        return mDefaultColors;

    // lower_bound() uses binary search, should be the fastest way.
    VIntegerRangeColorElement testValue(int64Value, VColorPair());
    VIntegerRangeVector::const_iterator position = std::lower_bound(mColorRanges.begin(), mColorRanges.end(), testValue);
    
    // The way this works, we have to essentially decrement the position to find the spot that was "hit",
    // unless we got an exact match. To do this, we check if the found mRangeMin is greater than the value.
    if (position == mColorRanges.end())
        return (position - 1)->mColors;

    if (position == mColorRanges.begin() && position->mRangeMin > int64Value)
        return mDefaultColors;
    
    if (position->mRangeMin > int64Value)
        --position;

    return position->mColors;
    }

VColorPair VIntegerRangeColorMapper::getColors(VDouble doubleValue) const
    {
    Vs64 int64Value = static_cast<Vs64>(doubleValue);
    return this->getColors(int64Value);
    }

void VIntegerRangeColorMapper::addColors(Vs64 rangeMin, const VColorPair& rangeColors)
    {
    VIntegerRangeColorElement rangeElement(rangeMin, rangeColors);
    VIntegerRangeVector::iterator position = std::lower_bound(mColorRanges.begin(), mColorRanges.end(), rangeElement);
    (void) mColorRanges.insert(position, rangeElement);
    }

void VIntegerRangeColorMapper::_readColorElement(const VSettingsNode& colorNode)
    {
    this->addColors(colorNode.getInt("value"), this->_readColorPair(colorNode));
    }

// VDoubleRangeColorMapper --------------------------------------------------------

VDoubleRangeColorMapper::VDoubleRangeColorMapper() :
VColorMapper(),
mColorRanges()
    {
    }

VDoubleRangeColorMapper::~VDoubleRangeColorMapper()
    {
    }

VColorPair VDoubleRangeColorMapper::getColors(const VString& stringValue) const
    {
    try
        {
        VDouble doubleValue = stringValue.parseDouble();
        return this->getColors(doubleValue);
        }
    catch (...) {} // Will occur if the string is not a parseable integer.
    
    return mDefaultColors;
    }

VColorPair VDoubleRangeColorMapper::getColors(int intValue) const
    {
    VDouble doubleValue = static_cast<VDouble>(intValue);
    return this->getColors(doubleValue);
    }

VColorPair VDoubleRangeColorMapper::getColors(Vs64 int64Value) const
    {
    VDouble doubleValue = static_cast<VDouble>(int64Value);
    return this->getColors(doubleValue);
    }

VColorPair VDoubleRangeColorMapper::getColors(VDouble doubleValue) const
    {
    if (mColorRanges.size() == 0)
        return mDefaultColors;

    // lower_bound() uses binary search, should be the fastest way.
    VDoubleRangeColorElement testValue(doubleValue, VColorPair());
    VDoubleRangeVector::const_iterator position = std::lower_bound(mColorRanges.begin(), mColorRanges.end(), testValue);
    
    // The way this works, we have to essentially decrement the position to find the spot that was "hit",
    // unless we got an exact match. To do this, we check if the found mRangeMin is greater than the value.
    if (position == mColorRanges.end())
        return (position - 1)->mColors;

    if (position == mColorRanges.begin() && position->mRangeMin > doubleValue)
        return mDefaultColors;
    
    if (position->mRangeMin > doubleValue)
        --position;

    return position->mColors;
    }

void VDoubleRangeColorMapper::addColors(VDouble rangeMin, const VColorPair& rangeColors)
    {
    VDoubleRangeColorElement rangeElement(rangeMin, rangeColors);
    VDoubleRangeVector::iterator position = std::lower_bound(mColorRanges.begin(), mColorRanges.end(), rangeElement);
    (void) mColorRanges.insert(position, rangeElement);
    }

void VDoubleRangeColorMapper::_readColorElement(const VSettingsNode& colorNode)
    {
    this->addColors(colorNode.getDouble("value"), this->_readColorPair(colorNode));
    }
