/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vbento.h"
#include "vtypes_internal.h"

#include "vexception.h"
#include "vbufferedfilestream.h"

// VBentoTextParser ----------------------------------------------------------

/**
This class performs parsing of Bento Text Format data to create a Bento
data hierarchy from the text.
*/
class VBentoTextNodeParser
    {
    public:

        VBentoTextNodeParser();
        virtual ~VBentoTextNodeParser() {}

        void parse(VTextIOStream& stream, VBentoNode& buildNode);
        void parse(const VString& s, VBentoNode& buildNode);

    private:

        typedef enum
            {
            START,
            IN_NODE,
            IN_NODE_NAME,
            IN_ATTRIBUTE,
            IN_ATTRIBUTE_NAME,
            IN_ATTRIBUTE_TYPE,
            IN_ATTRIBUTE_PRE_VALUE,
            IN_ATTRIBUTE_PRE_VALUE_QUALIFIER,
            IN_ATTRIBUTE_VALUE_DOUBLE_QUOTED,
            IN_ATTRIBUTE_VALUE_SINGLE_QUOTED,
            IN_ATTRIBUTE_VALUE_UNQUOTED

            } TokenState;

        void _parseCharacter(const VChar& c);

        TokenState mTokenState;
        VString mPendingToken;
        bool mTokenEscapePending;
        VBentoNode* mRootNode;
        VBentoNode* mPendingNode;
        VBentoNodePtrVector mParseNodeStack;
        VString mPendingAttributeName;
        VString mPendingAttributeType;
        VString mPendingAttributeQualifier;
        VString mPendingAttributeValue;

        VBentoTextNodeParser(const VBentoTextNodeParser&); // not copyable
        VBentoTextNodeParser& operator=(const VBentoTextNodeParser&); // not assignable
    };

VBentoTextNodeParser::VBentoTextNodeParser() :
mTokenState(START),
mPendingToken(), // -> empty
mTokenEscapePending(false),
mRootNode(NULL),
mPendingNode(NULL),
mParseNodeStack(), // -> empty
mPendingAttributeName(), // -> empty
mPendingAttributeType(), // -> empty
mPendingAttributeValue() // -> empty
    {
    }

void VBentoTextNodeParser::parse(VTextIOStream& stream, VBentoNode& node)
    {
    mRootNode = &node;

    try
        {
        for (;;)
            {
            VChar c = stream.readCharacter();
            this->_parseCharacter(c);
            }
        }
    catch (const VEOFException& /*ex*/) {} // normal EOF on input stream
    catch (const VException& ex)
        {
        throw VException(VSTRING_FORMAT("The Bento text stream was incorrectly formatted: %s", ex.what()));
        }
    }

void VBentoTextNodeParser::parse(const VString& s, VBentoNode& node)
    {
    mRootNode = &node;

    try
        {
        int length = s.length();
        for (int i = 0; i < length; ++i)
            {
            VChar c = s[i];
            this->_parseCharacter(c);
            }
        }
    catch (const VEOFException& /*ex*/) {} // normal EOF on input stream
    catch (const VException& ex)
        {
        throw VException(VSTRING_FORMAT("The Bento text stream was incorrectly formatted: %s", ex.what()));
        }
    }

static bool _isSkippable(const VChar& c)
    {
    return (c.charValue() <= 0x20) || (c.charValue() == 0x7F);
    }

void VBentoTextNodeParser::_parseCharacter(const VChar& c)
    {
    switch (mTokenState)
        {
        case START:
            if (_isSkippable(c))
                ; // nothing
            else if (c == '{')
                {
                mTokenState = IN_NODE;
                mPendingNode = mRootNode;
                mParseNodeStack.push_back(mPendingNode);
                }
            else
                throw VException(VSTRING_FORMAT("Parser expected whitespace or { but got '%c'.", c.charValue()));
            break;
        case IN_NODE:
            if (_isSkippable(c))
                ; // nothing
            else if (c == '\"')
                {
                mTokenState = IN_NODE_NAME;
                }
            else if (c == '[')
                {
                mTokenState = IN_ATTRIBUTE;
                mPendingAttributeName = VString::EMPTY();
                mPendingAttributeType = VString::EMPTY();
                mPendingAttributeQualifier = VString::EMPTY();
                mPendingAttributeValue = VString::EMPTY();
                }
            else if (c == '{')
                {
                mTokenState = IN_NODE;
                VBentoNode* child = new VBentoNode();
                mPendingNode->addChildNode(child);
                mPendingNode = child;
                mParseNodeStack.push_back(child);
                }
            else if (c == '}')
                {
                mTokenState = IN_NODE;
                mParseNodeStack.pop_back(); // pop the last node
                if (mParseNodeStack.size() == 0)
                    mPendingNode = NULL; // we're back at top level outside all nodes
                else
                    mPendingNode = mParseNodeStack.back(); // the new last node is now pending
                }
            else
                throw VException(VSTRING_FORMAT("Parser expected whitespace, node name, [, {, or } but got '%c'.", c.charValue()));
            break;
        case IN_NODE_NAME:
            if (c == '\\') // backslash (escape) char
                {
                if (mTokenEscapePending)
                    {
                    mPendingToken += c;
                    mTokenEscapePending = false;
                    }
                else
                    mTokenEscapePending = true;
                }
            else if (c == '\"') // double-quote char
                {
                if (mTokenEscapePending)
                    {
                    mPendingToken += c;
                    mTokenEscapePending = false;
                    }
                else
                    {
                    mPendingNode->setName(mPendingToken);
                    mPendingToken = VString::EMPTY();
                    mTokenState = IN_NODE;
                    }
                }
            else
                {
                mPendingToken += c;
                mTokenEscapePending = false;
                }
            break;
        case IN_ATTRIBUTE:
            if (_isSkippable(c))
                ; // nothing
            else if (c == '\"')
                {
                mTokenState = IN_ATTRIBUTE_NAME;
                }
            else if (c == '(')
                {
                mTokenState = IN_ATTRIBUTE_TYPE;
                }
            else if (c == '=')
                {
                mTokenState = IN_ATTRIBUTE_PRE_VALUE;
                }
            else if (c == ']')
                {
                mTokenState = IN_NODE;

                mPendingNode->_addAttribute(VBentoAttribute::newObjectFromBentoTextValues(mPendingAttributeName, mPendingAttributeType, mPendingAttributeValue, mPendingAttributeQualifier));

                mPendingAttributeName = VString::EMPTY();
                mPendingAttributeType = VString::EMPTY();
                mPendingAttributeQualifier = VString::EMPTY();
                mPendingAttributeValue = VString::EMPTY();
                }
            else
                throw VException(VSTRING_FORMAT("Parser expected whitespace, attr name/type/value, or ] but got '%c'.", c.charValue()));
            break;
        case IN_ATTRIBUTE_NAME:
            if (c == '\\') // backslash (escape) char
                {
                if (mTokenEscapePending)
                    {
                    mPendingToken += c;
                    mTokenEscapePending = false;
                    }
                else
                    mTokenEscapePending = true;
                }
            else if (c == '\"') // double-quote char
                {
                if (mTokenEscapePending)
                    {
                    mPendingToken += c;
                    mTokenEscapePending = false;
                    }
                else
                    {
                    mPendingAttributeName = mPendingToken;
                    mPendingToken = VString::EMPTY();
                    mTokenState = IN_ATTRIBUTE;
                    }
                }
            else
                {
                mPendingToken += c;
                mTokenEscapePending = false;
                }
            break;
        case IN_ATTRIBUTE_TYPE:
            if (c == ')')
                {
                mPendingAttributeType = mPendingToken;
                mPendingToken = VString::EMPTY();
                mTokenState = IN_ATTRIBUTE;
                }
            else
                {
                mPendingToken += c;
                }
            break;
        case IN_ATTRIBUTE_PRE_VALUE:
            if (c == '(')
                {
                mTokenState = IN_ATTRIBUTE_PRE_VALUE_QUALIFIER;
                }
            else if (c == '\\') // backslash (escape) char
                {
                mTokenEscapePending = true;
                mTokenState = IN_ATTRIBUTE_VALUE_UNQUOTED;
                }
            else if (c == '\"') // double-quote char
                {
                if (mTokenEscapePending)
                    {
                    mPendingToken += c;
                    mTokenEscapePending = false;
                    }
                else
                    {
                    mPendingToken += c;
                    mTokenState = IN_ATTRIBUTE_VALUE_DOUBLE_QUOTED;
                    }
                }
            else if (c == '\'') // single-quote char
                {
                if (mTokenEscapePending)
                    {
                    mPendingToken += c;
                    mTokenEscapePending = false;
                    }
                else
                    {
                    mPendingToken += c;
                    mTokenState = IN_ATTRIBUTE_VALUE_SINGLE_QUOTED;
                    }
                }
            else
                {
                mPendingToken += c;
                mTokenState = IN_ATTRIBUTE_VALUE_UNQUOTED;
                mTokenEscapePending = false;
                }
            break;
        case IN_ATTRIBUTE_PRE_VALUE_QUALIFIER:
            if (c == ')')
                {
                mPendingAttributeQualifier = mPendingToken;
                mPendingToken = VString::EMPTY();
                mTokenState = IN_ATTRIBUTE_PRE_VALUE;
                }
            else
                {
                mPendingToken += c;
                }
            break;
        case IN_ATTRIBUTE_VALUE_DOUBLE_QUOTED:
        case IN_ATTRIBUTE_VALUE_SINGLE_QUOTED:
            if (c == '\\') // backslash (escape) char
                {
                if (mTokenEscapePending)
                    {
                    mPendingToken += c;
                    mTokenEscapePending = false;
                    }
                else
                    mTokenEscapePending = true;
                }
            else if (((c == '\"') && (mTokenState == IN_ATTRIBUTE_VALUE_DOUBLE_QUOTED)) ||  // matched double-quote char
                    ((c == '\'') && (mTokenState == IN_ATTRIBUTE_VALUE_SINGLE_QUOTED)))     // matched single-quote char
                {
                if (mTokenEscapePending)
                    {
                    mPendingToken += c;
                    mTokenEscapePending = false;
                    }
                else
                    {
                    mPendingToken += c;
                    mPendingAttributeValue = mPendingToken;
                    mPendingToken = VString::EMPTY();
                    mTokenState = IN_ATTRIBUTE;
                    }
                }
            else
                {
                mPendingToken += c;
                mTokenEscapePending = false;
                }
            break;
        case IN_ATTRIBUTE_VALUE_UNQUOTED:
            if (c == '\\') // backslash (escape) char
                {
                if (mTokenEscapePending)
                    {
                    mPendingToken += c;
                    mTokenEscapePending = false;
                    }
                else
                    mTokenEscapePending = true;
                }
            else if (_isSkippable(c)) // whitespace of some kind (a space or any lower unprintable character like CR, LF, tab, ...)
                {
                if (mTokenEscapePending)
                    {
                    mPendingToken += c;
                    mTokenEscapePending = false;
                    }
                else
                    {
                    mPendingAttributeValue = mPendingToken;
                    mPendingToken = VString::EMPTY();
                    mTokenState = IN_ATTRIBUTE;
                    }
                }
            else if (c == ']') // end of attribute (unless an escape is pending)
                {
                if (mTokenEscapePending)
                    {
                    mPendingToken += c;
                    mTokenEscapePending = false;
                    }
                else
                    {
                    mPendingAttributeValue = mPendingToken;
                    mPendingToken = VString::EMPTY();
                    mTokenState = IN_NODE;

                    mPendingNode->_addAttribute(VBentoAttribute::newObjectFromBentoTextValues(mPendingAttributeName, mPendingAttributeType, mPendingAttributeValue, mPendingAttributeQualifier));

                    mPendingAttributeName = VString::EMPTY();
                    mPendingAttributeType = VString::EMPTY();
                    mPendingAttributeQualifier = VString::EMPTY();
                    mPendingAttributeValue = VString::EMPTY();
                    }
                }
            else
                {
                mPendingToken += c;
                mTokenEscapePending = false;
                }
            break;
        default:
            break;
        }
    }

// VBentoAttribute -----------------------------------------------------------

VBentoAttribute::VBentoAttribute() :
mName("uninitialized"), mDataType(VString::EMPTY())
    {
    }

VBentoAttribute::VBentoAttribute(VBinaryIOStream& stream, const VString& dataType) :
mName(VString::EMPTY()), mDataType(dataType)
    {
    stream.readString(mName);
    }

VBentoAttribute::VBentoAttribute(const VString& name, const VString& dataType) :
mName(name), mDataType(dataType)
    {
    }

VBentoAttribute::~VBentoAttribute()
    {
    }

const VString& VBentoAttribute::getName() const
    {
    return mName;
    }

const VString& VBentoAttribute::getDataType() const
    {
    return mDataType;
    }

Vs64 VBentoAttribute::calculateContentSize() const
    {
    Vs64    lengthOfType = 4;
    Vs64    lengthOfName = VBentoNode::_getBinaryStringLength(mName);
    Vs64    lengthOfData = this->getDataLength();

    return lengthOfType + lengthOfName + lengthOfData;
    }

Vs64 VBentoAttribute::calculateTotalSize() const
    {
    Vs64    contentSize = this->calculateContentSize();
    Vs64    lengthOfLength = VBentoNode::_getLengthOfLength(contentSize);

    return lengthOfLength + contentSize;
    }

void VBentoAttribute::writeToStream(VBinaryIOStream& stream) const
    {
    Vs64    contentSize = this->calculateContentSize();

    VBentoNode::_writeLengthToStream(stream, contentSize);
    VBentoNode::_writeFourCharCodeToStream(stream, mDataType);
    stream.writeString(mName);

    this->writeDataToBinaryStream(stream);
    }

static void _escapeString(VString& s)
    {
    // Insert a backslash in front of any special character.
    s.replace("\\", "\\\\");
    s.replace("{", "\\{");
    s.replace("}", "\\}");
    s.replace("\"", "\\\"");
    s.replace("'", "\\'");
    }

static void _unescapeString(VString& s)
    {
    // Remove any backslash that precedes a special character.
    s.replace("\\'", "\'");
    s.replace("\\\"", "\"");
    s.replace("\\}", "}");
    s.replace("\\{", "{");
    s.replace("\\\\", "\\");
    }

void VBentoAttribute::writeToBentoTextStream(VTextIOStream& stream) const
    {
    VString name(mName);
    _escapeString(name);
    VString valueString;
    this->getValueAsBentoTextString(valueString);
    
    // The less-used types must self-describe their type in text form.
    // But String, bool, and vs32 are most common and we can infer them
    // from how we format them, so we can have a cleaner format for them.
    // - string are "quoted"
    // - char are 'quoted'
    // - vs32 (int) are unquoted and are numeric (a leading minus sign is OK)
    // - bool are unquoted true or false
    // - every other type has a (type) before the equal sign and is quoted.
    // Examples:
    // - A string:           "address"="123 Main St."
    // - An encoded string:  "address"=(US-ASCII)"123 Main St."
    // - A char:             "initial"='X'
    // - An integer:         "speed"=70
    // - A boolean:          "active"=true
    // - A 16-bit unsigned:  "message_id"(vu16)="7"
    // - A 64-bit signed:    "file_size(vs64)"="2723674238"
    // - A VISize:           "dim(sizi)"="25,30"
    // - A VPoint:           "location(pt_d)"="25.387,30.702"
    // - A VPoint3D:         "location(pt3d)"="25.387,30.702,37.252"
    // - A VLine:            "track(line)"="32.775,26.539:42.383,29.373"
    // - A VRect:            "bounds(recd)"="32.775,26.539:100.0*100.0"
    // - A VIPolygon:        "outline(poli)"="(24,30)(40,42)(56,30)"
    // - A VColor:           "shading(rgba)"="127,64,200,255"
    // - Binary data:        "thing(bina)"="0x165231FCE64546DE45AD" (0x is optional)
    if (mDataType == VBentoString::DATA_TYPE_ID())
        {
        _escapeString(valueString);
        const VBentoString* thisString = static_cast<const VBentoString*>(this); // already type-checked above, no need to dynamic cast
        const VString& encoding = thisString->getEncoding();
        if (encoding.isEmpty())
            stream.writeString(VSTRING_FORMAT("[\"%s\"=\"%s\"]", name.chars(), valueString.chars()));
        else
            stream.writeString(VSTRING_FORMAT("[\"%s\"=(%s)\"%s\"]", name.chars(), encoding.chars(), valueString.chars()));
        }
    else if (mDataType == VBentoChar::DATA_TYPE_ID())
        {
        _escapeString(valueString);
        stream.writeString(VSTRING_FORMAT("[\"%s\"='%s']", name.chars(), valueString.chars()));
        }
    else if ((mDataType == VBentoS32::DATA_TYPE_ID()) || (mDataType == VBentoBool::DATA_TYPE_ID()))
        {
        stream.writeString(VSTRING_FORMAT("[\"%s\"=%s]", name.chars(), valueString.chars()));
        }
    else if (mDataType == VBentoStringArray::DATA_TYPE_ID())
        {
        VString dataType(mDataType);
        _escapeString(dataType);
        // Single-quote but do not escape the value string. It contains double-quoted, escaped elements.
        stream.writeString(VSTRING_FORMAT("[\"%s\"(%s)='%s']", name.chars(), dataType.chars(), valueString.chars()));
        }
    else
        {
        VString dataType(mDataType);
        _escapeString(dataType);
        _escapeString(valueString);
        stream.writeString(VSTRING_FORMAT("[\"%s\"(%s)=\"%s\"]", name.chars(), dataType.chars(), valueString.chars()));
        }
    }

void VBentoAttribute::writeToXMLTextStream(VTextIOStream& stream, bool /*lineWrap*/, int /*indentDepth*/) const
    {
    if (! this->xmlAppearsAsArray())
        {
        // Simple attributes do not use line wrap nor indent. They appear inline in the node's tag.
        stream.writeString(mName);
        stream.writeString("=\"");
        VString xmlText;
        this->getValueAsXMLText(xmlText);
        stream.writeString(xmlText);
        stream.writeString("\"");
        }
    }

void VBentoAttribute::printHexDump(VHex& hexDump) const
    {
    VMemoryStream buffer;
    VBinaryIOStream stream(buffer);

    this->writeToStream(stream);

    hexDump.printHex(buffer.getBuffer(), buffer.getEOFOffset());
    }

VBentoAttribute* VBentoAttribute::newObjectFromStream(VBinaryIOStream& stream)
    {
    Vs64    theDataLength = VBentoNode::_readLengthFromStream(stream);
    VString    theDataType;

    VBentoNode::_readFourCharCodeFromStream(stream, theDataType);
    
    // Put the most used and preferred types first for efficiency.
    if (theDataType == VBentoS32::DATA_TYPE_ID())
        return new VBentoS32(stream);
    else if (theDataType == VBentoString::DATA_TYPE_ID())
        return new VBentoString(stream);
    else if (theDataType == VBentoBool::DATA_TYPE_ID())
        return new VBentoBool(stream);
    else if (theDataType == VBentoChar::DATA_TYPE_ID())
        return new VBentoChar(stream);
    else if (theDataType == VBentoS64::DATA_TYPE_ID())
        return new VBentoS64(stream);
    else if (theDataType == VBentoDouble::DATA_TYPE_ID())
        return new VBentoDouble(stream);
    else if (theDataType == VBentoDuration::DATA_TYPE_ID())
        return new VBentoDuration(stream);
    else if (theDataType == VBentoInstant::DATA_TYPE_ID())
        return new VBentoInstant(stream);
    // Now the less used or less preferred types in order of definition.
    else if (theDataType == VBentoS8::DATA_TYPE_ID())
        return new VBentoS8(stream);
    else if (theDataType == VBentoU8::DATA_TYPE_ID())
        return new VBentoU8(stream);
    else if (theDataType == VBentoS16::DATA_TYPE_ID())
        return new VBentoS16(stream);
    else if (theDataType == VBentoU16::DATA_TYPE_ID())
        return new VBentoU16(stream);
    else if (theDataType == VBentoU32::DATA_TYPE_ID())
        return new VBentoU32(stream);
    else if (theDataType == VBentoU64::DATA_TYPE_ID())
        return new VBentoU64(stream);
    else if (theDataType == VBentoFloat::DATA_TYPE_ID())
        return new VBentoFloat(stream);
    else if (theDataType == VBentoSize::DATA_TYPE_ID())
        return new VBentoSize(stream);
    else if (theDataType == VBentoISize::DATA_TYPE_ID())
        return new VBentoISize(stream);
    else if (theDataType == VBentoPoint::DATA_TYPE_ID())
        return new VBentoPoint(stream);
    else if (theDataType == VBentoIPoint::DATA_TYPE_ID())
        return new VBentoIPoint(stream);
    else if (theDataType == VBentoPoint3D::DATA_TYPE_ID())
        return new VBentoPoint3D(stream);
    else if (theDataType == VBentoIPoint3D::DATA_TYPE_ID())
        return new VBentoIPoint3D(stream);
    else if (theDataType == VBentoLine::DATA_TYPE_ID())
        return new VBentoLine(stream);
    else if (theDataType == VBentoILine::DATA_TYPE_ID())
        return new VBentoILine(stream);
    else if (theDataType == VBentoRect::DATA_TYPE_ID())
        return new VBentoRect(stream);
    else if (theDataType == VBentoIRect::DATA_TYPE_ID())
        return new VBentoIRect(stream);
    else if (theDataType == VBentoPolygon::DATA_TYPE_ID())
        return new VBentoPolygon(stream);
    else if (theDataType == VBentoIPolygon::DATA_TYPE_ID())
        return new VBentoIPolygon(stream);
    else if (theDataType == VBentoColor::DATA_TYPE_ID())
        return new VBentoColor(stream);
    else if (theDataType == VBentoBinary::DATA_TYPE_ID())
        return new VBentoBinary(stream);
    else if (theDataType == VBentoS8Array::DATA_TYPE_ID())
        return new VBentoS8Array(stream);
    else if (theDataType == VBentoS16Array::DATA_TYPE_ID())
        return new VBentoS16Array(stream);
    else if (theDataType == VBentoS32Array::DATA_TYPE_ID())
        return new VBentoS32Array(stream);
    else if (theDataType == VBentoS64Array::DATA_TYPE_ID())
        return new VBentoS64Array(stream);
    else if (theDataType == VBentoStringArray::DATA_TYPE_ID())
        return new VBentoStringArray(stream);
    else if (theDataType == VBentoBoolArray::DATA_TYPE_ID())
        return new VBentoBoolArray(stream);
    else if (theDataType == VBentoDoubleArray::DATA_TYPE_ID())
        return new VBentoDoubleArray(stream);
    else if (theDataType == VBentoDurationArray::DATA_TYPE_ID())
        return new VBentoDurationArray(stream);
    else if (theDataType == VBentoInstantArray::DATA_TYPE_ID())
        return new VBentoInstantArray(stream);
    else
        return new VBentoUnknownValue(stream, theDataLength, theDataType);
    }

VBentoAttribute* VBentoAttribute::newObjectFromStream(VTextIOStream& /*stream*/)
    {
    // Reading unknown data types from a text stream is not (yet) supported.
    return new VBentoUnknownValue();
    }

VBentoAttribute* VBentoAttribute::newObjectFromBentoTextValues(const VString& attributeName, const VString& attributeType, const VString& attributeValue, const VString& attributeQualifier)
    {
    // First we have to determine the data type. If it is supplied,
    // it is wrapped in parentheses, so we just strip them. If it
    // is not supplied we must infer the type from the format of
    // the value. We support:
    //   numeric value strings imply int (leading minus sign is allowed)
    //   "quoted" value strings imply string
    //     - optional encoding name in parens before quoted string, for example: (US-ASCII)"foo"
    //   'quoted' value strings imply char
    //   true or false value strings imply bool
    //   NOW, PAST, FUTURE, NEVER value strings imply instant
    VBentoAttribute* result = NULL;
    VString actualValue = attributeValue;

    if (! attributeType.isEmpty())
        {
        if (actualValue.startsWith('\"') || actualValue.startsWith('\''))
            {
            actualValue.substringInPlace(1, actualValue.length() - 1);
            
            if (attributeType != VBentoStringArray::DATA_TYPE_ID()) // A string array's elements are themselves escaped, and will be parsed individually.
                _unescapeString(actualValue);
            }

        if (attributeType == VBentoS8::DATA_TYPE_ID())
            result = new VBentoS8(attributeName, static_cast<Vs8>(actualValue.parseS64()));
        else if (attributeType == VBentoU8::DATA_TYPE_ID())
            result = new VBentoU8(attributeName, static_cast<Vu8>(actualValue.parseU64()));
        else if (attributeType == VBentoS16::DATA_TYPE_ID())
            result = new VBentoS16(attributeName, static_cast<Vs16>(actualValue.parseS64()));
        else if (attributeType == VBentoU16::DATA_TYPE_ID())
            result = new VBentoU16(attributeName, static_cast<Vu16>(actualValue.parseU64()));
        else if (attributeType == VBentoS32::DATA_TYPE_ID())
            result = new VBentoS32(attributeName, static_cast<Vs32>(actualValue.parseS64()));
        else if (attributeType == VBentoU32::DATA_TYPE_ID())
            result = new VBentoU32(attributeName, static_cast<Vu32>(actualValue.parseU64()));
        else if (attributeType == VBentoS64::DATA_TYPE_ID())
            result = new VBentoS64(attributeName, actualValue.parseS64());
        else if (attributeType == VBentoU64::DATA_TYPE_ID())
            result = new VBentoU64(attributeName, actualValue.parseU64());
        else if (attributeType == VBentoBool::DATA_TYPE_ID())
            result = new VBentoBool(attributeName, actualValue == "true");
        else if (attributeType == VBentoString::DATA_TYPE_ID())
            result = new VBentoString(attributeName, actualValue, attributeQualifier/*the encoding*/);
        else if (attributeType == VBentoChar::DATA_TYPE_ID())
            result = new VBentoChar(attributeName, actualValue.length() == 0 ? VChar(0) : VChar(actualValue[0]));
        else if (attributeType == VBentoFloat::DATA_TYPE_ID())
            {
            VDouble d;
            ::sscanf(actualValue, VSTRING_FORMATTER_DOUBLE, &d);
            result = new VBentoFloat(attributeName, static_cast<VFloat>(d));
            }
        else if (attributeType == VBentoDouble::DATA_TYPE_ID())
            {
            VDouble d;
            ::sscanf(actualValue, VSTRING_FORMATTER_DOUBLE, &d);
            result = new VBentoDouble(attributeName, d);
            }
        else if (attributeType == VBentoDuration::DATA_TYPE_ID())
            {
            // Although we always generate with a "ms" suffix, allow any valid
            // VDuration magnitude suffix, by letting VDuration parse it.
            VDuration d;
            d.setDurationString(actualValue);
            result = new VBentoDuration(attributeName, d);
            }
        else if (attributeType == VBentoInstant::DATA_TYPE_ID())
            {
            VInstant i;
            i.setUTCString(actualValue);
            result = new VBentoInstant(attributeName, i);
            }
        else if (attributeType == VBentoSize::DATA_TYPE_ID())
            {
            VDouble width;
            VDouble height;
            ::sscanf(actualValue, "%lf,%lf", &width, &height);
            result = new VBentoSize(attributeName, VSize(width, height));
            }
        else if (attributeType == VBentoISize::DATA_TYPE_ID())
            {
            int width;
            int height;
            ::sscanf(actualValue, "%d,%d", &width, &height);
            result = new VBentoISize(attributeName, VISize(width, height));
            }
        else if (attributeType == VBentoPoint::DATA_TYPE_ID())
            {
            VDouble x;
            VDouble y;
            ::sscanf(actualValue, "%lf,%lf", &x, &y);
            result = new VBentoPoint(attributeName, VPoint(x, y));
            }
        else if (attributeType == VBentoIPoint::DATA_TYPE_ID())
            {
            int x;
            int y;
            ::sscanf(actualValue, "%d,%d", &x, &y);
            result = new VBentoIPoint(attributeName, VIPoint(x, y));
            }
        else if (attributeType == VBentoPoint3D::DATA_TYPE_ID())
            {
            VDouble x;
            VDouble y;
            VDouble z;
            ::sscanf(actualValue, "%lf,%lf,%lf", &x, &y, &z);
            result = new VBentoPoint3D(attributeName, VPoint3D(x, y, z));
            }
        else if (attributeType == VBentoIPoint3D::DATA_TYPE_ID())
            {
            int x;
            int y;
            int z;
            ::sscanf(actualValue, "%d,%d,%d", &x, &y, &z);
            result = new VBentoIPoint3D(attributeName, VIPoint3D(x, y, z));
            }
        else if (attributeType == VBentoLine::DATA_TYPE_ID())
            {
            VDouble x1;
            VDouble y1;
            VDouble x2;
            VDouble y2;
            ::sscanf(actualValue, "%lf,%lf:%lf,%lf", &x1, &y1, &x2, &y2);
            result = new VBentoLine(attributeName, VLine(VPoint(x1, y1), VPoint(x2, y2)));
            }
        else if (attributeType == VBentoILine::DATA_TYPE_ID())
            {
            int x1;
            int y1;
            int x2;
            int y2;
            ::sscanf(actualValue, "%d,%d:%d,%d", &x1, &y1, &x2, &y2);
            result = new VBentoILine(attributeName, VILine(VIPoint(x1, y1), VIPoint(x2, y2)));
            }
        else if (attributeType == VBentoRect::DATA_TYPE_ID())
            {
            VDouble x;
            VDouble y;
            VDouble width;
            VDouble height;
            ::sscanf(actualValue, "%lf,%lf:%lf*%lf", &x, &y, &width, &height);
            result = new VBentoRect(attributeName, VRect(VPoint(x, y), VSize(width, height)));
            }
        else if (attributeType == VBentoIRect::DATA_TYPE_ID())
            {
            int x;
            int y;
            int width;
            int height;
            ::sscanf(actualValue, "%d,%d:%d*%d", &x, &y, &width, &height);
            result = new VBentoIRect(attributeName, VIRect(VIPoint(x, y), VISize(width, height)));
            }
        else if (attributeType == VBentoPolygon::DATA_TYPE_ID())
            {
            VPolygon p;
            VBentoPolygon::readPolygonFromBentoTextString(actualValue, p);
            result = new VBentoPolygon(attributeName, p);
            }
        else if (attributeType == VBentoIPolygon::DATA_TYPE_ID())
            {
            VIPolygon p;
            VBentoIPolygon::readPolygonFromBentoTextString(actualValue, p);
            result = new VBentoIPolygon(attributeName, p);
            }
        else if (attributeType == VBentoColor::DATA_TYPE_ID())
            {
            int r;
            int g;
            int b;
            int alpha;
            ::sscanf(actualValue, "%d,%d,%d,%d", &r, &g, &b, &alpha);
            result = new VBentoColor(attributeName, VColor(r, g, b, alpha));
            }
        else if (attributeType == VBentoBinary::DATA_TYPE_ID())
            {
            result = VBentoBinary::newFromBentoTextString(attributeName, actualValue);
            }
        else if (attributeType == VBentoS8Array::DATA_TYPE_ID())
            {
            result = VBentoS8Array::newFromBentoTextString(attributeName, actualValue);
            }
        else if (attributeType == VBentoS16Array::DATA_TYPE_ID())
            {
            result = VBentoS16Array::newFromBentoTextString(attributeName, actualValue);
            }
        else if (attributeType == VBentoS32Array::DATA_TYPE_ID())
            {
            result = VBentoS32Array::newFromBentoTextString(attributeName, actualValue);
            }
        else if (attributeType == VBentoS64Array::DATA_TYPE_ID())
            {
            result = VBentoS64Array::newFromBentoTextString(attributeName, actualValue);
            }
        else if (attributeType == VBentoStringArray::DATA_TYPE_ID())
            {
            result = VBentoStringArray::newFromBentoTextString(attributeName, actualValue);
            }
        else if (attributeType == VBentoBoolArray::DATA_TYPE_ID())
            {
            result = VBentoBoolArray::newFromBentoTextString(attributeName, actualValue);
            }
        else if (attributeType == VBentoDoubleArray::DATA_TYPE_ID())
            {
            result = VBentoDoubleArray::newFromBentoTextString(attributeName, actualValue);
            }
        else if (attributeType == VBentoDurationArray::DATA_TYPE_ID())
            {
            result = VBentoDurationArray::newFromBentoTextString(attributeName, actualValue);
            }
        else if (attributeType == VBentoInstantArray::DATA_TYPE_ID())
            {
            result = VBentoInstantArray::newFromBentoTextString(attributeName, actualValue);
            }
        else
            throw VException(VSTRING_FORMAT("Parser encountered unknown data type '%s'", attributeType.chars()));
        }
    else
        {
        // Infer the type from the format of the value.
        if ((attributeValue == "true") || (attributeValue == "false"))
            {
            result = new VBentoBool(attributeName, attributeValue == "true");
            }
        else if ((attributeValue == "NOW") || (attributeValue == "NEVER") || (attributeValue == "PAST") || (attributeValue == "FUTURE"))
            {
            VInstant when;
            when.setLocalString(attributeValue);
            result = new VBentoInstant(attributeName, when);
            }
        else if (attributeValue.startsWith('\"') || attributeValue.startsWith('('))
            {
            actualValue.substringInPlace(1, actualValue.length() - 1);
            _unescapeString(actualValue);
            result = new VBentoString(attributeName, actualValue, attributeQualifier/*the encoding*/);
            }
        else if (attributeValue.startsWith('\''))
            {
            attributeValue.getSubstring(actualValue, 1, attributeValue.length() - 1);
            _unescapeString(actualValue);
            VChar c;
            if (actualValue.length() > 0)
                c = actualValue[0];
            result = new VBentoChar(attributeName, c);
            }
        else
            {
            result = new VBentoS32(attributeName, static_cast<Vs32>(actualValue.parseS64()));
            }
        }

    return result;
    }

// static
void VBentoAttribute::_escapeXMLValue(VString& text)
    {
    text.replace("&", "&amp;");
    text.replace("'", "&apos;");
    text.replace("\"", "&quot;");
    text.replace("<", "&lt;");
    text.replace(">", "&gt;");

    // Escape any non-printing characters. 0x00 thru 0x1F, and 0x7F
    for (int c = 0; c < 0x20; ++c)
        text.replace(VChar(c), VSTRING_FORMAT("\\u%04x", c));

    text.replace(VChar(0x7F), VSTRING_FORMAT("\\u%04x", 0x7F));
    }

static VString _indent(int depth)
    {
    VString spaces;
    spaces.preflight(depth);

    for (int i = 0; i < depth; ++i)
        spaces += ' ';

    return spaces;
    }

static void _indentIfRequested(VTextIOStream& stream, bool lineWrap, int depth)
    {
    if (lineWrap && (depth > 0))
        stream.writeString(_indent(depth));
    }

static void _lineEndIfRequested(VTextIOStream& stream, bool lineWrap)
    {
    if (lineWrap)
        stream.writeLineEnd();
    }

/**
This helper understands how to indent and use a line end afterwards, according to the lineWrap and depth values.
*/
static void _writeLineItemToStream(VTextIOStream& stream, bool lineWrap, int depth, const VString& lineText)
    {
    _indentIfRequested(stream, lineWrap, depth);
    stream.writeString(lineText);
    _lineEndIfRequested(stream, lineWrap);
    }

// VBentoSize ----------------------------------------------------------------

void VBentoSize::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s width=\"%lf\" height=\"%lf\"/>", this->getName().chars(), mValue.getWidth(), mValue.getHeight()));
    }

// VBentoISize ---------------------------------------------------------------

void VBentoISize::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s width=\"%d\" height=\"%d\"/>", this->getName().chars(), mValue.getWidth(), mValue.getHeight()));
    }

// VBentoPoint ---------------------------------------------------------------

void VBentoPoint::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s x=\"%lf\" y=\"%lf\"/>", this->getName().chars(), mValue.getX(), mValue.getY()));
    }

// VBentoIPoint --------------------------------------------------------------

void VBentoIPoint::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s x=\"%d\" y=\"%d\"/>", this->getName().chars(), mValue.getX(), mValue.getY()));
    }

// VBentoPoint3D -------------------------------------------------------------

void VBentoPoint3D::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s x=\"%lf\" y=\"%lf\" z=\"%lf\"/>", this->getName().chars(), mValue.getX(), mValue.getY(), mValue.getZ()));
    }

// VBentoIPoint3D ------------------------------------------------------------

void VBentoIPoint3D::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s x=\"%d\" y=\"%d\" z=\"%d\"/>", this->getName().chars(), mValue.getX(), mValue.getY(), mValue.getZ()));
    }

// VBentoLine ----------------------------------------------------------------

void VBentoLine::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\"/>", this->getName().chars(), mValue.getP1().getX(), mValue.getP1().getY(), mValue.getP2().getX(), mValue.getP2().getY()));
    }

// VBentoILine ---------------------------------------------------------------

void VBentoILine::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"/>", this->getName().chars(), mValue.getP1().getX(), mValue.getP1().getY(), mValue.getP2().getX(), mValue.getP2().getY()));
    }

// VBentoRect ----------------------------------------------------------------

void VBentoRect::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s left=\"%lf\" top=\"%lf\" width=\"%lf\" height=\"%lf\"/>", this->getName().chars(), mValue.getLeft(), mValue.getTop(), mValue.getWidth(), mValue.getHeight()));
    }

// VBentoIRect ---------------------------------------------------------------

void VBentoIRect::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s left=\"%d\" top=\"%d\" width=\"%d\" height=\"%d\"/>", this->getName().chars(), mValue.getLeft(), mValue.getTop(), mValue.getWidth(), mValue.getHeight()));
    }

// VBentoPolygon -------------------------------------------------------------

// static
void VBentoPolygon::_formatPolygonAsBentoTextString(const VPolygon& p, VString& s)
    {
    // example: "(1.0,2.0)(3.0,4.0)(5.0,6.0)"
    int numPoints = p.getNumPoints();
    for (int i = 0; i < numPoints; ++i)
        {
        VPoint point = p.getPoint(i);
        s += VSTRING_FORMAT("(%lf,%lf)", point.getX(), point.getY());
        }
    }

// static
void VBentoPolygon::readPolygonFromBentoTextString(const VString& s, VPolygon& p)
    {
    // example: "(1.0,2.0)(3.0,4.0)(5.0,6.0)"
    p.eraseAll();
    VString nextPointText;
    int openParenIndex = s.indexOf('(');
    int closeParenIndex = s.indexOf(')', openParenIndex);
    while (openParenIndex != -1 && closeParenIndex != -1)
        {
        s.getSubstring(nextPointText, openParenIndex+1, closeParenIndex);
        VDouble x;
        VDouble y;
        ::sscanf(nextPointText, "%lf,%lf", &x, &y);
        p.add(VPoint(x, y));
        
        openParenIndex = s.indexOf('(', closeParenIndex);
        closeParenIndex = s.indexOf(')', openParenIndex);
        }
    }

void VBentoPolygon::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s>", this->getName().chars()));
    
    const VPointVector& pts = mValue.getPoints();
    for (VPointVector::const_iterator i = pts.begin(); i != pts.end(); ++i)
        _writeLineItemToStream(stream, lineWrap, indentDepth + 1, VSTRING_FORMAT("<point x=\"%lf\" y=\"%lf\"/>", (*i).getX(), (*i).getY()));
    
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("</%s>", this->getName().chars()));
    }

// VBentoIPolygon -------------------------------------------------------------

// static
void VBentoIPolygon::_formatPolygonAsBentoTextString(const VIPolygon& p, VString& s)
    {
    // example: "(1,2)(3,4)(5,6)"
    int numPoints = p.getNumPoints();
    for (int i = 0; i < numPoints; ++i)
        {
        VIPoint point = p.getPoint(i);
        s += VSTRING_FORMAT("(%d,%d)", point.getX(), point.getY());
        }
    }

// static
void VBentoIPolygon::readPolygonFromBentoTextString(const VString& s, VIPolygon& p)
    {
    // example: "(1,2)(3,4)(5,6)"
    p.eraseAll();
    VString nextPointText;
    int openParenIndex = s.indexOf('(');
    int closeParenIndex = s.indexOf(')', openParenIndex);
    while (openParenIndex != -1 && closeParenIndex != -1)
        {
        s.getSubstring(nextPointText, openParenIndex+1, closeParenIndex);
        int x;
        int y;
        ::sscanf(nextPointText, "%d,%d", &x, &y);
        p.add(VIPoint(x, y));
        
        openParenIndex = s.indexOf('(', closeParenIndex);
        closeParenIndex = s.indexOf(')', openParenIndex);
        }
    }

void VBentoIPolygon::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s>", this->getName().chars()));
    
    const VIPointVector& pts = mValue.getPoints();
    for (VIPointVector::const_iterator i = pts.begin(); i != pts.end(); ++i)
        _writeLineItemToStream(stream, lineWrap, indentDepth + 1, VSTRING_FORMAT("<point x=\"%d\" y=\"%d\"/>", (*i).getX(), (*i).getY()));
    
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("</%s>", this->getName().chars()));
    }

// VBentoBinary --------------------------------------------------------------

// static
VBentoBinary* VBentoBinary::newFromBentoTextString(const VString& name, const VString& bentoText)
    {
    int bufferLength = (bentoText.length() + 1) / 2;

    bool hasLeading0x = bentoText.startsWith("0x") || bentoText.startsWith("0X");
    if (hasLeading0x)
        --bufferLength;

    Vu8* buffer = new Vu8[bufferLength];
    VHex::hexStringToBuffer(bentoText, buffer, hasLeading0x);
    
    VBentoBinary* result = new VBentoBinary(name, buffer, VMemoryStream::kAllocatedByOperatorNew, true /*adoptBuffer*/, bufferLength, bufferLength);
    return result;
    }

void VBentoBinary::writeDataToBinaryStream(VBinaryIOStream& stream) const
    {
    Vs64 length = mValue.getEOFOffset();
    VReadOnlyMemoryStream reader(mValue.getBuffer(), length);

    VBentoNode::_writeLengthToStream(stream, length);
    (void) VStream::streamCopy(reader, stream, length);
    }

void VBentoBinary::_getValueAsHexString(VString& s) const
    {
    VHex::bufferToHexString(mValue.getBuffer(), mValue.getEOFOffset(), s, true/*want leading 0x*/);
    }

// VBentoS8Array --------------------------------------------------------------

// static
VBentoS8Array* VBentoS8Array::newFromBentoTextString(const VString& name, const VString& s)
    {
    VBentoS8Array* result = new VBentoS8Array(name);

    // example: "0,1,2"
    VString nextElementText;
    int previousSeparatorIndex = -1;
    int nextSeparatorIndex = s.indexOf(',');
    do
        {
        s.getSubstring(nextElementText, previousSeparatorIndex+1, nextSeparatorIndex);
        int value;
        ::sscanf(nextElementText, VSTRING_FORMATTER_INT, &value); // Note: %hhd for Vs8 not universally supported; using plain old int conversion instead.
        result->appendValue(static_cast<Vs8>(value));
        
        previousSeparatorIndex = nextSeparatorIndex;
        nextSeparatorIndex = s.indexOf(',', previousSeparatorIndex+1);
        } while (previousSeparatorIndex != -1);

    return result;
    }

void VBentoS8Array::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s>", this->getName().chars()));

    for (Vs8Array::const_iterator i = mValue.begin(); i != mValue.end(); ++i)
        _writeLineItemToStream(stream, lineWrap, indentDepth + 1, VSTRING_FORMAT("<item value=\"%d\"/>", (int) (*i)));

    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("</%s>", this->getName().chars()));
    }

// VBentoS16Array --------------------------------------------------------------

// static
VBentoS16Array* VBentoS16Array::newFromBentoTextString(const VString& name, const VString& s)
    {
    VBentoS16Array* result = new VBentoS16Array(name);

    // example: "0,1,2"
    VString nextElementText;
    int previousSeparatorIndex = -1;
    int nextSeparatorIndex = s.indexOf(',');
    do
        {
        s.getSubstring(nextElementText, previousSeparatorIndex+1, nextSeparatorIndex);
        Vs16 value;
        ::sscanf(nextElementText, VSTRING_FORMATTER_S16, &value);
        result->appendValue(value);
        
        previousSeparatorIndex = nextSeparatorIndex;
        nextSeparatorIndex = s.indexOf(',', previousSeparatorIndex+1);
        } while (previousSeparatorIndex != -1);

    return result;
    }

void VBentoS16Array::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s>", this->getName().chars()));

    for (Vs16Array::const_iterator i = mValue.begin(); i != mValue.end(); ++i)
        _writeLineItemToStream(stream, lineWrap, indentDepth + 1, VSTRING_FORMAT("<item value=\"%hd\"/>", (*i)));

    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("</%s>", this->getName().chars()));
    }

// VBentoS32Array --------------------------------------------------------------

// static
VBentoS32Array* VBentoS32Array::newFromBentoTextString(const VString& name, const VString& s)
    {
    VBentoS32Array* result = new VBentoS32Array(name);

    // example: "0,1,2"
    VString nextElementText;
    int previousSeparatorIndex = -1;
    int nextSeparatorIndex = s.indexOf(',');
    do
        {
        s.getSubstring(nextElementText, previousSeparatorIndex+1, nextSeparatorIndex);
        Vs32 value;
        ::sscanf(nextElementText, VSTRING_FORMATTER_S32, &value);
        result->appendValue(value);
        
        previousSeparatorIndex = nextSeparatorIndex;
        nextSeparatorIndex = s.indexOf(',', previousSeparatorIndex+1);
        } while (previousSeparatorIndex != -1);

    return result;
    }

void VBentoS32Array::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s>", this->getName().chars()));

    for (Vs32Array::const_iterator i = mValue.begin(); i != mValue.end(); ++i)
        _writeLineItemToStream(stream, lineWrap, indentDepth + 1, VSTRING_FORMAT("<item value=\"%ld\"/>", (*i)));

    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("</%s>", this->getName().chars()));
    }

// VBentoS64Array --------------------------------------------------------------

// static
VBentoS64Array* VBentoS64Array::newFromBentoTextString(const VString& name, const VString& s)
    {
    VBentoS64Array* result = new VBentoS64Array(name);

    // example: "0,1,2"
    VString nextElementText;
    int previousSeparatorIndex = -1;
    int nextSeparatorIndex = s.indexOf(',');
    do
        {
        s.getSubstring(nextElementText, previousSeparatorIndex+1, nextSeparatorIndex);
        Vs64 value;
        ::sscanf(nextElementText, VSTRING_FORMATTER_S64, &value);
        result->appendValue(value);
        
        previousSeparatorIndex = nextSeparatorIndex;
        nextSeparatorIndex = s.indexOf(',', previousSeparatorIndex+1);
        } while (previousSeparatorIndex != -1);

    return result;
    }

void VBentoS64Array::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s>", this->getName().chars()));

    for (Vs64Array::const_iterator i = mValue.begin(); i != mValue.end(); ++i)
        _writeLineItemToStream(stream, lineWrap, indentDepth + 1, VSTRING_FORMAT("<item value=\"%lld\"/>", (*i)));

    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("</%s>", this->getName().chars()));
    }

// VBentoBoolArray --------------------------------------------------------------

// static
VBentoBoolArray* VBentoBoolArray::newFromBentoTextString(const VString& name, const VString& s)
    {
    VBentoBoolArray* result = new VBentoBoolArray(name);

    // example: "true,false,true"
    VString nextElementText;
    int previousSeparatorIndex = -1;
    int nextSeparatorIndex = s.indexOf(',');
    do
        {
        s.getSubstring(nextElementText, previousSeparatorIndex+1, nextSeparatorIndex);
        result->appendValue(nextElementText.equalsIgnoreCase("true"));
        
        previousSeparatorIndex = nextSeparatorIndex;
        nextSeparatorIndex = s.indexOf(',', previousSeparatorIndex+1);
        } while (previousSeparatorIndex != -1);

    return result;
    }

void VBentoBoolArray::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s>", this->getName().chars()));

    for (VBoolArray::const_iterator i = mValue.begin(); i != mValue.end(); ++i)
        _writeLineItemToStream(stream, lineWrap, indentDepth + 1, VSTRING_FORMAT("<item value=\"%s\"/>", (*i) ? "true":"false"));

    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("</%s>", this->getName().chars()));
    }

// VBentoStringArray --------------------------------------------------------------

typedef enum
    {
    kStringArrayParseState_Init,            // " -> InArray; else error
    kStringArrayParseState_InArray,         // " -> InElement; else error
    kStringArrayParseState_InElement,       // " add pending string then -> ElementEnded; \ -> EscapePending; else append character
    kStringArrayParseState_ElementEnded,    // , -> InArray; " -> Done; whitespace -> ignore; else error
    kStringArrayParseState_EscapePending,   // append character then -> InElement
    kStringArrayParseState_Done             // whitespace -> ignore; else error
    } StringArrayParseState;

// static
VBentoStringArray* VBentoStringArray::newFromBentoTextString(const VString& name, const VString& s)
    {
    VBentoStringArray* result = new VBentoStringArray(name);

    // example: "one","two\"quote\"two","three,comma,three"
    // The complete string we receive contains zero or more elements.
    // Each individual element string is double-quoted.
    // Any double-quote inside an element string is escaped with a backslash.
    
    VString pendingString;
    StringArrayParseState state = kStringArrayParseState_InArray;
    int length = s.length();
    for (int i = 0; i < length; ++i)
        {
        VChar c = s[i];

        switch (state)
            {
            case kStringArrayParseState_Init:
                if (c == '"')
                    state = kStringArrayParseState_InArray;
                else
                    throw VException(VSTRING_FORMAT("VBentoStringArray::newFromBentoTextString: At character %d in Init state, required \" but got %c.", i, c.charValue()));
                break;

            case kStringArrayParseState_InArray:
                if (c == '"')
                    state = kStringArrayParseState_InElement;
                else
                    throw VException(VSTRING_FORMAT("VBentoStringArray::newFromBentoTextString: At character %d in InArray state, required \" but got %c.", i, c.charValue()));
                break;

            case kStringArrayParseState_InElement:
                if (c == '"')
                    {
                    result->appendValue(pendingString);
                    pendingString = VString::EMPTY();
                    state = kStringArrayParseState_ElementEnded;
                    }
                else if (c == '\\')
                    state = kStringArrayParseState_EscapePending;
                else
                    pendingString += c;
                break;

            case kStringArrayParseState_ElementEnded:
                if (c == ',')
                    state = kStringArrayParseState_InArray;
                else if (c == '"')
                    state = kStringArrayParseState_Done;
                else if (c.isWhitespace())
                    ; // skip any whitespace between elements
                else
                    throw VException(VSTRING_FORMAT("VBentoStringArray::newFromBentoTextString: At character %d in ElementEnded state, required comma, \" or whitespace but got %c.", i, c.charValue()));
                break;

            case kStringArrayParseState_EscapePending:
                pendingString += c;
                state = kStringArrayParseState_InElement;
                break;

            case kStringArrayParseState_Done:
                if (! c.isWhitespace())
                    throw VException(VSTRING_FORMAT("VBentoStringArray::newFromBentoTextString: At character %d in Done state, required whitespace but got %c.", i, c.charValue()));
                break;

            }
        }

    return result;
    }

void VBentoStringArray::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s>", this->getName().chars()));

    for (VStringVector::const_iterator i = mValue.begin(); i != mValue.end(); ++i)
        {
        VString value = (*i);
        VBentoAttribute::_escapeXMLValue(value);
        _writeLineItemToStream(stream, lineWrap, indentDepth + 1, VSTRING_FORMAT("<item value=\"%s\"/>", value.chars()));
        }

    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("</%s>", this->getName().chars()));
    }

// VBentoDoubleArray --------------------------------------------------------------

// static
VBentoDoubleArray* VBentoDoubleArray::newFromBentoTextString(const VString& name, const VString& s)
    {
    VBentoDoubleArray* result = new VBentoDoubleArray(name);

    // example: "0.0,1.11,2.222"
    VString nextElementText;
    int previousSeparatorIndex = -1;
    int nextSeparatorIndex = s.indexOf(',');
    do
        {
        s.getSubstring(nextElementText, previousSeparatorIndex+1, nextSeparatorIndex);
        VDouble value;
        ::sscanf(nextElementText, VSTRING_FORMATTER_DOUBLE, &value);
        result->appendValue(value);
        
        previousSeparatorIndex = nextSeparatorIndex;
        nextSeparatorIndex = s.indexOf(',', previousSeparatorIndex+1);
        } while (previousSeparatorIndex != -1);

    return result;
    }

void VBentoDoubleArray::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s>", this->getName().chars()));

    for (VDoubleArray::const_iterator i = mValue.begin(); i != mValue.end(); ++i)
        _writeLineItemToStream(stream, lineWrap, indentDepth + 1, VSTRING_FORMAT("<item value=\"%lf\"/>", (*i)));

    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("</%s>", this->getName().chars()));
    }

// VBentoDurationArray --------------------------------------------------------------

// static
VBentoDurationArray* VBentoDurationArray::newFromBentoTextString(const VString& name, const VString& s)
    {
    VBentoDurationArray* result = new VBentoDurationArray(name);

    // example: "0ms,1111ms,2723847ms"
    VString nextElementText;
    int previousSeparatorIndex = -1;
    int nextSeparatorIndex = s.indexOf(',');
    do
        {
        s.getSubstring(nextElementText, previousSeparatorIndex+1, nextSeparatorIndex);
        Vs64 value;
        ::sscanf(nextElementText, VSTRING_FORMATTER_S64 "ms", &value);
        result->appendValue(VDuration::MILLISECOND() * value);
        
        previousSeparatorIndex = nextSeparatorIndex;
        nextSeparatorIndex = s.indexOf(',', previousSeparatorIndex+1);
        } while (previousSeparatorIndex != -1);

    return result;
    }

void VBentoDurationArray::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s>", this->getName().chars()));

    for (VDurationVector::const_iterator i = mValue.begin(); i != mValue.end(); ++i)
        _writeLineItemToStream(stream, lineWrap, indentDepth + 1, VSTRING_FORMAT("<item value=\"%s\"/>", (*i).getDurationString().chars()));

    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("</%s>", this->getName().chars()));
    }

// VBentoInstantArray --------------------------------------------------------------

// static
VBentoInstantArray* VBentoInstantArray::newFromBentoTextString(const VString& name, const VString& s)
    {
    VBentoInstantArray* result = new VBentoInstantArray(name);

    // example: ""2007-11-23 09:08:07.234 UTC","2007-12-17 13:14:15.678 UTC","2008-01-06 05:06:07.890""
    VString nextElementText;
    int previousSeparatorIndex = -1;
    int nextSeparatorIndex = s.indexOf(',');
    do
        {
        s.getSubstring(nextElementText, previousSeparatorIndex+1, nextSeparatorIndex);
        if (nextElementText.startsWith('"'))
            nextElementText.substringInPlace(1);
        if (nextElementText.endsWith('"'))
            nextElementText.substringInPlace(0, nextElementText.length() - 1);

        VInstant value = VInstant::NEVER_OCCURRED(); // avoid unnecessary clock read
        value.setUTCString(nextElementText);
        result->appendValue(value);
        
        previousSeparatorIndex = nextSeparatorIndex;
        nextSeparatorIndex = s.indexOf(',', previousSeparatorIndex+1);
        } while (previousSeparatorIndex != -1);

    return result;
    }

void VBentoInstantArray::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("<%s>", this->getName().chars()));

    for (VInstantVector::const_iterator i = mValue.begin(); i != mValue.end(); ++i)
        _writeLineItemToStream(stream, lineWrap, indentDepth + 1, VSTRING_FORMAT("<item value=\"%s\"/>", (*i).getUTCString().chars()));

    _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("</%s>", this->getName().chars()));
    }

// VBentoNode ----------------------------------------------------------------

VBentoNode::VBentoNode() :
mName("uninitialized"),
mAttributes(),
mParentNode(NULL),
mChildNodes()
    {
    }

VBentoNode::VBentoNode(const VString& name) :
mName(name),
mAttributes(),
mParentNode(NULL),
mChildNodes()
    {
    }

VBentoNode::VBentoNode(VBinaryIOStream& stream) :
mName(),
mAttributes(),
mParentNode(NULL),
mChildNodes()
    {
    this->readFromStream(stream);
    }

VBentoNode::VBentoNode(VTextIOStream& bentoTextStream) :
mName(),
mAttributes(),
mParentNode(NULL),
mChildNodes()
    {
    this->readFromBentoTextStream(bentoTextStream);
    }

VBentoNode::~VBentoNode()
    {
    try
        {
        VSizeType    numAttributes = mAttributes.size();
        for (VSizeType i = 0; i < numAttributes; ++i)
            delete mAttributes[i];

        VSizeType    numChildNodes = mChildNodes.size();
        for (VSizeType i = 0; i < numChildNodes; ++i)
            delete mChildNodes[i];
        }
    catch (...) // block exceptions from propagating
        {
        }

    mParentNode = NULL; //we do not own parent, it owns us
    }

VBentoNode::VBentoNode(const VBentoNode& original) :
mName(original.getName()),
mAttributes(),
mParentNode(NULL),
mChildNodes()
    {
    const VBentoAttributePtrVector& originalAttributes = original.getAttributes();
    for (VBentoAttributePtrVector::const_iterator i = originalAttributes.begin(); i != originalAttributes.end(); ++i)
        mAttributes.push_back((*i)->clone());

    const VBentoNodePtrVector& originalNodes = original.getNodes();
    for (VBentoNodePtrVector::const_iterator i = originalNodes.begin(); i != originalNodes.end(); ++i)
        {
        VBentoNode* child = new VBentoNode(**i);
        mChildNodes.push_back(child);
        child->mParentNode = this;
        }
    }

void VBentoNode::clear()
    {
    VSizeType    numAttributes = mAttributes.size();
    for (VSizeType i = 0; i < numAttributes; ++i)
        delete mAttributes[i];

    VSizeType    numChildNodes = mChildNodes.size();
    for (VSizeType i = 0; i < numChildNodes; ++i)
        delete mChildNodes[i];

    mAttributes.clear();
    mChildNodes.clear();
    }

void VBentoNode::orphanAttributes()
    {
    mAttributes.clear(); // does not actually delete the objects
    }

void VBentoNode::orphanNodes()
    {
    size_t numChildNodes = mChildNodes.size();
    for (VSizeType i = 0; i < numChildNodes; ++i)
        mChildNodes[i]->mParentNode = NULL;
    mChildNodes.clear(); // does not actually delete the objects
    }

void VBentoNode::orphanNode(const VBentoNode* node)
    {
    VBentoNodePtrVector::iterator position = std::find(mChildNodes.begin(), mChildNodes.end(), node);
    if (position != mChildNodes.end())
        {
        (**position).mParentNode = NULL;
        mChildNodes.erase(position);
        }
    }

void VBentoNode::adoptFrom(VBentoNode* node)
    {
    // Destroy any existing attributes and children of this node.
    this->clear();
    
    // Copy that node's name, then adopt its attributes and child nodes using shallow vector copy.
    mName = node->getName();
    mAttributes = node->mAttributes;
    mChildNodes = node->mChildNodes;   
    
    // We now own that node's attribute and child objects. Tell it to let go of them.
    node->orphanAttributes();
    node->orphanNodes();
    
    size_t numChildNodes = mChildNodes.size();
    for (VSizeType i = 0; i < numChildNodes; ++i)
        mChildNodes[i]->mParentNode = this; 
    }

void VBentoNode::updateFrom(const VBentoNode& source)
    {
    // Copy the name if not empty.
    if (source.getName().isNotEmpty())
        mName = source.getName();

    // Copy (adding as necessary) the attributes.
    const VBentoAttributePtrVector& sourceAttributes = source.getAttributes();
    for (VBentoAttributePtrVector::const_iterator i = sourceAttributes.begin(); i != sourceAttributes.end(); ++i)
        {
        VBentoAttribute* targetAttribute = this->_findMutableAttribute((*i)->getName(), (*i)->getDataType());
        if (targetAttribute == NULL)
            {
            // Clone the source attribute and add it.
            VBentoAttribute* clonedAttribute = (*i)->clone();
            this->_addAttribute(clonedAttribute);
            }
        else
            {
            // Copy source attribute to target using VBentoAttribute assignment operator.
            VBentoAttribute* sourceAttribute = *i;
            *targetAttribute = *sourceAttribute;
            }
        }

    const VBentoNodePtrVector& sourceChildren = source.getNodes();
    for (VBentoNodePtrVector::const_iterator i = sourceChildren.begin(); i != sourceChildren.end(); ++i)
        {
        const VBentoNode* targetChild = this->findNode((*i)->getName());
        if (targetChild == NULL)
            {
            // Clone the source node and add it.
            VBentoNode* clonedChild = new VBentoNode(**i);
            this->addChildNode(clonedChild);
            }
        else
            {
            // Recursively update the target child node.
            const_cast<VBentoNode*>(targetChild)->updateFrom(**i); // const_cast: WORKAROUND. Alternative is to define a findMutableNode() API.
            }
        }
    }

void VBentoNode::addChildNode(VBentoNode* node)
    {
    node->mParentNode = this;
    mChildNodes.push_back(node);
    }

VBentoNode* VBentoNode::addNewChildNode(const VString& name)
    {
    VBentoNode* child = new VBentoNode(name);
    child->mParentNode = this;
    mChildNodes.push_back(child);
    return child;
    }

void VBentoNode::addInt(const VString& name, int value) { this->addS32(name, static_cast<Vs32>(value)); }
void VBentoNode::addBool(const VString& name, bool value) { this->_addAttribute(new VBentoBool(name, value)); }
void VBentoNode::addString(const VString& name, const VString& value, const VString& encoding) { this->_addAttribute(new VBentoString(name, value, encoding)); }
void VBentoNode::addStringIfNotEmpty(const VString& name, const VString& value, const VString& encoding) { if (!value.isEmpty()) this->_addAttribute(new VBentoString(name, value, encoding)); }
void VBentoNode::addChar(const VString& name, const VChar& value) { this->_addAttribute(new VBentoChar(name, value)); }
void VBentoNode::addDouble(const VString& name, VDouble value) { this->_addAttribute(new VBentoDouble(name, value)); }
void VBentoNode::addDuration(const VString& name, const VDuration& value) { this->_addAttribute(new VBentoDuration(name, value)); }
void VBentoNode::addInstant(const VString& name, const VInstant& value) { this->_addAttribute(new VBentoInstant(name, value)); }
void VBentoNode::addSize(const VString& name, const VSize& value) { this->_addAttribute(new VBentoSize(name, value)); }
void VBentoNode::addISize(const VString& name, const VISize& value) { this->_addAttribute(new VBentoISize(name, value)); }
void VBentoNode::addPoint(const VString& name, const VPoint& value) { this->_addAttribute(new VBentoPoint(name, value)); }
void VBentoNode::addIPoint(const VString& name, const VIPoint& value) { this->_addAttribute(new VBentoIPoint(name, value)); }
void VBentoNode::addPoint3D(const VString& name, const VPoint3D& value) { this->_addAttribute(new VBentoPoint3D(name, value)); }
void VBentoNode::addIPoint3D(const VString& name, const VIPoint3D& value) { this->_addAttribute(new VBentoIPoint3D(name, value)); }
void VBentoNode::addLine(const VString& name, const VLine& value) { this->_addAttribute(new VBentoLine(name, value)); }
void VBentoNode::addILine(const VString& name, const VILine& value) { this->_addAttribute(new VBentoILine(name, value)); }
void VBentoNode::addRect(const VString& name, const VRect& value) { this->_addAttribute(new VBentoRect(name, value)); }
void VBentoNode::addIRect(const VString& name, const VIRect& value) { this->_addAttribute(new VBentoIRect(name, value)); }
void VBentoNode::addPolygon(const VString& name, const VPolygon& value) { this->_addAttribute(new VBentoPolygon(name, value)); }
void VBentoNode::addIPolygon(const VString& name, const VIPolygon& value) { this->_addAttribute(new VBentoIPolygon(name, value)); }
void VBentoNode::addColor(const VString& name, const VColor& value) { this->_addAttribute(new VBentoColor(name, value)); }
void VBentoNode::addS8(const VString& name, Vs8 value) { this->_addAttribute(new VBentoS8(name, value)); }
void VBentoNode::addU8(const VString& name, Vu8 value) { this->_addAttribute(new VBentoU8(name, value)); }
void VBentoNode::addS16(const VString& name, Vs16 value) { this->_addAttribute(new VBentoS16(name, value)); }
void VBentoNode::addU16(const VString& name, Vu16 value) { this->_addAttribute(new VBentoU16(name, value)); }
void VBentoNode::addS32(const VString& name, Vs32 value) { this->_addAttribute(new VBentoS32(name, value)); }
void VBentoNode::addU32(const VString& name, Vu32 value) { this->_addAttribute(new VBentoU32(name, value)); }
void VBentoNode::addS64(const VString& name, Vs64 value) { this->_addAttribute(new VBentoS64(name, value)); }
void VBentoNode::addU64(const VString& name, Vu64 value) { this->_addAttribute(new VBentoU64(name, value)); }
void VBentoNode::addFloat(const VString& name, VFloat value) { this->_addAttribute(new VBentoFloat(name, value)); }
void VBentoNode::addBinary(const VString& name, const Vu8* data, Vs64 length) { this->_addAttribute(new VBentoBinary(name, data, length)); }
void VBentoNode::addBinary(const VString& name, Vu8* data, VMemoryStream::BufferAllocationType allocationType, bool adoptBuffer, Vs64 suppliedBufferSize, Vs64 suppliedEOFOffset) { this->_addAttribute(new VBentoBinary(name, data, allocationType, adoptBuffer, suppliedBufferSize, suppliedEOFOffset)); }
VBentoS8Array* VBentoNode::addS8Array(const VString& name) { VBentoS8Array* attr = new VBentoS8Array(name); this->_addAttribute(attr); return attr;}
VBentoS8Array* VBentoNode::addS8Array(const VString& name, const Vs8Array& value) { VBentoS8Array* attr = new VBentoS8Array(name, value); this->_addAttribute(attr); return attr;}
VBentoS16Array* VBentoNode::addS16Array(const VString& name) { VBentoS16Array* attr = new VBentoS16Array(name); this->_addAttribute(attr); return attr;}
VBentoS16Array* VBentoNode::addS16Array(const VString& name, const Vs16Array& value) { VBentoS16Array* attr = new VBentoS16Array(name, value); this->_addAttribute(attr); return attr;}
VBentoS32Array* VBentoNode::addS32Array(const VString& name) { VBentoS32Array* attr = new VBentoS32Array(name); this->_addAttribute(attr); return attr;}
VBentoS32Array* VBentoNode::addS32Array(const VString& name, const Vs32Array& value) { VBentoS32Array* attr = new VBentoS32Array(name, value); this->_addAttribute(attr); return attr;}
VBentoS64Array* VBentoNode::addS64Array(const VString& name) { VBentoS64Array* attr = new VBentoS64Array(name); this->_addAttribute(attr); return attr;}
VBentoS64Array* VBentoNode::addS64Array(const VString& name, const Vs64Array& value) { VBentoS64Array* attr = new VBentoS64Array(name, value); this->_addAttribute(attr); return attr;}
VBentoStringArray* VBentoNode::addStringArray(const VString& name) { VBentoStringArray* attr = new VBentoStringArray(name); this->_addAttribute(attr); return attr;}
VBentoStringArray* VBentoNode::addStringArray(const VString& name, const VStringVector& value) { VBentoStringArray* attr = new VBentoStringArray(name, value); this->_addAttribute(attr); return attr;}
VBentoBoolArray* VBentoNode::addBoolArray(const VString& name) { VBentoBoolArray* attr = new VBentoBoolArray(name); this->_addAttribute(attr); return attr;}
VBentoBoolArray* VBentoNode::addBoolArray(const VString& name, const VBoolArray& value) { VBentoBoolArray* attr = new VBentoBoolArray(name, value); this->_addAttribute(attr); return attr;}
VBentoDoubleArray* VBentoNode::addDoubleArray(const VString& name) { VBentoDoubleArray* attr = new VBentoDoubleArray(name); this->_addAttribute(attr); return attr;}
VBentoDoubleArray* VBentoNode::addDoubleArray(const VString& name, const VDoubleArray& value) { VBentoDoubleArray* attr = new VBentoDoubleArray(name, value); this->_addAttribute(attr); return attr;}
VBentoDurationArray* VBentoNode::addDurationArray(const VString& name) { VBentoDurationArray* attr = new VBentoDurationArray(name); this->_addAttribute(attr); return attr;}
VBentoDurationArray* VBentoNode::addDurationArray(const VString& name, const VDurationVector& value) { VBentoDurationArray* attr = new VBentoDurationArray(name, value); this->_addAttribute(attr); return attr;}
VBentoInstantArray* VBentoNode::addInstantArray(const VString& name) { VBentoInstantArray* attr = new VBentoInstantArray(name); this->_addAttribute(attr); return attr;}
VBentoInstantArray* VBentoNode::addInstantArray(const VString& name, const VInstantVector& value) { VBentoInstantArray* attr = new VBentoInstantArray(name, value); this->_addAttribute(attr); return attr;}

void VBentoNode::writeToStream(VBinaryIOStream& stream) const
    {
    Vs64    contentSize = this->_calculateContentSize();
    VBentoNode::_writeLengthToStream(stream, contentSize);

    VSizeType    numAttributes = mAttributes.size();
    stream.writeSize32(numAttributes);

    VSizeType    numChildNodes = mChildNodes.size();
    stream.writeSize32(numChildNodes);

    stream.writeString(mName);

    for (VSizeType i = 0; i < numAttributes; ++i)
        mAttributes[i]->writeToStream(stream);

    for (VSizeType i = 0; i < numChildNodes; ++i)
        mChildNodes[i]->writeToStream(stream);
    }

void VBentoNode::writeToBentoTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _lineEndIfRequested(stream, lineWrap);
    _indentIfRequested(stream, lineWrap, indentDepth);

    VString name(mName);
    _escapeString(name);
    stream.writeString(VSTRING_FORMAT("{ \"%s\" ", name.chars()));

    VSizeType    numAttributes = mAttributes.size();
    for (VSizeType i = 0; i < numAttributes; ++i)
        {
        mAttributes[i]->writeToBentoTextStream(stream);
        stream.writeString(" ");
        }

    VSizeType    numChildNodes = mChildNodes.size();
    for (VSizeType i = 0; i < numChildNodes; ++i)
        {
        mChildNodes[i]->writeToBentoTextStream(stream, lineWrap, indentDepth + 1);
        stream.writeString(" ");
        }

    if (numChildNodes != 0)
        {
        _lineEndIfRequested(stream, lineWrap);
        _indentIfRequested(stream, lineWrap, indentDepth);
        }

    stream.writeString("}");
    }

void VBentoNode::writeToBentoTextString(VString& s) const
    {
    VMemoryStream buffer;
    VTextIOStream stream(buffer);
    this->writeToBentoTextStream(stream);
    stream.writeLineEnd();
    stream.seek0();
    stream.readLine(s);
    }

void VBentoNode::readFromStream(VBinaryIOStream& stream)
    {
    /* unused Vs64    contentSize = */ (void) VBentoNode::_readLengthFromStream(stream);
    Vs32    numAttributes = stream.readS32();
    Vs32    numChildNodes = stream.readS32();

    stream.readString(mName);

    for (int i = 0; i < numAttributes; ++i)
        this->_addAttribute(VBentoAttribute::newObjectFromStream(stream));

    for (int i = 0; i < numChildNodes; ++i)
        this->addChildNode(new VBentoNode(stream));
    }

void VBentoNode::readFromBentoTextStream(VTextIOStream& bentoTextStream)
    {
    VBentoTextNodeParser parser;
    parser.parse(bentoTextStream, *this);
    }

void VBentoNode::readFromBentoTextString(const VString& bentoTextString)
    {
    VBentoTextNodeParser parser;
    parser.parse(bentoTextString, *this);
    }
    
VBentoNode* VBentoNode::getParentNode() const
    {
    return mParentNode;
    }

const VBentoNodePtrVector& VBentoNode::getNodes() const
    {
    return mChildNodes;
    }

const VBentoNode* VBentoNode::findNode(const VString& nodeName) const
    {
    for (VBentoNodePtrVector::const_iterator i = mChildNodes.begin(); i != mChildNodes.end(); ++i)
        {
        if (nodeName.equalsIgnoreCase((*i)->getName()))
            return (*i);
        }

    return NULL;
    }

const VBentoNode* VBentoNode::findNode(const VString& nodeName, const VString& attributeName, const VString& dataType) const
    {
    for (VBentoNodePtrVector::const_iterator i = mChildNodes.begin(); i != mChildNodes.end(); ++i)
        {
        if (nodeName.equalsIgnoreCase((*i)->getName()))
            {
            if ((*i)->_findAttribute(attributeName, dataType) != NULL)
                return (*i);
            }
        }

    return NULL;
    }

int VBentoNode::getInt(const VString& name, int defaultValue) const
    {
    return static_cast<int>(this->getS32(name, static_cast<Vs32>(defaultValue)));
    }

int VBentoNode::getInt(const VString& name) const
    {
    return static_cast<int>(this->getS32(name));
    }

bool VBentoNode::getBool(const VString& name, bool defaultValue) const
    {
    const VBentoBool*    attribute = dynamic_cast<const VBentoBool*> (this->_findAttribute(name, VBentoBool::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

bool VBentoNode::getBool(const VString& name) const
    {
    const VBentoBool*    attribute = dynamic_cast<const VBentoBool*> (this->_findAttribute(name, VBentoBool::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoBool::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VString& VBentoNode::getString(const VString& name, const VString& defaultValue) const
    {
    const VBentoString*    attribute = dynamic_cast<const VBentoString*> (this->_findAttribute(name, VBentoString::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VString& VBentoNode::getString(const VString& name) const
    {
    const VBentoString*    attribute = dynamic_cast<const VBentoString*> (this->_findAttribute(name, VBentoString::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoString::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VChar& VBentoNode::getChar(const VString& name, const VChar& defaultValue) const
    {
    const VBentoChar*    attribute = dynamic_cast<const VBentoChar*> (this->_findAttribute(name, VBentoChar::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VChar& VBentoNode::getChar(const VString& name) const
    {
    const VBentoChar*    attribute = dynamic_cast<const VBentoChar*> (this->_findAttribute(name, VBentoChar::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoChar::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

VDouble VBentoNode::getDouble(const VString& name, VDouble defaultValue) const
    {
    const VBentoDouble*    attribute = dynamic_cast<const VBentoDouble*> (this->_findAttribute(name, VBentoDouble::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

VDouble VBentoNode::getDouble(const VString& name) const
    {
    const VBentoDouble*    attribute = dynamic_cast<const VBentoDouble*> (this->_findAttribute(name, VBentoDouble::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoDouble::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VDuration& VBentoNode::getDuration(const VString& name, const VDuration& defaultValue) const
    {
    const VBentoDuration*    attribute = dynamic_cast<const VBentoDuration*> (this->_findAttribute(name, VBentoDuration::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VDuration& VBentoNode::getDuration(const VString& name) const
    {
    const VBentoDuration*    attribute = dynamic_cast<const VBentoDuration*> (this->_findAttribute(name, VBentoDuration::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoDuration::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VInstant& VBentoNode::getInstant(const VString& name, const VInstant& defaultValue) const
    {
    const VBentoInstant*    attribute = dynamic_cast<const VBentoInstant*> (this->_findAttribute(name, VBentoInstant::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VInstant& VBentoNode::getInstant(const VString& name) const
    {
    const VBentoInstant*    attribute = dynamic_cast<const VBentoInstant*> (this->_findAttribute(name, VBentoInstant::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoInstant::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VSize& VBentoNode::getSize(const VString& name, const VSize& defaultValue) const
    {
    const VBentoSize*    attribute = dynamic_cast<const VBentoSize*> (this->_findAttribute(name, VBentoSize::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VSize& VBentoNode::getSize(const VString& name) const
    {
    const VBentoSize*    attribute = dynamic_cast<const VBentoSize*> (this->_findAttribute(name, VBentoSize::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoSize::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VISize& VBentoNode::getISize(const VString& name, const VISize& defaultValue) const
    {
    const VBentoISize*    attribute = dynamic_cast<const VBentoISize*> (this->_findAttribute(name, VBentoISize::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VISize& VBentoNode::getISize(const VString& name) const
    {
    const VBentoISize*    attribute = dynamic_cast<const VBentoISize*> (this->_findAttribute(name, VBentoISize::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoISize::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VPoint& VBentoNode::getPoint(const VString& name, const VPoint& defaultValue) const
    {
    const VBentoPoint*    attribute = dynamic_cast<const VBentoPoint*> (this->_findAttribute(name, VBentoPoint::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VPoint& VBentoNode::getPoint(const VString& name) const
    {
    const VBentoPoint*    attribute = dynamic_cast<const VBentoPoint*> (this->_findAttribute(name, VBentoPoint::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoPoint::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VIPoint& VBentoNode::getIPoint(const VString& name, const VIPoint& defaultValue) const
    {
    const VBentoIPoint*    attribute = dynamic_cast<const VBentoIPoint*> (this->_findAttribute(name, VBentoIPoint::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VIPoint& VBentoNode::getIPoint(const VString& name) const
    {
    const VBentoIPoint*    attribute = dynamic_cast<const VBentoIPoint*> (this->_findAttribute(name, VBentoIPoint::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoIPoint::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VPoint3D& VBentoNode::getPoint3D(const VString& name, const VPoint3D& defaultValue) const
    {
    const VBentoPoint3D*    attribute = dynamic_cast<const VBentoPoint3D*> (this->_findAttribute(name, VBentoPoint3D::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VPoint3D& VBentoNode::getPoint3D(const VString& name) const
    {
    const VBentoPoint3D*    attribute = dynamic_cast<const VBentoPoint3D*> (this->_findAttribute(name, VBentoPoint3D::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoPoint3D::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VIPoint3D& VBentoNode::getIPoint3D(const VString& name, const VIPoint3D& defaultValue) const
    {
    const VBentoIPoint3D*    attribute = dynamic_cast<const VBentoIPoint3D*> (this->_findAttribute(name, VBentoIPoint3D::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VIPoint3D& VBentoNode::getIPoint3D(const VString& name) const
    {
    const VBentoIPoint3D*    attribute = dynamic_cast<const VBentoIPoint3D*> (this->_findAttribute(name, VBentoIPoint3D::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoIPoint3D::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VLine& VBentoNode::getLine(const VString& name, const VLine& defaultValue) const
    {
    const VBentoLine*    attribute = dynamic_cast<const VBentoLine*> (this->_findAttribute(name, VBentoLine::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VLine& VBentoNode::getLine(const VString& name) const
    {
    const VBentoLine*    attribute = dynamic_cast<const VBentoLine*> (this->_findAttribute(name, VBentoLine::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoLine::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VILine& VBentoNode::getILine(const VString& name, const VILine& defaultValue) const
    {
    const VBentoILine*    attribute = dynamic_cast<const VBentoILine*> (this->_findAttribute(name, VBentoILine::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VILine& VBentoNode::getILine(const VString& name) const
    {
    const VBentoILine*    attribute = dynamic_cast<const VBentoILine*> (this->_findAttribute(name, VBentoILine::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoILine::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VRect& VBentoNode::getRect(const VString& name, const VRect& defaultValue) const
    {
    const VBentoRect*    attribute = dynamic_cast<const VBentoRect*> (this->_findAttribute(name, VBentoRect::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VRect& VBentoNode::getRect(const VString& name) const
    {
    const VBentoRect*    attribute = dynamic_cast<const VBentoRect*> (this->_findAttribute(name, VBentoRect::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoRect::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VIRect& VBentoNode::getIRect(const VString& name, const VIRect& defaultValue) const
    {
    const VBentoIRect*    attribute = dynamic_cast<const VBentoIRect*> (this->_findAttribute(name, VBentoIRect::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VIRect& VBentoNode::getIRect(const VString& name) const
    {
    const VBentoIRect*    attribute = dynamic_cast<const VBentoIRect*> (this->_findAttribute(name, VBentoIRect::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoIRect::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VPolygon& VBentoNode::getPolygon(const VString& name, const VPolygon& defaultValue) const
    {
    const VBentoPolygon*    attribute = dynamic_cast<const VBentoPolygon*> (this->_findAttribute(name, VBentoPolygon::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VPolygon& VBentoNode::getPolygon(const VString& name) const
    {
    const VBentoPolygon*    attribute = dynamic_cast<const VBentoPolygon*> (this->_findAttribute(name, VBentoPolygon::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoPolygon::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VIPolygon& VBentoNode::getIPolygon(const VString& name, const VIPolygon& defaultValue) const
    {
    const VBentoIPolygon*    attribute = dynamic_cast<const VBentoIPolygon*> (this->_findAttribute(name, VBentoIPolygon::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VIPolygon& VBentoNode::getIPolygon(const VString& name) const
    {
    const VBentoIPolygon*    attribute = dynamic_cast<const VBentoIPolygon*> (this->_findAttribute(name, VBentoIPolygon::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoIPolygon::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VColor& VBentoNode::getColor(const VString& name, const VColor& defaultValue) const
    {
    const VBentoColor*    attribute = dynamic_cast<const VBentoColor*> (this->_findAttribute(name, VBentoColor::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VColor& VBentoNode::getColor(const VString& name) const
    {
    const VBentoColor*    attribute = dynamic_cast<const VBentoColor*> (this->_findAttribute(name, VBentoColor::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoColor::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

Vs8 VBentoNode::getS8(const VString& name, Vs8 defaultValue) const
    {
    const VBentoS8*    attribute = dynamic_cast<const VBentoS8*> (this->_findAttribute(name, VBentoS8::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

Vs8 VBentoNode::getS8(const VString& name) const
    {
    const VBentoS8*    attribute = dynamic_cast<const VBentoS8*> (this->_findAttribute(name, VBentoS8::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoS8::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

Vu8 VBentoNode::getU8(const VString& name, Vu8 defaultValue) const
    {
    const VBentoU8*    attribute = dynamic_cast<const VBentoU8*> (this->_findAttribute(name, VBentoU8::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;

    return attribute->getValue();
    }

Vu8 VBentoNode::getU8(const VString& name) const
    {
    const VBentoU8*    attribute = dynamic_cast<const VBentoU8*> (this->_findAttribute(name, VBentoU8::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoU8::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

Vs16 VBentoNode::getS16(const VString& name, Vs16 defaultValue) const
    {
    const VBentoS16*    attribute = dynamic_cast<const VBentoS16*> (this->_findAttribute(name, VBentoS16::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;

    return attribute->getValue();
    }

Vs16 VBentoNode::getS16(const VString& name) const
    {
    const VBentoS16*    attribute = dynamic_cast<const VBentoS16*> (this->_findAttribute(name, VBentoS16::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoS16::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

Vu16 VBentoNode::getU16(const VString& name, Vu16 defaultValue) const
    {
    const VBentoU16*    attribute = dynamic_cast<const VBentoU16*> (this->_findAttribute(name, VBentoU16::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

Vu16 VBentoNode::getU16(const VString& name) const
    {
    const VBentoU16*    attribute = dynamic_cast<const VBentoU16*> (this->_findAttribute(name, VBentoU16::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoU16::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

Vs32 VBentoNode::getS32(const VString& name, Vs32 defaultValue) const
    {
    const VBentoS32*    attribute = dynamic_cast<const VBentoS32*> (this->_findAttribute(name, VBentoS32::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

Vs32 VBentoNode::getS32(const VString& name) const
    {
    const VBentoS32*    attribute = dynamic_cast<const VBentoS32*> (this->_findAttribute(name, VBentoS32::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoS32::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

Vu32 VBentoNode::getU32(const VString& name, Vu32 defaultValue) const
    {
    const VBentoU32*    attribute = dynamic_cast<const VBentoU32*> (this->_findAttribute(name, VBentoU32::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

Vu32 VBentoNode::getU32(const VString& name) const
    {
    const VBentoU32*    attribute = dynamic_cast<const VBentoU32*> (this->_findAttribute(name, VBentoU32::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoU32::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

Vs64 VBentoNode::getS64(const VString& name, Vs64 defaultValue) const
    {
    const VBentoS64*    attribute = dynamic_cast<const VBentoS64*> (this->_findAttribute(name, VBentoS64::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

Vs64 VBentoNode::getS64(const VString& name) const
    {
    const VBentoS64*    attribute = dynamic_cast<const VBentoS64*> (this->_findAttribute(name, VBentoS64::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoS64::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

Vu64 VBentoNode::getU64(const VString& name, Vu64 defaultValue) const
    {
    const VBentoU64*    attribute = dynamic_cast<const VBentoU64*> (this->_findAttribute(name, VBentoU64::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

Vu64 VBentoNode::getU64(const VString& name) const
    {
    const VBentoU64*    attribute = dynamic_cast<const VBentoU64*> (this->_findAttribute(name, VBentoU64::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoU64::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

VFloat VBentoNode::getFloat(const VString& name, VFloat defaultValue) const
    {
    const VBentoFloat*    attribute = dynamic_cast<const VBentoFloat*> (this->_findAttribute(name, VBentoFloat::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

VFloat VBentoNode::getFloat(const VString& name) const
    {
    const VBentoFloat*    attribute = dynamic_cast<const VBentoFloat*> (this->_findAttribute(name, VBentoFloat::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoFloat::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

bool VBentoNode::getBinary(const VString& name, VReadOnlyMemoryStream& returnedReader) const
    {
    const VBentoBinary*    attribute = dynamic_cast<const VBentoBinary*> (this->_findAttribute(name, VBentoBinary::DATA_TYPE_ID()));

    if (attribute == NULL)
        return false;

    returnedReader = attribute->getReader();
    return true;
    }
    
VReadOnlyMemoryStream VBentoNode::getBinary(const VString& name) const
    {
    const VBentoBinary*    attribute = dynamic_cast<const VBentoBinary*> (this->_findAttribute(name, VBentoBinary::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoBinary::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getReader();
    }

const Vs8Array& VBentoNode::getS8Array(const VString& name, const Vs8Array& defaultValue) const
    {
    const VBentoS8Array* attribute = dynamic_cast<const VBentoS8Array*> (this->_findAttribute(name, VBentoS8Array::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const Vs8Array& VBentoNode::getS8Array(const VString& name) const
    {
    const VBentoS8Array* attribute = dynamic_cast<const VBentoS8Array*> (this->_findAttribute(name, VBentoS8Array::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoS8Array::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const Vs16Array& VBentoNode::getS16Array(const VString& name, const Vs16Array& defaultValue) const
    {
    const VBentoS16Array* attribute = dynamic_cast<const VBentoS16Array*> (this->_findAttribute(name, VBentoS16Array::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const Vs16Array& VBentoNode::getS16Array(const VString& name) const
    {
    const VBentoS16Array* attribute = dynamic_cast<const VBentoS16Array*> (this->_findAttribute(name, VBentoS16Array::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoS16Array::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const Vs32Array& VBentoNode::getS32Array(const VString& name, const Vs32Array& defaultValue) const
    {
    const VBentoS32Array* attribute = dynamic_cast<const VBentoS32Array*> (this->_findAttribute(name, VBentoS32Array::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const Vs32Array& VBentoNode::getS32Array(const VString& name) const
    {
    const VBentoS32Array* attribute = dynamic_cast<const VBentoS32Array*> (this->_findAttribute(name, VBentoS32Array::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoS32Array::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const Vs64Array& VBentoNode::getS64Array(const VString& name, const Vs64Array& defaultValue) const
    {
    const VBentoS64Array* attribute = dynamic_cast<const VBentoS64Array*> (this->_findAttribute(name, VBentoS64Array::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const Vs64Array& VBentoNode::getS64Array(const VString& name) const
    {
    const VBentoS64Array* attribute = dynamic_cast<const VBentoS64Array*> (this->_findAttribute(name, VBentoS64Array::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoS64Array::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VStringVector& VBentoNode::getStringArray(const VString& name, const VStringVector& defaultValue) const
    {
    const VBentoStringArray* attribute = dynamic_cast<const VBentoStringArray*> (this->_findAttribute(name, VBentoStringArray::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;

    return attribute->getValue();
    }

const VStringVector& VBentoNode::getStringArray(const VString& name) const
    {
    const VBentoStringArray* attribute = dynamic_cast<const VBentoStringArray*> (this->_findAttribute(name, VBentoStringArray::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoStringArray::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VBoolArray& VBentoNode::getBoolArray(const VString& name, const VBoolArray& defaultValue) const
    {
    const VBentoBoolArray* attribute = dynamic_cast<const VBentoBoolArray*> (this->_findAttribute(name, VBentoBoolArray::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VBoolArray& VBentoNode::getBoolArray(const VString& name) const
    {
    const VBentoBoolArray* attribute = dynamic_cast<const VBentoBoolArray*> (this->_findAttribute(name, VBentoBoolArray::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoBoolArray::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VDoubleArray& VBentoNode::getDoubleArray(const VString& name, const VDoubleArray& defaultValue) const
    {
    const VBentoDoubleArray* attribute = dynamic_cast<const VBentoDoubleArray*> (this->_findAttribute(name, VBentoDoubleArray::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VDoubleArray& VBentoNode::getDoubleArray(const VString& name) const
    {
    const VBentoDoubleArray* attribute = dynamic_cast<const VBentoDoubleArray*> (this->_findAttribute(name, VBentoDoubleArray::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoDoubleArray::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VDurationVector& VBentoNode::getDurationArray(const VString& name, const VDurationVector& defaultValue) const
    {
    const VBentoDurationArray* attribute = dynamic_cast<const VBentoDurationArray*> (this->_findAttribute(name, VBentoDurationArray::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VDurationVector& VBentoNode::getDurationArray(const VString& name) const
    {
    const VBentoDurationArray* attribute = dynamic_cast<const VBentoDurationArray*> (this->_findAttribute(name, VBentoDurationArray::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoDurationArray::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

const VInstantVector& VBentoNode::getInstantArray(const VString& name, const VInstantVector& defaultValue) const
    {
    const VBentoInstantArray* attribute = dynamic_cast<const VBentoInstantArray*> (this->_findAttribute(name, VBentoInstantArray::DATA_TYPE_ID()));

    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VInstantVector& VBentoNode::getInstantArray(const VString& name) const
    {
    const VBentoInstantArray* attribute = dynamic_cast<const VBentoInstantArray*> (this->_findAttribute(name, VBentoInstantArray::DATA_TYPE_ID()));

    if (attribute == NULL)
        throw VException(VSTRING_FORMAT("Attribute type '%s' name '%s' not found.", VBentoInstantArray::DATA_TYPE_ID().chars(), name.chars()));

    return attribute->getValue();
    }

void VBentoNode::setInt(const VString& name, int value)
    {
    VBentoS32* attribute = dynamic_cast<VBentoS32*>(this->_findMutableAttribute(name, VBentoS32::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addInt(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setBool(const VString& name, bool value)
    {
    VBentoBool* attribute = dynamic_cast<VBentoBool*>(this->_findMutableAttribute(name, VBentoBool::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addBool(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setString(const VString& name, const VString& value, const VString& encoding)
    {
    VBentoString* attribute = dynamic_cast<VBentoString*>(this->_findMutableAttribute(name, VBentoString::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addString(name, value, encoding);
    else
        {
        attribute->setValue(value);
        attribute->setEncoding(encoding);
        }
    }

void VBentoNode::setChar(const VString& name, const VChar& value)
    {
    VBentoChar* attribute = dynamic_cast<VBentoChar*>(this->_findMutableAttribute(name, VBentoChar::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addChar(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setDouble(const VString& name, VDouble value)
    {
    VBentoDouble* attribute = dynamic_cast<VBentoDouble*>(this->_findMutableAttribute(name, VBentoDouble::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addDouble(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setDuration(const VString& name, const VDuration& value)
    {
    VBentoDuration* attribute = dynamic_cast<VBentoDuration*>(this->_findMutableAttribute(name, VBentoDuration::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addDuration(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setInstant(const VString& name, const VInstant& value)
    {
    VBentoInstant* attribute = dynamic_cast<VBentoInstant*>(this->_findMutableAttribute(name, VBentoInstant::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addInstant(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setSize(const VString& name, const VSize& value)
    {
    VBentoSize* attribute = dynamic_cast<VBentoSize*>(this->_findMutableAttribute(name, VBentoSize::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addSize(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setISize(const VString& name, const VISize& value)
    {
    VBentoISize* attribute = dynamic_cast<VBentoISize*>(this->_findMutableAttribute(name, VBentoISize::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addISize(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setPoint(const VString& name, const VPoint& value)
    {
    VBentoPoint* attribute = dynamic_cast<VBentoPoint*>(this->_findMutableAttribute(name, VBentoPoint::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addPoint(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setIPoint(const VString& name, const VIPoint& value)
    {
    VBentoIPoint* attribute = dynamic_cast<VBentoIPoint*>(this->_findMutableAttribute(name, VBentoIPoint::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addIPoint(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setPoint3D(const VString& name, const VPoint3D& value)
    {
    VBentoPoint3D* attribute = dynamic_cast<VBentoPoint3D*>(this->_findMutableAttribute(name, VBentoPoint3D::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addPoint3D(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setIPoint3D(const VString& name, const VIPoint3D& value)
    {
    VBentoIPoint3D* attribute = dynamic_cast<VBentoIPoint3D*>(this->_findMutableAttribute(name, VBentoIPoint3D::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addIPoint3D(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setLine(const VString& name, const VLine& value)
    {
    VBentoLine* attribute = dynamic_cast<VBentoLine*>(this->_findMutableAttribute(name, VBentoLine::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addLine(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setILine(const VString& name, const VILine& value)
    {
    VBentoILine* attribute = dynamic_cast<VBentoILine*>(this->_findMutableAttribute(name, VBentoILine::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addILine(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setRect(const VString& name, const VRect& value)
    {
    VBentoRect* attribute = dynamic_cast<VBentoRect*>(this->_findMutableAttribute(name, VBentoRect::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addRect(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setIRect(const VString& name, const VIRect& value)
    {
    VBentoIRect* attribute = dynamic_cast<VBentoIRect*>(this->_findMutableAttribute(name, VBentoIRect::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addIRect(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setPolygon(const VString& name, const VPolygon& value)
    {
    VBentoPolygon* attribute = dynamic_cast<VBentoPolygon*>(this->_findMutableAttribute(name, VBentoPolygon::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addPolygon(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setIPolygon(const VString& name, const VIPolygon& value)
    {
    VBentoIPolygon* attribute = dynamic_cast<VBentoIPolygon*>(this->_findMutableAttribute(name, VBentoIPolygon::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addIPolygon(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setColor(const VString& name, const VColor& value)
    {
    VBentoColor* attribute = dynamic_cast<VBentoColor*>(this->_findMutableAttribute(name, VBentoColor::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addColor(name, value);
    else
        attribute->setValue(value);
    }

void VBentoNode::setS64(const VString& name, Vs64 value)
    {
    VBentoS64* attribute = dynamic_cast<VBentoS64*>(this->_findMutableAttribute(name, VBentoS64::DATA_TYPE_ID()));
    if (attribute == NULL)
        this->addS64(name, value);
    else
        attribute->setValue(value);
    }

const VBentoAttributePtrVector& VBentoNode::getAttributes() const
    {
    return mAttributes;
    }

const VString& VBentoNode::getName() const
    {
    return mName;
    }

void VBentoNode::setName(const VString& name)
    {
    mName = name;
    }

void VBentoNode::writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const
    {
    _indentIfRequested(stream, lineWrap, indentDepth);
    stream.writeString("<");
    stream.writeString(mName);

    bool hasArrayAttributes = false;
    for (VBentoAttributePtrVector::const_iterator i = mAttributes.begin(); i != mAttributes.end(); ++i)
        {
        if ((*i)->xmlAppearsAsArray())
            {
            hasArrayAttributes = true;
            }
        else
            {
            stream.writeString(" ");
            (*i)->writeToXMLTextStream(stream, lineWrap, indentDepth + 1);
            }
        }

    if (hasArrayAttributes || (mChildNodes.size() != 0))
        stream.writeString(">"); // leave tag open for array/child elements
    else
        stream.writeString("/>"); // close tag, we're all done

    _lineEndIfRequested(stream, lineWrap);

    // Write any array attributes as child xml nodes.
    for (VBentoAttributePtrVector::const_iterator i = mAttributes.begin(); i != mAttributes.end(); ++i)
        {
        if ((*i)->xmlAppearsAsArray())
            {
            (*i)->writeToXMLTextStream(stream, lineWrap, indentDepth + 1);
            }
        }

    // Write child nodes as child xml nodes.
    for (VBentoNodePtrVector::const_iterator i = mChildNodes.begin(); i != mChildNodes.end(); ++i)
        (*i)->writeToXMLTextStream(stream, lineWrap, indentDepth + 1);

    // Close the tag if we left it open for child xml nodes.
    if (hasArrayAttributes || (mChildNodes.size() != 0))
        {
        _writeLineItemToStream(stream, lineWrap, indentDepth, VSTRING_FORMAT("</%s>", mName.chars()));
        }
    }

void VBentoNode::printXML() const
    {
    try
        {
        VBufferedFileStream    stdoutStream(stdout, false/*don't close on destruct*/);
        VTextIOStream        printStream(stdoutStream, VTextIOStream::kUseUnixLineEndings);

        this->writeToXMLTextStream(printStream);

        stdoutStream.flush();
        }
    catch (const VException& ex)
        {
        std::cout << "VBentoNode::printXML unable to print: '" << ex.what() << "'" << std::endl;
        }
    }

void VBentoNode::printHexDump(VHex& hexDump) const
    {
    VMemoryStream   buffer;
    VBinaryIOStream stream(buffer);

    VSizeType   numAttributes = mAttributes.size();
    VSizeType   numChildNodes = mChildNodes.size();
    Vs64        totalSize = this->_calculateContentSize();

    VBentoNode::_writeLengthToStream(stream, totalSize);
    stream.writeSize32(numAttributes);
    stream.writeSize32(numChildNodes);
    stream.writeString(mName);

    hexDump.printHex(buffer.getBuffer(), buffer.getEOFOffset());

    for (VSizeType i = 0; i < numAttributes; ++i)
        mAttributes[i]->printHexDump(hexDump);

    for (VSizeType i = 0; i < numChildNodes; ++i)
        mChildNodes[i]->printHexDump(hexDump);
    }

Vs64 VBentoNode::_calculateContentSize() const
    {
    Vs64 lengthOfCounters = 8; // 4 bytes each for #attributes and #children
    Vs64 lengthOfName = VBentoNode::_getBinaryStringLength(mName);

    Vs64 lengthOfAttributes = 0;
    for (VBentoAttributePtrVector::const_iterator i = mAttributes.begin(); i != mAttributes.end(); ++i)
        lengthOfAttributes += (*i)->calculateTotalSize();

    Vs64 lengthOfChildren = 0;
    for (VBentoNodePtrVector::const_iterator i = mChildNodes.begin(); i != mChildNodes.end(); ++i)
        lengthOfChildren += (*i)->_calculateTotalSize();

    Vs64 contentSize = lengthOfCounters + lengthOfName + lengthOfAttributes + lengthOfChildren;

    return contentSize;
    }

Vs64 VBentoNode::_calculateTotalSize() const
    {
    Vs64    contentSize = this->_calculateContentSize();
    Vs64    lengthOfLength = VBentoNode::_getLengthOfLength(contentSize);

    return lengthOfLength + contentSize;
    }

void VBentoNode::_addAttribute(VBentoAttribute* attribute)
    {
    mAttributes.push_back(attribute);
    }

const VBentoAttribute* VBentoNode::_findAttribute(const VString& name, const VString& dataType) const
    {
    // Just return from the mutable find, with appropriate cast.
    return const_cast<VBentoNode*>(this)->_findMutableAttribute(name, dataType); // const_cast: NON-CONST WRAPPER
    }

VBentoAttribute* VBentoNode::_findMutableAttribute(const VString& name, const VString& dataType)
    {
    for (VBentoAttributePtrVector::const_iterator i = mAttributes.begin(); i != mAttributes.end(); ++i)
        {
        if (name.equalsIgnoreCase((*i)->getName()) &&
            ((*i)->getDataType() == dataType))
            return (*i);
        }

    return NULL;
    }

// static
Vs64 VBentoNode::_readLengthFromStream(VBinaryIOStream& stream)
    {
    return stream.readDynamicCount();
    }

// static
void VBentoNode::_writeLengthToStream(VBinaryIOStream& stream, Vs64 length)
    {
    stream.writeDynamicCount(length);
    }

// static
Vs64 VBentoNode::_getLengthOfLength(Vs64 length)
    {
    return VBinaryIOStream::getDynamicCountLength(length);
    }

// static
void VBentoNode::_readFourCharCodeFromStream(VBinaryIOStream& stream, VString& code)
    {
    code.preflight(4);
    (void) stream.read(code.getDataBuffer(), CONST_S64(4));
    code.postflight(4);
    }

// static
void VBentoNode::_writeFourCharCodeToStream(VBinaryIOStream& stream, const VString& code)
    {
    int    codeLength = code.length();

    (void) stream.write(code.getDataBufferConst(), V_MIN(4, codeLength));

    // In case code is less than 4 chars, pad with spaces. Please don't use such codes,
    // it's not efficient!
    for (int i = codeLength; i < 4; ++i)
        stream.writeS8(' ');
    }

// static
Vs64 VBentoNode::_getBinaryStringLength(const VString& s)
    {
    int textLength = s.length();
    Vs64 lengthOfLength = VBentoNode::_getLengthOfLength(textLength);

    return lengthOfLength + textLength;
    }

// VBentoUnknownValue --------------------------------------------------------

VBentoUnknownValue::VBentoUnknownValue(VBinaryIOStream& stream, Vs64 dataLength, const VString& dataType) :
VBentoAttribute(stream, dataType),
mValue(dataLength)
    {
    VBinaryIOStream    memoryIOStream(mValue);

    (void) VStream::streamCopy(stream, memoryIOStream, dataLength);
    }

void VBentoUnknownValue::writeDataToBinaryStream(VBinaryIOStream& stream) const
    {
    // To ensure that there are no side-effects and we are indeed const in behavior,
    // we save and restore mValue stream's offset, while doing a const-cast so
    // that we are allowed to use manipulate the stream.
    Vs64 savedOffset = mValue.getIOOffset();
    VBinaryIOStream    memoryIOStream(const_cast<VBentoUnknownValue*>(this)->mValue); // const_cast: WORKAROUND. Save/restore state.

    memoryIOStream.seek0();
    (void) VStream::streamCopy(memoryIOStream, stream, mValue.getEOFOffset());
    memoryIOStream.seek(savedOffset, SEEK_SET);
    }

// VBentoCallbackParser ------------------------------------------------------

VBentoCallbackParser::VBentoCallbackParser(VBinaryIOStream& stream)
    {
    VBentoCallbackParser::processNode(0, stream);
    }

void VBentoCallbackParser::processNode(int depth, VBinaryIOStream& stream)
    {
    Vs64    theDataLength = VBentoNode::_readLengthFromStream(stream);
    Vs32    numAttributes = stream.readS32();
    Vs32    numChildNodes = stream.readS32();
    VString    theName;

    stream.readString(theName);

    this->nodeHeaderComplete(depth, theDataLength, numAttributes, numChildNodes, theName);

    for (int i = 0; i < numAttributes; ++i)
        {
        this->processAttribute(depth, stream);
        }

    this->nodeAttributesComplete(depth, theDataLength, numAttributes, numChildNodes, theName);

    for (int i = 0; i < numChildNodes; ++i)
        {
        this->processNode(depth+1, stream);
        }

    this->nodeComplete(depth, theDataLength, numAttributes, numChildNodes, theName);
    }

void VBentoCallbackParser::processAttribute(int depth, VBinaryIOStream& stream)
    {
    Vs64    theDataLength = VBentoNode::_readLengthFromStream(stream);
    VString    type;
    VString    theName;

    VBentoNode::_readFourCharCodeFromStream(stream, type);
    stream.readString(theName);

    this->attributeHeaderComplete(depth, theDataLength, type, theName);

    this->readAttributeData(depth, stream, static_cast<Vu64> (theDataLength));

    this->attributeComplete(depth, theDataLength, type, theName);
    }

void VBentoCallbackParser::nodeHeaderComplete(int /*depth*/, Vs64 /*length*/, Vs32 /*numAttributes*/, Vs32 /*numChildren*/, const VString& /*name*/)
    {
    }

void VBentoCallbackParser::nodeAttributesComplete(int /*depth*/, Vs64 /*length*/, Vs32 /*numAttributes*/, Vs32 /*numChildren*/, const VString& /*name*/)
    {
    }

void VBentoCallbackParser::nodeComplete(int /*depth*/, Vs64 /*length*/, Vs32 /*numAttributes*/, Vs32 /*numChildren*/, const VString& /*name*/)
    {
    }

void VBentoCallbackParser::attributeHeaderComplete(int /*depth*/, Vs64 /*length*/, const VString& /*type*/, const VString& /*name*/)
    {
    }

void VBentoCallbackParser::attributeComplete(int /*depth*/, Vs64 /*length*/, const VString& /*type*/, const VString& /*name*/)
    {
    }

void VBentoCallbackParser::readAttributeData(int /*depth*/, VBinaryIOStream& stream, Vu64 dataLength)
    {
    stream.skip(static_cast<Vu64> (dataLength));
    }

// VBentoArray ----------------------------------------------------------------------

void VBentoArray::_getValueAsBentoTextString(VString& s) const
    {
    int numElements = this->_getNumElements();
    if (numElements > 0)
        {
        this->_appendElementBentoText(0, s);
        for (int i = 1; i < numElements; ++i)
            {
            s += ',';
            this->_appendElementBentoText(i, s);
            }
        }
    }

